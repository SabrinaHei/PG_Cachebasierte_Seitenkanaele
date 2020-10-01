# -*- Makefile -*-

PROJECT_VERSION := git-$(shell git rev-parse HEAD 2>/dev/null)
KERNEL_VERSION := $(shell uname -r)
COMPILER_VERSION := $(shell clang --version | grep "clang"|sed 's/(.*)//')
OS_VERSION := $(shell cat /etc/os-release  | grep "PRETTY_NAME" | sed 's/PRETTY_NAME=//'|sed 's/\"//g')
GNUPLOT_VERSION := $(shell gnuplot -V)
DATE := $(shell date +"%a, %d. %h %Y")


CC= clang
CFLAGS= -Wall -g -std=gnu11 -O0
MAKROS=-DPROJECT_VERSION='"$(PROJECT_VERSION)"' \
-DKERNEL_VERSION='"$(KERNEL_VERSION)"' \
-DCOMPILER_VERSION='"$(COMPILER_VERSION)"'\
-DOS_VERSION='"$(OS_VERSION)"'\
-DGNUPLOT_VERSION='"$(GNUPLOT_VERSION)"'\
-DDATE='"$(DATE)"'



LIBS=-L./ -lmylibrary
ODIR=../obj
_OBJ= calibration.o memoryaccess.o version.o
OBJ=$(patsubst %,$(ODIR)/%,$(_OBJ))
DEPS=calibration.h ../memoryaccess/memoryaccess.h ../version/version.h

main: $(OBJ)
	$(CC) -o $(ODIR)/calibration $^  $(LIBS) $(CFLAGS)
	@echo "\n\n\e[39;1mVerwendung Calibration:\e[39;0m\nAufräumen: make clean\nAusführen: make run" 

$(ODIR)/calibration.o: calibration.c $(DEPS)
	$(CC) -c -o $@ $<  $(CFLAGS) 

$(ODIR)/memoryaccess.o: ../memoryaccess/memoryaccess.c $(DEPS)
	$(CC) -c -o $@ $<  $(CFLAGS) 

$(ODIR)/version.o: ../version/version.c $(DEPS)
	$(CC) -c -o $@ $<  $(CFLAGS) $(MAKROS)



.PHONY: run clean
clean: 
	rm -f $(OBJ) $(ODIR)/calibration *~



run:
	LD_LIBRARY_PATH=./ $(ODIR)/calibration


