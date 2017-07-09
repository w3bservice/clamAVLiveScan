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

void loadAllExtensions()
{
	char ext[16];
	int len;
	char *s = (char *)SCANEXTENSIONS;
	char *tmp = NULL;

	while((tmp = strchr(s, ';')) != NULL) {
		len = (tmp - s);
		if (len < 15) {
			memset(ext, 0, 16);
			ext[0] = '.';
			memcpy(ext + 1, s, len);
			addAnExtensions(ext);
		}
		s = tmp + 1;
	}
	if (s != NULL && (len = strlen(s)) < 15) {
		memset(ext, 0, 16);
		ext[0] = '.';
		memcpy(ext + 1, s, len);
		addAnExtensions(ext);
	}
	return;
}

/******************************************************************************/

void addAnExtensions(char *ext)
{
	struct sEXTENSIONS *n = NULL;

	if ((n = (struct sEXTENSIONS *)malloc(sizeof(struct sEXTENSIONS))) == NULL) {
		return;
	}
	if ((n->ext = (char *)calloc((strlen(ext) + 1), sizeof(char))) == NULL) {
		free(n);
		return;
	}
	strcpy(n->ext, ext);
	n->next = NULL;
	if (PARAMS.extHead == NULL) {
		PARAMS.extHead = n;
	} else {
		PARAMS.extLast->next = n;
	}
	PARAMS.extLast = n;
	return;
}

/******************************************************************************/

short int isValidExtension(char *path)
{
	struct sEXTENSIONS *n = NULL;
	char *ext = NULL;
	int len;

	if ((ext = strrchr(path, '.')) == NULL) {
		return RET_E;
	}
	len = strlen(ext);
	if (len == 0 || len > 15) {
		return RET_E;
	}
	n = PARAMS.extHead;
	while(n != NULL) {
		if (strcasecmp(ext, n->ext) == 0) {
			return RET_O;
		}
		n = n->next;
	}
	return RET_E;
}

/******************************************************************************/

void delAllExtensions()
{
	struct sEXTENSIONS *n = NULL;
	struct sEXTENSIONS *nNext = NULL;

	n = PARAMS.extHead;
	while(n != NULL) {
		nNext = n->next;
		free(n->ext);
		free(n);
		n = nNext;
	}
	PARAMS.extHead = NULL;
	PARAMS.extLast = NULL;
	return;
}
