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

short int addIncludePath(char *path, short int checkExists)
{
    struct sINCLUDEPATHS *n = NULL;
    int len = strlen(path);
    
    // Check Folder
    if (checkExists == 1) {
        if (folderExists(path) == RET_E) {
            MessageBox(WIN.hMain, "Folder Not Found (Or Can't Open)", "Error", MB_OK|MB_ICONERROR);
            return RET_E;
        }
    }
    // Create
    if ((n = (struct sINCLUDEPATHS *)malloc(sizeof(struct sINCLUDEPATHS))) == NULL) {
		return RET_E;
	}
    if (path[len - 1] == '\\') {
        if ((n->path = (char *)calloc((len + 1), sizeof(char))) == NULL) {
			free(n);
            return RET_E;
		}
        strcpy(n->path, path);
    } else {
        if ((n->path = (char *)calloc((len + 2), sizeof(char))) == NULL) {
            free(n);
            return RET_E;
		}
        sprintf(n->path, "%s\\", path);
    }
    n->isRunning = 0;
    n->hThread = NULL;
    n->hReadDir = NULL;
    n->next = NULL;
    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Add
    if (PARAMS.includeHead == NULL) {
        n->prev = NULL;
        PARAMS.includeHead = n;
    } else {
        n->prev = PARAMS.includeLast;
        PARAMS.includeLast->next = n;
    }
    PARAMS.includeLast = n;
    // Unlock
    ReleaseMutex(PARAMS.hMutex);
    return RET_O;
}

/******************************************************************************/

void delIncludePath(char *path)
{
    struct sINCLUDEPATHS *n = NULL;
    
    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Search Node
    n = PARAMS.includeHead;
    while(n != NULL) {
        if (strcasecmp(path, n->path) == 0) {
            break;
        }
        n = n->next;
    }
    if (n == NULL) {
        // Unlock
        ReleaseMutex(PARAMS.hMutex);
        return;
    }
    // Stop Thread
    stopIncludeThread(n);
    // Delete Node
    if (PARAMS.includeHead == n) {
		PARAMS.includeHead = n->next;
	}
	if (PARAMS.includeLast == n) {
		PARAMS.includeLast = n->prev;
	}
	if (n->next != NULL) {
		n->next->prev = n->prev;
	}
	if (n->prev != NULL) {
		n->prev->next = n->next;
	}
	free(n->path);
	free(n);
	// Unlock
    ReleaseMutex(PARAMS.hMutex);
    return;
}

/******************************************************************************/

void delAllIncludePaths()
{
    struct sINCLUDEPATHS *n = NULL;
    struct sINCLUDEPATHS *nNext = NULL;
    
    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Delete All
    n = PARAMS.includeHead;
    while(n != NULL) {
        nNext = n->next;
        // Stop Thread
        stopIncludeThread(n);
        free(n->path);
        free(n);
        n = nNext;
    }
    PARAMS.includeHead = NULL;
    PARAMS.includeLast = NULL;
    // Unlock
    ReleaseMutex(PARAMS.hMutex);
    return;
}

/******************************************************************************/

short int includePathExists(char *path)
{
    struct sINCLUDEPATHS *n = NULL;
    short int ret = RET_E;
    
    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Search
    n = PARAMS.includeHead;
    while(n != NULL) {
        if (strcasecmp(path, n->path) == 0) {
            break;
        }
        n = n->next;
    }
    if (n != NULL) {
        ret = RET_O;
    }
    // Unlock
    ReleaseMutex(PARAMS.hMutex);
    return ret;
}

/******************************************************************************/

void refreshIncludeList()
{
    struct sINCLUDEPATHS *n = NULL;
    
    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Clean-Up
    SendMessage(GetDlgItem(WIN.hMain, IDLISTLIVEINCLUDE), LB_RESETCONTENT, 0, 0);
    // Add All
    n = PARAMS.includeHead;
    while(n != NULL) {
        SendDlgItemMessage(WIN.hMain, IDLISTLIVEINCLUDE, LB_ADDSTRING, 0, (LPARAM)n->path);
        n = n->next;
    }
    // Unlock
    ReleaseMutex(PARAMS.hMutex);
    return;
}

/******************************************************************************/

DWORD WINAPI startIncludesThread(LPVOID arg)
{
    struct sINCLUDEPATHS *n = (struct sINCLUDEPATHS *)arg;
    DWORD outLen;
    char fName[MAXPATH];
    char path[MAXPATH];
    FILE_NOTIFY_INFORMATION out[MAXPATH];
    
    // Check Folder Exists
    if (folderExists(n->path) == RET_E) {
        goto stopIncThread;
    }
    // Start ReadDirectory Events
    if ((n->hReadDir = CreateFile(n->path, FILE_LIST_DIRECTORY | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL)) == INVALID_HANDLE_VALUE) {
        goto stopIncThread;
    }
    addToLogFile("[+] Live Scan Started For: ", n->path, NULL);
    // Start Loop
    while(n->isRunning == 1) {
        if (ReadDirectoryChangesW(n->hReadDir, &out, sizeof(out), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_ATTRIBUTES, &outLen, NULL, NULL)) {
            // Check Action & FileName Length
            if (out[0].Action != 0) {
                memset(fName, 0, MAXPATH);
                memset(path, 0, MAXPATH);
                convertToChar(out[0].FileName, fName);
                sprintf(path, "%s%s", n->path, fName);
                addToTempScanList(path);
                // Clean-Up
                memset(out[0].FileName, 0, out[0].FileNameLength);
                out[0].Action = 0;
            }
        }
    }
stopIncThread:
    // Close ReadDirectory Handle
    if (n->hReadDir != NULL) {
        CloseHandle(n->hReadDir);
        n->hReadDir = NULL;
    }
    // Close Thread Handle
    if (n->hThread != NULL) {
        CloseHandle(n->hThread);
        n->hThread = NULL;
    }
    n->isRunning = 0;
    return 0;
}

/******************************************************************************/

void stopIncludeThread(struct sINCLUDEPATHS *n)
{
    // Close Thread Handle
    if (n->hThread != NULL) {
        TerminateThread(n->hThread, 0);
        CloseHandle(n->hThread);
        n->hThread = NULL;
    }
    // Close ReadDirectory Handle
    if (n->hReadDir != NULL) {
        CloseHandle(n->hReadDir);
        n->hReadDir = NULL;
    }
    n->isRunning = 0;
    return;
}

/******************************************************************************/

void stopAllIncludeThreads()
{
    struct sINCLUDEPATHS *n = NULL;
    
    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Stop All Threads
    n = PARAMS.includeHead;
    while(n != NULL) {
        stopIncludeThread(n);
        n = n->next;
    }
    // Unlock
    ReleaseMutex(PARAMS.hMutex);
    return;
}
