// urlencoder.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "..\\..\\zinc\\base64.h"
#include "..\\..\\zinc\\zalloc.h"
#include "..\\..\\zinc\\utstrings.h"
#include "resource.h"
#include "autobuild.h"
#include <stdlib.h>

BOOL CALLBACK DialogProc_Main(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_INITDIALOG:
			{
				SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				SetFocus(GetDlgItem(hwnd, IDC_EDIT1));
				char buf[32];
				wsprintf(buf, "build# %.3d", __BUILD__);
				SetDlgItemText(hwnd, IDC_BUILD, buf);
			}
			return FALSE;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_DEC:
					{
						int len = SendDlgItemMessage(hwnd, IDC_EDIT1, WM_GETTEXTLENGTH, 0, 0L);
						unsigned char *lpenc = (unsigned char *) zalloc(len+1);
						if(lpenc)
						{
							GetDlgItemText(hwnd, IDC_EDIT1, (char*)lpenc, len+1);
							char *lpdec = alloc_de64((const char *)lpenc, NULL);
							SetDlgItemText(hwnd, IDC_EDIT4, urldecode((char *) lpenc));
							zfree(lpenc);
							if(lpdec)
							{
								SetDlgItemText(hwnd, IDC_EDIT2, (const char*) lpdec);
								lpenc = alloc_en64((const unsigned char *)lpdec, strlen(lpdec));
								if(lpdec)
								{
									SetDlgItemText(hwnd, IDC_EDIT3, (const char*) lpenc);
									free_str((char *&)lpenc);
								}
								else
								{
									SetDlgItemText(hwnd, IDC_EDIT3, "no enough memory!");
								}
								free_str(lpdec);
							}
							else
							{
								SetDlgItemText(hwnd, IDC_EDIT2, "no enough memory!");
							}
						}
					}
					break;
				case IDC_ENC:
					{
						int len = SendDlgItemMessage(hwnd, IDC_EDIT1, WM_GETTEXTLENGTH, 0, 0L);
						char * lpdec = (LPSTR) zalloc(len+1);
						if(lpdec)
						{
							GetDlgItemText(hwnd, IDC_EDIT1, lpdec, len+1);
							unsigned char *lpenc = alloc_en64((const unsigned char *)lpdec, strlen(lpdec));
							char *lphtmlenc = alloc_urlencode(lpdec);
							zfree(lpdec);
							if(lpenc)
							{
								SetDlgItemText(hwnd, IDC_EDIT2, (const char*) lpenc);
								lpdec = alloc_de64((const char *)lpenc, NULL);
								if(lpdec)
								{
									SetDlgItemText(hwnd, IDC_EDIT3, (const char*) lpdec);
									free_str(lpdec);
								}
								else
								{
									SetDlgItemText(hwnd, IDC_EDIT3, "no enough memory!");
								}
								free_str((char *&)lpenc);
							}
							else
							{
								SetDlgItemText(hwnd, IDC_EDIT2, "no enough memory!");
							}

							if(lphtmlenc)
							{
								SetDlgItemText(hwnd, IDC_EDIT4, (const char*) lphtmlenc);
								free_str(lphtmlenc);
							}
							else
							{
								SetDlgItemText(hwnd, IDC_EDIT4, "no enough memory!");
							}
						}
					}
					break;
				case IDCANCEL:
					EndDialog(hwnd, LOWORD(wParam));
					break;
			}
			break;
		case WM_LBUTTONDOWN:
			PostMessage(hwnd, WM_NCLBUTTONDOWN, HTCAPTION, lParam);
			break;
	}
	return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DLGMN), NULL, (DLGPROC)DialogProc_Main);

	return 0;
}



