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

short int addExcludePath(char *path, short int checkExists)
{
    struct sEXCLUDEPATHS *n = NULL;
    short int isFile = RET_E;
    int len = strlen(path);

    // Check File/Folder
    if (checkExists == 1) {
        if ((isFile = pathIsFile(path)) == RET_E) {
            MessageBox(WIN.hMain, "Can't Determine Path Type (Or Not Found)", "Error", MB_OK|MB_ICONERROR);
            return RET_E;
        }
    }
    // Create
    if ((n = (struct sEXCLUDEPATHS *)malloc(sizeof(struct sEXCLUDEPATHS))) == NULL) {
        return RET_E;
	}
    if (isFile == 0 && path[len - 1] != '\\') {
        if ((n->path = (char *)calloc((len + 2), sizeof(char))) == NULL) {
			free(n);
	        return RET_E;
		}
		sprintf(n->path, "%s\\", path);
    } else {
        if ((n->path = (char *)calloc((len + 1), sizeof(char))) == NULL) {
	        free(n);
	        return RET_E;
		}
		strcpy(n->path, path);
    }
    n->isFile = isFile;
    n->next = NULL;
    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Add
    if (PARAMS.excludeHead == NULL) {
        n->prev = NULL;
        PARAMS.excludeHead = n;
    } else {
        n->prev = PARAMS.excludeLast;
        PARAMS.excludeLast->next = n;
    }
    PARAMS.excludeLast = n;
    // Unlock
    ReleaseMutex(PARAMS.hMutex);
    return RET_O;
}

/******************************************************************************/

void delExcludePath(char *path)
{
    struct sEXCLUDEPATHS *n = NULL;

    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Search Node
    n = PARAMS.excludeHead;
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
    // Delete Node
    if (PARAMS.excludeHead == n) {
		PARAMS.excludeHead = n->next;
	}
	if (PARAMS.excludeLast == n) {
		PARAMS.excludeLast = n->prev;
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

void delAllExcludePaths()
{
    struct sEXCLUDEPATHS *n = NULL;
    struct sEXCLUDEPATHS *nNext = NULL;

    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Delete All
    n = PARAMS.excludeHead;
    while(n != NULL) {
        nNext = n->next;
        free(n->path);
        free(n);
        n = nNext;
    }
    PARAMS.excludeHead = NULL;
    PARAMS.excludeLast = NULL;
    // Unlock
    ReleaseMutex(PARAMS.hMutex);
    return;
}

/******************************************************************************/

short int excludePathExists(char *path)
{
    struct sEXCLUDEPATHS *n = NULL;
    short int ret = RET_E;

    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Search
    n = PARAMS.excludeHead;
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

void refreshExcludeList()
{
    struct sEXCLUDEPATHS *n = NULL;

    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    // Clean-Up
    SendMessage(GetDlgItem(WIN.hMain, IDLISTLIVEEXCLUDE), LB_RESETCONTENT, 0, 0);
    // Add All
    n = PARAMS.excludeHead;
    while(n != NULL) {
        SendDlgItemMessage(WIN.hMain, IDLISTLIVEEXCLUDE, LB_ADDSTRING, 0, (LPARAM)n->path);
        n = n->next;
    }
    // Unlock
    ReleaseMutex(PARAMS.hMutex);
    return;
}
