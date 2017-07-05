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

int strIsInt(char *str)
{
	int i;
	int len = strlen(str);

	if (len == 0 || len > 9) {
		return RET_E;
	}
	for(i = 0; i < len; i++) {
		if (str[i] < '0' || str[i] > '9') {
			return RET_E;
		}
	}
	return atoi(str);
}

/******************************************************************************/

int strIsPort(char *str)
{
	int i;
	int len = strlen(str);
	int val;

	if (len == 0 || len > 5) {
		return RET_E;
	}
	for(i = 0; i < len; i++) {
		if (str[i] < '0' || str[i] > '9') {
			return RET_E;
		}
	}
	val = atoi(str);
	if (val < 1 || val > 65535) {
		return RET_E;
	}
	return val;
}

/******************************************************************************/

short int strIsIPv4(char *str)
{
	struct in_addr addr;

	if (strcmp(str, "255.255.255.255") != 0) {
		if (inet_addr(str) == INADDR_NONE) {
			return RET_E;
		}
	}
	return RET_O;
}

/******************************************************************************/

short int fileExists(char *path)
{
	FILE *fp;

    if (strlen(path) == 0) {
        return RET_E;
    }
	if ((fp = (FILE *)fopen64(path, "r")) == NULL) {
		return RET_E;
	}
	fclose(fp);
	return RET_O;
}

/******************************************************************************/

int readFileLine(FILE *fp, char *buf, int bufLen)
{
	int c;
	int len = 0;

	if (fp == NULL || bufLen <= 0) {
		return 0;
	}
	while(!feof(fp)) {
		c = fgetc(fp);
		if (c == EOF || c == '\n' || c == '\r') {
			if (c == '\r') { // Windows Case
				c = fgetc(fp);
			}
			break;
		}
		buf[len] = (char)c;
		len++;
		if (len == bufLen) {
			break;
		}
	}
	return len;
}

/******************************************************************************/

short int folderExists(char *path)
{
	DIR *dp;

    if (strlen(path) == 0) {
        return RET_E;
    }
	if ((dp = opendir(path)) == NULL) {
		return RET_E;
	}
	closedir(dp);
	return RET_O;
}

/******************************************************************************/

short int createAFolder(char *path)
{
	if (folderExists(path) == RET_O) {
		return RET_O;
	}
	if (mkdir(path) == 0) {
		return RET_O;
	}
	return RET_E;
}

/******************************************************************************/

short int pathIsFile(char *path)
{
    if (folderExists(path) == RET_O) {
        return 0;
    }
    if (fileExists(path) == RET_O) {
        return 1;
    }
    return RET_E;
}

/******************************************************************************/

void selectAPath(HWND hDlg, int editID, char *filters)
{
    OPENFILENAME ofn;
    char buffer[MAXPATH];

    // Set Params
    memset(buffer, 0, MAXPATH);
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = (MAXPATH - 4);
    ofn.lpstrFilter = filters;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_LONGNAMES;
    // Show Dialog
    if (GetOpenFileName(&ofn)) {
		if (strlen(buffer) > 1) {
            // Check File
            if (editID != IDEDITLIVELOG && editID != IDEDITCLAMDSCANLOG && fileExists(buffer) == RET_E) {
                MessageBox(hDlg, "File Not Found (Or Can't Open)", "Error", MB_OK|MB_ICONERROR);
                return;
            }
            // Set File
            setTextToEdit(hDlg, editID, buffer);
        	// Set Param
            switch(editID) {
                // clamdPath
                case IDEDITCLAMDEXE:
                    memset(PARAMS.clamdPath, 0, MAXPATH);
                    strcpy(PARAMS.clamdPath, buffer);
                    break;
                // clamdConfPath
                case IDEDITCLAMDCONF:
                    memset(PARAMS.clamdConfPath, 0, MAXPATH);
                    strcpy(PARAMS.clamdConfPath, buffer);
                    break;
                // liveLogPath
                case IDEDITLIVELOG:
                    memset(PARAMS.liveLogPath, 0, MAXPATH);
                    strcpy(PARAMS.liveLogPath, buffer);
                    break;
                // clamdscanPath
                case IDEDITCLAMDSCANEXE:
                    memset(PARAMS.clamdscanPath, 0, MAXPATH);
                    strcpy(PARAMS.clamdscanPath, buffer);
                    break;
                // clamdscanLogPath
                case IDEDITCLAMDSCANLOG:
                    memset(PARAMS.clamdscanLogPath, 0, MAXPATH);
                    strcpy(PARAMS.clamdscanLogPath, buffer);
                    break;
                // freshclamPath
                case IDEDITFRESHCLAMEXE:
                    memset(PARAMS.freshclamPath, 0, MAXPATH);
                    strcpy(PARAMS.freshclamPath, buffer);
                    break;
                // freshclamConfPath
                case IDEDITFRESHCLAMCONF:
                    memset(PARAMS.freshclamConfPath, 0, MAXPATH);
                    strcpy(PARAMS.freshclamConfPath, buffer);
                    break;
            }
            // Save Params
            saveParams();
		}
	}
    return;
}

