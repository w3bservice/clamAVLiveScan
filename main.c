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

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WIN.hInst = hInstance;
	// Load Vars
	if (loadVars() == RET_E) {
        return RET_E;
    }
    // Auto-Start With Windows (Bypass Parameters)
    if (strstr((char *)lpCmdLine, "--auto") != NULL) {
        PARAMS.startAuto = 1;
    }
    addToLogFile("[+] ------ START PROGRAM ------", NULL, NULL);
    // Start Window
    DialogBox(hInstance, "DIALOGMAIN", NULL, (DLGPROC)dlgProcMain);
    // Unload
    unloadVars();
    addToLogFile("[/] ------ STOP PROGRAM ------", NULL, NULL);
    return RET_O;
}

/******************************************************************************/

DWORD WINAPI autoHideMain(LPVOID arg)
{
    int i;
    
    for(i = 0; i < 30; i++) { // Wait 3 Seconds
        Sleep(100);
        if (IsWindowVisible(WIN.hMain)) {
            minimizeToNotifyTray();
            return 0;
        }
    }
    return 0;
}

/******************************************************************************/

BOOL APIENTRY dlgProcMain(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
		// Init
		case WM_INITDIALOG:
            WIN.hMain = hDlg;
            // Load Icons
            IMGS.iconMain = (HICON)LoadImage(WIN.hInst, MAKEINTRESOURCE(IDICONMAIN), IMAGE_ICON, 32, 32, 0);
            IMGS.iconMainSmall = (HICON)LoadImage(WIN.hInst, MAKEINTRESOURCE(IDICONMAINSMALL), IMAGE_ICON, 16, 16, 0);
            // Set Icon
            SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)IMGS.iconMain);
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)IMGS.iconMainSmall);
            // Init Tray Icon
			initTrayIcon();
            // Set Limits
            SendDlgItemMessage(hDlg, IDEDITCLAMDEXE, EM_LIMITTEXT, (WPARAM)(MAXPATH - 1), 0);
            SendDlgItemMessage(hDlg, IDEDITCLAMDCONF, EM_LIMITTEXT, (WPARAM)(MAXPATH - 1), 0);
            SendDlgItemMessage(hDlg, IDEDITCLAMDIP, EM_LIMITTEXT, (WPARAM)(MAXIP - 1), 0);
            SendDlgItemMessage(hDlg, IDEDITCLAMDPORT, EM_LIMITTEXT, (WPARAM)5, 0);
            SendDlgItemMessage(hDlg, IDEDITLIVELOG, EM_LIMITTEXT, (WPARAM)(MAXPATH - 1), 0);
            SendDlgItemMessage(hDlg, IDEDITQUARANTINE, EM_LIMITTEXT, (WPARAM)(MAXPATH - 1), 0);
            // Set Defaults
            SetDlgItemText(hDlg, IDEDITCLAMDIP, DEFAULTCLAMDIP);
            SetDlgItemText(hDlg, IDEDITCLAMDPORT, DEFAULTCLAMDPORT);
            SetDlgItemText(hDlg, IDEDITCLAMDSTATUS, "NOT RUNNING");
            SetDlgItemText(hDlg, IDEDITLIVESTATUS, "NOT RUNNING");
            // Load Parameters To GUI
            loadAllGUIParams();
            EnableWindow(GetDlgItem(hDlg, IDBTNACTIONSSCAN), FALSE);
            EnableWindow(GetDlgItem(WIN.hMain, IDBTNACTIONPROCESSSCAN), FALSE);
            EnableMenuItem(SYSTRAY.hPopMenu, IDTRAYMENUSCAN, MF_DISABLED);
            EnableMenuItem(SYSTRAY.hPopMenu, IDTRAYMENUSCANPROC, MF_DISABLED);
            // Check/Fix Regedit
            if (PARAMS.autoStartWindows == 1) {
                if (addToRegedit(0) == RET_E) {
                    CheckDlgButton(hDlg, IDCHECKAUTOSTART, BST_UNCHECKED);
                    PARAMS.autoStartWindows = 0;
                    saveParams();
                }
            }
            // Auto-Start
            if (PARAMS.clamdAutoStart == 1) {
	            startStopClamd(0);
			}
			if (PARAMS.liveAutoStart == 1) {
                startStopLive(0);
			}
            if (PARAMS.startAuto == 1) {
				CreateThread(NULL, 0, autoHideMain, NULL, 0, NULL);
            }
			return TRUE;
		// Command
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
                // Select clamd.exe
                case IDBTNSELECTCLAMDEXE:
					selectAPath(hDlg, IDEDITCLAMDEXE, "clamd.exe\0clamd.exe\0All Files (*)\0*\0");
					break;
                // Select clamd.conf
                case IDBTNSELECTCLAMDCONF:
					selectAPath(hDlg, IDEDITCLAMDCONF, "clamd.conf\0clamd.conf\0All Files (*)\0*\0");
					break;
                // Select Log File
                case IDBTNSELECTLIVELOG:
					selectAPath(hDlg, IDEDITLIVELOG, "Log Files (*.log)\0*.log\0All Files (*)\0*\0");
					break;
                // Select Quarantine Folder
                case IDBTNSELECTQUARANTINE:
					quarantineSelectFolder();
					break;
                // Start/Stop Clamd
                case IDBTNCLAMDSTART:
                    startStopClamd(1);
                    break;
                // Start/Stop Live
                case IDBTNLIVESTART:
                    startStopLive(1);
                    break;
                // Check Clamd Auto-Start
                case IDCHECKCLAMDAUTO:
                    if (IsDlgButtonChecked(hDlg, IDCHECKCLAMDAUTO) == BST_CHECKED) {
                        PARAMS.clamdAutoStart = 1;
                    } else {
                        PARAMS.clamdAutoStart = 0;
                    }
                    saveParams();
                    break;
                // Check Live Scan Auto-Start
                case IDCHECKLIVEAUTO:
                    if (IsDlgButtonChecked(hDlg, IDCHECKLIVEAUTO) == BST_CHECKED) {
                        PARAMS.liveAutoStart = 1;
                    } else {
                        PARAMS.liveAutoStart = 0;
                    }
                    saveParams();
                    break;
                // Check Live Scan Log
                case IDCHECKLIVELOG:
                    if (IsDlgButtonChecked(hDlg, IDCHECKLIVELOG) == BST_CHECKED) {
                        PARAMS.liveLogEnable = 1;
                    } else {
                        PARAMS.liveLogEnable = 0;
                    }
                    saveParams();
                    break;
                // Check Quarantine
                case IDCHECKQUARANTINE:
                    if (IsDlgButtonChecked(hDlg, IDCHECKQUARANTINE) == BST_CHECKED) {
                        PARAMS.quarantineEnable = 1;
                    } else {
                        PARAMS.quarantineEnable = 0;
                    }
                    saveParams();
                    break;
                // Check Windows Auto-Start
                case IDCHECKAUTOSTART:
                    if (IsDlgButtonChecked(hDlg, IDCHECKAUTOSTART) == BST_CHECKED) {
                        // Add To Regedit
                        if (addToRegedit(1) == RET_O) {
                            PARAMS.autoStartWindows = 1;
                            saveParams();
                        }
                    } else {
                        // Delete From Regedit
                        if (delFromRegedit() == RET_O) {
                            PARAMS.autoStartWindows = 0;
                            saveParams();
                        }
                    }
                    break;
                // Check Minimize To Systray
                case IDCHECKMINIMIZE:
                    if (IsDlgButtonChecked(hDlg, IDCHECKMINIMIZE) == BST_CHECKED) {
                        PARAMS.minimizeSystray = 1;
                    } else {
                        PARAMS.minimizeSystray = 0;
                    }
                    saveParams();
                    break;
                // Add Include Path
                case IDBTNLIVEINCADD:
                    addLiveInclude();
                    break;
                // Remove Include Path
                case IDBTNLIVEINCDEL:
                    delLiveInclude();
                    break;
                // Add Exclude Path
                case IDBTNLIVEEXCADD:
                    addLiveExclude();
                    break;
                // Remove Exclude Path
                case IDBTNLIVEEXCDEL:
                    delLiveExclude();
                    break;
                // Show Live Logs
                case IDBTNACTIONSLOGS:
                    showLivelogs();
                    break;
                // Show Scan File/Folder
                case IDBTNACTIONSSCAN:
                    showClamdscan();
                    break;
                // Show Scan Process
                case IDBTNACTIONPROCESSSCAN:
                    showProcessScan();
                    break;
                // Show Update Databases
                case IDBTNACTIONSUPDATE:
                    showFreshclam();
                    break;
                // Systray Menu Show
                case IDTRAYMENUSHOW:
                    maximizeFromNotifyTray();
                    break;
                // Systray Menu Show Live Logs
                case IDTRAYMENULOGS:
                    showLivelogs();
                    break;
                // Systray Menu Show Scan File/Folder
                case IDTRAYMENUSCAN:
                    showClamdscan();
                    break;
                // Systray Menu Show Scan All Processes
                case IDTRAYMENUSCANPROC:
                    showProcessScan();
                    break;
                // Systray Menu Show Update Databases
                case IDTRAYMENUUPDATE:
                    showFreshclam();
                    break;
                // Exit
				case IDBTNEXIT:
                case IDTRAYMENUEXIT: // Systray Exit
					if (MessageBox(hDlg, "Do You Want Stop All And Exit The Program ?", "Exit Program ?", MB_YESNO|MB_ICONQUESTION) == 6) {
						EndDialog(hDlg, 0);
					}
					break;
			}
			return TRUE;
		// Close
        case WM_CLOSE:
            if (MessageBox(hDlg, "Do You Want Stop All And Exit The Program ?", "Exit Program ?", MB_YESNO|MB_ICONQUESTION) == 6) {
				EndDialog(hDlg, 0);
			}
			return TRUE;
        // Change Window Status
        case WM_SIZE:
            // Minimize
			if (LOWORD(wParam) == SIZE_MINIMIZED) {
				if (PARAMS.minimizeSystray == 1) {
                	minimizeToNotifyTray();
                }
			}
			return TRUE;
        // SysTray Icon
        case WM_TRAYICON:
            if (LOWORD(lParam) == WM_LBUTTONUP) {
				maximizeFromNotifyTray();
            } else if (LOWORD(lParam) == WM_RBUTTONUP) {
			    showSystrayPopMenu();
            }
			return TRUE;
		// Default
		default:
			return FALSE;
	}
}
