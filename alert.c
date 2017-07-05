/*
 ClamAV Live Scan
 mad.coder@mail.com
 https://github.com/madcoder42/clamAVLiveScan

 Copyright 2017 madcoder42

 This file is part of ClamAV Live Scan.

 ClamAV Live Scan is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 ClamAV Live Scan is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with ClamAV Live Scan.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"

BOOL APIENTRY dlgAlertProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
		// INIT
		case WM_INITDIALOG:
            WIN.hAlert = hDlg;
            // Set Icon
            SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)IMGS.iconMain);
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)IMGS.iconMainSmall);
            // Set Limits
            SendDlgItemMessage(hDlg, IDEDITALERTMSG, EM_LIMITTEXT, 0, 0);
			return TRUE;
        // CLOSE
		case WM_CLOSE:
            if (MessageBox(hDlg, "Do You Really Want Close Alert Window ?\n(Results Were Lost)", "Close ?", MB_YESNO|MB_ICONQUESTION) == 6) {
                EndDialog(hDlg, 0);
                WIN.hAlert = NULL;
            }
			return TRUE;
        // COMMAND
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDBTNALERTCLOSE:
					// Close
					if (MessageBox(hDlg, "Do You Really Want Close Alert Window ?\n(Results Were Lost)", "Close ?", MB_YESNO|MB_ICONQUESTION) == 6) {
                        EndDialog(hDlg, 0);
                        WIN.hAlert = NULL;
                    }
					break;
                case IDBTNALERTCLEAR:
                    // Clear
                    if (MessageBox(hDlg, "Do You Really Want Clear All Alert ?", "Clear ?", MB_YESNO|MB_ICONQUESTION) == 6) {
                        SetDlgItemText(hDlg, IDEDITALERTMSG, "");
                    }
                    break;
				case IDBTNALERTCOPY:
					// Copy To Clipboard
					copyAlertsToClipboard();
					break;
			}
			return TRUE;
		// DEFAULT
		default:
			return FALSE;
	}
}

/******************************************************************************/

DWORD WINAPI startAlertThread(LPVOID arg)
{
    DialogBox(WIN.hInst, "DIALOGALERT", NULL, (DLGPROC)dlgAlertProc);
    return 0;
}

/******************************************************************************/

short int checkForAlert(char *path, char *res)
{
    time_t t;
	struct tm *tt;
	char *tmp = NULL;
    int lenPath = strlen(path);
    int lenRes = strlen(res);
    
    if (lenRes < 7) {
        return RET_E;
    }
    if (strncasecmp(res, path, lenPath) != 0) {
        return RET_E;
    }
    tmp = res + (lenRes - 6);
    if (strcasecmp(tmp, " FOUND") != 0) {
        return RET_E;
    }
    // Quarantine File
    if (PARAMS.quarantineEnable == 1) {
        quarantineFile(path);
	}
    // Show Alert Window
    if (WIN.hAlert == NULL) {
        CreateThread(NULL, 0, startAlertThread, NULL, 0, NULL);
    }
    // Check Window
    Sleep(500);
    if (WIN.hAlert == NULL) {
        Sleep(1000);
        if (WIN.hAlert == NULL) {
            MessageBox(HWND_DESKTOP, "Threat Found But Can't Get Alert Window\nPlease Check Logs", "Error", MB_OK|MB_ICONERROR);
            return RET_O;
        }
    }
    // Focus Alert Window
    setFocusToWindow(WIN.hAlert);
    // Add Text
    tmp = NULL;
    if ((tmp = (char *)calloc((lenRes + 32), sizeof(char))) == NULL) {
		return RET_O;
	}
    t = time(NULL);
    tt = localtime(&t);
    sprintf(tmp, "[%d-%02d-%02d %02d:%02d:%02d] %s\r\n", (tt->tm_year + 1900), (tt->tm_mon + 1), tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec, res);
    addTextToEdit(WIN.hAlert, IDEDITALERTMSG, tmp);
    free(tmp);
    return RET_O;
}

/******************************************************************************/

void copyAlertsToClipboard()
{
    HGLOBAL hMem;
    int len = GetWindowTextLength(GetDlgItem(WIN.hAlert, IDEDITALERTMSG));
    char txt[len + 2];
    
    // Get Text
    memset(txt, 0, (len + 2));
    GetDlgItemText(WIN.hAlert, IDEDITALERTMSG, txt, len);
    // Copy To ClipBoard
	if ((hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1))) == NULL) {
        MessageBox(WIN.hAlert, "Can't Copy To Clipboard", "Error", MB_OK|MB_ICONERROR);
		return;
	}
	memcpy(GlobalLock(hMem), txt, (len + 1));
	GlobalUnlock(hMem);
	OpenClipboard(0);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
	return;
}
