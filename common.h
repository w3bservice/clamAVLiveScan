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

// INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <dirent.h>
#include <time.h>
#include <winsock2.h>
#include <windows.h>
#include <shlobj.h>
#include <tlhelp32.h>
#include <psapi.h>

// GUI
// EDITS
#define IDEDITCLAMDEXE 1000
#define IDEDITCLAMDCONF 1001
#define IDEDITCLAMDIP 1002
#define IDEDITCLAMDPORT 1003
#define IDEDITCLAMDSTATUS 1004
#define IDEDITLIVESTATUS 1005
#define IDEDITLIVELOG 1006
#define IDEDITQUARANTINE 1007
//--- ALERT
#define IDEDITALERTMSG 1008
//--- LIVE LOGS
#define IDEDITLIVELOGSMSG 1009
//--- CLAMDSCAN
#define IDEDITCLAMDSCANEXE 1010
#define IDEDITCLAMDSCANLOG 1011
#define IDEDITCLAMDSCANPATH 1012
//--- PROCESSSCAN
#define IDEDITPROCESSSCANLOG 1013
//--- FRESHCLAM
#define IDEDITFRESHCLAMEXE 1014
#define IDEDITFRESHCLAMCONF 1015

// BUTTONS
#define IDBTNSELECTCLAMDEXE 2000
#define IDBTNSELECTCLAMDCONF 2001
#define IDBTNCLAMDSTART 2002
#define IDBTNLIVESTART 2003
#define IDBTNLIVEINCADD 2004
#define IDBTNLIVEINCDEL 2005
#define IDBTNLIVEEXCADD 2006
#define IDBTNLIVEEXCDEL 2007
#define IDBTNSELECTLIVELOG 2008
#define IDBTNSELECTQUARANTINE 2009
#define IDBTNACTIONSLOGS 2010
#define IDBTNACTIONSSCAN 2011
#define IDBTNACTIONPROCESSSCAN 2012
#define IDBTNACTIONSUPDATE 2013
#define IDBTNEXIT 2014
//--- ALERT
#define IDBTNALERTCOPY 2015
#define IDBTNALERTCLEAR 2016
#define IDBTNALERTCLOSE 2017
//--- LIVE LOGS
#define IDBTNLIVELOGSCOPY 2018
#define IDBTNLIVELOGSCLEAR 2019
#define IDBTNLIVELOGSCLOSE 2020
//--- CLAMDSCAN
#define IDBTNSELECTCLAMDSCANEXE 2021
#define IDBTNSELECTCLAMDSCANLOG 2022
#define IDBTNSELECTCLAMDSCANPATH 2023
#define IDBTNCLAMDSCANSTART 2024
#define IDBTNCLAMDSCANCLOSE 2025
//--- PROCESSSCAN
#define IDBTNPROCESSSCANSTART 2026
#define IDBTNPROCESSSCANCOPY 2027
#define IDBTNPROCESSSCANCLOSE 2028
//--- FRESHCLAM
#define IDBTNSELECTFRESHCLAMEXE 2029
#define IDBTNSELECTFRESHCLAMCONF 2030
#define IDBTNFRESHCLAMSTART 2031
#define IDBTNFRESHCLAMCLOSE 2032

// CHECKBOXES
#define IDCHECKCLAMDAUTO 3000
#define IDCHECKLIVEAUTO 3001
#define IDCHECKLIVELOG 3002
#define IDCHECKQUARANTINE 3003
#define IDCHECKAUTOSTART 3004
#define IDCHECKMINIMIZE 3005
#define IDCHECKCLAMDSCANLOG 3006

// LISTS
#define IDLISTLIVEINCLUDE 4000
#define IDLISTLIVEEXCLUDE 4001

// MENUS
#define IDTRAYMENUSHOW 7000
#define IDTRAYMENULOGS 7001
#define IDTRAYMENUSCAN 7002
#define IDTRAYMENUSCANPROC 7003
#define IDTRAYMENUUPDATE 7004
#define IDTRAYMENUEXIT 7005

// ICONS
#define IDICONMAIN 8000
#define IDICONMAINSMALL 8001

// TRAY
#define WM_TRAYICON 9000

// DEFINES
#define RET_O 0
#define RET_E -1

#define SCANEXTENSIONS "exe;dll;ocx;src;sys;msi;com;zip;rar;7z;cab;jar;pdf;xls;xlsx;doc;docx;ppt;pptx;swf"

#define MAXPATH 8192
#define MAXIP 16
#define MAXBUFFER 1408
#define MAXCOUNTSCANLIST 5

