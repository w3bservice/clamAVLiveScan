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

BOOL APIENTRY dlgFreshclamProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
		// INIT
		case WM_INITDIALOG:
			WIN.hFreshclam = hDlg;
			// Set Icon
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)IMGS.iconMain);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)IMGS.iconMainSmall);
			// Set Limits
			SendDlgItemMessage(hDlg, IDEDITFRESHCLAMEXE, EM_LIMITTEXT, (WPARAM)(MAXPATH - 1), 0);
			SendDlgItemMessage(hDlg, IDEDITFRESHCLAMCONF, EM_LIMITTEXT, (WPARAM)(MAXPATH - 1), 0);
			// Set Defaults
			setTextToEdit(hDlg, IDEDITFRESHCLAMEXE, PARAMS.freshclamPath);
			setTextToEdit(hDlg, IDEDITFRESHCLAMCONF, PARAMS.freshclamConfPath);
			return TRUE;
		// CLOSE
		case WM_CLOSE:
			EndDialog(hDlg, 0);
			WIN.hFreshclam = NULL;
			return TRUE;
		// COMMAND
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				// Select freshclam.exe
				case IDBTNSELECTFRESHCLAMEXE:
					selectAPath(hDlg, IDEDITFRESHCLAMEXE, "freshclam.exe\0freshclam.exe\0All Files (*)\0*\0");
					break;
					// Select freshclam.conf
				case IDBTNSELECTFRESHCLAMCONF:
					selectAPath(hDlg, IDEDITFRESHCLAMCONF, "freshclam.conf\0freshclam.conf\0All Files (*)\0*\0");
					break;
					// Start Freshclam Update
				case IDBTNFRESHCLAMSTART:
					startFreshclamUpdate();
					break;
					// Close
				case IDBTNFRESHCLAMCLOSE:
					EndDialog(hDlg, 0);
					WIN.hFreshclam = NULL;
					break;
			}
			return TRUE;
		// DEFAULT
		default:
			return FALSE;
	}
}

/******************************************************************************/

DWORD WINAPI startFreshclamThread(LPVOID arg)
{
	DialogBox(WIN.hInst, "DIALOGFRESHCLAM", NULL, (DLGPROC)dlgFreshclamProc);
	return 0;
}

/******************************************************************************/

void showFreshclam()
{
	if (WIN.hFreshclam == NULL) {
		CreateThread(NULL, 0, startFreshclamThread, NULL, 0, NULL);
	} else {
		// Show/Focus
		ShowWindow(WIN.hFreshclam, SW_RESTORE);
		BringWindowToTop(WIN.hFreshclam);
		SetFocus(WIN.hFreshclam);
	}
	return;
}

/******************************************************************************/

void startFreshclamUpdate()
{
	int lenArgs = (MAXPATH * 2) + 256;
	char args[lenArgs];
	char freshClamFolder[MAXPATH];

	// Check Freshclam
	if (strlen(PARAMS.freshclamPath) == 0 || fileExists(PARAMS.freshclamPath) == RET_E) {
		MessageBox(WIN.hFreshclam, "Please Select A Valid \"freshclamPath.exe\"", "Warning !", MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	if (strlen(PARAMS.freshclamConfPath) == 0 || fileExists(PARAMS.freshclamConfPath) == RET_E) {
		MessageBox(WIN.hFreshclam, "Please Select A Valid \"freshclamPath.conf\"", "Warning !", MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// Get FreshClam Folder
	if (getFolderFromFilePath(PARAMS.freshclamPath, freshClamFolder) == RET_E) {
		MessageBox(WIN.hFreshclam, "Can't Get Freshclam Folder", "Error", MB_OK|MB_ICONERROR);
		return;
	}
	// Prepare Arguments
	memset(args, 0, lenArgs);
	sprintf(args, "/K \"echo [/] Starting Update, Please Waiting... & echo. & \"%s\"", PARAMS.freshclamPath);
	strcat(args, " --config-file=\"");
	strcat(args, PARAMS.freshclamConfPath);
	strcat(args, "\" & echo. & echo [+] End Of Update !\"");
	// Start Command
	if ((int)ShellExecute(NULL, "open", "cmd", args, freshClamFolder, SW_SHOW) > 32) {
		MessageBox(WIN.hFreshclam, "Update Started !\n(Show Command-Line Window To Watch Results)\n(You Can Close This Window)", "OK !", MB_OK|MB_ICONINFORMATION);
	} else {
		MessageBox(WIN.hFreshclam, "Can't Start Update !", "Error !", MB_OK|MB_ICONERROR);
	}
	return;
}
