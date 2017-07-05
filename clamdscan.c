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

BOOL APIENTRY dlgClamdscanProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
		// INIT
		case WM_INITDIALOG:
            WIN.hClamdscan = hDlg;
            // Set Icon
            SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)IMGS.iconMain);
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)IMGS.iconMainSmall);
            // Set Limits
            SendDlgItemMessage(hDlg, IDEDITCLAMDSCANEXE, EM_LIMITTEXT, (WPARAM)(MAXPATH - 1), 0);
            SendDlgItemMessage(hDlg, IDEDITCLAMDSCANPATH, EM_LIMITTEXT, (WPARAM)(MAXPATH - 1), 0);
            // Set Defaults
            setTextToEdit(hDlg, IDEDITCLAMDSCANEXE, PARAMS.clamdscanPath);
            CheckDlgButton(hDlg, IDCHECKCLAMDSCANLOG, (PARAMS.clamdscanLogEnable == 1) ? BST_CHECKED : BST_UNCHECKED);
            SetDlgItemText(hDlg, IDEDITCLAMDSCANLOG, PARAMS.clamdscanLogPath);
			return TRUE;
        // CLOSE
		case WM_CLOSE:
            EndDialog(hDlg, 0);
            WIN.hClamdscan = NULL;
			return TRUE;
        // COMMAND
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
                // Select clamdscan.exe
                case IDBTNSELECTCLAMDSCANEXE:
					selectAPath(hDlg, IDEDITCLAMDSCANEXE, "clamdscan.exe\0clamdscan.exe\0All Files (*)\0*\0");
					break;
                // Select Clamdscan Log File
                case IDBTNSELECTCLAMDSCANLOG:
					selectAPath(hDlg, IDEDITCLAMDSCANLOG, "Log Files (*.log)\0*.log\0All Files (*)\0*\0");
					break;
                // Check Clamdscan Log
                case IDCHECKCLAMDSCANLOG:
                    if (IsDlgButtonChecked(hDlg, IDCHECKCLAMDSCANLOG) == BST_CHECKED) {
                        PARAMS.clamdscanLogEnable = 1;
                    } else {
                        PARAMS.clamdscanLogEnable = 0;
                    }
                    saveParams();
                    break;
                // Select Path To Scan
                case IDBTNSELECTCLAMDSCANPATH:
                    selectPathToScan();
                    break;
                // Start
                case IDBTNCLAMDSCANSTART:
                    startSingleScan();
                    break;
                // Close
				case IDBTNCLAMDSCANCLOSE:
                    EndDialog(hDlg, 0);
                    WIN.hClamdscan = NULL;
					break;
			}
			return TRUE;
		// DEFAULT
		default:
			return FALSE;
	}
}

/******************************************************************************/

DWORD WINAPI startClamdscanThread(LPVOID arg)
{
    DialogBox(WIN.hInst, "DIALOGCLAMDSCAN", NULL, (DLGPROC)dlgClamdscanProc);
    return 0;
}

/******************************************************************************/

void showClamdscan()
{
    if (WIN.hClamdscan == NULL) {
        CreateThread(NULL, 0, startClamdscanThread, NULL, 0, NULL);
    } else {
        // Show/Focus
        ShowWindow(WIN.hClamdscan, SW_RESTORE);
        BringWindowToTop(WIN.hClamdscan);
        SetFocus(WIN.hClamdscan);
    }
    return;
}

/******************************************************************************/

void selectPathToScan()
{
    char path[MAXPATH];

    memset(path, 0, MAXPATH);
    if (MessageBox(WIN.hClamdscan, "Do You Want Scan Folder ?\n(Yes=Scan Folder, No=Scan File)", "Scan Type ?", MB_YESNO|MB_ICONQUESTION) == 6) {
        // Select Folder
        if (selectAFolder(WIN.hClamdscan, path) == RET_E) {
            return;
        }
    } else {
        // Select File
        if (selectAFile(WIN.hClamdscan, path) == RET_E) {
            return;
        }
    }
    // Set Text
    setTextToEdit(WIN.hClamdscan, IDEDITCLAMDSCANPATH, path);
    return;
}

