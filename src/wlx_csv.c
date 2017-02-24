// =====================================================================================
//
//       Filename:  wlx_csv.c
//         Author:  LeeoNix
//        Created:  2015-6-23 16:51:23
//
//    Description:
//
// =====================================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <windows.h>
#include <commctrl.h>

#include <csv.h>
#include "listplug.h"

#define buffer_size 4096

static HINSTANCE hInst;    //hInstance
static HWND hListView;

static int column_count;
static int sort_column;
static BOOL asc_sort;

static char field_buf[buffer_size];

#ifdef _DEBUG
#define send_debug_str OutputDebugString
static void send_debug_str_fmt(const char *fmt, ...)
{
    static char buffer[BUFSIZ * 8];

    va_list args;
    va_start(args, fmt);
    _vsnprintf(buffer, (sizeof(buffer) / sizeof(char)) - 1, fmt, args);
    va_end(args);

    OutputDebugString(buffer);
}
#else
#define send_debug_str(s)
#define send_debug_str_fmt(fmt, ...)
#endif

#define CN_KEYDOWN 0xBD00
#define CN_KEYUP   0xBD01
#define CN_CHAR    0xBD02

// define for gcc
#ifndef ListView_SetExtendedListViewStyle
#define LVS_EX_GRIDLINES        0x00000001
#define LVS_EX_FULLROWSELECT    0x00000020
#define LVS_EX_DOUBLEBUFFER     0x00010000
#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 54)
#define ListView_SetExtendedListViewStyle(hwndLV, dw)\
    SendMessage((hwndLV), LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw)
#endif

#ifndef ListView_GetSelectionMark
#define LVM_GETSELECTIONMARK    (LVM_FIRST + 66)
#define ListView_GetSelectionMark(hwnd) \
    (int)SendMessage((hwnd), LVM_GETSELECTIONMARK, 0, 0)
#endif

#ifndef ListView_GetHeader
#define LVM_GETHEADER               (LVM_FIRST + 31)
#define ListView_GetHeader(hwnd)\
    (HWND)SendMessage((hwnd), LVM_GETHEADER, 0, 0L)
#endif

#ifndef HDF_SORTUP
#define HDF_SORTUP              0x0400
#endif

#ifndef HDF_SORTDOWN
#define HDF_SORTDOWN            0x0200
#endif

static void setListViewSortIcon(HWND listView, int col, int sortOrder);
static int CALLBACK listview_compare_fun(LPARAM lp1, LPARAM lp2, LPARAM sort_param);
static void listview_copy_selected();

static WNDPROC old_window_proc;

static LRESULT WINAPI control_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case WM_KEYDOWN:
    case CN_KEYDOWN:
        if (wparam == 'C' && GetKeyState(VK_CONTROL) < 0) {
            send_debug_str_fmt("hwnd:%X, msg:%X, wparam:%X, lparam:%X", hwnd, msg, wparam, lparam);
            listview_copy_selected();
        }
        break;
    }               // -----  end switch  -----
    return CallWindowProc(old_window_proc, hwnd, msg, wparam, lparam);
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        hInst = hModule;
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

int scan_multi_bytes(const char *field, int field_len)
{
    int i, n = 0, all_ascii = 1;
    unsigned char c;

    for (i = 0; i < field_len; ++i) {
        c = (unsigned char) field[i];
        if (c & 0x80) {
            all_ascii = 0;
        }

        if (n == 0) {
            if (c >= 0x80) {
                if (c >= 0xFC && c <= 0xFD) {
                    n = 5;
                } else if (c >= 0xF8) {
                    n = 4;
                } else if (c >= 0xF0) {
                    n = 3;
                } else if (c >= 0xE0) {
                    n = 2;
                } else if (c >= 0xC0) {
                    n = 1;
                } else {
                    return 0;
                }
            }
        } else {
            if ((c & 0xC0) != 0x80) {
                return 0;
            }
            n--;
        }
    }

    if (n > 0 || all_ascii) {
        return 0;
    }

    return 1;
}

static void utf8_to_ansi(char *dest, const char *src, int sz)
{
    static wchar_t _wquickbuf[buffer_size];

    int len, ulen;
    wchar_t *ustr;

    ulen = MultiByteToWideChar(CP_UTF8, 0, src, sz, NULL, 0);
    ustr = ulen < buffer_size ? _wquickbuf : (wchar_t *)malloc(ulen * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, src, sz, ustr, ulen);

    len = WideCharToMultiByte(CP_ACP, 0, ustr, ulen, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, ustr, ulen, dest, len, NULL, NULL);
    dest[len] = 0;

    if (ulen >= buffer_size) {
        free(ustr);
    }
}

