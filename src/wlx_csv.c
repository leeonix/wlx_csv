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

HINSTANCE hInst;    //hInstance
HWND hMainWnd;      //our main wnd
HWND hListView;

BOOL WINAPI DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        hInst = (HINSTANCE) hModule;
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

static void listview_add_column(HWND hwnd, unsigned int col_idx, int width, char *head)
{
    LV_COLUMN lv_col;
    memset(&lv_col, 0, sizeof(LV_COLUMN));
    lv_col.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_FMT;
    lv_col.fmt = LVCFMT_LEFT;
    lv_col.pszText = head;
    lv_col.cx  = width;
    ListView_InsertColumn(hwnd, col_idx, (LPARAM)&lv_col);
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

static int field_no = 0;
static char field_buf[BUFSIZ];

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

static void adjust_column_width(field_len)
{
    int col_width = (int)ListView_GetColumnWidth(hListView, field_no);
    int field_width = cal_width(field_len);
    if (field_width > col_width) {
        ListView_SetColumnWidth(hListView, field_no, field_width);
    }
}

static char *get_field_buf(const char *field, int field_len)
{
    memcpy(field_buf, field, field_len);
    field_buf[field_len] = '\0';
    return field_buf;
}

static void read_csv_file(int token, int record_no, const char *field, int field_len)
{
    // No. one record is column
    if (record_no == 1) {
        switch (token) {
        case TK_EOL:
            field_no = -1;
            break;
        case TK_String:
        case TK_Quote:
            listview_add_column(hListView, field_no, cal_width(field_len), get_field_buf(field, field_len));
            field_no += 1;
            break;
        }               // -----  end switch  -----
    } else {
        // field No is -1, then insert item.
        if (field_no == -1) {
            listview_insert_item(hListView, record_no - 2);
            field_no = 0;
        }
        if (field_no >= 0) {
            switch (token) {
            case TK_EOL:
                field_no = -1;
                break;
            case TK_String:
            case TK_Quote:
                adjust_column_width(field_len);
                listview_set_item(hListView, record_no - 2, field_no, get_field_buf(field, field_len));
                field_no += 1;
                break;
            }               // -----  end switch  -----
        }
    }
    if (token == TK_EOF || token == TK_ERR) {
        field_no = 0;
        field_buf[0] = '\0';
    }
}

HWND __stdcall ListLoad(HWND ParentWin, char *FileToLoad, int ShowFlags)
{
    HWND hwnd = NULL;
    RECT r;
    GetClientRect(ParentWin, &r);
    // Create window invisbile, only show when data fully loaded!
    hwnd = CreateWindow("SysListView32", "",
                        LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS |
                        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
                        r.left, r.top, r.right - r.left, r.bottom - r.top,
                        ParentWin, NULL, hInst, NULL);
    if (hwnd != NULL) {
        hListView = hwnd;
        ListView_SetExtendedListViewStyle(hwnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);

        csv_read_file(&read_csv_file, FileToLoad);

        PostMessage(ParentWin, WM_COMMAND, MAKELONG(0, itm_percent), (LPARAM) hwnd);
        ShowWindow(hwnd, SW_SHOW);
    }
    return hwnd;
}

void __stdcall ListGetDetectString(char *DetectString, int maxlen)
{
    strncpy(DetectString, "EXT=\"CSV\"", maxlen);
    DetectString[maxlen] = '\0';
}

