#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>


#include <sched.h>


#include "../library/mylibrary.h"
#include "rcvsnd.h"
#include "sender.h"
#include "receiver.h"

int main() {
    //Variablendeklarationen
    pid_t sender, receiver, father;     //Zum PIDs speichern
    int rep=50;                         //Anzahl der Test Wiederholungen



    //Analysedatei öffnen
    FILE *fptr;
    char *filename = "../../Testergebnisse/Chat-Fehleranalyse/analyse.txt";
    if ((fptr = fopen(filename, "a+")) == NULL) {
        printf("\e[31;1Fehler beim Öffnen der Datei %s\e[39;0m\n", filename);
        exit(1);
    }
    fprintf(fptr,
            "|Frequenz(Bits/s)||Bitstringlänge||Anzahl fehlerhafter Bits||Anzahl fehlerhafter Zeichen||Fehlerrate Bits||Fehlerrate Zeichen||Falsche 1-Bits||Falsche 0-Bits||Threshold||Fenstergröße Empfänger(ns)||Fenstergröße Sender(ns)|\n");
    fprintf(fptr,
            "------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    fflush(fptr);

    //Zeigerposition holen
    long start_file_position=ftell(fptr);           //da Datei im Modus "append" geöffnet ist und bereits Inhalt in die Datei geschrieben wurde, zeigt der Filepointer auf das aktuelle Ende der Datei


    //Adressen die zur Kommunikation an Sender/Empfänger übergeben werden müssen
    struct address{
        void* addr1;
        void* addr2;
    };
    struct address* addr=malloc(sizeof(struct address));
    addr->addr1=&func4;
    addr->addr2=&func1;

    //Parameter zur Übergabe zum Ausführen der Check-Funktionalität
    char* argv[2];
    argv[0]="\0";
    argv[1]="check";


    //Vaterprozess auf Kern 3 ausführen, sodass Kinder auf Kern 0 und 1 nicht beeinträchtigt werden
    father=getpid();
    cpu_set_t my_set;                   //Für cpu Bitmaske
    CPU_ZERO(&my_set);          //Bitmaske auf 0 setzen
    CPU_SET(3, &my_set);   //Bit setzen, dass entsprechenden Kern maskiert
    sched_setaffinity(father, sizeof(cpu_set_t), &my_set);   //CPU_affinity für diesen Prozess auf CPU Kern 3 setzen


   for(int i = 0; i < rep; i++) {
        if (fork() == 0) {              //Kindprozess
            receiver=getpid();
            printf("\e[93;1m%d Empfängerprozess startet.\e[0m\n", i);
            fflush(stdout);
            chat_receive(2, argv, (void *) addr, 1);
            kill(receiver,SIGKILL);     //Prozess fertig und beenden, sodass kein Einfluss in weiterem Proframmverlauf/bei weiteren forks
        } else {
            if (fork() == 0) {          //Kindprozess
                sender=getpid();
                printf("\e[93;1m%d Senderprozess startet.\e[0m\n",i);
                fflush(stdout);
                sleep(1);               //Empfängerprozess genug Zeit geben zum starten
                chat_send(0, "", addr);
                kill(sender, SIGKILL);          //Prozess fertig und beenden, sodass kein Einfluss in weiterem Proframmverlauf/bei weiteren forks
            } else {
                while (wait(NULL) > 0);
                printf("\e[93;1m%d Analyseprozess sagt: Empfänger und Sender fertig.\e[0m\n", i);
            }
        }
    }


   //Gerade erhaltene Daten weiter verarbeiten

    //Variablendeklarationen
    int freq,stringlength,err_bit,err_char, onebit, zerobit,threshold;
    uint64_t recv_window,send_window;
    float err_rate_bit,err_rate_char;
    char s1[10],s2[10],s3[10],s4[10],s5[10],s6[10],s7[10],s8[10],s9[10],s10[10],s11[10];
    int sum_stringlength=0,sum_err_bit=0,sum_err_char=0,sum_onebit=0,sum_zerobit=0;
    float sum_err_rate_bit=0,sum_err_rate_char=0;


    //In diesem Durchlauf produzierte Messungen auslesen und verarbeiten
    fseek(fptr,start_file_position,SEEK_SET);
    while(fscanf(fptr, "| %20[^ \t] || %20[^ \t] || %20[^ \t] || %20[^ \t] || %20[^ \t] || %20[^ \t] || %20[^ \t] || %20[^ \t] || %20[^ \t] || %20[^ \t] || %20[^ \t\n]\n",s1,s2,s3,s4,s5,s6,s7,s8,s9,s10,s11)!= EOF) { //&freq,&stringlength,&err_bit,&err_char,&err_rate_bit,&err_rate_char,&onebit,&zerobit, &threshold,&recv_window,&send_window) != EOF) {
        freq=strtol(s1,NULL,10);
        stringlength=strtol(s2,NULL,10);
        err_bit=strtol(s3,NULL,10);
        err_char=strtol(s4,NULL,10);
        int i=0;
        while(s5[i]!='%'){
            i++;
        }
        s5[i]='\0';
        err_rate_bit=strtof(s5,NULL);
        i=0;
        while(s6[i]!='%'){
            i++;
        }
        s6[i]='\0';
        err_rate_char=strtof(s6,NULL);
        onebit=strtol(s7,NULL,10);
        zerobit=strtol(s8,NULL,10);
        threshold=strtol(s9,NULL,10);
        recv_window=strtoll(s10,NULL,10);
        send_window=strtoll(s11,NULL,10);

        //Aufsummieren
        sum_stringlength+=stringlength;
        sum_err_bit+=err_bit;
        sum_err_char+=err_char;
        sum_err_rate_bit+=err_rate_bit;
        sum_err_rate_char+=err_rate_char;
        sum_onebit+=onebit;
        sum_zerobit+=zerobit;
    }

    //Durchschnitte in Datei schreiben
    fprintf(fptr,"---------------------------------------------------------------------------------------Durchschnittswerte bei dieser Konfiguration--------------------------------------------------------------------------------------------\n");
    fprintf(fptr,"|\t%d\t ||\t%.1f\t ||\t    %.1f   \t   ||\t       %.1f  \t\t||    %.2f%%\t ||    %.2f%%\t     ||  %.1f\t     ||    %.1f\t     ||    %d\t||  \t   %lu\t    ||\t%lu\n",freq,(float)sum_stringlength/rep,(float)sum_err_bit/rep,(float)sum_err_char/rep,sum_err_rate_bit/rep, sum_err_rate_char/rep,(float)sum_onebit/rep,(float)sum_zerobit/rep, threshold, recv_window,send_window);
    fprintf(fptr,"------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

    fclose(fptr);
    free(addr);
}