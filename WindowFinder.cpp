



#include "WindowFinder.h"
#include "main.h"
#include "resource.h"


#define BULLSEYE_CENTER_X_OFFSET		15
#define BULLSEYE_CENTER_Y_OFFSET		18





// Synopsis :
// 1. This routine launches the "Search Window" dialog box.
//
// 2. The dialog box is a MODAL dialog box that will not return until the user 
// clicks on the "OK" or "Cancel" button.
long StartSearchWindowDialog (HWND hwndMain)
{
  long lRet = 0;

  lRet = (long)DialogBox
  (
    (HINSTANCE)g_hInst, // handle to application instance 
    (LPCTSTR)MAKEINTRESOURCE(IDD_DIALOG_SEARCH_WINDOW), // identifies dialog box template 
    (HWND)hwndMain, // handle to owner window 
    (DLGPROC)SearchWindowDialogProc // pointer to dialog box procedure 
  );

  return lRet;
}





// Synopsis :
// 1. This function checks a hwnd to see if it is actually the "Search Window" Dialog's or Main Window's
// own window or one of their children. If so a FALSE will be returned so that these windows will not
// be selected. 
//
// 2. Also, this routine checks to see if the hwnd to be checked is already a currently found window.
// If so, a FALSE will also be returned to avoid repetitions.
BOOL CheckWindowValidity (HWND hwndDialog, HWND hwndToCheck)
{
  HWND hwndTemp = NULL;
  BOOL bRet = TRUE;

  // The window must not be NULL.
  if (hwndToCheck == NULL)
  {
    bRet = FALSE;
	goto CheckWindowValidity_0;
  }

  // It must also be a valid window as far as the OS is concerned.
  if (IsWindow(hwndToCheck) == FALSE)
  {
    bRet = FALSE;
	goto CheckWindowValidity_0;
  }

  // Ensure that the window is not the current one which has already been found.
  if (hwndToCheck == g_hwndFoundWindow)
  {
    bRet = FALSE;
	goto CheckWindowValidity_0;
  }

  // It must also not be the main window itself.
  if (hwndToCheck == g_hwndMainWnd)
  {
    bRet = FALSE;
	goto CheckWindowValidity_0;
  }
  
  // It also must not be the "Search Window" dialog box itself.
  if (hwndToCheck == hwndDialog)
  {
    bRet = FALSE;
	goto CheckWindowValidity_0;
  }

  // It also must not be one of the dialog box's children...
  hwndTemp = GetParent (hwndToCheck);
  if ((hwndTemp == hwndDialog) || (hwndTemp == g_hwndMainWnd))
  {
    bRet = FALSE;
	goto CheckWindowValidity_0;
  }

  

CheckWindowValidity_0:

  return bRet;
}





// Synopsis :
// 1. This is the handler for WM_MOUSEMOVE messages sent to the "Search Window" dialog proc.
//
// 2. Note that we do not handle every WM_MOUSEMOVE message sent. Instead, we check to see 
// if "g_bStartSearchWindow" is TRUE. This BOOL will be set to TRUE when the Window
// Searching Operation is actually started. See the WM_COMMAND message handler in 
// SearchWindowDialogProc() for more details.
//
// 3. Because the "Search Window" dialog immediately captures the mouse when the Search Operation 
// is started, all mouse movement is monitored by the "Search Window" dialog box. This is 
// regardless of whether the mouse is within or without the "Search Window" dialog. 
//
// 4. One important note is that the horizontal and vertical positions of the mouse cannot be 
// calculated from "lParam". These values can be inaccurate when the mouse is outside the
// dialog box. Instead, use the GetCursorPos() API to capture the position of the mouse.
long DoMouseMove 
(
  HWND hwndDialog, 
  UINT message, 
  WPARAM wParam, 
  LPARAM lParam
)
{
  POINT		screenpoint;
  HWND		hwndFoundWindow = NULL;
  TCHAR		szText[256];
  long		lRet = 0;

  // Must use GetCursorPos() instead of calculating from "lParam".
  GetCursorPos (&screenpoint);  

  // Display global positioning in the dialog box.
  wsprintf (szText, _T("%d"), screenpoint.x);
  SetDlgItemText (hwndDialog, IDC_STATIC_X_POS, szText);
  
  wsprintf (szText, _T("%d"), screenpoint.y);
  SetDlgItemText (hwndDialog, IDC_STATIC_Y_POS, szText);

  // Determine the window that lies underneath the mouse cursor.
  hwndFoundWindow = WindowFromPoint (screenpoint);

  // Check first for validity.
  if (CheckWindowValidity (hwndDialog, hwndFoundWindow))
  {
    // We have just found a new window.

    // Display some information on this found window.
	DisplayInfoOnFoundWindow (hwndDialog, hwndFoundWindow, TVI_ROOT);

    // If there was a previously found window, we must instruct it to refresh itself. 
	// This is done to remove any highlighting effects drawn by us.
    if (g_hwndFoundWindow)
    {
      RefreshWindow (g_hwndFoundWindow);
    }

    // Indicate that this found window is now the current global found window.
    g_hwndFoundWindow = hwndFoundWindow;

    // We now highlight the found window.
    HighlightFoundWindow (hwndDialog, g_hwndFoundWindow);
  }

  return lRet;
}





