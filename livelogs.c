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

BOOL APIENTRY dlgLivelogsProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
		// INIT
		case WM_INITDIALOG:
			WIN.hLivelogs = hDlg;
			// Set Icon
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)IMGS.iconMain);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)IMGS.iconMainSmall);
			// Set Limits
			SendDlgItemMessage(hDlg, IDEDITLIVELOGSMSG, EM_LIMITTEXT, 0, 0);
			// Load Last Of Log File
			if (PARAMS.liveLogEnable == 1 && fileExists(PARAMS.liveLogPath) == RET_O) {
				CreateThread(NULL, 0, loadLastOfLogFile, NULL, 0, NULL);
			}
			return TRUE;
		// CLOSE
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			WIN.hLivelogs = NULL;
			return TRUE;
		// COMMAND
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				// Close
				case IDBTNLIVELOGSCLOSE:
					EndDialog(hDlg, 0);
					WIN.hLivelogs = NULL;
					break;
					// Clear
				case IDBTNLIVELOGSCLEAR:
					if (MessageBox(hDlg, "Do You Really Want Clear All Live Logs ?", "Clear ?", MB_YESNO|MB_ICONQUESTION) == 6) {
						SetDlgItemText(hDlg, IDEDITLIVELOGSMSG, "");
					}
					break;
					// Copy To Clipboard
				case IDBTNLIVELOGSCOPY:
					copyLivelogsToClipboard();
					break;
			}
			return TRUE;
		// DEFAULT
		default:
			return FALSE;
	}
}

/******************************************************************************/

DWORD WINAPI startLivelogsThread(LPVOID arg)
{
	DialogBox(WIN.hInst, "DIALOGLIVELOGS", NULL, (DLGPROC)dlgLivelogsProc);
	return 0;
}

/******************************************************************************/

void showLivelogs()
{
	if (WIN.hLivelogs == NULL) {
		CreateThread(NULL, 0, startLivelogsThread, NULL, 0, NULL);
	} else {
		// Show/Focus
		ShowWindow(WIN.hLivelogs, SW_RESTORE);
		BringWindowToTop(WIN.hLivelogs);
		SetFocus(WIN.hLivelogs);
	}
	return;
}

/******************************************************************************/

DWORD WINAPI loadLastOfLogFile()
{
	FILE *fp = NULL;
	char txt[8192];
	long long s = 0;
	char *tmp = NULL;

	Sleep(500);
	// Lock
	WaitForSingleObject(PARAMS.hMutex, INFINITE);
	// Open Log File
	if ((fp = (FILE *)fopen64(PARAMS.liveLogPath, "rb")) == NULL) {
		goto stopLoadLast;
	}
	// Get File Size
	if (fseeko64(fp, 0, SEEK_END) == -1) {
		goto stopLoadLast;
	}
	if ((s = (long long)ftello64(fp)) == -1) {
		goto stopLoadLast;
	}
	// Set Good Position
	if (s > 8190) {
		if (fseeko64(fp, -8190, SEEK_END) == -1) {
			goto stopLoadLast;
		}
	} else {
		if (fseeko64(fp, 0, SEEK_SET) == -1) {
			goto stopLoadLast;
		}
	}
	// Get Text From Log
	memset(txt, 0, 8192);
	if (fread(txt, sizeof(char), 8191, fp) > 0) {
		if ((tmp = strchr(txt, '\n')) != NULL && strlen(tmp) > 1) {
			// Set Text To Edit
			SetDlgItemText(WIN.hLivelogs, IDEDITLIVELOGSMSG, "");
			addTextToEdit(WIN.hLivelogs, IDEDITLIVELOGSMSG, tmp + 1);
		}
	}
stopLoadLast:
	tmp = NULL;
	if (fp != NULL) {
		fclose(fp);
	}
	// Unlock
	ReleaseMutex(PARAMS.hMutex);
	return 0;
}

/******************************************************************************/

void copyLivelogsToClipboard()
{
	HGLOBAL hMem;
	int len = GetWindowTextLength(GetDlgItem(WIN.hLivelogs, IDEDITLIVELOGSMSG));
	char txt[len + 2];

	// Get Text
	memset(txt, 0, (len + 2));
	GetDlgItemText(WIN.hLivelogs, IDEDITLIVELOGSMSG, txt, len);
	// Copy To ClipBoard
	if ((hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1))) == NULL) {
		MessageBox(WIN.hLivelogs, "Can't Copy To Clipboard", "Error", MB_OK|MB_ICONERROR);
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