/******************************************************************************/

short int selectAFile(HWND hDlg, char *outFile)
{
    OPENFILENAME ofn;
    char buffer[MAXPATH];
    int len;

    // Set Params
    memset(buffer, 0, MAXPATH);
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = (MAXPATH - 2);
    ofn.lpstrFilter = "All Files (*)\0*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_LONGNAMES;
    // Show Dialog
    if (GetOpenFileName(&ofn)) {
		if (strlen(buffer) > 1) {
            // Check File
            if (fileExists(buffer) == RET_O) {
        	   strcpy(outFile, buffer);
        	   return RET_O;
            }
            MessageBox(hDlg, "File Not Found (Or Can't Open)", "Error", MB_OK|MB_ICONERROR);
		}
	}
    return RET_E;
}

/******************************************************************************/

short int selectAFolder(HWND hDlg, char *outFolder)
{
    BROWSEINFO bi;
    LPITEMIDLIST pidl;
    char buffer[MAXPATH];

    // Set Params
    memset(buffer, 0, MAXPATH);
    ZeroMemory(&bi, sizeof(bi));
    bi.hwndOwner = hDlg;
	bi.pidlRoot = NULL;
    bi.pszDisplayName = buffer;
    //bi.lpszTitle = (LPCTSTR)"Select A Folder...";
    bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
    bi.lpfn = NULL;
    bi.lParam = 0;
    // Select Folder
    if (pidl = SHBrowseForFolder(&bi)) {
        // Get Folder
        if (SHGetPathFromIDList(pidl, buffer) && strlen(buffer) > 1) {
			if (buffer[strlen(buffer) - 1] != '\\') {
			    strcat(buffer, "\\");
			}
			if (folderExists(buffer) == RET_O) {
                strcpy(outFolder, buffer);
                return RET_O;
            }
            MessageBox(hDlg, "Folder Not Found (Or Can't Open)", "Error", MB_OK|MB_ICONERROR);
		}
	}
	return RET_E;
}

/******************************************************************************/

void convertToChar(WCHAR *in, char *out)
{
    while(*out++ = (char)*in++);
    return;
}

/******************************************************************************/

short int checkFileIsFree(char *path)
{
    HANDLE h = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
    
    if (h == INVALID_HANDLE_VALUE) {
        // Exists But Already Open
        if (GetLastError() == ERROR_SHARING_VIOLATION) {
            return 1;
        }
        // Error
        return RET_E;
    }
    CloseHandle(h);
    // OK
    return RET_O;
}

/******************************************************************************/

void addTextToEdit(HWND hWin, int idEdit, char *txt)
{
	HWND hEdit = GetDlgItem(hWin, idEdit);
	int ndx = GetWindowTextLength(hEdit);

    // TODO Check Overflow Size
	SetFocus(hEdit);
	#ifdef WIN32
	SendMessage(hEdit, EM_SETSEL, (WPARAM)ndx, (LPARAM)ndx);
	#else
	SendMessage(hEdit, EM_SETSEL, 0, MAKELONG(ndx, ndx));
	#endif
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)((LPSTR)txt));
    return;
}

/******************************************************************************/

void setFocusToWindow(HWND hDlg)
{
    FLASHWINFO pfwi;
    
    // On Top
    SetWindowPos(hDlg, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOSIZE|SWP_NOMOVE);
    // Show/Focus
    ShowWindow(hDlg, SW_RESTORE);
    BringWindowToTop(hDlg);
    SetFocus(hDlg);
    // Flash
    memset(&pfwi, 0, sizeof(FLASHWINFO));
    pfwi.cbSize = sizeof(FLASHWINFO);
    pfwi.hwnd = hDlg;
    pfwi.dwFlags = FLASHW_TRAY;
    pfwi.uCount = 4;
    pfwi.dwTimeout = 0;
    FlashWindowEx(&pfwi);
	return;
}

/******************************************************************************/

void setTextToEdit(HWND hDlg, int editID, char *txt)
{
    int len = strlen(txt);
    
    SetDlgItemText(hDlg, editID, txt);
    if (len > 0) {
        SendMessage(GetDlgItem(hDlg, editID), EM_SETSEL, (WPARAM)len, (LPARAM)len);
    }
    return;
}

/******************************************************************************/

short int getFolderFromFilePath(char *filePath, char *folderPath)
{
    int i;
    
    i = strlen(filePath) - 1;
    while(i > 1 && filePath[i] != '\\') {
        i--;
    }
    if (i <= 1) {
        return RET_E;
    }
    memset(folderPath, 0, MAXPATH);
    strcpy(folderPath, filePath);
    folderPath[i + 1] = '\0';
    return RET_O;
}

/******************************************************************************/

int getRandomNumber(int min, int max)
{
    srand(time(NULL) + rand());
	return (rand() % (max - min + 1) + min);
}