// Synopsis :
// 1. Handler for WM_LBUTTONUP message sent to the "Search Window" dialog box.
// 
// 2. We restore the screen cursor to the previous one.
//
// 3. We stop the window search operation and release the mouse capture.
long DoMouseUp
(
  HWND hwndDialog, 
  UINT message, 
  WPARAM wParam, 
  LPARAM lParam
)
{
  long lRet = 0;

  // If we had a previous cursor, set the screen cursor to the previous one.
  // The cursor is to stay exactly where it is currently located when the 
  // left mouse button is lifted.
  if (g_hCursorPrevious)
  {
    SetCursor (g_hCursorPrevious);
  }

  // If there was a found window, refresh it so that its highlighting is erased. 
  if (g_hwndFoundWindow)
  {
    RefreshWindow (g_hwndFoundWindow);
  }

  // Set the bitmap on the Finder Tool icon to be the bitmap with the bullseye bitmap.
  SetFinderToolImage (hwndDialog, TRUE);

  // Very important : must release the mouse capture.
  ReleaseCapture ();

  // Make the main window appear normally.
  if(g_bHideSearchWindow) {
	ShowWindow (hwndDialog, SW_SHOWNORMAL);
	g_bHideSearchWindow = FALSE;
  }
  //ShowWindow (g_hwndMainWnd, SW_SHOWNORMAL);

  // Set the global search window flag to FALSE.
  g_bStartSearchWindow = FALSE;

  return lRet;
}





// Synopsis :
// 1. This routine sets the Finder Tool icon to contain an appropriate bitmap.
//
// 2. If bSet is TRUE, we display the BullsEye bitmap. Otherwise the empty window
// bitmap is displayed.
BOOL SetFinderToolImage (HWND hwndDialog, BOOL bSet)
{
  HBITMAP hBmpToSet = NULL;
  BOOL bRet = TRUE;

  if (bSet)
  {
    // Set a FILLED image.
	hBmpToSet = g_hBitmapFinderToolFilled;
  }
  else
  {
    // Set an EMPTY image.
	hBmpToSet = g_hBitmapFinderToolEmpty;
  }

  SendDlgItemMessage
  (
    (HWND)hwndDialog, // handle of dialog box 
    (int)IDC_STATIC_ICON_FINDER_TOOL, // identifier of control 
    (UINT)STM_SETIMAGE, // message to send 
    (WPARAM)IMAGE_BITMAP, // first message parameter 
    (LPARAM)hBmpToSet // second message parameter 
  );

  return bRet;
}





// Synopsis :
// 1. This routine moves the mouse cursor hotspot to the exact 
// centre position of the bullseye in the finder tool static control.
//
// 2. This function, when used together with DoSetFinderToolImage(),
// gives the illusion that the bullseye image has indeed been transformed
// into a cursor and can be moved away from the Finder Tool Static
// control.
BOOL MoveCursorPositionToBullsEye (HWND hwndDialog)
{
  BOOL bRet = FALSE;
  HWND hwndToolFinder = NULL;
  RECT rect;
  POINT screenpoint;

  // Get the window handle of the Finder Tool static control.
  hwndToolFinder = GetDlgItem (hwndDialog, IDC_STATIC_ICON_FINDER_TOOL);

  if (hwndToolFinder)
  {
    // Get the screen coordinates of the static control,
	// add the appropriate pixel offsets to the center of 
	// the bullseye and move the mouse cursor to this exact
	// position.
    GetWindowRect (hwndToolFinder, &rect);
	screenpoint.x = rect.left + BULLSEYE_CENTER_X_OFFSET;
    screenpoint.y = rect.top + BULLSEYE_CENTER_Y_OFFSET;
	SetCursorPos (screenpoint.x, screenpoint.y);
  }

  return bRet;
}


