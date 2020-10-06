#include <stdshit.h>
#include <win32hlp.h>

void lstView_autoSize(HWND hList, int iCol)
{
	ListView_SetColumnWidth(hList, iCol, LVSCW_AUTOSIZE_USEHEADER);
	int headSize = ListView_GetColumnWidth(hList, iCol);
	ListView_SetColumnWidth(hList, iCol, LVSCW_AUTOSIZE);
	int bodySize = ListView_GetColumnWidth(hList, iCol);
	if(bodySize < headSize)
		ListView_SetColumnWidth(hList, iCol, headSize);
}

void lstView_autoSize(HWND hList)
{
	int nItem = ListView_GetItemCount(hList);
	for(int i = 0; i < nItem; i++)
		lstView_autoSize(hList, i);
}

void ShowDlgItem(HWND hwnd, int id, BOOL show)
{
	hwnd = GetDlgItem(hwnd, id);
	ShowWindow(hwnd, show ? SW_SHOW : SW_HIDE);
}
