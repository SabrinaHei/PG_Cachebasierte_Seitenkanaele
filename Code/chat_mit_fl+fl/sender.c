#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <math.h>

#include "../memoryaccess/memoryaccess.h"
#include "rcvsnd.h"
#include "sender.h"

/*
 * Zeitfenster bei 1 s Intervall (in Nanosekunden):
 * Sender: 0 - 800.000.000
 * Receiver: 200.000.000 - 750.000.000
 * Break: 8.000.000 - 1.000.000.000
*/

extern inline void flush(void* address);

//Aus Config-Datei einzulesende globale Variablen
uint64_t interval;                          //Wert < SEC
uint64_t time_sender_window_start;          //Sendefenster Start
uint64_t time_sender_window_end;            //Sendefenster Ende
uint64_t tests_per_interval;                //Zeitmessungen pro Test
uint64_t window_size;                       //Größe des Sendefensters in ns

double relative;                            //relative Zeitkomponente für gute Ausnutzung des jew. Aktionsfensters
struct timespec s_timestart;                //Globale Startzeit nach Synchonisation




void* s_synchronize(void* func){
    void (*addr)(void);
    addr=func;            //Sender greift auf Speicherstelle zu, wenn bereit
    struct timespec timestamp;

    for (int i=0; i<2; i++) {
        sched_yield();

        //Warten bis Senderfenster Anfang, Sleep verringert vorzeitiges ablegen von Daten
        clock_gettime(CLOCK_MONOTONIC,&timestamp);
        timestamp.tv_sec+=1;
        timestamp.tv_nsec=time_sender_window_start;
        clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&timestamp, NULL);

        //Bereitschaftssignal an Empfänger übertragen
        for (uint64_t transmits = 0; transmits < window_size/14; transmits++)
            access_memory(addr);

        clock_gettime(CLOCK_MONOTONIC,&s_timestart);

        //Warten bis Senderfenster Ende
        while(1){
            clock_gettime(CLOCK_MONOTONIC,&timestamp);
            if(timestamp.tv_nsec>=time_sender_window_end)
                break;
            BUSY_WAIT
        }
    }
    return NULL;
}


void* sender(void* param, void* func) {
    void (*addr)(void);
    addr = func;

    int* string_to_send=((struct send_data*)param)->bitstring;
    int stringlength= ((struct send_data*)param)->strl;
    int bit_to_send;
    int window=0;
    struct timespec timestamp;


    //Vorm übertragen der Daten 1 Sekunde warten
    while(1){
        flush(addr);                    //flushen verhindert Fehlmessungen durch frühzeitiges Ablegen von Daten
        clock_gettime(CLOCK_MONOTONIC, &timestamp);
        if(s_timestart.tv_sec+1<=timestamp.tv_sec)
            break;
        BUSY_WAIT
    }

    sched_yield();

    for(int i=0; i<stringlength;i++) {
        WAIT_SWINDOW_START                          //Warten auf Start des Sendefensters

        bit_to_send=string_to_send[i];
        if (bit_to_send == 0) {                     //Nicht auf Daten zugreifen -> nichts im Cache ablegen
            flush(addr);                            //Flushen -> Kein Inhalt in Cache -> Bit 0
            clock_gettime(CLOCK_MONOTONIC, &timestamp);
            int sleeptime=window_size*10/100;       //Für schlafen bis Fensterende-10% der Fenstergröße
            if(overlimit==1){                       //wenn start ns > ende ns -> Sonderbehandlung
                timestamp.tv_sec= start_sec+1;
                if(end_nsec-sleeptime<0){           //Schlafenszeit in vorheriger Sekunde -> underlimit
                    sleeptime-=end_nsec;
                    timestamp.tv_nsec=SEC-sleeptime;
                    timestamp.tv_sec--;
                }else{
                    timestamp.tv_nsec=end_nsec-sleeptime;
                }
            }else{  //Bit=1
                timestamp.tv_nsec=end_nsec-sleeptime;   //Für schlafen bis Fensterende-10% der fenstergröße
            }
            clock_nanosleep(CLOCK_MONOTONIC,TIMER_ABSTIME,&timestamp, NULL);
            flush(addr);                                //Flushen wiederholen, falls vorgeladen
        }else {
            for(uint64_t transmits=0; transmits<(window_size/(10+relative)); transmits++) //hat sich als Anzahl der Wiederholungen bewährt, um das Fenster möglichst effizient auszunutzen
                access_memory(addr);                    //auf Adresse Zugreifen -> in Cache laden
        }

        WAIT_WINDOW_END                                 //Warten auf Ende des Sendefensters
        sched_yield();                                  //CPU-Zeit des Prozesses ggf. abgeben, für unterbrechungsfreiere Messungen
    }
    return 0;
}