#define DEFAULTQUARANTINEFOLDER "Quarantine"
#define DEFAULTCONFIGNAME "ClamAVLiveScan.cfg"
#define DEFAULTINCLUDENAME "ClamAVLiveScanInclude.cfg"
#define DEFAULTEXCLUDENAME "ClamAVLiveScanExclude.cfg"
#define DEFAULTCLAMDIP "127.0.0.1"
#define DEFAULTCLAMDPORT "3310"
#define DEFAULTAUTOSTARTCLAMD 0
#define DEFAULTAUTOSTARTLIVE 0
#define DEFAULTLOGLIVE 0
#define DEFAULTLOGNAME "ClamAVLiveScan.log"
#define DEFAULTAUTOSTARTWIN 0
#define DEFAULTMINIMIZE 0
#define DEFAULTQUARANTINE 0

// STRUCTS
struct sWIN
{
    HINSTANCE hInst;
    HWND hMain;
    HWND hAlert;
    HWND hLivelogs;
    HWND hClamdscan;
    HWND hProcessScan;
    HWND hFreshclam;
};

struct sIMGS
{
    HICON iconMain;
    HICON iconMainSmall;
};

struct sSYSTRAY
{
    HWND hTaskBar;
    HMENU hPopMenu;
    NOTIFYICONDATA trayIcon;
};

struct sEXTENSIONS
{
    char *ext;
    struct sEXTENSIONS *next;
};

struct sINCLUDEPATHS
{
    short int isRunning;
    HANDLE hThread;
    HANDLE hReadDir;
    char *path;
    struct sINCLUDEPATHS *next;
    struct sINCLUDEPATHS *prev;
};

struct sEXCLUDEPATHS
{
    short int isFile;
    char *path;
    struct sEXCLUDEPATHS *next;
    struct sEXCLUDEPATHS *prev;
};

struct sPARAMS
{
    short int startAuto;
    char appPath[MAXPATH]; // Full AppPath File
    char appFolder[MAXPATH]; // Full AppPath Folder
    char configPath[MAXPATH];
    char configIncludePath[MAXPATH];
    char configExcludePath[MAXPATH];
    char clamdPath[MAXPATH];
    char clamdConfPath[MAXPATH];
    char clamdIP[MAXIP];
    char quarantinePath[MAXPATH];
    int clamdPort;
    short int clamdAutoStart;
    short int quarantineEnable;
    short int liveAutoStart;
    short int liveLogEnable;
    char liveLogPath[MAXPATH];
    short int autoStartWindows;
    short int minimizeSystray;
    // Clamdscan
    char clamdscanPath[MAXPATH];
    short int clamdscanLogEnable;
    char clamdscanLogPath[MAXPATH];
    // Freshclam
    char freshclamPath[MAXPATH];
    char freshclamConfPath[MAXPATH];
    // Lists
    struct sEXTENSIONS *extHead;
    struct sEXTENSIONS *extLast;
    struct sINCLUDEPATHS *includeHead;
    struct sINCLUDEPATHS *includeLast;
    struct sEXCLUDEPATHS *excludeHead;
    struct sEXCLUDEPATHS *excludeLast;
    HANDLE hMutex;
};

struct sTEMPSCANLIST
{
    char *path;
    struct sTEMPSCANLIST *next;
};

struct sSCANLIST
{
    char *path;
    short int countScan;
    struct sSCANLIST *next;
};

struct sCLAMD
{
    short int isRunning;
    HANDLE hThread;
    SHELLEXECUTEINFO hProc;
    DWORD pid;
};

struct sLIVE
{
    short int isRunning;
    short int isRunningNet;
    SOCKET sock;
    HANDLE hThread;
    HANDLE hThreadNet;
    HANDLE hMutex;
    HANDLE hMutexTemp;
    struct sTEMPSCANLIST *tempScanHead;
    struct sTEMPSCANLIST *tempScanLast;
    struct sSCANLIST *scanHead;
    struct sSCANLIST *scanLast;
};

struct sTEMPPROCESSSCANLIST
{
    char *path;
    struct sTEMPPROCESSSCANLIST *next;
};

struct sPROCESSSCAN
{
    short int isRunning;
    short int isRunningNet;
    int countInfected;
    HANDLE hThread;
    HANDLE hThreadNet;
    SOCKET sock;
    struct sTEMPPROCESSSCANLIST *scanProcHead;
    struct sTEMPPROCESSSCANLIST *scanProcLast;
};

// GLOBALS
WSADATA MYWSADATA;
struct sWIN WIN;
struct sIMGS IMGS;
struct sSYSTRAY SYSTRAY;
struct sPARAMS PARAMS;
struct sCLAMD CLAMD;
struct sLIVE LIVE;
struct sPROCESSSCAN PROCESSSCAN;

