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

void startStopClamd(short int showMsg)
{
	char ip[MAXIP];
	char port[6];

	if (CLAMD.isRunning == 0) {
		// Check Paths
		if (fileExists(PARAMS.clamdPath) == RET_E) {
			if (showMsg == 1) {
				MessageBox(WIN.hMain, "Invalid clamd.exe Path (Or Can't Open)", "Error", MB_OK|MB_ICONERROR);
			}
			return;
		}
		if (fileExists(PARAMS.clamdConfPath) == RET_E) {
			if (showMsg == 1) {
				MessageBox(WIN.hMain, "Invalid clamd.conf Path (Or Can't Open)", "Error", MB_OK|MB_ICONERROR);
			}
			return;
		}
		// Get Clamd Params
		memset(ip, 0, MAXIP);
		memset(port, 0, 6);
		GetDlgItemText(WIN.hMain, IDEDITCLAMDIP, ip, MAXIP);
		GetDlgItemText(WIN.hMain, IDEDITCLAMDPORT, port, 6);
		// Check IP/Port
		if (strIsIPv4(ip) == RET_E) {
			if (showMsg == 1) {
				MessageBox(WIN.hMain, "Invalid Clamd TCP IP Address", "Error", MB_OK|MB_ICONERROR);
			}
			return;
		}
		if (strIsPort(port) == RET_E) {
			if (showMsg == 1) {
				MessageBox(WIN.hMain, "Invalid Clamd TCP Port", "Error", MB_OK|MB_ICONERROR);
			}
			return;
		}
		// Set Params
		memset(PARAMS.clamdIP, 0, MAXIP);
		strcpy(PARAMS.clamdIP, ip);
		PARAMS.clamdPort = atoi(port);
		// Save Params
		saveParams();
		// Disable GUI
		enableDisableClamdGUI(0);
		// Stop All Others Clamd
		checkAndKillClamd();
		// Start Thread
		CLAMD.isRunning = 1;
		SetDlgItemText(WIN.hMain, IDEDITCLAMDSTATUS, "WAITING...");
		addToLogFile("[/] -- Starting Clamd... --", NULL, NULL);
		if ((CLAMD.hThread = CreateThread(NULL, 0, startClamdThread, NULL, 0, NULL)) == NULL) {
			CLAMD.isRunning = 0;
			enableDisableClamdGUI(1);
			if (showMsg == 1) {
				addToLogFile("[-] -- Can't Start Clamd Thread --", NULL, NULL);
				MessageBox(WIN.hMain, "Can't Start Clamd Thread", "Error", MB_OK|MB_ICONERROR);
			}
		} else if (showMsg == 1) {
			MessageBox(WIN.hMain, "Clamd Thread Started !\n\nBut Please Wait 15 Or 30 Seconds Before Start Scan(s)\n(Because Clamd Load Virus Databases)", "OK", MB_OK|MB_ICONINFORMATION);
		}
	} else {
		// Stop
		SetDlgItemText(WIN.hMain, IDEDITCLAMDSTATUS, "WAITING...");
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNCLAMDSTART), FALSE);
		LIVE.isRunning = 0;
		CLAMD.isRunning = 0;
	}
	return;
}

/******************************************************************************/

DWORD WINAPI startClamdThread(LPVOID arg)
{
	int i = 0;

	// Clamd Loop
	while(CLAMD.isRunning == 1) {
		if (i == 0) {
			// Check Process
			if (clamdProcRunning() == RET_E) {
				// Stop Process
				stopClamdProc();
				Sleep(500);
				addToLogFile("[/] -- Starting Clamd... --", NULL, NULL);
				// Start Process
				if (startClamdProc() == RET_E) {
					addToLogFile("[-] -- Can't Start Clamd Process --", NULL, NULL);
					goto endClamdThread;
				}
				SetDlgItemText(WIN.hMain, IDEDITCLAMDSTATUS, "RUNNING");
				addToLogFile("[+] -- Clamd Started --", NULL, NULL);
			}
		}
		Sleep(1000);
		i++;
		if (i == 7) { // Each 7 Seconds
			i = 0;
		}
	}
endClamdThread:
	// Stop Clamd Process
	stopClamdProc();
	addToLogFile("[/] -- Clamd Stopped --", NULL, NULL);
	// Stop Clamd Thread
	CloseHandle(CLAMD.hThread);
	CLAMD.hThread = NULL;
	CLAMD.isRunning = 0;
	// Enable Clamd GUI
	enableDisableClamdGUI(1);
	return 0;
}

/******************************************************************************/

