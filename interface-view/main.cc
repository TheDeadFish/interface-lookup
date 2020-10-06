#include <stdshit.h>
#include <win32hlp.h>
#include "resource.h"
#include "resize.h"
#include "interface_data.h"
#include "selfont.h"
#include "util.h"

const char progName[] = "interface-view";

static InterfaceData s_iData;
static HWND s_hList;
static WndResize s_resize;
static int nTabPage;
static InterfaceData::Interface* s_curType;

void loadFile(HWND hwnd, cch* file)
{
	if(file && s_iData.load(file))
		contError(hwnd, "failed to load def list: %s\n", file);
}

void item_select(HWND hwnd);
void selectTab(HWND hwnd)
{
	int nPage = getDlgTabPage(hwnd, IDC_TAB);
	ShowDlgItem(hwnd, IDC_LIST1, nPage == 0);
	ShowDlgItem(hwnd, IDC_EDIT, nPage != 0);
	nTabPage = nPage;
	item_select(hwnd);
}

void nameEdtChange(HWND hwnd);
void mainDlgInit(HWND hwnd, cch* file)
{
	init_font();
	sendDlgMsg(hwnd, IDC_EDIT, WM_SETFONT, (WPARAM)g_hfont);

	s_resize.init(hwnd);
	s_resize.add(hwnd, IDC_NAME, HOR_BOTH);
	s_resize.add(hwnd, IDC_VERSION, HOR_BOTH);
	s_resize.add(hwnd, IDC_LIST1, HVR_BOTH);
	s_resize.add(hwnd, IDC_EDIT, HVR_BOTH);
	s_resize.add(hwnd, IDC_SIMPLE, HOR_RIGH);
	s_resize.add(hwnd, IDC_X64, HOR_RIGH);
	s_resize.add(hwnd, IDC_FONT, HOR_RIGH);

	addDlgTabPage(hwnd, IDC_TAB, 0, "List");
	addDlgTabPage(hwnd, IDC_TAB, 1, "Class");
	addDlgTabPage(hwnd, IDC_TAB, 2, "Vtab");

	loadFile(hwnd, file);

	s_hList = GetDlgItem(hwnd, IDC_LIST1);
	ListView_SetExtendedListViewStyle(s_hList,
		LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);

	lstView_insColumn(s_hList, 0, 170, "Name");
	lstView_insColumn(s_hList, 1, 100, "Base");
	lstView_insColumn(s_hList, 2, 78, "File");
	ListView_SetColumnWidth(s_hList, 2, LVSCW_AUTOSIZE_USEHEADER);

	nameEdtChange(hwnd);
	selectTab(hwnd);
}

void select_version(HWND hwnd)
{
	int iBase = dlgCombo_getSel(hwnd, IDC_BASE);
	int iVer = dlgCombo_getSel(hwnd, IDC_VERSION);
	InterfaceData::Interface* type = Void(dlgCombo_getData(hwnd, IDC_BASE, iBase));
	if(type && (iVer >= 0)) type->setSel = iVer;
	item_select(hwnd);
}

void select_base(HWND hwnd)
{
	int iSel = dlgCombo_getSel(hwnd, IDC_BASE);
	InterfaceData::Interface* type = Void(dlgCombo_getData(hwnd, IDC_BASE, iSel));
	dlgCombo_reset(hwnd, IDC_VERSION);
	if(iSel >= 0) { for(auto& arg : type->argSet) {
		dlgCombo_addStr(hwnd, IDC_VERSION, arg.files); }
		dlgCombo_setSel(hwnd, IDC_VERSION, type->setSel); }
	EnableDlgItem(hwnd, IDC_VERSION,
		sendDlgMsg(hwnd, IDC_VERSION, CB_GETCOUNT)>1);
}

void init_base(HWND hwnd)
{
	dlgCombo_reset(hwnd, IDC_BASE);
	INTERFACEDATA_INTERFACE_ITER(s_curType,
		int iPos = dlgCombo_addStr(hwnd, IDC_BASE, pos->type);
		dlgCombo_setData(hwnd, IDC_BASE, iPos, (LPARAM)pos););
	dlgCombo_setSel(hwnd, IDC_BASE, 0);
	EnableDlgItem(hwnd, IDC_BASE, !!s_curType);
	select_base(hwnd);
}


void item_select(HWND hwnd)
{
	int nSel = listView_getCurSel(s_hList);
	InterfaceData::Interface* type = Void(lstView_getData(s_hList, nSel));
	if(!type || (s_curType != type)) { s_curType = type; init_base(hwnd); }

	if(nTabPage) {

		Bstr str;

		if(type)
		{
			// formatting config
			InterfaceData::FmtConf fc;
			fc.pSize = IsDlgButtonChecked(hwnd, IDC_X64) ? 8 : 4;
			fc.simple = IsDlgButtonChecked(hwnd, IDC_SIMPLE);

			// vtable mode
			if(nTabPage == 2) {
				str.fmtcat("struct %s\n{\r\n  %s_vtab* vtab;\n};\n\n"
					"struct %s_vtab\n{\n", type->type, type->type, type->type);
				fc.simple = 2;
			}

			type->fmtFuncs(str, 0, fc);
			if(nTabPage == 2) {	str.fmtcat("};"); }
		}

		SetDlgItemTextW(hwnd, IDC_EDIT, crlf_widen(str.data));
	}
}

void nameEdtChange(HWND hwnd)
{
	// prefix search
	char buff[100];
	GetDlgItemTextA(hwnd, IDC_NAME, buff, 100);
	xArray lst = s_iData.find(buff);

	SetWindowRedraw(s_hList, FALSE);
	ListView_DeleteAllItems(s_hList);
	ListView_SetItemCount(s_hList, lst.len);

	for(auto* x : lst) {
		int i = lstView_iosText(s_hList, -1, x->type, (LPARAM)x);
		lstView_iosText(s_hList, i, 1, x->base);
		lstView_iosText(s_hList, i, 2, xstr(x->getFiles()));
	}

	// resize the listview
	if(strlen(buff) >= 2)
		lstView_autoSize(s_hList);

	ListView_SetItemState(s_hList, 0,
		LVIS_FOCUSED | LVIS_SELECTED, 0xF);

	SetWindowRedraw(s_hList, TRUE);
	item_select(hwnd);
}

void select_font_(HWND hwnd)
{
	select_font(hwnd);
	sendDlgMsg(hwnd, IDC_EDIT, WM_SETFONT, (WPARAM)g_hfont, TRUE);
}

BOOL CALLBACK mainDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DLGMSG_SWITCH(
		ON_MESSAGE(WM_INITDIALOG, mainDlgInit(hwnd, (cch*)lParam))
		ON_MESSAGE(WM_SIZE, s_resize.resize(hwnd, wParam, lParam))

		CASE_COMMAND(
			ON_COMMAND(IDC_X64, item_select(hwnd))
			ON_COMMAND(IDC_SIMPLE, item_select(hwnd))
			ON_COMMAND(IDCANCEL, EndDialog(hwnd, 0))
			ON_COMMAND(IDC_FONT, select_font_(hwnd))
			ON_CONTROL(CBN_SELCHANGE, IDC_BASE, select_base(hwnd))
			ON_CONTROL(CBN_SELCHANGE, IDC_VERSION, select_version(hwnd))
			ON_CONTROL(EN_CHANGE, IDC_NAME, nameEdtChange(hwnd))

	  ,)

		CASE_NOTIFY(
			ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, selectTab(hwnd))
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
