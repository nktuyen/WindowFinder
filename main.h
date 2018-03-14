
#ifndef MAIN_H
  #define MAIN_H

#include <windows.h>
#include <tchar.h>
#include <Commctrl.h>

extern HINSTANCE	g_hInst;
extern HWND			g_hwndMainWnd;
extern HANDLE		g_hApplicationMutex;
extern DWORD		g_dwLastError;
extern BOOL			g_bStartSearchWindow;
extern BOOL			g_bHideSearchWindow;
extern HCURSOR		g_hCursorSearchWindow;
extern HCURSOR		g_hCursorPrevious;
extern HBITMAP		g_hBitmapFinderToolFilled;
extern HBITMAP		g_hBitmapFinderToolEmpty;
extern HWND			g_hwndFoundWindow;
extern HPEN			g_hRectanglePen;
extern HWND			g_hwndTreeControl;
extern HGDIOBJ		g_hPrevPen;
extern HGDIOBJ		g_hPrevBrush;

#define WINDOW_FINDER_MAIN_WINDOW_TITLE	_T("Window Finder")
#define WINDOW_FINDER_APP_MUTEX_NAME	_T("WINDOWFINDERMUTEX")
#define WM_START_SEARCH_WINDOW			WM_USER + 100
#define WINDOW_FINDER_TOOL_TIP			_T("Window Finder")
#define ABOUT_WINDOW_FINDER				_T("Window Finder\r\n(C) 2001 Lim Bio Liong.")

#define	CLS_UNKNOW		0
#define	CLS_WND			1
#define	CLS_BUTTON		2
#define	CLS_STATIC		3
#define	CLS_EDIT		4
#define	CLS_LISTBOX		5
#define	CLS_COMBOBOX	6
#define	CLS_TREECTRL	7
#define	CLS_LISTCTRL	8
#define	CLS_HEADERCTRL	9


BOOL InitializeApplication
(
  HINSTANCE hThisInst, 
  HINSTANCE hPrevInst, 
  LPTSTR lpszArgs, 
  int nWinMode
);

BOOL UninitializeApplication ();

BOOL InitialiseResources();

BOOL UninitialiseResources();

int APIENTRY WinMain
(
  HINSTANCE hInstance,
  HINSTANCE hPrevInstance,
  LPSTR     lpCmdLine,
  int       nCmdShow
);

LRESULT CALLBACK MainWndProc
(
  HWND hwnd, 
  UINT message, 
  WPARAM wParam, 
  LPARAM lParam
);

#endif