void enableDisableClamdGUI(short int enable)
{
	if (enable == 0) {
		// Disable GUI
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNSELECTCLAMDEXE), FALSE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNSELECTCLAMDCONF), FALSE);
		EnableWindow(GetDlgItem(WIN.hMain, IDEDITCLAMDIP), FALSE);
		EnableWindow(GetDlgItem(WIN.hMain, IDEDITCLAMDPORT), FALSE);
		SetDlgItemText(WIN.hMain, IDEDITCLAMDSTATUS, "STARTING...");
		SetDlgItemText(WIN.hMain, IDBTNCLAMDSTART, "Stop Clamd");
		// Clamdscan / process
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNACTIONSSCAN), TRUE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNACTIONPROCESSSCAN), TRUE);
		EnableMenuItem(SYSTRAY.hPopMenu, IDTRAYMENUSCAN, MF_ENABLED);
		EnableMenuItem(SYSTRAY.hPopMenu, IDTRAYMENUSCANPROC, MF_ENABLED);
		if (WIN.hClamdscan != NULL) {
			EnableWindow(GetDlgItem(WIN.hClamdscan, IDBTNCLAMDSCANSTART), TRUE);
		}
	} else {
		// Enable GUI
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNSELECTCLAMDEXE), TRUE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNSELECTCLAMDCONF), TRUE);
		EnableWindow(GetDlgItem(WIN.hMain, IDEDITCLAMDIP), TRUE);
		EnableWindow(GetDlgItem(WIN.hMain, IDEDITCLAMDPORT), TRUE);
		SetDlgItemText(WIN.hMain, IDEDITCLAMDSTATUS, "NOT RUNNING");
		SetDlgItemText(WIN.hMain, IDBTNCLAMDSTART, "Start Clamd");
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNCLAMDSTART), TRUE);
		// Clamdscan / process
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNACTIONSSCAN), FALSE);
		EnableWindow(GetDlgItem(WIN.hMain, IDBTNACTIONPROCESSSCAN), FALSE);
		EnableMenuItem(SYSTRAY.hPopMenu, IDTRAYMENUSCAN, MF_DISABLED);
		EnableMenuItem(SYSTRAY.hPopMenu, IDTRAYMENUSCANPROC, MF_DISABLED);
		if (WIN.hClamdscan != NULL) {
			EnableWindow(GetDlgItem(WIN.hClamdscan, IDBTNCLAMDSCANSTART), FALSE);
		}
	}
	return;
}

/******************************************************************************/

short int startClamdProc()
{
	char args[10240];
	char path[MAXPATH];
	int i;
	short int ret = RET_O;

	// Get Arguments
	memset(args, 0, 10240);
	sprintf(args, "--config-file=\"%s\"", PARAMS.clamdConfPath);
	// Get Start Path
	memset(path, 0, MAXPATH);
	strcpy(path, PARAMS.clamdPath);
	i = strlen(path) - 1;
	while(i > 1 && path[i] != '\\') {
		i--;
	}
	if (i <= 1) {
		return RET_E;
	}
	path[i + 1] = '\0';
	// Prepare
	memset(&CLAMD.hProc, 0, sizeof(SHELLEXECUTEINFO));
	CLAMD.hProc.cbSize = sizeof(SHELLEXECUTEINFO);
	CLAMD.hProc.fMask = SEE_MASK_NOCLOSEPROCESS;
	CLAMD.hProc.hwnd = HWND_DESKTOP;
	CLAMD.hProc.lpVerb = "open";
	CLAMD.hProc.lpFile = PARAMS.clamdPath;
	CLAMD.hProc.lpParameters = args;
	CLAMD.hProc.lpDirectory = path;
	CLAMD.hProc.nShow = SW_HIDE;
	CLAMD.hProc.hInstApp = NULL;
	CLAMD.hProc.hProcess = NULL;
	// Start
	if (!ShellExecuteEx(&CLAMD.hProc)) {
		return RET_E;
	}
	Sleep(500);
	// Get PID
	if (CLAMD.hProc.hProcess == NULL || (CLAMD.pid = GetProcessId(CLAMD.hProc.hProcess)) == 0) {
		return RET_E;
	}
	return RET_O;
}

/******************************************************************************/

short int clamdProcRunning()
{
	HANDLE hProcessSnap = NULL;
	PROCESSENTRY32 pe32;
	short int ret = RET_E;

	if (CLAMD.hProc.hProcess == NULL || CLAMD.pid == 0) {
		return RET_E;
	}
	if ((hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE) {
		return RET_E;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32)) {
		goto stopProcRun;
	}
	do {
		if (CLAMD.pid == pe32.th32ProcessID) {
			ret = RET_O;
			goto stopProcRun;
		}
	} while(Process32Next(hProcessSnap, &pe32));
stopProcRun:
	if (hProcessSnap != NULL) {
		CloseHandle(hProcessSnap);
	}
	return ret;
}

/******************************************************************************/

void stopClamdProc()
{
	if (CLAMD.hProc.hProcess != NULL) {
		TerminateProcess(CLAMD.hProc.hProcess, 0);
	}
	memset(&CLAMD.hProc, 0, sizeof(SHELLEXECUTEINFO));
	CLAMD.hProc.hProcess = NULL;
	CLAMD.pid = 0;
	return;
}

/******************************************************************************/

void checkAndKillClamd()
{
	HANDLE hProcessSnap = NULL;
	HANDLE hProc = NULL;
	PROCESSENTRY32 pe32;

	if ((hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE) {
		return;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32)) {
		goto stopKillProc;
	}
	do {
		if (strcasecmp((char *)pe32.szExeFile, "clamd.exe") == 0) {
			if ((hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, TRUE, pe32.th32ProcessID)) != NULL) {
				TerminateProcess(hProc, 0);
				hProc = NULL;
			}
		}
	} while(Process32Next(hProcessSnap, &pe32));
stopKillProc:
	if (hProcessSnap != NULL) {
		CloseHandle(hProcessSnap);
	}
	return;
}
