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

short int checkAlreadyRun()
{
    HANDLE hProcessSnap = NULL;
    HANDLE hProc = NULL;
    PROCESSENTRY32 pe32;
    char path[MAXPATH];
    short int ret = RET_E;
    DWORD pid = GetCurrentProcessId();

    if ((hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)) == INVALID_HANDLE_VALUE) {
        return RET_E;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (!Process32First(hProcessSnap, &pe32)) {
        goto stopCheckRun;
    }
    do {
        if (pe32.th32ProcessID != pid && strstr(PARAMS.appPath, (char *)pe32.szExeFile) != NULL) {
            if ((hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID)) != NULL) {
                memset(path, 0, MAXPATH);
                if (GetModuleFileNameEx(hProc, NULL, path, MAXPATH) != 0) {
                    if (strcasecmp(PARAMS.appPath, path) == 0) {
                        ret = RET_O;
                        goto stopCheckRun;
                    }
                }
                CloseHandle(hProc);
            }
        }
    } while(Process32Next(hProcessSnap, &pe32));
stopCheckRun:
    if (hProc != NULL) {
        CloseHandle(hProc);
    }
    if (hProcessSnap != NULL) {
        CloseHandle(hProcessSnap);
    }
    return ret;
}
