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

BOOL APIENTRY dlgProcessScanProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg) {
		// INIT
		case WM_INITDIALOG:
			WIN.hProcessScan = hDlg;
			// Set Icon
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)IMGS.iconMain);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)IMGS.iconMainSmall);
			// Set Limits
			SendDlgItemMessage(hDlg, IDEDITPROCESSSCANLOG, EM_LIMITTEXT, 0, 0);
			// Set Others
			EnableWindow(GetDlgItem(hDlg, IDBTNPROCESSSCANCOPY), FALSE);
			return TRUE;
		// CLOSE
		case WM_CLOSE:
			if (PROCESSSCAN.isRunning == 0) {
				EndDialog(hDlg, 0);
				WIN.hProcessScan = NULL;
			}
			return TRUE;
		// COMMAND
		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				// Start
				case IDBTNPROCESSSCANSTART:
					startStopProcessScan();
					break;
					// Copy
				case IDBTNPROCESSSCANCOPY:
					copyProcessToClipboard();
					break;
					// Close
				case IDBTNPROCESSSCANCLOSE:
					EndDialog(hDlg, 0);
					WIN.hProcessScan = NULL;
					break;
			}
			return TRUE;
		// DEFAULT
		default:
			return FALSE;
	}
}

/******************************************************************************/

DWORD WINAPI startProcessScanThread(LPVOID arg)
{
	DialogBox(WIN.hInst, "DIALOGPROCESSSCAN", NULL, (DLGPROC)dlgProcessScanProc);
	return 0;
}

/******************************************************************************/

void showProcessScan()
{
	if (WIN.hProcessScan == NULL) {
		CreateThread(NULL, 0, startProcessScanThread, NULL, 0, NULL);
	} else {
		// Show/Focus
		ShowWindow(WIN.hProcessScan, SW_RESTORE);
		BringWindowToTop(WIN.hProcessScan);
		SetFocus(WIN.hProcessScan);
	}
	return;
}

/******************************************************************************/

void startStopProcessScan()
{
	if (PROCESSSCAN.isRunning == 0) {
		// Start Process Scan
		if (CLAMD.isRunning == 0 || CLAMD.hThread == NULL) {
			MessageBox(WIN.hProcessScan, "Clamd Not Running !\nPlease \"Start Clamd\" On Main Window Before", "Warning !", MB_OK|MB_ICONEXCLAMATION);
			return;
		}
		EnableWindow(GetDlgItem(WIN.hProcessScan, IDBTNPROCESSSCANSTART), FALSE);
		// Clean Logs
		SetDlgItemText(WIN.hProcessScan, IDEDITPROCESSSCANLOG, "");
		PROCESSSCAN.countInfected = 0;
		// Start Thread
		PROCESSSCAN.isRunning = 1;
		if ((PROCESSSCAN.hThread = CreateThread(NULL, 0, scanAllProcessThread, NULL, 0, NULL)) == NULL) {
			addToProcessResult(0, "[-] Error: Can't Start Scan Process Thread", NULL, NULL);
			EnableWindow(GetDlgItem(WIN.hProcessScan, IDBTNPROCESSSCANSTART), TRUE);
			PROCESSSCAN.isRunning = 0;
			return;
		}
		EnableWindow(GetDlgItem(WIN.hProcessScan, IDBTNPROCESSSCANCOPY), FALSE);
		EnableWindow(GetDlgItem(WIN.hProcessScan, IDBTNPROCESSSCANCLOSE), FALSE);
		SetDlgItemText(WIN.hProcessScan, IDBTNPROCESSSCANSTART, "Stop Scan");
		EnableWindow(GetDlgItem(WIN.hProcessScan, IDBTNPROCESSSCANSTART), TRUE);
	} else {
		// Stop Process Scan
		EnableWindow(GetDlgItem(WIN.hProcessScan, IDBTNPROCESSSCANSTART), FALSE);
		PROCESSSCAN.isRunning = 0;
		// Wait End
		while(PROCESSSCAN.hThread != NULL) {
			Sleep(750);
		}
		// Clean List
		delAllProcessScanList();
		EnableWindow(GetDlgItem(WIN.hProcessScan, IDBTNPROCESSSCANCOPY), TRUE);
		EnableWindow(GetDlgItem(WIN.hProcessScan, IDBTNPROCESSSCANCLOSE), TRUE);
		SetDlgItemText(WIN.hProcessScan, IDBTNPROCESSSCANSTART, "Start Scan");
		EnableWindow(GetDlgItem(WIN.hProcessScan, IDBTNPROCESSSCANSTART), TRUE);
	}
	return;
}

