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

short int checkExitsRegedit()
{
	HKEY hKey;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
		return RET_E;
	}
	if (RegQueryValueEx(hKey, (LPCTSTR)"ClamAVLiveScan", NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return RET_E;
	}
	RegCloseKey(hKey);
	return RET_O;
}

/******************************************************************************/

short int addToRegedit(short int showMsg)
{
	HKEY hKey;
	char path[10240];

	if (checkExitsRegedit() == RET_O) {
		return RET_O;
	}
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\", 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
		if (showMsg == 1) {
			MessageBox(WIN.hMain, "Can't Open Regedit Key", "Error", MB_OK|MB_ICONERROR);
		}
		return RET_E;
	}
	memset(path, 0, 10240);
	sprintf(path, "\"%s\" --auto", PARAMS.appPath);
	if (RegSetValueEx(hKey, (LPCTSTR)"ClamAVLiveScan", 0, REG_SZ, (const BYTE *)path, (DWORD)strlen(path)) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		if (showMsg == 1) {
			MessageBox(WIN.hMain, "Can't Write Value To Regedit", "Error", MB_OK|MB_ICONERROR);
		}
		return RET_E;
	}
	RegCloseKey(hKey);
	return RET_O;
}

/******************************************************************************/

short int delFromRegedit()
{
	HKEY hKey;

	if (checkExitsRegedit() == RET_E) {
		return RET_O;
	}
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, (LPCTSTR)"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run\\", 0, KEY_WRITE, &hKey) != ERROR_SUCCESS) {
		MessageBox(WIN.hMain, "Can't Open Regedit Key", "Error", MB_OK|MB_ICONERROR);
		return RET_E;
	}
	if (RegDeleteValue(hKey, (LPCTSTR)"ClamAVLiveScan") != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		MessageBox(WIN.hMain, "Can't Delete Value On Regedit", "Error", MB_OK|MB_ICONERROR);
		return RET_E;
	}
	RegCloseKey(hKey);
	return RET_O;
}
