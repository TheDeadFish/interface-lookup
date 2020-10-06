#include <stdshit.h>
#include "selfont.h"
const char regKey[] = "SOFTWARE\\DeadFishShitware\\interface-view-Vv1k";

static LOGFONT g_lFont;
HFONT g_hfont;

static
void initLogFont(LOGFONT* lf,
	LPCWSTR name, int PointSize)
{
	memset(lf, 0, sizeof(*lf));
	lf->lfHeight = -MulDiv(PointSize, 96, 72);
	wcscpy(lf->lfFaceName, name);
}

static
HFONT create_font(LPCWSTR name, int pt)
{
	LOGFONT lf; initLogFont(&lf, name, pt);
	return CreateFontIndirect(&lf);
}


static
BOOL choose_font(LOGFONT* lf, HWND hwnd)
{
	CHOOSEFONT cf = {};
	cf.lStructSize = sizeof (cf);
	cf.lpLogFont = lf;
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS
		| CF_INITTOLOGFONTSTRUCT;
	return ChooseFont(&cf);
}

void init_font(void)
{
	initLogFont(&g_lFont, L"Courier New", 10);

	// fetch font from registry
	HKEY hKey = NULL;
	RegCreateKeyA(HKEY_CURRENT_USER, regKey, &hKey);
	DWORD dataSize = sizeof(g_lFont);
	RegQueryValueExA(hKey, "logFont", 0, NULL,
		(LPBYTE)&g_lFont, &dataSize);
	RegCloseKey(hKey);

	g_hfont = CreateFontIndirect(&g_lFont);
}

void select_font(HWND hwnd)
{
	if(!choose_font(&g_lFont, hwnd))
		return;
	DeleteObject(g_hfont);
	g_hfont = CreateFontIndirect(&g_lFont);

	// save font to registry
	HKEY hKey = NULL;
	RegCreateKeyA(HKEY_CURRENT_USER, regKey, &hKey);
	RegSetValueExA(hKey, "logFont", 0, REG_BINARY,
		(LPBYTE)&g_lFont, sizeof(g_lFont));
	RegCloseKey(hKey);
}