/******************************************************************************/

void startSingleScan()
{
    char path[MAXPATH];
    char listFile[MAXPATH];
    char clamdScanFolder[MAXPATH];
    int lenArgs = (MAXPATH * 4) + 256;
    char args[lenArgs];
    short int isFile = 0;
    FILE *fp = NULL;
    
    // Check Clamd
    if (strlen(PARAMS.clamdPath) == 0 || fileExists(PARAMS.clamdPath) == RET_E) {
        MessageBox(WIN.hClamdscan, "Please Select A Valid \"clamd.exe\" On Main Window", "Warning !", MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    if (strlen(PARAMS.clamdConfPath) == 0 || fileExists(PARAMS.clamdConfPath) == RET_E) {
        MessageBox(WIN.hClamdscan, "Please Select A Valid \"clamd.conf\" On Main Window", "Warning !", MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    if (CLAMD.isRunning == 0 || CLAMD.hThread == NULL) {
        MessageBox(WIN.hClamdscan, "Clamd Not Running !\nPlease \"Start Clamd\" On Main Window Before", "Warning !", MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    if (strlen(PARAMS.clamdscanPath) == 0 || fileExists(PARAMS.clamdscanPath) == RET_E) {
        MessageBox(WIN.hClamdscan, "Please Select A Valid \"clamdscan.exe\"", "Warning !", MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    if (PARAMS.clamdscanLogEnable == 1 && strlen(PARAMS.clamdscanLogPath) == 0) {
        MessageBox(WIN.hClamdscan, "Please Select A Log File", "Warning !", MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    // Get Clamdscan Folder
    if (getFolderFromFilePath(PARAMS.clamdscanPath, clamdScanFolder) == RET_E) {
        MessageBox(WIN.hClamdscan, "Can't Get Clamdscan Folder", "Error", MB_OK|MB_ICONERROR);
        return;
    }
    // Get Scan Path
    memset(path, 0, MAXPATH);
    GetDlgItemText(WIN.hClamdscan, IDEDITCLAMDSCANPATH, path, (MAXPATH - 1));
    if ((isFile = pathIsFile(path)) == RET_E) {
        MessageBox(WIN.hClamdscan, "Please Select Valid File Or Folder To Scan", "Warning !", MB_OK|MB_ICONEXCLAMATION);
        return;
    }
    // Create File-List
    memset(listFile, 0, MAXPATH);
    snprintf(listFile, MAXPATH, "%stmp%d", PARAMS.appFolder, time(NULL));
    if ((fp = fopen(listFile, "w")) == NULL) {
        MessageBox(WIN.hClamdscan, "Can't Create Temp List File", "Error !", MB_OK|MB_ICONERROR);
        return;
    }
    fprintf(fp, "%s\n", path);
    fclose(fp);
    // Prepare Arguments
    memset(args, 0, lenArgs);
    sprintf(args, "/K \"echo [/] Starting Scan, Please Waiting... & echo (Path = \"%s\") & echo. & \"%s\"", path, PARAMS.clamdscanPath);
    if (PARAMS.clamdscanLogEnable == 1) {
        strcat(args, " --log=\"");
        strcat(args, PARAMS.clamdscanLogPath);
        strcat(args, "\"");
    }
    strcat(args, " --infected --remove=no --config-file=\"");
    strcat(args, PARAMS.clamdConfPath);
    strcat(args, "\" --file-list=\"");
    strcat(args, listFile);
    strcat(args, "\" & echo. & echo [+] End Of Scan !\"");
    // Start Command
    if ((int)ShellExecute(NULL, "open", "cmd", args, clamdScanFolder, SW_SHOW) > 32) {
        MessageBox(WIN.hClamdscan, "Scan Started !\n(Show Command-Line Window To Watch Results)\n(You Can Close This Window)", "OK !", MB_OK|MB_ICONINFORMATION);
        unlink(listFile);
    } else {
        MessageBox(WIN.hClamdscan, "Can't Start This Scan !", "Error !", MB_OK|MB_ICONERROR);
    }
    return;
}
