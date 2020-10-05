#include <stdshit.h>
#include <win32hlp.h>
#include "resource.h"
#include "resize.h"
const char progName[] = "interface-view";

static HWND s_hList;
static WndResize s_resize;

void nameEdtChange(HWND hwnd);
void mainDlgInit(HWND hwnd, cch* file)
{
	s_resize.init(hwnd);
	s_resize.add(hwnd, IDC_NAME, HOR_BOTH);
	s_resize.add(hwnd, IDC_VERSION, HOR_BOTH);
	s_resize.add(hwnd, IDC_LIST1, HOR_BOTH);
	s_resize.add(hwnd, IDC_EDIT, HVR_BOTH);

	//loadFile(hwnd, file);

	s_hList = GetDlgItem(hwnd, IDC_LIST1);
	ListView_SetExtendedListViewStyle(s_hList,
		LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	lstView_insColumn(s_hList, 0, 170, "Name");
	lstView_insColumn(s_hList, 1, 100, "Base");
	lstView_insColumn(s_hList, 2, 78, "File");
	ListView_SetColumnWidth(s_hList, 2, LVSCW_AUTOSIZE_USEHEADER);
	nameEdtChange(hwnd);
}

void item_select(HWND hwnd)
{
	int nSel = listView_getCurSel(s_hList);
	/*if((nSel < 0)||(s_valIndex == 2)) return;
	WCHAR buff[100];
	lstView_getText(s_hList, nSel, s_valIndex, buff, 100);
	SetDlgItemTextW(hwnd, IDC_MASK, buff); */
}

void nameEdtChange(HWND hwnd)
{
	// prefix search
	/*char buff[100];
	GetDlgItemTextA(hwnd, IDC_NAME, buff, 100);
	xArray list = s_defLst.find(buff);

	// get value
	GetDlgItemTextA(hwnd, IDC_VAL, buff, 100);
	char* end; u64 val = strtoui64(buff, &end);
	s_valIndex = !buff[0];

	// handle mask mode
	SetDlgItemTextA(hwnd, IDC_MASK, "");
	if(IsDlgButtonChecked(hwnd, IDC_MASKMODE)) {
		xArray num = s_defLst.numGet(list);
		compute_mask(hwnd, num, val);
		listViewInit(hwnd, num);
		return;
	}

	// handle number mode
	if(end != NULL) {
		xArray num = s_defLst.numFind(list, val);
		listViewInit(hwnd, num);
	} else {
		listViewInit(hwnd, list);
	}*/
}

BOOL CALLBACK mainDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DLGMSG_SWITCH(
		ON_MESSAGE(WM_INITDIALOG, mainDlgInit(hwnd, (cch*)lParam))
		ON_MESSAGE(WM_SIZE, s_resize.resize(hwnd, wParam, lParam))

		CASE_COMMAND(
			ON_COMMAND(IDCANCEL, EndDialog(hwnd, 0))
			ON_CONTROL(EN_CHANGE, IDC_NAME, nameEdtChange(hwnd))
	  ,)

		CASE_NOTIFY(
			ON_LVN_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, item_select(hwnd))
	  ,)
	,)
}



int main(int argc, char** argv)
{
	cch* file = argv[1];
	if(!file) file = "interface_data.txt";
	DialogBoxParamW(NULL, MAKEINTRESOURCEW(IDD_DIALOG1),
		NULL, mainDlgProc, (LPARAM)file);
}
