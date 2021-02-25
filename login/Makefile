###########################################
# Login
# -------------------------------------
# file       : Makefile
# author     : Ben Kietzman
# begin      : 2002-04-04
# copyright  : kietzman.org
# email      : ben@kietzman.org
###########################################

BIN=bin/
INSTALLBIN=/usr/local/bin/
OBJ=obj/
BINARIES=$(BIN)windows_login

all: $(BINARIES)

$(BIN)windows_login: ../libcommon.a $(OBJ)windows_login.o
	-if [ ! -d $(BIN) ]; then mkdir $(BIN); fi
	g++ -o $(BIN)windows_login $(OBJ)windows_login.o -L/usr/local/lib -L/opt/csw/lib -L/data/extras/lib -L../ -lcommon -lsmbclient

$(OBJ)windows_login.o: windows_login.cpp
	-if [ ! -d $(OBJ) ]; then mkdir $(OBJ); fi
	g++ -Wall -ggdb -c $< -o $@ -I/usr/include/samba-4.0 -I/usr/local/include -I/opt/csw/include/ -I/data/extras/include -I../

install: $(BINARIES)
	install --mode=775 --strip $(BIN)windows_login $(INSTALLBIN)

uninstall:
	-rm -fr $(INSTALLBIN)windows_login

clean:
	-rm -fr $(OBJ) $(BIN)