/******************************************************************************/

DWORD WINAPI scanAllProcessThread(LPVOID arg)
{
	HANDLE hProcessSnap = NULL;
	HANDLE hProc = NULL;
	PROCESSENTRY32 pe32;
	HMODULE hMods[2048];
	DWORD cbNeeded;
	char path[MAXPATH];
	char mod[MAXPATH];
	unsigned int i;
	int countProc = 0;
	int countMod = 0;

	addToProcessResult(0, "[/] Starting List All Processes And Modules, Please Waiting...", NULL, NULL);
	// List All Processes
	if ((hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE) {
		addToProcessResult(0, "[-] Error: Can't List Processes", NULL, NULL);
		goto stopProcThread;
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32)) {
		addToProcessResult(0, "[-] Error: Can't Get First Process", NULL, NULL);
		goto stopProcThread;
	}
	do {
		// Get Process Handle From PID
		if ((hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID)) != NULL) {
			// Get Real Full Path
			memset(path, 0, MAXPATH);
			if (GetProcessImageFileName(hProc, path, MAXPATH) != 0 && getTheRealPath(path) == RET_O) {
				if (addToProcessScanList(path) == RET_O) {
					countProc++;
				}
				// Get All Modules
				if (EnumProcessModules(hProc, hMods, sizeof(hMods), &cbNeeded) != 0) {
					for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++) {
						// Get Real Module Path
						memset(mod, 0, MAXPATH);
						if (GetModuleFileNameEx(hProc, hMods[i], mod, MAXPATH) != 0) {
							if (addToProcessScanList(mod) == RET_O) {
								countMod++;
							}
						}
						if (i == 2047) {
							break;
						}
					}
				}
			}
			CloseHandle(hProc);
			hProc = NULL;
		}
	} while(PROCESSSCAN.isRunning == 1 && Process32Next(hProcessSnap, &pe32));
	// Check Processes
	if (countProc == 0 && countMod == 0) {
		addToProcessResult(0, "[-] No Process Or Module Found", NULL, NULL);
		goto stopProcThread;
	}
	// Start Process Scan
	memset(mod, 0, MAXPATH);
	sprintf(mod, "[+] OK: Start Scan (%d Processes And %d Modules)\r\n", countProc, countMod);
	addToProcessResult(0, mod, NULL, NULL);
	startProcessListScan();
stopProcThread:
	if (hProcessSnap != NULL) {
		CloseHandle(hProcessSnap);
	}
	// Stop Thread
	CloseHandle(PROCESSSCAN.hThread);
	PROCESSSCAN.hThread = NULL;
	if (PROCESSSCAN.isRunning == 1) {
		startStopProcessScan();
	} else {
		memset(mod, 0, MAXPATH);
		sprintf(mod, "[/] Cancel By User (%d Infected)", PROCESSSCAN.countInfected);
		addToProcessResult(1, mod, NULL, NULL);
	}
	return 0;
}

/******************************************************************************/

short int getTheRealPath(char *path)
{
	int i;
	int j;
	int len = strlen(path);
	short int count = 0;
	char device[MAXPATH];
	char out[MAXPATH];
	char drive[3];

	if (len < 4) {
		return RET_E;
	}
	for(i = 0; i < len; i++) {
		if (path[i] == '\\') {
			count++;
			if (count == 3) {
				memset(device, 0, MAXPATH);
				memcpy(device, path, i);
				for(j = 'A'; j <= 'Z'; j++) {
					memset(out, 0, MAXPATH);
					memset(drive, 0, 3);
					sprintf(drive, "%c:", (char)j);
					if (QueryDosDevice(drive, out, MAXPATH) != 0 && strcmp(out, device) == 0) {
						memset(out, 0, MAXPATH);
						sprintf(out, "%c:%s", (char)j, path + i);
						memset(path, 0, MAXPATH);
						strcpy(path, out);
						return RET_O;
					}
				}
				return RET_E;
			}
		}
	}
	return RET_E;
}

/******************************************************************************/