//Generiert zufälligen Bitstring
void* random_string(){
    //Für Analyse in Datei schreiben, Datei öffnen
    FILE* fptr;
    char* filename="check.txt";
    if ((fptr = fopen(filename, "w")) == NULL) {
        printf("\e[31;1Fehler beim Öffnen der Datei %s\e[39;0m\n",filename);
        exit(1);
    }

    //zufällige Länge generieren und in Datei schreiben
    srand(time(NULL));
    int stringlength=rand()%2002+21;
    fprintf(fptr,"%d\n",stringlength);

    //zufälligen Bitstring generieren und in Datei schreiben
    int* data_to_send=malloc(stringlength*sizeof(int));
    for(int i=0; i<stringlength; i++){
        data_to_send[i]=rand()%2;
        fprintf(fptr,"%d\n", data_to_send[i]);
    }

    fclose(fptr);

    //Stringlänge in Datei schreiben
    filename="stringlength.txt";
    if ((fptr = fopen(filename, "w")) == NULL) {
        printf("\e[31;1Fehler beim Öffnen der Datei %s\e[39;0m\n",filename);
        exit(1);
    }
    fprintf(fptr,"%d\n\n",stringlength);
    fflush(fptr);
    fclose(fptr);

    struct send_data* sendData=(struct send_data*) malloc(sizeof(struct send_data*));
    sendData->strl=stringlength;
    sendData->bitstring=data_to_send;

    return sendData;
}

int* ascii_to_binary(int input){
    int val=0;
    int remainder;
    int i=1;            //Dezimalstelle
    int k=0;            //Array-Index
    int* data=(int*)malloc(7*sizeof(int));
    memset(data,0,7*sizeof(*data));

    while(input>0){
        remainder=input%2;          //letztes Bit 1 oder 0 (Binärdaten)
        val+=i*remainder;           //Bits in dezimal aufrechnen
        input/=2;
        i*=10;                      //Stellenweise erhöhen
        data[k]=remainder;
        k++;                        //Array-Index erhöhen
    }

    return data;


}