// Synopsis :
// 1. This function starts the window searching operation.
//
// 2. A very important part of this function is to capture 
// all mouse activities from now onwards and direct all mouse 
// messages to the "Search Window" dialog box procedure.
long SearchWindow (HWND hwndDialog)
{
  long lRet = 0;

  if( (IsDlgButtonChecked(hwndDialog, IDC_CHK_HIDE_TOOL_WINDOW)) && (! g_bHideSearchWindow) ) {
	g_bHideSearchWindow = ShowWindow (hwndDialog, SW_HIDE);
  }

  // Set the global "g_bStartSearchWindow" flag to TRUE.
  g_bStartSearchWindow = TRUE;

  // Display the empty window bitmap image in the Finder Tool static control.
  SetFinderToolImage (hwndDialog, FALSE);

  MoveCursorPositionToBullsEye (hwndDialog);

  // Set the screen cursor to the BullsEye cursor.
  if (g_hCursorSearchWindow)
  {
    g_hCursorPrevious = SetCursor (g_hCursorSearchWindow);
  }
  else
  {
    g_hCursorPrevious = NULL;
  }

  // Very important : capture all mouse activities from now onwards and
  // direct all mouse messages to the "Search Window" dialog box procedure.
  SetCapture (hwndDialog);

  // Hide the main window.
  //ShowWindow (g_hwndMainWnd, SW_HIDE);
  
  return lRet;
}


long DisplayInfoOnFoundWindow (HWND hwndDialog, HWND hwndFoundWindow, HTREEITEM hParentNode)
{
	long lRet = TRUE;

  //Clear previous window's info
  ResetFoundWindowInfo(hwndDialog);

  DisplayChildInfoOnFoundWindow(hwndFoundWindow, hParentNode);

 
  return lRet;
}

BOOL CALLBACK EnumChildProc(HWND   hwnd, LPARAM lParam)
{
	DisplayChildInfoOnFoundWindow(hwnd, (HTREEITEM)lParam);

	return TRUE;
}


long DisplayChildInfoOnFoundWindow(HWND hwndFoundWindow, HTREEITEM hParentNode)
{
	TCHAR		*pszText=NULL;
	TCHAR		szClassName[256]={0};
	TCHAR		szWindowText[256]={0};
	long		lRet = 0;

	// Get the class name of the found window.
	GetClassName (hwndFoundWindow, szClassName, sizeof (szClassName) - 1);

	//Get text of the found window
	GetWindowText(hwndFoundWindow, szWindowText, sizeof(szWindowText) - 1);


	if(NULL != hParentNode) {
		TCHAR szText[256]={0};

		wsprintf(szText, _T("%s::%s(%p)"), szClassName, szWindowText,  hwndFoundWindow);

		TVINSERTSTRUCT item;
		memset(&item, 0, sizeof(TVINSERTSTRUCT));

		item.hParent = hParentNode;
		item.item.pszText = szText;
		item.item.lParam = (LPARAM)hwndFoundWindow;
		item.item.mask = TVIF_TEXT | TVIF_PARAM;

		EnumChildWindows(hwndFoundWindow, EnumChildProc, (LPARAM)TreeView_InsertItem(g_hwndTreeControl, &item) );
	}
 
	return lRet;
}


