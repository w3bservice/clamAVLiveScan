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

void startStopLive(short int showMsg)
{
	if (LIVE.isRunning == 0) {
		// Check Clamd
		if (showMsg == 1 && CLAMD.isRunning == 0 || CLAMD.hThread == NULL) {
			MessageBox(WIN.hProcessScan, "Clamd Not Running !\nPlease \"Start Clamd\" On Main Window Before", "Warning !", MB_OK|MB_ICONEXCLAMATION);
			return;
		}
		// Check Params
		if ((int)SendDlgItemMessage(WIN.hMain, IDLISTLIVEINCLUDE, LB_GETCOUNT, 0, 0) <= 0) {
			if (showMsg == 1) {
				MessageBox(WIN.hMain, "No Scan Include Folder\n(Please Add One At Least)", "Warning", MB_OK|MB_ICONEXCLAMATION);
			}
			return;
		}
		// Disable GUI
		enableDisableLiveGUI(0);
		// Start Thread
		addToLogFile("[/] -- Live Scan Starting... --", NULL, NULL);
		LIVE.isRunning = 1;
		if ((LIVE.hThread = CreateThread(NULL, 0, startLiveThread, NULL, 0, NULL)) == NULL) {
			LIVE.isRunning = 0;
			enableDisableLiveGUI(1);
			if (showMsg == 1) {
				addToLogFile("[-] -- Can't Start Live Scan Thread --", NULL, NULL);
				MessageBox(WIN.hMain, "Can't Start Live Scan Thread", "Error", MB_OK|MB_ICONERROR);
			}
			return;
		}
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVEINCADD), FALSE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVEINCDEL), FALSE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVEEXCADD), FALSE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVEEXCDEL), FALSE);
	} else {
		// Stop
		SetDlgItemText(WIN.hMain, IDEDITLIVESTATUS, "WAITING...");
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVESTART), FALSE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVEINCADD), TRUE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVEINCDEL), TRUE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVEEXCADD), TRUE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVEEXCDEL), TRUE);
		LIVE.isRunning = 0;
	}
	return;
}

/******************************************************************************/

DWORD WINAPI startLiveThread(LPVOID arg)
{
	struct sINCLUDEPATHS *nInc = NULL;
	int countInc = 0;
	int countScan = 0;

	// Set Live Status
	SetDlgItemText(WIN.hMain, IDEDITLIVESTATUS, "RUNNING");
	addToLogFile("[+] -- Live Scan Started --", NULL, NULL);
	// Live Loop
	while(LIVE.isRunning == 1) {
		// Start All Includes Thread
		if (countInc == 0) {
			// Lock
			WaitForSingleObject(PARAMS.hMutex, INFINITE);
			// Start All
			nInc = PARAMS.includeHead;
			while(nInc != NULL) {
				if (nInc->isRunning == 0 && nInc->hThread == NULL) {
					// Create Thread
					nInc->isRunning = 1;
					if ((nInc->hThread = CreateThread(NULL, 0, startIncludesThread, (LPVOID)nInc, 0, NULL)) == NULL) {
						nInc->isRunning = 0;
					}
				}
				nInc = nInc->next;
			}
			// Unlock
			ReleaseMutex(PARAMS.hMutex);
		}
		// Start Scan
		if (countScan == 0) {
			// Add Temp To Scan List
			addTempToScanList();
			// Start Net Scan
			if (CLAMD.isRunning == 1 && LIVE.isRunningNet == 0 && LIVE.hThreadNet == NULL) {
				// Start Thread
				LIVE.isRunningNet = 1;
				if ((LIVE.hThreadNet = CreateThread(NULL, 0, startLiveClamdScan, NULL, 0, NULL)) == NULL) {
					LIVE.isRunningNet = 0;
				}
			}
		}
		// Sleep
		Sleep(500);
		// Increment
		countInc++;
		if (countInc == 6) { // Each 3 Seconds
			countInc = 0;
		}
		countScan++;
		if (countScan == 2) { // Each 1 Second
			countScan = 0;
		}
	}
	// Stop All Includes Thread
	stopAllIncludeThreads();
	// Clean-Up Lists
	delAllTempScanList();
	delAllScanList();
	addToLogFile("[/] -- Live Scan Stopped --", NULL, NULL);
	// Stop Live Thread
	CloseHandle(LIVE.hThread);
	LIVE.hThread = NULL;
	LIVE.isRunning = 0;
	// Enable Live GUI
	enableDisableLiveGUI(1);
	return 0;
}

/******************************************************************************/

