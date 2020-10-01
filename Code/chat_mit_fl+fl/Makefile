# -*- Makefile -*-

PROJECT_VERSION := git-$(shell git rev-parse HEAD 2>/dev/null)
KERNEL_VERSION := $(shell uname -r)
COMPILER_VERSION := $(shell clang --version | grep "clang"|sed 's/(.*)//')
OS_VERSION := $(shell cat /etc/os-release  | grep "PRETTY_NAME" | sed 's/PRETTY_NAME=//'|sed 's/\"//g')
GNUPLOT_VERSION := $(shell gnuplot -V)
DATE := $(shell date +"%a, %d. %h %Y")

CC= clang
CFLAGS= -Wall -g -std=gnu11 -O0 #-fno-pie
MAKROS= -DPROJECT_VERSION='"$(PROJECT_VERSION)"' \
-DKERNEL_VERSION='"$(KERNEL_VERSION)"' \
-DCOMPILER_VERSION='"$(COMPILER_VERSION)"'\
-DOS_VERSION='"$(OS_VERSION)"'\
-DGNUPLOT_VERSION='"$(GNUPLOT_VERSION)"'\
-DDATE='"$(DATE)"'
 #-Wextra -Wpedantic

LIBS= -L. -lmylibrary -lm
ODIR=../obj
_OBJ= memoryaccess.o version.o receiver.o sender.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))
DEPS=../memoryaccess/memoryaccess.h ../version/version.h rcvsnd.h receiver.h sender.h chat.h
LIBPATH= LD_LIBRARY_PATH=./



main: chat_firstreceive.c chat_firstsend.c $(OBJ) $(DEPS)
	$(CC) -o $(ODIR)/firstreceive chat_firstreceive.c $(OBJ) $(CFLAGS) $(LIBS) 
	$(CC) -o $(ODIR)/firstsend chat_firstsend.c  $(OBJ) $(CFLAGS) $(LIBS) 
	@echo "\n\n\e[39;1mVerwendung Receiver & Sender:\e[39;0m\nAufräumen: make clean\nChat mit startendem Receiver ausführen: make runr [ARGS='check' (für Analyse)]\nChat mit startendem Sender ausführen: make runs [ARGS='check' (für Analyse)]\nKonfiguration anpassen: make conf\nAutomatische Analyse: make analyse\nAnalyse Ergebnisse anzeigen: make showanalyse"


$(ODIR)/receiver.o: receiver.c $(DEPS)
	$(CC) -c -o $@ $<  $(CFLAGS)

$(ODIR)/sender.o: sender.c $(DEPS)
	$(CC) -c -o $@ $<  $(CFLAGS)

$(ODIR)/memoryaccess.o: ../memoryaccess/memoryaccess.c $(DEPS)
	$(CC) -c -o $@ $<  $(CFLAGS)

$(ODIR)/version.o: ../version/version.c $(DEPS)
	$(CC) -c -o $@ $<  $(CFLAGS) $(MAKROS)

.PHONY: runr runs clean conf analyse showanalyse


clean: 
	rm -f $(OBJ) $(ODIR)/conf $(ODIR)/analyse $(ODIR)/firstsend $(ODIR)/firstreceive *~

runr: 
	@$(LIBPATH) $(ODIR)/firstreceive $(ARGS)
	
runs:
	@$(LIBPATH) $(ODIR)/firstsend $(ARGS)
	
conf: config.c
	@$(CC) -o $(ODIR)/$@ $<
	@$(ODIR)/conf
	
analyse: ./analyse.c $(OBJ) $(DEPS)
	$(CC) -o $(ODIR)/analyse ./analyse.c  $(OBJ) $(CFLAGS) $(LIBS) 
	$(LIBPATH) $(ODIR)/analyse
	
showanalyse:
	cat ../../Testergebnisse/Chat-Fehleranalyse/analyse.txt




