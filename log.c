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

void addToLogFile(char *str1, char *str2, char *str3)
{
	time_t t;
	struct tm *tt;
	char d[24];
	char *tmp = NULL;
	int len = 0;
	FILE *fp = NULL;

	// Check Parameter
	if (PARAMS.liveLogEnable == 0 && WIN.hLivelogs == NULL) {
		return;
	}
	// Get String Length
	if (str1 != NULL) {
		len += strlen(str1);
		if (str2 != NULL) {
			len += strlen(str2);
			if (str3 != NULL) {
				len += strlen(str3);
			}
		}
	}
	if (len == 0) {
		return;
	}
	// Get Current Date/Time
	if ((t = time(NULL)) == -1) {
		return;
	}
	if ((tt = localtime(&t)) == NULL) {
		return;
	}
	memset(d, 0, 24);
	sprintf(d, "[%d-%02d-%02d %02d:%02d:%02d]", (tt->tm_year + 1900), (tt->tm_mon + 1), tt->tm_mday, tt->tm_hour, tt->tm_min, tt->tm_sec);
	// Get Log Message
	if ((tmp = (char *)calloc((len + 32), sizeof(char))) == NULL) {
		return;
	}
	sprintf(tmp, "%s %s", d, str1);
	if (str2 != NULL) {
		strcat(tmp, str2);
		if (str3 != NULL) {
			strcat(tmp, str3);
		}
	}
	// Lock
	WaitForSingleObject(PARAMS.hMutex, INFINITE);
	// Write To File
	if (PARAMS.liveLogEnable == 1) {
		if ((fp = (FILE *)fopen64(PARAMS.liveLogPath, "a")) != NULL) {
			fprintf(fp, "%s\n", tmp);
			fclose(fp);
		}
	}
	// Watch Live Scan
	if (WIN.hLivelogs != NULL) {
		strcat(tmp, "\r\n");
		addTextToEdit(WIN.hLivelogs, IDEDITLIVELOGSMSG, tmp);
	}
	// Unlock
	ReleaseMutex(PARAMS.hMutex);
	// Free
	free(tmp);
	return;
}
