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

void addToTempScanList(char *path)
{
    struct sTEMPSCANLIST *n = NULL;
    
    // Lock
    WaitForSingleObject(LIVE.hMutexTemp, INFINITE);
    // Add To Scan List
    if ((n = (struct sTEMPSCANLIST *)malloc(sizeof(struct sTEMPSCANLIST))) == NULL) {
    	ReleaseMutex(LIVE.hMutexTemp);
		return;
	}
    if ((n->path = (char *)calloc(strlen(path) + 1, sizeof(char))) == NULL) {
    	free(n);
    	ReleaseMutex(LIVE.hMutexTemp);
		return;
	}
    strcpy(n->path, path);
    n->next = NULL;
    if (LIVE.tempScanHead == NULL) {
        LIVE.tempScanHead = n;
    } else {
        LIVE.tempScanLast->next = n;
    }
    LIVE.tempScanLast = n;
    // Unlock
    ReleaseMutex(LIVE.hMutexTemp);
    return;
}

/******************************************************************************/

void delAllTempScanList()
{
    struct sTEMPSCANLIST *n = NULL;
    struct sTEMPSCANLIST *nNext = NULL;

    // Lock
    WaitForSingleObject(LIVE.hMutexTemp, INFINITE);
    // Delete All
    n = LIVE.tempScanHead;
    while(n != NULL) {
        nNext = n->next;
        free(n->path);
        free(n);
        n = nNext;
    }
    LIVE.tempScanHead = NULL;
    LIVE.tempScanLast = NULL;
    // Unlock
    ReleaseMutex(LIVE.hMutexTemp);
    return;
}

/******************************************************************************/

void addTempToScanList()
{
    struct sTEMPSCANLIST *n = NULL;
    
    while(LIVE.tempScanHead != NULL) {
        // Lock
        WaitForSingleObject(LIVE.hMutexTemp, INFINITE);
        // Get Node
        n = LIVE.tempScanHead;
        LIVE.tempScanHead = n->next;
        // Unlock
        ReleaseMutex(LIVE.hMutexTemp);
        // Check Extension
        if (isValidExtension(n->path) == RET_O) {
            // Add To Scan List
            if (fileExists(n->path) == RET_O && checkFileIsFree(n->path) != RET_E) {
                addToScanList(n->path, 0);
            }
        }
        // Clean-up
        free(n->path);
        free(n);
    }
    return;
}

/******************************************************************************/

void addToScanList(char *path, short int countScan)
{
    struct sSCANLIST *n = NULL;
    struct sEXCLUDEPATHS *nEx = NULL;

    // Check Count Scan (Check Is Free)
    if (countScan >= MAXCOUNTSCANLIST) {
        addToLogFile("[-] Max Tries Scan For File: ", path, NULL);
        return;
    }
    // Check For Exclude Path
    // Lock
    WaitForSingleObject(PARAMS.hMutex, INFINITE);
    nEx = PARAMS.excludeHead;
    while(nEx != NULL) {
        if (strncasecmp(path, nEx->path, strlen(nEx->path)) == 0) {
            // Unlock
            ReleaseMutex(PARAMS.hMutex);
            return;
        }
        nEx = nEx->next;
    }
    // Unlock
    ReleaseMutex(PARAMS.hMutex);
    // Check Already Exists In Scan List
    // Lock
    WaitForSingleObject(LIVE.hMutex, INFINITE);
    n = LIVE.scanHead;
    while(n != NULL) {
        if (strcasecmp(path, n->path) == 0) {
            // Unlock
            ReleaseMutex(LIVE.hMutex);
            return;
        }
        n = n->next;
    }
    // Add To Scan List
    if ((n = (struct sSCANLIST *)malloc(sizeof(struct sSCANLIST))) == NULL) {
        ReleaseMutex(LIVE.hMutex);
		return;
	}
    if ((n->path = (char *)calloc(strlen(path) + 1, sizeof(char))) == NULL) {
		free(n);
		ReleaseMutex(LIVE.hMutex);
		return;
	}
    strcpy(n->path, path);
    n->countScan = countScan;
    n->next = NULL;
    if (LIVE.scanHead == NULL) {
        LIVE.scanHead = n;
    } else {
        LIVE.scanLast->next = n;
    }
    LIVE.scanLast = n;
    // Unlock
    ReleaseMutex(LIVE.hMutex);
    return;
}

/******************************************************************************/

void delAllScanList()
{
    struct sSCANLIST *n = NULL;
    struct sSCANLIST *nNext = NULL;

    // Lock
    WaitForSingleObject(LIVE.hMutex, INFINITE);
    // Delete All
    n = LIVE.scanHead;
    while(n != NULL) {
        nNext = n->next;
        free(n->path);
        free(n);
        n = nNext;
    }
    LIVE.scanHead = NULL;
    LIVE.scanLast = NULL;
    // Unlock
    ReleaseMutex(LIVE.hMutex);
    return;
}