////////////////////////////////////////////////////////////////////////////////

static void listview_copy_selected()
{
    int index = ListView_GetSelectionMark(hListView);
    if (index >= 0) {
        char *s = field_buf;
        int count = sizeof(field_buf);
        int left_count = count;
        int i, buf_len;
        for (i = 0; i < column_count; ++i) {
            ListView_GetItemText(hListView, index, i, s, left_count);
            count = strlen(s);
            if (count == 0) {
                break;
            }
            s[count] = ',';
            s[count + 1] = 0;
            s += count + 1;

            left_count -= count;
            if (left_count <= 0) {
                break;
            }
        }

        buf_len = strlen(field_buf);
        if (field_buf[buf_len - 1] == ',') {
            field_buf[buf_len - 1] = 0;
        }

        send_debug_str(field_buf);

        if (OpenClipboard(NULL)) {
            HGLOBAL h = GlobalAlloc(GMEM_MOVEABLE, buf_len + 1);
            char *data = (char *) GlobalLock(h);
            memcpy(data, field_buf, buf_len + 1);
            EmptyClipboard();
            SetClipboardData(CF_TEXT, data);
            GlobalUnlock(h);
            CloseClipboard();
        }
    }
}

static void listview_add_column(HWND hwnd, unsigned int col_idx, int width, char *head)
{
    LV_COLUMN lv_col;
    memset(&lv_col, 0, sizeof(LV_COLUMN));
    lv_col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lv_col.fmt = LVCFMT_LEFT;
    lv_col.pszText = head;
    lv_col.cx  = width;
    ListView_InsertColumn(hwnd, col_idx, (LPARAM)&lv_col);
    column_count += 1;
}

static void listview_insert_item(HWND hwnd, int item)
{
    LV_ITEM lv_item;
    memset(&lv_item, 0, sizeof(LV_ITEM));
    lv_item.mask = LVIF_TEXT;
    lv_item.iItem = item;
    ListView_InsertItem(hwnd, &lv_item);
}

static void listview_set_item(HWND hwnd, int item, int subitem, char *text)
{
    LV_ITEM lv_item;
    memset(&lv_item, 0, sizeof(LV_ITEM));
    lv_item.mask = LVIF_TEXT;
    lv_item.iItem = item;
    lv_item.iSubItem = subitem;
    lv_item.pszText = text;
    lv_item.cchTextMax = strlen(text);
    ListView_SetItem(hwnd, &lv_item);
}

static int cal_width(int field_len)
{
    int result = field_len * 8;
    if (result > 300) {
        return 300;
    } else if (result < 40) {
        return 40;
    }
    return result;
}

static void adjust_column_width(int field_len, int field_no)
{
    int col_width = (int)ListView_GetColumnWidth(hListView, field_no);
    int field_width = cal_width(field_len);
    if (field_width > col_width) {
        ListView_SetColumnWidth(hListView, field_no, field_width);
    }
}

static char *get_field_buf(const char *field, int field_len)
{
    if (scan_multi_bytes(field, field_len)) {
        utf8_to_ansi(field_buf, field, field_len);
    } else {
        memcpy(field_buf, field, field_len);
        field_buf[field_len] = '\0';
    }
    return field_buf;
}

static void read_csv_file(int token, int record_no, const char *field, int field_len)
{
    static int field_no = 0;
    if (token == TK_EOF || token == TK_ERR) {
        field_no = 0;
        field_buf[0] = '\0';
        return;
    }

    // No. one record add column
    if (record_no == 1) {
        switch (token) {
        case TK_String:
        case TK_Quote: {
            char buf[16];
            sprintf(buf, "%d", field_no);
            listview_add_column(hListView, field_no, cal_width(field_len), buf);
        }
        break;
        }               // -----  end switch  -----
    }
    if (field_no == 0) {
        listview_insert_item(hListView, record_no - 1);
    }
    switch (token) {
    case TK_EOL:
        field_no = 0;
        break;
    case TK_String:
    case TK_Quote:
        adjust_column_width(field_len, field_no);
        listview_set_item(hListView, record_no - 1, field_no, get_field_buf(field, field_len));
        field_no += 1;
        break;
    }               // -----  end switch  -----
}

