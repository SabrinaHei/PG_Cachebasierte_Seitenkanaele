#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>


#include "../memoryaccess/memoryaccess.h"
#include "rcvsnd.h"
#include "receiver.h"

#define UNUSUAL_TIME 250    //Zeitmessungen über diesem Wert weisen auf eine Fehlmessung hin

//Aus Config-Datei einzulesende globale Variablen
uint64_t threshold;
uint64_t interval;                          //Wert < SEC
uint64_t freq;                              //Frequenz, Wiederholungen pro Sekunde
uint64_t time_receiver_window_start;        //Empfangsfenster Start
uint64_t time_receiver_window_end;          //Empfangsfenster Ende
uint64_t tests_per_interval;                //Zeitmessungen pro Test
uint64_t window_size;                       //Größe des Empfangsfensters in ns

struct timespec r_timestart;                //Globale Startzeit nach Synchonisation
uint64_t res[1000]={0};                     //Für CPU Training
uint64_t s_window_size;

/*
 * Zeitfenster bei 1 s Intervall (in Nanosekunden):
 * Sender: 0 - 750.000.000
 * Receiver: 200.000.000 - 800.000.000
 * Break: 8.000.000 - 1.000.000.000
*/


void* r_synchronize(void* func){
    void (*addr)(void);
    addr=func;                          //Sender legt darüber 1 ab, wenn bereit

    struct timespec r_timestamp;
    int window=0;                       //Zeitfenster für Kommunikationsaktivitäten
    int tests=0;
    int success=0;                      //Counter für Bereitschaftssignal
    uint64_t totaltime, avgtime;
    tests_per_interval*=5;
    int* measured_times=malloc(10*tests_per_interval*sizeof(int));             //speichert die gemessenenen Zeiten fürs flushen

    TRAIN_CPU                           //liefert bessere Performance

    //Bereitschaftssignal Empfangen
    while(1) {
        sched_yield();

        //Warten bis Empfängerfenster Anfang
        while(window==0){
            clock_gettime(CLOCK_MONOTONIC,&r_timestamp);
            if(r_timestamp.tv_nsec>=time_receiver_window_start && r_timestamp.tv_nsec<time_receiver_window_end)
                window=1;
            BUSY_WAIT
        }

        //Zeitmessung
        int tests = 0;
        while(tests<tests_per_interval*10){
            measured_times[tests] = measuretime_flush(addr);
            if(measured_times[tests]<UNUSUAL_TIME) tests += 1;
            BUSY_WAIT
        }

        //Zeit Durchschnitt berechnen
        totaltime=0;
        for (int k = tests-1; k>=0; k--)totaltime+=measured_times[k];
        avgtime = ((double) totaltime) / tests;

        clock_gettime(CLOCK_MONOTONIC,&r_timestart);        //Startsekunde für Synchro bei Übertragungsanfang holen


        //Warten bis Empfängerfenster Ende
        while(window==1){
            clock_gettime(CLOCK_MONOTONIC,&r_timestamp);
            if(r_timestamp.tv_nsec>=time_receiver_window_end)
                window=0;
            BUSY_WAIT
        }

        //Bestimmen ob Durchschnittszeit 1 oder 0 entspricht
        if (avgtime >= threshold) {
            success++;
            if(success>=2) {                //zweimal Bereitschaftssignal hintereinander erhalten, um Fehlstarts zu vermeiden
                printf("\n");
                break;
            }
        }else success=0;
    }
    free(measured_times);
    return NULL;
}

