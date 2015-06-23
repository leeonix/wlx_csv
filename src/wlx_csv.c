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

HWND __stdcall ListLoad(HWND ParentWin, char *FileToLoad, int ShowFlags)
{
    return NULL;
}

