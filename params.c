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

void saveParams()
{
	FILE *fp = NULL;

	// Lock
	WaitForSingleObject(PARAMS.hMutex, INFINITE);
	// Open
	if ((fp = fopen(PARAMS.configPath, "w")) == NULL) {
		// Unlock
		ReleaseMutex(PARAMS.hMutex);
		MessageBox(WIN.hMain, "Can't Save Parameters To Configuration File", "Warning", MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// Write
	fprintf(fp, "%s\n", PARAMS.clamdPath);
	fprintf(fp, "%s\n", PARAMS.clamdConfPath);
	fprintf(fp, "%s\n", PARAMS.clamdIP);
	fprintf(fp, "%d\n", PARAMS.clamdPort);
	fprintf(fp, "%d\n", PARAMS.clamdAutoStart);
	fprintf(fp, "%s\n", PARAMS.quarantinePath);
	fprintf(fp, "%d\n", PARAMS.quarantineEnable);
	fprintf(fp, "%d\n", PARAMS.liveAutoStart);
	fprintf(fp, "%d\n", PARAMS.liveLogEnable);
	fprintf(fp, "%s\n", PARAMS.liveLogPath);
	fprintf(fp, "%d\n", PARAMS.autoStartWindows);
	fprintf(fp, "%d\n", PARAMS.minimizeSystray);
	//--- clamdscan
	fprintf(fp, "%s\n", PARAMS.clamdscanPath);
	fprintf(fp, "%d\n", PARAMS.clamdscanLogEnable);
	fprintf(fp, "%s\n", PARAMS.clamdscanLogPath);
	//--- freshclam
	fprintf(fp, "%s\n", PARAMS.freshclamPath);
	fprintf(fp, "%s\n", PARAMS.freshclamConfPath);
	// Close
	fclose(fp);
	// Unlock
	ReleaseMutex(PARAMS.hMutex);
	return;
}

/******************************************************************************/

void loadParams()
{
	FILE *fp = NULL;
	char buf[10240];
	int len;
	int i;
	int iVal;
	short int rewrite = 0;

	// Open
	if ((fp = fopen(PARAMS.configPath, "r")) == NULL) {
		// Save Default Parameters
		saveParams();
		return;
	}
	for(i = 0; i < 17; i++) {
		// Check EOF
		if (feof(fp)) {
			rewrite = 1;
			break;
		}
		// Read Line
		memset(buf, 0, 10240);
		len = readFileLine(fp, buf, 10239);
		// Set Parameter Value
		switch(i) {
			// clamdPath
			case 0:
				if (len >= MAXPATH || fileExists(buf) == RET_E) {
					rewrite = 1;
				} else {
					memset(PARAMS.clamdPath, 0, MAXPATH);
					strcpy(PARAMS.clamdPath, buf);
				}
				break;
			// clamdConfPath
			case 1:
				if (len >= MAXPATH || fileExists(buf) == RET_E) {
					rewrite = 1;
				} else {
					memset(PARAMS.clamdConfPath, 0, MAXPATH);
					strcpy(PARAMS.clamdConfPath, buf);
				}
				break;
			// clamdIP
			case 2:
				if (len >= MAXIP || strIsIPv4(buf) == RET_E) {
					rewrite = 1;
				} else {
					memset(PARAMS.clamdIP, 0, MAXIP);
					strcpy(PARAMS.clamdIP, buf);
				}
				break;
			// clamdPort
			case 3:
				if ((iVal = strIsPort(buf)) == RET_E) {
					rewrite = 1;
				} else {
					PARAMS.clamdPort = iVal;
				}
				break;
			// clamdAutoStart
			case 4:
				if ((iVal = strIsInt(buf)) == RET_E || (iVal != 0 && iVal != 1)) {
					rewrite = 1;
				} else {
					PARAMS.clamdAutoStart = iVal;
				}
				break;
			// quarantinePath
			case 5:
				if (len >= (MAXPATH - 1)) {
					rewrite = 1;
				} else {
					memset(PARAMS.quarantinePath, 0, MAXPATH);
					strcpy(PARAMS.quarantinePath, buf);
					if (PARAMS.quarantinePath[strlen(PARAMS.quarantinePath) - 1] != '\\') {
						strcat(PARAMS.quarantinePath, "\\");
						rewrite = 1;
					}
				}
				break;
			// quarantineEnable
			case 6:
				if ((iVal = strIsInt(buf)) == RET_E || (iVal != 0 && iVal != 1)) {
					rewrite = 1;
				} else {
					PARAMS.quarantineEnable = iVal;
				}
				break;
			// liveAutoStart
			case 7:
				if ((iVal = strIsInt(buf)) == RET_E || (iVal != 0 && iVal != 1)) {
					rewrite = 1;
				} else {
					PARAMS.liveAutoStart = iVal;
				}
				break;
			// liveLogEnable
			case 8:
				if ((iVal = strIsInt(buf)) == RET_E || (iVal != 0 && iVal != 1)) {
					rewrite = 1;
				} else {
					PARAMS.liveLogEnable = iVal;
				}
				break;
			// liveLogPath
			case 9:
				if (len >= MAXPATH) {
					rewrite = 1;
				} else {
					memset(PARAMS.liveLogPath, 0, MAXPATH);
					strcpy(PARAMS.liveLogPath, buf);
				}
				break;
			// autoStartWindows
			case 10:
				if ((iVal = strIsInt(buf)) == RET_E || (iVal != 0 && iVal != 1)) {
					rewrite = 1;
				} else {
					PARAMS.autoStartWindows = iVal;
				}
				break;
			// minimizeSystray
			case 11:
				if ((iVal = strIsInt(buf)) == RET_E || (iVal != 0 && iVal != 1)) {
					rewrite = 1;
				} else {
					PARAMS.minimizeSystray = iVal;
				}
				break;
			// clamdscanPath
			case 12:
				if (len >= MAXPATH || fileExists(buf) == RET_E) {
					rewrite = 1;
				} else {
					memset(PARAMS.clamdscanPath, 0, MAXPATH);
					strcpy(PARAMS.clamdscanPath, buf);
				}
				break;
			// clamdscanLogEnable
			case 13:
				if ((iVal = strIsInt(buf)) == RET_E || (iVal != 0 && iVal != 1)) {
					rewrite = 1;
				} else {
					PARAMS.clamdscanLogEnable = iVal;
				}
				break;
			// clamdscanLogPath
			case 14:
				if (len >= MAXPATH) {
					rewrite = 1;
				} else {
					memset(PARAMS.clamdscanLogPath, 0, MAXPATH);
					strcpy(PARAMS.clamdscanLogPath, buf);
				}
				break;
			// freshclamPath
			case 15:
				if (len >= MAXPATH || fileExists(buf) == RET_E) {
					rewrite = 1;
				} else {
					memset(PARAMS.freshclamPath, 0, MAXPATH);
					strcpy(PARAMS.freshclamPath, buf);
				}
				break;
			// freshclamConfPath
			case 16:
				if (len >= MAXPATH || fileExists(buf) == RET_E) {
					rewrite = 1;
				} else {
					memset(PARAMS.freshclamConfPath, 0, MAXPATH);
					strcpy(PARAMS.freshclamConfPath, buf);
				}
				break;
		}
	}
	// Close
	fclose(fp);
	// Create Files/Folders
	if (fileExists(PARAMS.liveLogPath) == RET_E) {
		if ((fp = fopen(PARAMS.liveLogPath, "w")) != NULL) {
			fclose(fp);
		}
	}
	if (createAFolder(PARAMS.quarantinePath) == RET_E) {
		memset(PARAMS.quarantinePath, 0, MAXPATH);
		snprintf(PARAMS.quarantinePath, MAXPATH, "%s%s\\", PARAMS.appFolder, DEFAULTQUARANTINEFOLDER);
		createAFolder(PARAMS.quarantinePath);
		rewrite = 1;
	}
	// Rewrite ?
	if (rewrite == 1) {
		saveParams();
	}
	return;
}

/******************************************************************************/

void saveIncludeParams()
{
	FILE *fp = NULL;
	struct sINCLUDEPATHS *n = NULL;

	// Lock
	WaitForSingleObject(PARAMS.hMutex, INFINITE);
	// Open
	if ((fp = fopen(PARAMS.configIncludePath, "w")) == NULL) {
		// Unlock
		ReleaseMutex(PARAMS.hMutex);
		MessageBox(WIN.hMain, "Can't Save Include Paths To Configuration File", "Warning", MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// Search
	n = PARAMS.includeHead;
	while(n != NULL) {
		// Write
		fprintf(fp, "%s\n", n->path);
		n = n->next;
	}
	// Unlock
	ReleaseMutex(PARAMS.hMutex);
	// Close
	fclose(fp);
	return;
}

/******************************************************************************/

void loadIncludeParams()
{
	FILE *fp = NULL;
	char buf[10240];
	int len;

	// Open
	if ((fp = fopen(PARAMS.configIncludePath, "r")) == NULL) {
		// Save Default Parameters
		saveIncludeParams();
		return;
	}
	while(!feof(fp)) {
		// Read Line
		memset(buf, 0, 10240);
		len = readFileLine(fp, buf, 10239);
		// Add Node
		if (len > 1) {
			addIncludePath(buf, 0);
		}
	}
	// Close
	fclose(fp);
	return;
}

/******************************************************************************/

void saveExcludeParams()
{
	FILE *fp = NULL;
	struct sEXCLUDEPATHS *n = NULL;

	// Lock
	WaitForSingleObject(PARAMS.hMutex, INFINITE);
	// Open
	if ((fp = fopen(PARAMS.configExcludePath, "w")) == NULL) {
		// Unlock
		ReleaseMutex(PARAMS.hMutex);
		MessageBox(WIN.hMain, "Can't Save Exclude Paths To Configuration File", "Warning", MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	// Search
	n = PARAMS.excludeHead;
	while(n != NULL) {
		// Write
		fprintf(fp, "%s\n", n->path);
		n = n->next;
	}
	// Unlock
	ReleaseMutex(PARAMS.hMutex);
	// Close
	fclose(fp);
	return;
}

/******************************************************************************/

void loadExcludeParams()
{
	FILE *fp = NULL;
	char buf[10240];
	int len;

	// Open
	if ((fp = fopen(PARAMS.configExcludePath, "r")) == NULL) {
		// Save Default Parameters
		saveExcludeParams();
		return;
	}
	while(!feof(fp)) {
		// Read Line
		memset(buf, 0, 10240);
		len = readFileLine(fp, buf, 10239);
		// Add Node
		if (len > 1) {
			addExcludePath(buf, 0);
		}
	}
	// Close
	fclose(fp);
	return;
}

/******************************************************************************/

void loadAllGUIParams()
{
	char tmp[8];

	// Params
	// clamdPath
	setTextToEdit(WIN.hMain, IDEDITCLAMDEXE, PARAMS.clamdPath);
	// clamdConfPath
	setTextToEdit(WIN.hMain, IDEDITCLAMDCONF, PARAMS.clamdConfPath);
	// clamdIP
	SetDlgItemText(WIN.hMain, IDEDITCLAMDIP, PARAMS.clamdIP);
	// clamdPort
	memset(tmp, 0, 8);
	sprintf(tmp, "%d", PARAMS.clamdPort);
	SetDlgItemText(WIN.hMain, IDEDITCLAMDPORT, tmp);
	// quarantinePath
	setTextToEdit(WIN.hMain, IDEDITQUARANTINE, PARAMS.quarantinePath);
	// quarantineEnable
	CheckDlgButton(WIN.hMain, IDCHECKQUARANTINE, (PARAMS.quarantineEnable == 1) ? BST_CHECKED : BST_UNCHECKED);
	// clamdAutoStart
	CheckDlgButton(WIN.hMain, IDCHECKCLAMDAUTO, (PARAMS.clamdAutoStart == 1) ? BST_CHECKED : BST_UNCHECKED);
	// liveAutoStart
	CheckDlgButton(WIN.hMain, IDCHECKLIVEAUTO, (PARAMS.liveAutoStart == 1) ? BST_CHECKED : BST_UNCHECKED);
	// liveLogEnable
	CheckDlgButton(WIN.hMain, IDCHECKLIVELOG, (PARAMS.liveLogEnable == 1) ? BST_CHECKED : BST_UNCHECKED);
	// liveLogPath
	setTextToEdit(WIN.hMain, IDEDITLIVELOG, PARAMS.liveLogPath);
	// autoStartWindows
	CheckDlgButton(WIN.hMain, IDCHECKAUTOSTART, (PARAMS.autoStartWindows == 1) ? BST_CHECKED : BST_UNCHECKED);
	// minimizeSystray
	CheckDlgButton(WIN.hMain, IDCHECKMINIMIZE, (PARAMS.minimizeSystray == 1) ? BST_CHECKED : BST_UNCHECKED);
	// Include List
	refreshIncludeList();
	// Exclude List
	refreshExcludeList();
	return;
}