void* receiver(int strl, void* func) {
    struct timespec timestamp;
    int window=0;                           //Zeitfenster für Kommunikationsaktivitäten
    int tests=0;
    uint64_t totaltime,avgtime;
    int datalength=strl;
    int* measured_times=malloc(tests_per_interval*sizeof(int));        //speichert die gemessenenen Zeiten beim flushen
    int* data=malloc(datalength*sizeof(int));                          //Empfangener Bitstring

    //Speicherstelle über die Kommuniziert wird
    void (*addr)(void);
    addr = func;

    TRAIN_CPU               //liefert bessere Performance

    //Vorm übertragen der Daten 1 Sekunde warten für bessere synchronisation
    while(1){
        clock_gettime(CLOCK_MONOTONIC, &timestamp);
        if(r_timestart.tv_sec+1<=timestamp.tv_sec)
            break;
        BUSY_WAIT
    }

    sched_yield();

    for (int i = 0; i < datalength; i++) {
        WAIT_RWINDOW_START          //Warten auf Start des Empfangsfensters

        //Zeitmessung
        tests = 0;
        while(tests<tests_per_interval){
            measured_times[tests] = measuretime_flush(addr);
            if(measured_times[tests]<UNUSUAL_TIME) tests++;
            BUSY_WAIT
        }

        WAIT_WINDOW_END             //Warten auf Ende des Empfangsfensters

        //Zeit Durchschnitt berechnen
        totaltime=0;
        for (int k = tests-1; k>=0; k--)totaltime+=measured_times[k];
        avgtime = totaltime / tests;

        //Bestimmen ob Durchschnittszeit 1 oder 0 entspricht
        if (avgtime < threshold ) {
            data[i]=0;
        }else{
            data[i]=1;
        }

        sched_yield();          //CPU-Zeit des Prozesses ggf. abgeben, für unterbrechungsfreiere Messungen
    }

    free(measured_times);
    return (void*) data;
}



//Vergleich des über den Cache übertragenen Bitstrings mit Bitstring in Datei
void check(int* data, int analyse){
    printf("\n--------------------Analyse--------------------\n");
    FILE* fptr;
    char* filename="check.txt";
    if ((fptr = fopen(filename, "r")) == NULL) {
        printf("\e[31;1mFehler beim Öffnen der Datei %s\e[39;0m\n",filename);
        exit(1);
    }
    int datalength;
    fscanf(fptr,"%d",&datalength);

    int* proof_data=malloc(datalength*sizeof(int));         //Gesendete-Daten aus Datei
    for(int i=0; i<datalength; i++){
        fscanf(fptr, "%d", &proof_data[i]);
    }

    fclose(fptr);

    //Wenn von Analyseprogramm gestartet, öffnen von Datei
    if(analyse) {
        //Analysedatei öffnen
        filename = "../../Testergebnisse/Chat-Fehleranalyse/analyse.txt";
        if ((fptr = fopen(filename, "a")) == NULL) {
            printf("\e[31;1Fehler beim Öffnen der Datei %s\e[39;0m\n", filename);
            exit(1);
        }
    }



    //Ausgabe der Bitstrings und Überprüfung von Fehlerhaften Bits
    int false=0;
    printf("\nErhaltener Bitstring:\n");
    for(int i=0; i<datalength;i++){
        if(i!=0 && i%7==0)printf(" ");
        printf("%d",data[i]);
    }
    int onebit=0;
    int zerobit=0;
    printf("\n\nGesendeter Bitstring:\n");
    for(int i=0; i<datalength;i++){
        if(i!=0 && i%7==0)printf(" ");
        printf("%d",proof_data[i]);
        if(data[i]!=proof_data[i]) {
            false++;          //fehlerhafte gesamt Bits zählen
            if (data[i] > proof_data[i])
                zerobit++;      //fehlerhafte 0-Bits zählen
            else
                onebit++;       //fehlerhafte 1-Bits zählen
        }
    }


    //Überprüfung von Fehlerhaften Zeichen
    int falsechar=0;
    for(int i=0; i<datalength/7;i++){
        for(int k=0; k<7; k++){
            if(data[k+7*i]!=proof_data[k+7*i]){
                falsechar++;                    //falls mind. 1 fehlerhaftes Bit, Buchstabe falsch
                break;
            }
        }
    }


    printf("\n\n=> Von %d erhaltenen Bits und somit %d Zeichen sind bei einer Frequenz von %lu Hz %d Bits und %d Zeichen fehlerhaft.\n",datalength, datalength/7, freq,false, falsechar);
    printf("Fehlerrate Bits: %.2f%%\n",((double)false)*100/datalength);
    printf("Fehlerrate Zeichen: %.2f%%\n",((double)falsechar)*100/(datalength/7));

    //Wenn von Analyseprogramm gestartet, in Datei schreiben
    if(analyse) {
        fprintf(fptr,
                "|\t%lu\t ||\t%d\t ||\t    %d   \t   ||\t         %d  \t\t||    %.2f%%\t ||    %.2f%%\t     ||    %d\t     ||    %d\t     ||    %lu\t||  \t   %lu\t    ||\t%lu\n",
                freq, datalength, false, falsechar, ((double) false) * 100 / datalength, ((double) falsechar) * 100 / (datalength / 7), onebit, zerobit, threshold, window_size, s_window_size);
        fclose(fptr);
    }
    free(proof_data);
    return;
}

