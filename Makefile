#
# ClamAV Live Scan
# mad.coder@mail.com
# https://github.com/madcoder42/clamAVLiveScan
#
# Copyright 2017 madcoder42
#
# This file is part of ClamAV Live Scan.
#
# ClamAV Live Scan is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ClamAV Live Scan is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with ClamAV Live Scan.  If not, see <http://www.gnu.org/licenses/>.
#

BIN		=	ClamAVLiveScan.exe
CC		=	gcc.exe
WINDRES	=	windres.exe
RES		=	ClamAVLiveScan.res
OBJ		=	alert.o clamd.o clamdscan.o exclude.o extensions.o freshclam.o include.o live.o livelogs.o log.o main.o misc.o params.o processscan.o quarantine.o regedit.o run.o scan.o tray.o vars.o $(RES)
LINKOBJ	=	alert.o clamd.o clamdscan.o exclude.o extensions.o freshclam.o include.o live.o livelogs.o log.o main.o misc.o params.o processscan.o quarantine.o regedit.o run.o scan.o tray.o vars.o $(RES)
#--- WIN32 ---
#LIBS	=	-L"C:/msys64/mingw32/i686-w64-mingw32/lib" -L"C:/msys64/mingw32/lib" -mwindows -lws2_32 -lcomdlg32 -lpsapi
#INCS	=	-I"C:/msys64/mingw32/i686-w64-mingw32/include" -I"C:/msys64/mingw32/include"
#--- WIN64 ---
LIBS	=	-L"C:/msys64/mingw64/x86_64-w64-mingw32/lib" -L"C:/msys64/mingw64/lib" -mwindows -lws2_32 -lcomdlg32 -lpsapi
INCS	=	-I"C:/msys64/mingw64/x86_64-w64-mingw32/include" -I"C:/msys64/mingw64/include"
#-------------
CFLAGS	=	$(INCS) -D_GNU_SOURCE -O2 -fexpensive-optimizations -funroll-loops
RM		=	rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after

clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o $(BIN) $(LIBS)

alert.o: alert.c
	$(CC) -c alert.c -o alert.o $(CFLAGS)

clamd.o: clamd.c
	$(CC) -c clamd.c -o clamd.o $(CFLAGS)

clamdscan.o: clamdscan.c
	$(CC) -c clamdscan.c -o clamdscan.o $(CFLAGS)

exclude.o: exclude.c
	$(CC) -c exclude.c -o exclude.o $(CFLAGS)

extensions.o: extensions.c
	$(CC) -c extensions.c -o extensions.o $(CFLAGS)

freshclam.o: freshclam.c
	$(CC) -c freshclam.c -o freshclam.o $(CFLAGS)

include.o: include.c
	$(CC) -c include.c -o include.o $(CFLAGS)

live.o: live.c
	$(CC) -c live.c -o live.o $(CFLAGS)

livelogs.o: livelogs.c
	$(CC) -c livelogs.c -o livelogs.o $(CFLAGS)

log.o: log.c
	$(CC) -c log.c -o log.o $(CFLAGS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS)

misc.o: misc.c
	$(CC) -c misc.c -o misc.o $(CFLAGS)

params.o: params.c
	$(CC) -c params.c -o params.o $(CFLAGS)

processscan.o: processscan.c
	$(CC) -c processscan.c -o processscan.o $(CFLAGS)

quarantine.o: quarantine.c
	$(CC) -c quarantine.c -o quarantine.o $(CFLAGS)

regedit.o: regedit.c
	$(CC) -c regedit.c -o regedit.o $(CFLAGS)

run.o: run.c
	$(CC) -c run.c -o run.o $(CFLAGS)

scan.o: scan.c
	$(CC) -c scan.c -o scan.o $(CFLAGS)

tray.o: tray.c
	$(CC) -c tray.c -o tray.o $(CFLAGS)

vars.o: vars.c
	$(CC) -c vars.c -o vars.o $(CFLAGS)

$(RES): icon.rc ressources.rc
	$(WINDRES) -i icon.rc --input-format=rc -o $(RES) -O coff