int chat_send(int params, char* string, void* addrs){
    pid_t sender_pid=getpid();

    //Senderprozess auf Kern 1 ausführen
    cpu_set_t my_set;                   //Für cpu Bitmaske
    CPU_ZERO(&my_set);          //Bitmaske auf 0 setzen
    CPU_SET(1, &my_set);   //Bit setzen, dass entsprechenden Kern maskiert, Kern 1
    sched_setaffinity(sender_pid, sizeof(cpu_set_t), &my_set);   //CPU_affinity für diesen Prozess auf CPU Kern 1 setzen

    struct address* addr=(struct address*)addrs;

    //Konfigurationsparameter aus Datei auslesen
    FILE *fptr;
    if ((fptr = fopen("rcvsnd.conf", "r")) == NULL) {
        printf("\e[31;1Fehler beim Öffnen der Konfigurationsdatei.\e[39;0m\n");
        exit(1);
    }

    char name[12];
    uint64_t value;
    uint64_t freq;

    while(fscanf(fptr, "%s %lu", name, &value) != EOF) {
        if (strcmp(name, "interval") == 0) interval = value;
        else if (strcmp(name, "tests") == 0) tests_per_interval = value;
        else if (strcmp(name, "rec_start") == 0); //time_receiver_window_start = value;
        else if (strcmp(name, "rec_end") == 0); //time_receiver_window_end = value;
        else if (strcmp(name, "send_start") == 0)time_sender_window_start=value;
        else if (strcmp(name, "send_end") == 0)time_sender_window_end=value;
        else if (strcmp(name, "freq") == 0) freq = value;
        else if (strcmp(name, "thresh") == 0);
        else printf("\e[31;1mFehler beim Lesen der Konfigurationsdatei: \nParameter %s konnte nicht zugeordnet werden.\n\e[39;0m", name);
    }
    fclose(fptr);
    window_size=time_sender_window_end-time_sender_window_start;

    struct send_data* sendData;
    if (params > 0) {
        int bit_num_char=7;             //Ein char hat 256 (0-255) mögl werte, also 2^8-1, 1.bit aber hier nicht benötigt -> 7 Bit für char
        int datalength=0;               //Länge des zu übertragenden Strings

        //Länge ermitteln von Nutzerdaten
        int i=0;
        while(string[i]!='\0'){
            datalength++;
            i++;
        }
        int* data_to_send=malloc(datalength*bit_num_char*sizeof(int));

        //Stringlänge in Dateischreiben
        FILE* fptr;
        char* filename="stringlength.txt";
        if ((fptr = fopen(filename, "w")) == NULL) {
            printf("\e[31;1Fehler beim Öffnen der Datei %s\e[39;0m\n",filename);
            exit(1);
        }
        fprintf(fptr,"%d\n\n",datalength*bit_num_char);
        fflush(fptr);


        //Für Fehleranalyse in Datei schreiben
        filename="check.txt";
        if ((fptr = fopen(filename, "w")) == NULL) {
            printf("\e[31;1Fehler beim Öffnen der Datei %s\e[39;0m\n",filename);
            exit(1);
        }
        fprintf(fptr,"%d\n",datalength*bit_num_char);

        //Zeichenfolge in Binärstring übersetzen und in Datei für Fehleranalyse schreiben
        for(i=0; i<datalength; i++){
            int* data=ascii_to_binary((int)string[i]);              //char in Binärstring übersetzen
            for(int k=0;k<bit_num_char;k++) {
                data_to_send[k + (i * bit_num_char)] = data[k];     //in Arbeits-Array übertragen
                fprintf(fptr, "%d\n", data_to_send[k + (i * bit_num_char)]);        //für Fehleranalyse in Datei
            }
            free(data);
        }

        sendData=malloc(sizeof (struct send_data));
        sendData->strl=datalength*bit_num_char;
        sendData->bitstring=data_to_send;

        fclose(fptr);

    }else sendData=random_string();         //Keine Nutzerdaten übergeben -> randomisierten String erzeugen

    //Synchronisation mit Empfänger
    s_synchronize(addr->addr1);

    printf("Daten werden übertragen: \t\t\e[43;30;1m");

    //Konfigurationsparameter fürs Senden vorbereiten
    interval/=freq;
    tests_per_interval*= (double) interval/10000000;
    time_sender_window_start = 0;
    time_sender_window_end = interval*80/100;
    window_size=time_sender_window_end-time_sender_window_start;

    //Anzahl Ziffern der geg. Frequenz berechnen und relativ dazu in Sendeaktivitäten einbeziehen
    int n=freq;
    int count=0;
    while(n!=0){
        n/=10;
        count++;
    }
    count-=2;
    if(count<=0)count=0;
    relative=pow(2, count);

    //Zu Sendende Daten auf Konsole ausgeben
    if(params>0) printf("%s\e[0m\n", string);
    else printf("<Check Sequenz>\e[0m\n");

    //Daten senden
    sender((void*)sendData, addr->addr2);

    free(sendData->bitstring);
    free(sendData);

    return 0;
}
