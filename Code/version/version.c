#include "version.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Versionierung der aktuellen Dateien, Variablen durch Makefile gesetzt
char* get_version(){
    char* ret=malloc(900*sizeof(char));

    //Git-Version
    strcat(ret,"\nProjekt Version:\t");
    #ifndef PROJECT_VERSION
    strcat(ret, "?");
    #else
    strcat(ret, PROJECT_VERSION);
    #endif


    //OS-Version
    strcat(ret, "\nBetriebssystem:\t");
    #ifndef OS_VERSION
    strcat(ret, "?");
    #else
    strcat(ret, OS_VERSION);
    #endif

    //Kernel-Version
    strcat(ret, "\nKernel Version:\t");
    #ifndef KERNEL_VERSION
    strcat(ret, "?");
    #else
    strcat(ret, KERNEL_VERSION);
    #endif


    //Compiler-Version
    strcat(ret, "\nCompiler Version:\t");
    #ifndef COMPILER_VERSION
    strcat(ret, "?");
    #else
    strcat(ret, COMPILER_VERSION);
    #endif


    //Gnuplot-Version
    strcat(ret, "\nGnuplot Version:\t");
    #ifndef GNUPLOT_VERSION
    strcat(ret, "?");
    #else
    strcat(ret, GNUPLOT_VERSION);
    #endif

    //Datum
    strcat(ret, "\nKompilierungs Datum:\t");
    #ifndef DATE
    strcat(ret, "?");
    #else
    strcat(ret, DATE);
    #endif


    return ret;
}
