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

short int loadVars()
{
    // Get AppPath
    memset(PARAMS.appPath, 0, MAXPATH);
    if (GetModuleFileName(NULL, PARAMS.appPath, (MAXPATH - 384)) == 0) {
        MessageBox(HWND_DESKTOP, "Can't Get Application Path", "Error", MB_OK|MB_ICONERROR);
        return RET_E;
    }
    // Get AppFolder
    if (getFolderFromFilePath(PARAMS.appPath, PARAMS.appFolder) == RET_E) {
        MessageBox(HWND_DESKTOP, "Can't Get Application Folder", "Error", MB_OK|MB_ICONERROR);
        return RET_E;
    }
    // Check Already Running
    if (checkAlreadyRun() == RET_O) {
        MessageBox(HWND_DESKTOP, "Application Already Running\nPlease Stop Other Process Before", "Warning", MB_OK|MB_ICONEXCLAMATION);
        return RET_E;
    }
    // Init Winsock
    if (WSAStartup(MAKEWORD(2, 2), &MYWSADATA) != 0) {
        MessageBox(HWND_DESKTOP, "Can't Init Winsock v2.2", "Error", MB_OK|MB_ICONERROR);
        return RET_E;
    }
    // Create Mutexs
    if ((PARAMS.hMutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {
        MessageBox(HWND_DESKTOP, "Can't Create Parameters Mutex", "Error", MB_OK|MB_ICONERROR);
        WSACleanup();
        return RET_E;
    }
    if ((LIVE.hMutex = CreateMutex(NULL, FALSE, NULL)) == NULL) {
        MessageBox(HWND_DESKTOP, "Can't Create Live Scan Mutex", "Error", MB_OK|MB_ICONERROR);
        CloseHandle(PARAMS.hMutex);
        WSACleanup();
        return RET_E;
    }
    if ((LIVE.hMutexTemp = CreateMutex(NULL, FALSE, NULL)) == NULL) {
        MessageBox(HWND_DESKTOP, "Can't Create Live Temp Scan Mutex", "Error", MB_OK|MB_ICONERROR);
        CloseHandle(PARAMS.hMutex);
        CloseHandle(LIVE.hMutex);
        WSACleanup();
        return RET_E;
    }
    // Init WIN
    WIN.hMain = NULL;
    WIN.hAlert = NULL;
    WIN.hLivelogs = NULL;
    WIN.hClamdscan = NULL;
    WIN.hProcessScan = NULL;
    WIN.hFreshclam = NULL;
    // Init PARAMS
    PARAMS.startAuto = 0;
    memset(PARAMS.configPath, 0, MAXPATH);
    snprintf(PARAMS.configPath, MAXPATH, "%s%s", PARAMS.appFolder, DEFAULTCONFIGNAME);
    memset(PARAMS.configIncludePath, 0, MAXPATH);
    snprintf(PARAMS.configIncludePath, MAXPATH, "%s%s", PARAMS.appFolder, DEFAULTINCLUDENAME);
    memset(PARAMS.configExcludePath, 0, MAXPATH);
    snprintf(PARAMS.configExcludePath, MAXPATH, "%s%s", PARAMS.appFolder, DEFAULTEXCLUDENAME);
    memset(PARAMS.clamdPath, 0, MAXPATH);
    memset(PARAMS.clamdConfPath, 0, MAXPATH);
    memset(PARAMS.clamdIP, 0, MAXIP);
    snprintf(PARAMS.clamdIP, MAXIP, "%s", DEFAULTCLAMDIP);
    memset(PARAMS.quarantinePath, 0, MAXPATH);
    snprintf(PARAMS.quarantinePath, MAXPATH, "%s%s\\", PARAMS.appFolder, DEFAULTQUARANTINEFOLDER);
    PARAMS.clamdPort = atoi(DEFAULTCLAMDPORT);
    PARAMS.quarantineEnable = DEFAULTQUARANTINE;
    PARAMS.clamdAutoStart = DEFAULTAUTOSTARTCLAMD;
    PARAMS.liveAutoStart = DEFAULTAUTOSTARTLIVE;
    PARAMS.liveLogEnable = DEFAULTLOGLIVE;
    memset(PARAMS.liveLogPath, 0, MAXPATH);
    snprintf(PARAMS.liveLogPath, MAXPATH, "%s%s", PARAMS.appFolder, DEFAULTLOGNAME);
    PARAMS.autoStartWindows = DEFAULTAUTOSTARTWIN;
    PARAMS.minimizeSystray = DEFAULTMINIMIZE;
    //--- clamdscan
    memset(PARAMS.clamdscanPath, 0, MAXPATH);
    PARAMS.clamdscanLogEnable = 0;
    memset(PARAMS.clamdscanLogPath, 0, MAXPATH);
    //--- freshclam
    memset(PARAMS.freshclamPath, 0, MAXPATH);
    memset(PARAMS.freshclamConfPath, 0, MAXPATH);
    //--- lists
    PARAMS.extHead = NULL;
    PARAMS.extLast = NULL;
    PARAMS.includeHead = NULL;
    PARAMS.includeLast = NULL;
    PARAMS.excludeHead = NULL;
    PARAMS.excludeLast = NULL;
    // Init CLAMD
    CLAMD.isRunning = 0;
    CLAMD.hThread = NULL;
    memset(&CLAMD.hProc, 0, sizeof(SHELLEXECUTEINFO));
    CLAMD.hProc.hProcess = NULL;
    CLAMD.pid = 0;
    // Init LIVE
    LIVE.isRunning = 0;
    LIVE.isRunningNet = 0;
    LIVE.sock = INVALID_SOCKET;
    LIVE.hThread = NULL;
    LIVE.hThreadNet = NULL;
    LIVE.scanHead = NULL;
    LIVE.scanLast = NULL;
    LIVE.tempScanHead = NULL;
    LIVE.tempScanLast = NULL;
    // Init PROCESS SCAN
    PROCESSSCAN.isRunning = 0;
    PROCESSSCAN.isRunningNet = 0;
    PROCESSSCAN.countInfected = 0;
    PROCESSSCAN.hThread = NULL;
    PROCESSSCAN.hThreadNet = NULL;
    PROCESSSCAN.sock = INVALID_SOCKET;
    PROCESSSCAN.scanProcHead = NULL;
    PROCESSSCAN.scanProcLast = NULL;
    // Load All Params
    loadParams();
    loadIncludeParams();
    loadExcludeParams();
    // Load All Extensions
    loadAllExtensions();
	return RET_O;
}

/******************************************************************************/

void unloadVars()
{
    // Close Windows
    if (WIN.hAlert != NULL) {
        EndDialog(WIN.hAlert, 0);
    }
    if (WIN.hLivelogs != NULL) {
        EndDialog(WIN.hLivelogs, 0);
    }
    if (WIN.hClamdscan != NULL) {
        EndDialog(WIN.hClamdscan, 0);
    }
    if (WIN.hProcessScan != NULL) {
        EndDialog(WIN.hProcessScan, 0);
    }
    if (WIN.hFreshclam != NULL) {
        EndDialog(WIN.hFreshclam, 0);
    }
    // Stop Clamd Process
    stopClamdProc();
    // Destroy Tray Icon
	destroyTrayIcon();
    // Close Threads
    stopAllIncludeThreads();
    if (CLAMD.hThread != NULL) {
        TerminateThread(CLAMD.hThread, 0);
        CloseHandle(CLAMD.hThread);
        addToLogFile("[/] -- Clamd Stopped --", NULL, NULL);
    }
    if (LIVE.hThreadNet != NULL) {
        TerminateThread(LIVE.hThreadNet, 0);
        CloseHandle(LIVE.hThreadNet);
    }
    if (LIVE.hThread != NULL) {
        TerminateThread(LIVE.hThread, 0);
        CloseHandle(LIVE.hThread);
        addToLogFile("[/] -- Live Scan Stopped --", NULL, NULL);
    }
    if (PROCESSSCAN.hThreadNet != NULL) {
        TerminateThread(PROCESSSCAN.hThreadNet, 0);
        CloseHandle(PROCESSSCAN.hThreadNet);
    }
    if (PROCESSSCAN.hThread != NULL) {
        TerminateThread(PROCESSSCAN.hThread, 0);
        CloseHandle(PROCESSSCAN.hThread);
        addToLogFile("[/] -- Process Scan Stopped --", NULL, NULL);
    }
    // Close Connection
    if (LIVE.sock != INVALID_SOCKET) {
        closesocket(LIVE.sock);
    }
    if (PROCESSSCAN.sock != INVALID_SOCKET) {
        closesocket(PROCESSSCAN.sock);
    }
    // Clean-Up Lists
    delAllIncludePaths();
    delAllExcludePaths();
    delAllTempScanList();
    delAllScanList();
    delAllProcessScanList();
    delAllExtensions();
    // Close Mutexs
    CloseHandle(PARAMS.hMutex);
    CloseHandle(LIVE.hMutex);
    CloseHandle(LIVE.hMutexTemp);
    // Clean-Up Winsock
    WSACleanup();
	return;
}