short int addToProcessScanList(char *path)
{
	struct sTEMPPROCESSSCANLIST *n = NULL;

	// Check Already Exists In List
	if (strlen(path) < 4 || processAlreadyExists(path) == RET_O) {
		return RET_E;
	}
	// Add To Process List
	if ((n = (struct sTEMPPROCESSSCANLIST *)malloc(sizeof(struct sTEMPPROCESSSCANLIST))) == NULL) {
		return RET_E;
	}
	if ((n->path = (char *)calloc(strlen(path) + 1, sizeof(char))) == NULL) {
		free(n);
		return RET_E;
	}
	strcpy(n->path, path);
	n->next = NULL;
	if (PROCESSSCAN.scanProcHead == NULL) {
		PROCESSSCAN.scanProcHead = n;
	} else {
		PROCESSSCAN.scanProcLast->next = n;
	}
	PROCESSSCAN.scanProcLast = n;
	return RET_O;
}

/******************************************************************************/

void delAllProcessScanList()
{
	struct sTEMPPROCESSSCANLIST *n = NULL;
	struct sTEMPPROCESSSCANLIST *nNext = NULL;

	// Delete All Process Scan List
	n = PROCESSSCAN.scanProcHead;
	while(n != NULL) {
		nNext = n->next;
		free(n->path);
		free(n);
		n = nNext;
	}
	PROCESSSCAN.scanProcHead = NULL;
	PROCESSSCAN.scanProcLast = NULL;
	return;
}

/******************************************************************************/

short int processAlreadyExists(char *path)
{
	struct sTEMPPROCESSSCANLIST *n = NULL;

	n = PROCESSSCAN.scanProcHead;
	while(n != NULL) {
		if (strcmp(path, n->path) == 0) {
			return RET_O;
		}
		n = n->next;
	}
	return RET_E;
}

/******************************************************************************/

void startProcessListScan()
{
	struct sTEMPPROCESSSCANLIST *n = NULL;
	char tmp[64];

	n = PROCESSSCAN.scanProcHead;
	while(PROCESSSCAN.isRunning == 1 && n != NULL) {
		// Wait For Network
		while(PROCESSSCAN.isRunning == 1 && PROCESSSCAN.isRunningNet == 1) {
			Sleep(500);
		}
		if (PROCESSSCAN.isRunning == 0) {
			return;
		}
		// Start Network Scan Thread
		PROCESSSCAN.isRunningNet = 1;
		if ((PROCESSSCAN.hThreadNet = CreateThread(NULL, 0, startProcessNetScanThread, (LPVOID)n->path, 0, NULL)) == NULL) {
			PROCESSSCAN.isRunningNet = 0;
			addToProcessResult(0, "[-] Error: Can't Create Thread For Path: ", n->path, NULL);
			Sleep(1000);
		}
		n = n->next;
	}
	// Wait End
	while(PROCESSSCAN.isRunning == 1 && PROCESSSCAN.isRunningNet == 1) {
		Sleep(500);
	}
	// Display Results (End/Cancel)
	if (PROCESSSCAN.isRunning == 1) {
		memset(tmp, 0, 64);
		sprintf(tmp, "[+] End Of Scan (%d Infected)", PROCESSSCAN.countInfected);
		addToProcessResult(1, tmp, NULL, NULL);
	}
	return;
}

/******************************************************************************/