HWND __stdcall ListLoad(HWND ParentWin, char *FileToLoad, int ShowFlags)
{
    HWND hwnd;
    RECT r;
    GetClientRect(ParentWin, &r);
    hwnd = CreateWindow("SysListView32", "",
                        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
                        r.left, r.top, r.right - r.left, r.bottom - r.top,
                        ParentWin, (HMENU) 9000, hInst, NULL);
    if (hwnd != NULL) {
        old_window_proc = (WNDPROC)(GetWindowLong(hwnd, GWL_WNDPROC));
        send_debug_str_fmt("old_window_proc is 0x%08X", old_window_proc);
        SetWindowLong(hwnd, GWL_WNDPROC, (LONG)(&control_proc));
        SetFocus(ParentWin);

        column_count = 0;
        sort_column = 0;
        asc_sort = TRUE;

        hListView = hwnd;
        ListView_SetExtendedListViewStyle(hwnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

        csv_read_file(&read_csv_file, FileToLoad);
        ListView_SortItemsEx(hwnd, listview_compare_fun, 1);
        setListViewSortIcon(hwnd, 0, 2);

        ShowWindow(hwnd, SW_SHOW);
    }
    return hwnd;
}

void __stdcall ListGetDetectString(char *DetectString, int maxlen)
{
    strncpy(DetectString, "EXT=\"CSV\"", maxlen);
    DetectString[maxlen] = '\0';
}

////////////////////////////////////////////////////////////////////////////////

// state can be
// sortOrder - 0 neither, 1 descending, 2 ascending
static void setListViewSortIcon(HWND listView, int col, int sortOrder)
{
    HWND headerWnd;
    char headerText[256];
    HD_ITEM item;
    int numColumns, curCol;

    headerWnd = ListView_GetHeader(listView);
    numColumns = Header_GetItemCount(headerWnd);

    for (curCol = 0; curCol < numColumns; curCol++) {
        item.mask = HDI_FORMAT | HDI_TEXT;
        item.pszText = headerText;
        item.cchTextMax = sizeof(headerText) - 1;
        SendMessage(headerWnd, HDM_GETITEM, curCol, (LPARAM)&item);

        if ((sortOrder != 0) && (curCol == col))
            switch (sortOrder) {
            case 1:
                item.fmt &= !HDF_SORTUP;
                item.fmt |= HDF_SORTDOWN;
                break;
            case 2:
                item.fmt &= !HDF_SORTDOWN;
                item.fmt |= HDF_SORTUP;
                break;
            }
        else {
            item.fmt &= !HDF_SORTUP & !HDF_SORTDOWN;
        }
        item.fmt |= HDF_STRING;
        item.mask = HDI_FORMAT | HDI_TEXT;
        SendMessage(headerWnd, HDM_SETITEM, curCol, (LPARAM)&item);
    }
}

static int CALLBACK listview_compare_fun(LPARAM lp1, LPARAM lp2, LPARAM sort_param)
{
    char buf1[64], buf2[64];
    int column = abs(sort_param) - 1;
    ListView_GetItemText(hListView, lp1, column, buf1, sizeof(buf1));
    ListView_GetItemText(hListView, lp2, column, buf2, sizeof(buf2));
    return sort_param > 0 ? strcmp(buf1, buf2) : strcmp(buf2, buf1);
}

static void on_column_click(LPNMLISTVIEW listview_info)
{
    if (listview_info->iSubItem == sort_column) {
        asc_sort = !asc_sort;
    } else {
        sort_column = listview_info->iSubItem;
        asc_sort = TRUE;
    }

    ListView_SortItemsEx(hListView, listview_compare_fun, asc_sort ? sort_column + 1 : -(sort_column + 1));
    setListViewSortIcon(hListView, sort_column, asc_sort + 1);
}

int __stdcall ListNotificationReceived(HWND ListWin, int Message, WPARAM wParam, LPARAM lParam)
{
    if (Message == WM_NOTIFY) {
        LPNMHDR notify_info = (LPNMHDR) lParam;
        if (notify_info->idFrom == 9000 && notify_info->code == LVN_COLUMNCLICK) {
            on_column_click((LPNMLISTVIEW) lParam);
        }
    }
    return DefWindowProc(ListWin, Message, wParam, lParam);
}

