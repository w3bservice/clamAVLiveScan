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

void quarantineFile(char *filePath)
{
    HANDLE hProcessSnap = NULL;
    HANDLE hProc = NULL;
    PROCESSENTRY32 pe32;
    char path[MAXPATH];
    char fileName[MAXPATH];
    char *tmp = NULL;
    short int err = 0;
    
    // Check/Create Quarantine Folder
    if (createAFolder(PARAMS.quarantinePath) == RET_E) {
        addToLogFile("[-] -- Can't Create Quarantine Folder --", NULL, NULL);
		return;
	}
    // Get The File Name
    if ((tmp = strrchr(filePath, '\\')) == NULL) {
		return;
	}
    memset(fileName, 0, MAXPATH);
    strncpy(fileName, tmp + 1, (MAXPATH - 1));
    // Try To Kill If Running
    if ((hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) != INVALID_HANDLE_VALUE) {
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hProcessSnap, &pe32)) {
            do {
                if ((hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, pe32.th32ProcessID)) != NULL) {
                    memset(path, 0, MAXPATH);
            		if (GetProcessImageFileName(hProc, path, MAXPATH) != 0 && getTheRealPath(path) == RET_O) {
						if (strcasecmp(filePath, path) == 0) {
                            TerminateProcess(hProc, 0);
						}
					}
					CloseHandle(hProc);
            		hProc = NULL;
				}
			} while(Process32Next(hProcessSnap, &pe32));
		}
    }
    // Get Quarantine Path
    do {
	    memset(path, 0, MAXPATH);
		snprintf(path, (MAXPATH - 1), "%s%s_%d_%d", PARAMS.quarantinePath, fileName, (int)time(NULL), getRandomNumber(100000, 999999));
		err++;
	} while(fileExists(path) == RET_O && err != 30);
	if (err == 30) {
		return;
	}
	// Move To Quarantine
	rename(filePath, path);
	return;
}

/******************************************************************************/

void quarantineSelectFolder()
{
	char path[MAXPATH];
	
	memset(path, 0, MAXPATH);
	if (selectAFolder(WIN.hMain, path) == RET_O) {
        memset(PARAMS.quarantinePath, 0, MAXPATH);
        strcpy(PARAMS.quarantinePath, path);
        setTextToEdit(WIN.hMain, IDEDITQUARANTINE, PARAMS.quarantinePath);
        saveParams();
	}
	return;
}