void enableDisableLiveGUI(short int enable)
{
	if (enable == 0) {
		// Disable GUI
		EnableWindow(GetDlgItem(WIN.hMain, IDCHECKLIVELOG), FALSE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNSELECTLIVELOG), FALSE);
		SetDlgItemText(WIN.hMain, IDEDITLIVESTATUS, "STARTING...");
		SetDlgItemText(WIN.hMain, IDBTNLIVESTART, "Stop Live Scan");
	} else {
		// Enable GUI
		EnableWindow(GetDlgItem(WIN.hMain, IDCHECKLIVELOG), TRUE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNSELECTLIVELOG), TRUE);
		SetDlgItemText(WIN.hMain, IDEDITLIVESTATUS, "NOT RUNNING");
		SetDlgItemText(WIN.hMain, IDBTNLIVESTART, "Start Live Scan");
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNLIVESTART), TRUE);
	}
	return;
}

/******************************************************************************/

void addLiveInclude()
{
	char path[MAXPATH];

	// Select Folder
	memset(path, 0, MAXPATH);
	if (selectAFolder(WIN.hMain, path) == RET_E) {
		return;
	}
	// Add To List
	if (addIncludePath(path, 1) == RET_E) {
		return;
	}
	// Save
	saveIncludeParams();
	// Add To GUI List
	SendDlgItemMessage(WIN.hMain, IDLISTLIVEINCLUDE, LB_ADDSTRING, 0, (LPARAM)path);
	return;
}

/******************************************************************************/

void addLiveExclude()
{
	char path[MAXPATH];
	short int isFile;

	memset(path, 0, MAXPATH);
	if (MessageBox(WIN.hMain, "Do You Want Add Folder ?\n(Yes=Add Folder, No=Add File)", "Add Folder ?", MB_YESNO|MB_ICONQUESTION) == 6) {
		// Select Folder
		if (selectAFolder(WIN.hMain, path) == RET_E) {
			return;
		}
		isFile = 0;
	} else {
		// Select File
		if (selectAFile(WIN.hMain, path) == RET_E) {
			return;
		}
		isFile = 1;
	}
	// Add To List
	if (addExcludePath(path, 1) == RET_E) {
		return;
	}
	// Save
	saveExcludeParams();
	// Add To GUI List
	SendDlgItemMessage(WIN.hMain, IDLISTLIVEEXCLUDE, LB_ADDSTRING, 0, (LPARAM)path);
	return;
}

/******************************************************************************/

void delLiveInclude()
{
	int sel;
	char path[MAXPATH];

	// Get Include Item
	if ((sel = (int)SendDlgItemMessage(WIN.hMain, IDLISTLIVEINCLUDE, LB_GETCURSEL, 0, 0)) < 0) {
		MessageBox(WIN.hMain, "Please Select Include Path Before", "Warning", MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	if (MessageBox(WIN.hMain, "Do You Remove Selected Include Path ?", "Remove Include ?", MB_YESNO|MB_ICONQUESTION) != 6) {
		return;
	}
	memset(path, 0, MAXPATH);
	SendMessage(GetDlgItem(WIN.hMain, IDLISTLIVEINCLUDE), LB_GETTEXT, (WPARAM)sel, (LPARAM)path);
	// Remove Include Item
	delIncludePath(path);
	SendMessage(GetDlgItem(WIN.hMain, IDLISTLIVEINCLUDE), LB_DELETESTRING, (WPARAM)sel, 0);
	// Save
	saveIncludeParams();
	return;
}

/******************************************************************************/

void delLiveExclude()
{
	int sel;
	char path[MAXPATH];

	// Get Exclude Item
	if ((sel = (int)SendDlgItemMessage(WIN.hMain, IDLISTLIVEEXCLUDE, LB_GETCURSEL, 0, 0)) < 0) {
		MessageBox(WIN.hMain, "Please Select Exclude Path Before", "Warning", MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	if (MessageBox(WIN.hMain, "Do You Remove Selected Exclude Path ?", "Remove Exclude ?", MB_YESNO|MB_ICONQUESTION) != 6) {
		return;
	}
	memset(path, 0, MAXPATH);
	SendMessage(GetDlgItem(WIN.hMain, IDLISTLIVEEXCLUDE), LB_GETTEXT, (WPARAM)sel, (LPARAM)path);
	// Remove Exclude Item
	delExcludePath(path);
	SendMessage(GetDlgItem(WIN.hMain, IDLISTLIVEEXCLUDE), LB_DELETESTRING, (WPARAM)sel, 0);
	// Save
	saveExcludeParams();
	return;
}