long RefreshWindow (HWND hwndWindowToBeRefreshed)
{
  long lRet = 0;

  InvalidateRect (hwndWindowToBeRefreshed, NULL, TRUE);
  UpdateWindow (hwndWindowToBeRefreshed);
  RedrawWindow (hwndWindowToBeRefreshed, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

  return lRet;
}



// Performs a highlighting of a found window.
// Comments below will demonstrate how this is done.
long HighlightFoundWindow (HWND hwndDialog, HWND hwndFoundWindow, BOOL bRestore)
{
  HDC		hWindowDC = NULL;  // The DC of the found window.
  RECT		rect;              // Rectangle area of the found window.
  long		lRet = 0;

  // Get the screen coordinates of the rectangle of the found window.
  GetWindowRect (hwndFoundWindow, &rect);

  // Get the window DC of the found window.
  hWindowDC = GetWindowDC (hwndFoundWindow);

  if (hWindowDC)
  {
	if(! bRestore) {
		// Select our created pen into the DC and backup the previous pen.
		g_hPrevPen = SelectObject (hWindowDC, g_hRectanglePen);

		// Select a transparent brush into the DC and backup the previous brush.
		g_hPrevBrush = SelectObject (hWindowDC, GetStockObject(HOLLOW_BRUSH));
	}

	// Draw a rectangle in the DC covering the entire window area of the found window.
	Rectangle (hWindowDC, 0, 0, rect.right - rect.left, rect.bottom - rect.top);

	if(! bRestore) {
		// Reinsert the previous pen and brush into the found window's DC.
		SelectObject (hWindowDC, g_hPrevPen);

		SelectObject (hWindowDC, g_hPrevBrush);
	}

	// Finally release the DC.
	ReleaseDC (hwndFoundWindow, hWindowDC);
  }

  return lRet;
}

void ResetFoundWindowInfo(HWND hwndDlg)
{
	HWND hTV = GetDlgItem(hwndDlg, IDC_WND_TREE);
	if(NULL != hTV) {
		TreeView_DeleteAllItems(hTV);
	}

	DisplaySingleWindowInfo(hwndDlg, NULL);
}


void InitControls(HWND hwndDlg)
{
	SetDlgItemText(hwndDlg, IDC_CHK_HIDE_TOOL_WINDOW, _T("Hide this window during dragging"));
	CheckDlgButton(hwndDlg, IDC_CHK_HIDE_TOOL_WINDOW, BST_CHECKED);

	SetDlgItemText(hwndDlg, IDC_STATIC_X_POS, NULL);
	SetDlgItemText(hwndDlg, IDC_STATIC_Y_POS, NULL);

	ResetFoundWindowInfo(hwndDlg);
}


BOOL CALLBACK SearchWindowDialogProc
(
  HWND hwndDlg, // handle to dialog box 
  UINT uMsg, // message 
  WPARAM wParam, // first message parameter 
  LPARAM lParam // second message parameter 
)
{
  BOOL bRet = FALSE;  // Default return value.

  switch (uMsg)
  {
    case WM_INITDIALOG :
	{
	  InitControls(hwndDlg);

	  g_hwndTreeControl = GetDlgItem(hwndDlg, IDC_WND_TREE);
	  bRet = TRUE;
	  break;
	}

	case WM_CLOSE:
	{
		PostQuitMessage(0);
		break;
	}

	case WM_MOUSEMOVE :
	{
      bRet = TRUE;

	  if (g_bStartSearchWindow)
	  {
	    // Only when we have started the Window Searching operation will we 
		// track mouse movement.
	    DoMouseMove(hwndDlg, uMsg, wParam, lParam);
	  }

	  break;
	}

	case WM_LBUTTONUP :
	{
      bRet = TRUE;

	  if (g_bStartSearchWindow)
	  {
	    // Only when we have started the window searching operation will we
		// be interested when the user lifts up the left mouse button.
	    DoMouseUp(hwndDlg, uMsg, wParam, lParam);
	  }

	  break;
	}

	case WM_COMMAND :
	{
      WORD wNotifyCode = HIWORD(wParam); // notification code 
      WORD wID = LOWORD(wParam);         // item, control, or accelerator identifier 
      HWND hwndCtl = (HWND)lParam;      // handle of control 

	  if ((wID == IDOK) || (wID == IDCANCEL))
	  {
	    bRet = TRUE;
	    EndDialog (hwndDlg, wID);
	  }

	  if (wID == IDC_STATIC_ICON_FINDER_TOOL)
	  {
	    // Because the IDC_STATIC_ICON_FINDER_TOOL static control is set with the SS_NOTIFY
		// flag, the Search Window's dialog box will be sent a WM_COMMAND message when this 
		// static control is clicked.
        bRet = TRUE;
		// We start the window search operation by calling the DoSearchWindow() function.
		SearchWindow(hwndDlg);
	    break;
	  }

	  break;
	}

	case WM_NOTIFY:
	{
		if(wParam == IDC_WND_TREE) {
			NMTREEVIEW *pNM = (NMTREEVIEW*)lParam;
			if(NULL != pNM) {
				switch(pNM->hdr.code)
				{
				case TVN_SELCHANGED:
				{
					DisplaySingleWindowInfo(hwndDlg, (HWND)pNM->itemNew.lParam);








					if(TVC_BYMOUSE  == pNM->action)
					{
						HighlightFoundWindow(hwndDlg, (HWND)pNM->itemNew.lParam);
						Sleep(100);
						RefreshWindow((HWND)pNM->itemNew.lParam);
						Sleep(100);
						HighlightFoundWindow(hwndDlg, (HWND)pNM->itemNew.lParam);
						Sleep(100);
						RefreshWindow((HWND)pNM->itemNew.lParam);
						Sleep(100);
						HighlightFoundWindow(hwndDlg, (HWND)pNM->itemNew.lParam);
						Sleep(100);
						RefreshWindow((HWND)pNM->itemNew.lParam);
					}

					break;
				}

				default:
					break;
				}
			}

		}
		break;
	}

	default :
	{
	  bRet = FALSE;
	  break;
	}
  }

  return bRet;
}

int ClassifyWindow(HWND hwnd)
{
	if(NULL == hwnd) {
		return CLS_UNKNOW;
	}

	TCHAR szClassName[256] = {0};
	int nChars = GetClassName(hwnd, szClassName, 255);
	if(0 >= nChars) {
		return CLS_UNKNOW;
	}

	if(0 == lstrcmpi(szClassName, _T("BUTTON"))) {
		return CLS_BUTTON;
	}
	else if(0 == lstrcmpi(szClassName, _T("STATIC"))) {
		return CLS_STATIC;
	}
	else if(0 == lstrcmpi(szClassName, _T("EDIT"))) {
		return CLS_EDIT;
	}
	else if(0 == lstrcmpi(szClassName, _T("LISTBOX"))) {
		return CLS_LISTBOX;
	}
	else if(0 == lstrcmpi(szClassName, _T("COMBOBOX"))) {
		return CLS_COMBOBOX;
	}
	else if(0 == lstrcmpi(szClassName, _T("SysTreeView32"))) {
		return CLS_TREECTRL;
	}
	else if(0 == lstrcmpi(szClassName, _T("SysListView32"))) {
		return CLS_LISTCTRL;
	}
	else if(0 == lstrcmpi(szClassName, _T("SysHeader32"))) {
		return CLS_HEADERCTRL;
	} 
	else {
		return CLS_WND;
	}
}

void DisplayTreeItemInfo(HWND hSourceTreeCtrl, HTREEITEM hSourceItem, HWND hwndDestTreeCtrl, HTREEITEM hDestParent)
{
	if(NULL != hDestParent) {
		TVINSERTSTRUCT item;
		memset(&item, 0, sizeof(TVINSERTSTRUCT));

		//Get source node info
		TCHAR szBuffer[256]={0};
		item.item.mask = TVIF_TEXT | TVIF_HANDLE;
		item.item.hItem = hSourceItem;
		item.item.cchTextMax = 255;
		

		DWORD dwProcessId = 0;
		DWORD dwThreadId = GetWindowThreadProcessId(hwndDestTreeCtrl, &dwProcessId);

		LRESULT res = 0L;
		BOOL bSuccess = FALSE;
		HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
		if(NULL != hProcess) {
			item.item.pszText = (LPTSTR)VirtualAllocEx(hProcess, NULL, sizeof(szBuffer), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if(NULL != item.item.pszText) {
				LPVOID lpItemBuffer = VirtualAllocEx(hProcess, NULL, sizeof(TVITEM), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
				if(NULL != lpItemBuffer) {
					SIZE_T nWriten = 0;
					bSuccess = WriteProcessMemory(hProcess, lpItemBuffer, &item.item, sizeof(TVITEM), &nWriten);
					if(bSuccess) {
						res = SendMessage(hSourceTreeCtrl, TVM_GETITEM, NULL, (LPARAM)lpItemBuffer);
					}
					bSuccess = VirtualFreeEx(hProcess, lpItemBuffer, sizeof(TVITEM), MEM_DECOMMIT);
				}
			}
		}

		if(res) {
			SIZE_T nRead = 0;
			bSuccess = ReadProcessMemory(hProcess, item.item.pszText, szBuffer, sizeof(szBuffer)-sizeof(TCHAR), &nRead);
			if(bSuccess) {
				//Insert to destination tree
				item.item.pszText = szBuffer;
				item.hParent = hDestParent;
				TreeView_InsertItem(hwndDestTreeCtrl, &item);
			}

			if(NULL != item.item.pszText) {
				bSuccess = VirtualFreeEx(hProcess, item.item.pszText, sizeof(szBuffer), MEM_DECOMMIT);
			}
		}
	}
}


void DisplaySingleWindowInfo(HWND hwndDlg, HWND hwnd)
{
	HWND hLV = GetDlgItem(hwndDlg, IDC_LVW_WINDOW_PROP);
	if(NULL != hLV) {
		ListView_DeleteAllItems(hLV);
		while(ListView_DeleteColumn(hLV,0)){}

		ShowWindow(hLV, SW_HIDE);
	}

	HWND hTV = GetDlgItem(hwndDlg, IDC_TVW_WND_DETAIL);
	if(NULL != hTV) {
		TreeView_DeleteAllItems(hTV);

		ShowWindow(hTV, SW_HIDE);
	}

	HWND hOthers = GetDlgItem(hwndDlg, IDC_STT_ADDITION);
	ShowWindow(hOthers, SW_HIDE);

	SetDlgItemText(hwndDlg, IDC_EDT_WND_CLASS_NAME, NULL);

	SetDlgItemText(hwndDlg, IDC_EDT_WND_TITLE, NULL);

	SetDlgItemText(hwndDlg, IDC_EDT_WND_HANDLE, NULL);

	SetDlgItemText(hwndDlg, IDC_EDT_WND_X, NULL);

	SetDlgItemText(hwndDlg, IDC_EDT_WND_Y, NULL);

	SetDlgItemText(hwndDlg, IDC_EDT_WND_WIDTH, NULL);

	SetDlgItemText(hwndDlg, IDC_EDT_WND_HEIGHT, NULL);

	if(NULL == hwnd) {
		return;
	}

	//Display window's class name
	TCHAR szClassName[256]={0};
	GetClassName(hwnd, szClassName, 255);
	SetDlgItemText(hwndDlg, IDC_EDT_WND_CLASS_NAME, szClassName);

	//Display window's title
	TCHAR szTitle[256]={0};
	GetWindowText(hwnd, szTitle, 255);
	SetDlgItemText(hwndDlg, IDC_EDT_WND_TITLE, szTitle);

	TCHAR szHandle[256]={0};
	wsprintf(szHandle, _T("0x%p"), hwnd);
	SetDlgItemText(hwndDlg, IDC_EDT_WND_HANDLE, szHandle);

	//Display window's location and size
	RECT rect={0};
	GetWindowRect(hwnd, &rect);
	SetDlgItemInt(hwndDlg, IDC_EDT_WND_X, rect.left, TRUE);
	SetDlgItemInt(hwndDlg, IDC_EDT_WND_Y, rect.top, TRUE);
	SetDlgItemInt(hwndDlg, IDC_EDT_WND_WIDTH, rect.right-rect.left, TRUE);
	SetDlgItemInt(hwndDlg, IDC_EDT_WND_HEIGHT, rect.bottom-rect.top, TRUE);

	switch(ClassifyWindow(hwnd))
	{
	case CLS_TREECTRL:
		{
			hOthers = GetDlgItem(hwndDlg, IDC_STT_ADDITION);
			if(NULL != hOthers) {
				ShowWindow(hOthers, SW_SHOW);
			}

			hTV = GetDlgItem(hwndDlg, IDC_TVW_WND_DETAIL);
			if(NULL != hTV) {
				ShowWindow(hTV, SW_SHOW);
			}

			DisplayTreeItemInfo(hwnd, TreeView_GetRoot(hwnd), hTV, TVI_ROOT);

		}
		break;
	case CLS_LISTCTRL:
		{
			hOthers = GetDlgItem(hwndDlg, IDC_STT_ADDITION);
			if(NULL != hOthers) {
				ShowWindow(hOthers, SW_SHOW);
			}

			hLV = GetDlgItem(hwndDlg, IDC_LVW_WINDOW_PROP);
			if(NULL != hLV) {
				ShowWindow(hLV, SW_SHOW);
			}
		}
		break;
	default:
		break;
	}
}