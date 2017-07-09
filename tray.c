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

void initTrayIcon()
{
	// Create Systray Menu
	createSystrayMenu();
	// Init Systray Icon
	memset(&SYSTRAY.trayIcon, 0, sizeof(NOTIFYICONDATA));
	SYSTRAY.trayIcon.cbSize = sizeof(NOTIFYICONDATA);
	SYSTRAY.trayIcon.hWnd = WIN.hMain;
	SYSTRAY.trayIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	SYSTRAY.trayIcon.uCallbackMessage = WM_TRAYICON;
	SYSTRAY.trayIcon.uID = IDICONMAINSMALL;
	SYSTRAY.trayIcon.hIcon = IMGS.iconMainSmall;
	strcpy(SYSTRAY.trayIcon.szTip, "ClamAV Live Scan");
	return;
}

/******************************************************************************/

void createSystrayMenu()
{
	// Create Menu
	SYSTRAY.hPopMenu = CreatePopupMenu();
	AppendMenu(SYSTRAY.hPopMenu, MF_STRING, IDTRAYMENUSHOW, (LPCTSTR)"Show Program");
	AppendMenu(SYSTRAY.hPopMenu, MF_SEPARATOR, -1, NULL);
	AppendMenu(SYSTRAY.hPopMenu, MF_STRING, IDTRAYMENULOGS, (LPCTSTR)"Show Live Logs");
	AppendMenu(SYSTRAY.hPopMenu, MF_SEPARATOR, -1, NULL);
	AppendMenu(SYSTRAY.hPopMenu, MF_STRING, IDTRAYMENUSCAN, (LPCTSTR)"Scan File/Folder");
	AppendMenu(SYSTRAY.hPopMenu, MF_SEPARATOR, -1, NULL);
	AppendMenu(SYSTRAY.hPopMenu, MF_STRING, IDTRAYMENUSCANPROC, (LPCTSTR)"Scan All Processes");
	AppendMenu(SYSTRAY.hPopMenu, MF_SEPARATOR, -1, NULL);
	AppendMenu(SYSTRAY.hPopMenu, MF_STRING, IDTRAYMENUUPDATE, (LPCTSTR)"Update Databases");
	AppendMenu(SYSTRAY.hPopMenu, MF_SEPARATOR, -1, NULL);
	AppendMenu(SYSTRAY.hPopMenu, MF_STRING, IDTRAYMENUEXIT, (LPCTSTR)"Stop All And Exit");
	return;
}

/******************************************************************************/

void minimizeToNotifyTray()
{
	Shell_NotifyIcon(NIM_ADD, &SYSTRAY.trayIcon);
	if (IsWindowVisible(WIN.hMain)) {
		ShowWindow(WIN.hMain, SW_HIDE);
	}
	return;
}

/******************************************************************************/

void maximizeFromNotifyTray()
{
	Shell_NotifyIcon(NIM_DELETE, &SYSTRAY.trayIcon);
	ShowWindow(WIN.hMain, SW_SHOW);
	ShowWindow(WIN.hMain, SW_RESTORE);
	SetForegroundWindow(WIN.hMain);
	return;
}

/******************************************************************************/

void destroyTrayIcon()
{
	Shell_NotifyIcon(NIM_DELETE, &SYSTRAY.trayIcon);
	return;
}

/******************************************************************************/

void showSystrayPopMenu()
{
	POINT cPosition;

	if (GetCursorPos(&cPosition)) {
		TrackPopupMenuEx(SYSTRAY.hPopMenu, 0, (int)cPosition.x - 170, (int)cPosition.y - 14, WIN.hMain, NULL);
	}
	return;
}