DWORD WINAPI startProcessNetScanThread(LPVOID arg)
{
	char *path = (char *)arg;
	int sAddrLen = sizeof(struct sockaddr);
	struct sockaddr_in sIn;
	char cmd[MAXPATH + 16];
	char buf[MAXBUFFER + 1];
	char *res = NULL;
	int count = 0;
	int len = 0;
	short int netError = 0;

	addToProcessResult(0, "[/] Starting Scan For File: ", path, NULL);
	// Start Clamd TCP Network Scan
	if ((PROCESSSCAN.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		goto stopProcessNet;
	}
	sIn.sin_family = AF_INET;
	sIn.sin_port = htons(PARAMS.clamdPort);
	sIn.sin_addr.s_addr = inet_addr(PARAMS.clamdIP);
	memset(&(sIn.sin_zero), 0, 8);
	if (connect(PROCESSSCAN.sock, (struct sockaddr *)&sIn, sAddrLen) == SOCKET_ERROR) {
		closesocket(PROCESSSCAN.sock);
		PROCESSSCAN.sock = INVALID_SOCKET;
		netError = 1;
		goto stopProcessNet;
	}
	memset(cmd, 0, (MAXPATH + 16));
	sprintf(cmd, "zCONTSCAN %s\0", path);
	if (send(PROCESSSCAN.sock, cmd, strlen(cmd) + 1, 0) == SOCKET_ERROR) {
		closesocket(PROCESSSCAN.sock);
		PROCESSSCAN.sock = INVALID_SOCKET;
		netError = 1;
		goto stopProcessNet;
	}
	if ((res = (char *)calloc(1, sizeof(char))) == NULL) {
		goto stopProcessNet;
	}
	do {
		memset(buf, 0, (MAXBUFFER + 1));
		if ((count = recv(PROCESSSCAN.sock, buf, MAXBUFFER, 0)) == SOCKET_ERROR) {
			closesocket(PROCESSSCAN.sock);
			PROCESSSCAN.sock = INVALID_SOCKET;
			goto stopProcessNet;
		}
		if (count > 0) {
			if ((res = (char *)realloc(res, (len + count + 1))) == NULL) {
				goto stopProcessNet;
			}
			memcpy(res + len, buf, count);
			len += count;
			res[len] = '\0';
		}
	} while(PROCESSSCAN.isRunning == 1 && count > 0);
	// Close Connection
	closesocket(PROCESSSCAN.sock);
	PROCESSSCAN.sock = INVALID_SOCKET;
	if (PROCESSSCAN.isRunning == 0) {
		goto stopProcessNet;
	}
	// Check Result
	if (strlen(res) > 0) {
		addToProcessResult(0, "[+] Scan File Result: ", res, NULL);
		// Check For Alert
		if (checkForAlert(path, res) == RET_O) {
			PROCESSSCAN.countInfected++;
		}
	}
stopProcessNet:
	if (netError == 1) {
		// Add To List
		addToProcessResult(0, "[-] (Maybe Clamd Not Running) Network Error For File: ", path, NULL);
	}
	// Clean-Up Results
	if (res != NULL) {
		free(res);
	}
	// Stop Socket
	if (PROCESSSCAN.sock != INVALID_SOCKET) {
		closesocket(PROCESSSCAN.sock);
		PROCESSSCAN.sock = INVALID_SOCKET;
	}
	// Stop Thread
	if (PROCESSSCAN.hThreadNet != NULL) {
		CloseHandle(PROCESSSCAN.hThreadNet);
		PROCESSSCAN.hThreadNet = NULL;
	}
	PROCESSSCAN.isRunningNet = 0;
	return 0;
}

/******************************************************************************/

void addToProcessResult(short int addLine, char *str1, char *str2, char *str3)
{
	time_t t;
	struct tm *tt;
	char d[24];
	char *tmp = NULL;
	int len = 0;

	// Get String Length
	if (str1 != NULL) {
		len += strlen(str1);
		if (str2 != NULL) {
			len += strlen(str2);
			if (str3 != NULL) {
				len += strlen(str3);
			}
		}
	}
	if (len == 0) {
		return;
	}
	// Get Current Date/Time
	if ((t = time(NULL)) == -1) {
		return;
	}
	if ((tt = localtime(&t)) == NULL) {
		return;
	}
	memset(d, 0, 24);
	if (addLine == 1) {
		sprintf(d, "\r\n[%d-%02d-%02d %02d:%02d:%02d]", (tt->tm_year + 1900), (tt->tm_mon + 1), tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
	} else {
		sprintf(d, "[%d-%02d-%02d %02d:%02d:%02d]", (tt->tm_year + 1900), (tt->tm_mon + 1), tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
	}
	// Get Log Message
	if ((tmp = (char *)calloc((len + 32), sizeof(char))) == NULL) {
		return;
	}
	sprintf(tmp, "%s %s", d, str1);
	if (str2 != NULL) {
		strcat(tmp, str2);
		if (str3 != NULL) {
			strcat(tmp, str3);
		}
	}
	// Watch Process Scan
	if (WIN.hProcessScan != NULL) {
		strcat(tmp, "\r\n");
		addTextToEdit(WIN.hProcessScan, IDEDITPROCESSSCANLOG, tmp);
	}
	// Free
	free(tmp);
	return;
}

/******************************************************************************/

void copyProcessToClipboard()
{
	HGLOBAL hMem;
	int len = GetWindowTextLength(GetDlgItem(WIN.hProcessScan, IDEDITPROCESSSCANLOG));
	char txt[len + 2];

	// Get Text
	memset(txt, 0, (len + 2));
	GetDlgItemText(WIN.hProcessScan, IDEDITPROCESSSCANLOG, txt, len);
	// Copy To ClipBoard
	if ((hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1))) == NULL) {
		MessageBox(WIN.hProcessScan, "Can't Copy To Clipboard", "Error", MB_OK|MB_ICONERROR);
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