//Wandelt einen Binärstring in ASCII Zeichen um
char binary_to_ascii(int* input){
    int val=0;
    int mult=1;

    for(int i=0;i<7; i++){
        val+=input[i]*mult;
        mult*=2;
    }

    int gotten =(char)val;
    printf("%c", gotten);
    return gotten;
}


//main
int chat_receive(int argc, char** argv, void* addrs, int analyse) {
    pid_t receiver_pid=getpid();

    //Empfangsprozess auf Kern 0 ausführen
    cpu_set_t my_set;                   //Für cpu Bitmaske
    CPU_ZERO(&my_set);          //Bitmaske auf 0 setzen
    CPU_SET(0, &my_set);   //Bit setzen, dass entsprechenden Kern maskiert, Kern 0
    sched_setaffinity(receiver_pid, sizeof(cpu_set_t), &my_set);   //CPU_affinity für diesen Prozess auf CPU Kern 0 setzen

    struct address* addr=(struct address*)addrs;        //Zum speichern von Kommunikationsadressen

    //Konfigurationsparameter aus Datei auslesen
    FILE *fptr;
    if ((fptr = fopen("rcvsnd.conf", "r")) == NULL) {
        printf("\e[31;1Fehler beim Öffnen der Konfigurationsdatei.\e[39;0m\n");
        exit(1);
    }

    char name[12];
    uint64_t value;

    while(fscanf(fptr, "%s %lu", name, &value) != EOF) {
        if (strcmp(name, "interval") == 0) interval = value;
        else if (strcmp(name, "tests") == 0) tests_per_interval = value;
        else if (strcmp(name, "rec_start") == 0) time_receiver_window_start = value;
        else if (strcmp(name, "rec_end") == 0) time_receiver_window_end = value;
        else if (strcmp(name, "send_start") == 0); //time_sender_window_start=value;
        else if (strcmp(name, "send_end") == 0); //time_sender_window_end=value;
        else if (strcmp(name, "freq") == 0) freq = value;
        else if (strcmp(name, "thresh") == 0) threshold = value;
        else printf("\e[31;1mFehler beim Lesen der Konfigurationsdatei: \nParameter %s konnte nicht zugeordnet werden.\n\e[39;0m", name);
    }
    fclose(fptr);

    printf("Empfange Daten:");
    fflush(stdout);

    //Synchronisation mit Sender, Intervall 1 Sekunde
    r_synchronize(addr->addr1);


    //Konfigurationsparameter fürs Empfangen vorbereiten
    interval/=freq;
    tests_per_interval*= (double) interval/100000000;
    time_receiver_window_start = interval*10/100;
    time_receiver_window_end = interval*75/100;
    window_size=time_receiver_window_end-time_receiver_window_start;
    s_window_size=(interval * 80 / 100) - 0;

    //Länge der zu empfangenden Daten holen
    char* filename="stringlength.txt";
    if ((fptr = fopen(filename, "r")) == NULL) {
        printf("\e[31;1mFehler beim Öffnen der Datei %s\e[39;0m\n",filename);
        exit(1);
    }
    int stringlength;
    fscanf(fptr,"%d",&stringlength);
    fclose(fptr);

    //Daten Empfangen
    int* data= (int*) receiver(stringlength, addr->addr2);

    //Bitstring zu ASCII Zeichen wandeln und ausgeben
    int bit_num_char=7;             //Ein char hat 256 (0-255) mögl werte, also 2^8-1, 1.bit nicht benötigt -> 7 Bit für ein char
    int* char_bits=malloc(bit_num_char*sizeof(int));
    printf("\t\t\t\t\t\e[104;30;1m");
    char* received_string=malloc(stringlength/bit_num_char*sizeof(char));
    for(int i=0; i<stringlength/bit_num_char; i++){
        for(int k=0; k<bit_num_char; k++)
            char_bits[k]=data[k+i*bit_num_char];
        received_string[i]=binary_to_ascii(char_bits);
    }
    printf("\e[0m\n");


    //Analyse von Fehlübertragungen
    if (argc > 1) {
        if (strcmp(argv[1], "check") == 0)
            check(data, analyse);
    }
    free(char_bits);
    free(data);
    if(strcmp(received_string, "quit")==0){
        printf("\e[93mDer Andere hat den Chat beendet!\e[39;0m\n");
        free(received_string);
        return 1;
    }
    free(received_string);
    return 0;
}