/*** main.c ***/
DWORD WINAPI autoHideMain(LPVOID arg);
BOOL APIENTRY dlgProcMain(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*** vars.c ***/
short int loadVars();
void unloadVars();

/*** run.c ***/
short int checkAlreadyRun();

/*** misc.c ***/
int strIsInt(char *str);
int strIsPort(char *str);
short int strIsIPv4(char *str);
short int fileExists(char *path);
int readFileLine(FILE *fp, char *buf, int bufLen);
short int folderExists(char *path);
short int createAFolder(char *path);
short int pathIsFile(char *path);
void selectAPath(HWND hDlg, int editID, char *filters);
short int selectAFile(HWND hDlg, char *outFile);
short int selectAFolder(HWND hDlg, char *outFolder);
void convertToChar(WCHAR *in, char *out);
short int checkFileIsFree(char *path);
void addTextToEdit(HWND hWin, int idEdit, char *txt);
void setFocusToWindow(HWND hDlg);
void setTextToEdit(HWND hDlg, int editID, char *txt);
short int getFolderFromFilePath(char *filePath, char *folderPath);
int getRandomNumber(int min, int max);

/*** extensions.c ***/
void loadAllExtensions();
void addAnExtensions(char *ext);
short int isValidExtension(char *path);
void delAllExtensions();

/*** params.c ***/
void saveParams();
void loadParams();
void saveIncludeParams();
void loadIncludeParams();
void saveExcludeParams();
void loadExcludeParams();
void loadAllGUIParams();

/*** include.c ***/
short int addIncludePath(char *path, short int checkExists);
void delIncludePath(char *path);
void delAllIncludePaths();
short int includePathExists(char *path);
void refreshIncludeList();
DWORD WINAPI startIncludesThread(LPVOID arg);
void stopIncludeThread(struct sINCLUDEPATHS *n);
void stopAllIncludeThreads();

/*** exclude.c ***/
short int addExcludePath(char *path, short int checkExists);
void delExcludePath(char *path);
void delExcludePaths();
void delAllExcludePaths();
short int excludePathExists(char *path);
void refreshExcludeList();

/*** clamd.c ***/
void startStopClamd(short int showMsg);
DWORD WINAPI startClamdThread(LPVOID arg);
void enableDisableClamdGUI(short int enable);
short int startClamdProc();
short int clamdProcRunning();
void stopClamdProc();
void checkAndKillClamd();

/*** live.c ***/
void startStopLive(short int showMsg);
DWORD WINAPI startLiveThread(LPVOID arg);
void enableDisableLiveGUI(short int enable);
void addLiveInclude();
void addLiveExclude();
void delLiveInclude();
void delLiveExclude();

/*** scan.c ***/
void addToTempScanList(char *path);
void delAllTempScanList();
void addTempToScanList();
void addToScanList(char *path, short int countScan);
void delAllScanList();
struct sSCANLIST *getPathToScan();
DWORD WINAPI startLiveClamdScan(LPVOID arg);

/*** log.c ***/
void addToLogFile(char *str1, char *str2, char *str3);

/*** alert.c ***/
BOOL APIENTRY dlgAlertProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI startAlertThread(LPVOID arg);
short int checkForAlert(char *path, char *res);
void copyAlertsToClipboard();

/*** regedit.c ***/
short int checkExitsRegedit();
short int addToRegedit(short int showMsg);
short int delFromRegedit();

/*** tray.c ***/
void initTrayIcon();
void createSystrayMenu();
void minimizeToNotifyTray();
void maximizeFromNotifyTray();
void destroyTrayIcon();
void showSystrayPopMenu();

/*** livelogs.c ***/
BOOL APIENTRY dlgLivelogsProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI startLivelogsThread(LPVOID arg);
void showLivelogs();
DWORD WINAPI loadLastOfLogFile();
void copyLivelogsToClipboard();

/*** clamdscan.c ***/
BOOL APIENTRY dlgClamdscanProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI startClamdscanThread(LPVOID arg);
void showClamdscan();
void selectPathToScan();
void startSingleScan();

/*** processscan.c ***/
BOOL APIENTRY dlgProcessScanProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI startProcessScanThread(LPVOID arg);
void showProcessScan();
void startStopProcessScan();
DWORD WINAPI scanAllProcessThread(LPVOID arg);
short int getTheRealPath(char *path);
short int addToProcessScanList(char *path);
void delAllProcessScanList();
short int processAlreadyExists(char *path);
void startProcessListScan();
DWORD WINAPI startProcessNetScanThread(LPVOID arg);
void addToProcessResult(short int addLine, char *str1, char *str2, char *str3);
void copyProcessToClipboard();

/*** freshclam.c ***/
BOOL APIENTRY dlgFreshclamProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI startFreshclamThread(LPVOID arg);
void showFreshclam();
void startFreshclamUpdate();

/*** quarantine.c ***/
void quarantineFile(char *path);
void quarantineSelectFolder();