/******************************************************************************/

struct sSCANLIST *getPathToScan()
{
    struct sSCANLIST *n = NULL;
    short int isFree = 0;
    
    // Lock
    WaitForSingleObject(LIVE.hMutex, INFINITE);
    // Get First Node
    if ((n = LIVE.scanHead) == NULL) {
        // Unlock
        ReleaseMutex(LIVE.hMutex);
        return NULL;
    }
    // Head Point To Next Node
    LIVE.scanHead = n->next;
    if (LIVE.scanLast == n) {
        LIVE.scanLast = NULL;
    }
    // Unlock
    ReleaseMutex(LIVE.hMutex);
    // Check File Exists
    if (fileExists(n->path) == RET_E) {
        free(n->path);
        free(n);
        return NULL;
    }
    // Check For File Is Free
    isFree = checkFileIsFree(n->path);
    if (isFree == RET_E) {
        // Delete Node
        free(n->path);
        free(n);
        return NULL;
    }
    if (isFree == 1) {
        // Already Open (Add To End Of List)
        addToScanList(n->path, (n->countScan + 1));
        free(n->path);
        free(n);
        return NULL;
    }
    return n;
}

/******************************************************************************/

DWORD WINAPI startLiveClamdScan(LPVOID arg)
{
    struct sSCANLIST *n = NULL;
    int sAddrLen = sizeof(struct sockaddr);
    struct sockaddr_in sIn;
    char cmd[MAXPATH + 16];
    char buf[MAXBUFFER + 1];
    char *res = NULL;
    int count = 0;
    int len = 0;
    short int netError = 0;
    
    // Get Node To Scan
    if ((n = getPathToScan()) == NULL) {
        goto stopLiveNet;
    }
    addToLogFile("[/] Starting Scan For File: ", n->path, NULL);
    // Start Clamd TCP Network Scan
    if ((LIVE.sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        goto stopLiveNet;
    }
    sIn.sin_family = AF_INET;
	sIn.sin_port = htons(PARAMS.clamdPort);
	sIn.sin_addr.s_addr = inet_addr(PARAMS.clamdIP);
	memset(&(sIn.sin_zero), 0, 8);
	if (connect(LIVE.sock, (struct sockaddr *)&sIn, sAddrLen) == SOCKET_ERROR) {
        closesocket(LIVE.sock);
        LIVE.sock = INVALID_SOCKET;
        netError = 1;
        goto stopLiveNet;
    }
    memset(cmd, 0, (MAXPATH + 16));
    sprintf(cmd, "zCONTSCAN %s\0", n->path);
    if (send(LIVE.sock, cmd, strlen(cmd) + 1, 0) == SOCKET_ERROR) {
        closesocket(LIVE.sock);
        LIVE.sock = INVALID_SOCKET;
        netError = 1;
        goto stopLiveNet;
    }
    if ((res = (char *)calloc(1, sizeof(char))) == NULL) {
        goto stopLiveNet;
    }
    do {
        memset(buf, 0, (MAXBUFFER + 1));
        if ((count = recv(LIVE.sock, buf, MAXBUFFER, 0)) == SOCKET_ERROR) {
            closesocket(LIVE.sock);
            LIVE.sock = INVALID_SOCKET;
            goto stopLiveNet;
        }
        if (count > 0) {
            if ((res = (char *)realloc(res, (len + count + 1))) == NULL) {
                goto stopLiveNet;
            }
            memcpy(res + len, buf, count);
            len += count;
            res[len] = '\0';
        }
    } while(count > 0);
    // Close Connection
    closesocket(LIVE.sock);
    LIVE.sock = INVALID_SOCKET;
    // Check Result
    if (strlen(res) > 0) {
        addToLogFile("[+] Scan File Result: ", res, NULL);
        // Check For Alert
        checkForAlert(n->path, res);
    }
stopLiveNet:
    // Clean-Up Node
    if (n != NULL) {
        if (netError == 1) {
            // Add To Scan List
            addToLogFile("[-] (Maybe Clamd Not Running) Network Error For File: ", n->path, NULL);
            addToScanList(n->path, (n->countScan + 1));
        }
        free(n->path);
        free(n);
    }
    // Clean-Up Results
    if (res != NULL) {
        free(res);
    }
    // Stop Socket
    if (LIVE.sock != INVALID_SOCKET) {
        closesocket(LIVE.sock);
        LIVE.sock = INVALID_SOCKET;
    }
    // Stop Thread
    if (LIVE.hThreadNet != NULL) {
        CloseHandle(LIVE.hThreadNet);
        LIVE.hThreadNet = NULL;
    }
    LIVE.isRunningNet = 0;
    return 0;
}
