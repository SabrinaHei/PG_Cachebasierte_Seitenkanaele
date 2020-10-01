#include <stdint.h>

#ifndef RCVSND_H
#define RCVSND_H

#define COMMENT_VERSION "Versions Kommentar:\tMit Randomisierter String Erzeugung und Fehlerüberprüfung + ASCII + Chat"  //Versions Kommentar

#define SEC 1000000000

 /*
 * Zeitfenster (in Nanosekunden, bei Intervallgröße von 1s):
 * Sender: 0 - 800.000.000
 * Receiver: 100.000.000 - 750.000.000
 * Break: 8.000.000 - 1.000.000.000
*/

#define BUSY_WAIT for(int j=0; j<interval/SEC*100; j++);

#define TRAIN_CPU \
    while (tests < 100000) { \
        res[tests % 1000] = measuretime_flush(addr);\
        tests += 1;\
        sched_yield();\
    }


//Warten auf Empänger/Sender Zeitfenster
#define WAIT_LOOP \
while(window==0){\
clock_gettime(CLOCK_MONOTONIC, &timestamp);\
if((timestamp.tv_nsec>=start_nsec && timestamp.tv_nsec<end_nsec)\
|| (overlimit==1 && timestamp.tv_nsec>=start_nsec && timestamp.tv_nsec<SEC)\
|| (overlimit==1 && timestamp.tv_sec>start_sec && timestamp.tv_nsec<end_nsec))\
window=1;\
BUSY_WAIT\
}

/*Warten bis Empfänger Window Anfang*/
#define WAIT_RWINDOW_START \
uint64_t cur_interval=(i*interval)%SEC; \
uint64_t start_nsec=(time_receiver_window_start+cur_interval)%SEC;\
uint64_t end_nsec=(time_receiver_window_end+cur_interval)%SEC;    \
clock_gettime(CLOCK_MONOTONIC, &timestamp);\
uint64_t start_sec=timestamp.tv_sec;\
int overlimit=0;\
if(start_nsec>end_nsec) overlimit=1;\
WAIT_LOOP\


/*Warten bis Sender Window Anfang*/
#define WAIT_SWINDOW_START  \
uint64_t cur_interval=(i*interval)%SEC;\
uint64_t start_nsec=(time_sender_window_start+cur_interval)%SEC;\
uint64_t end_nsec=(time_sender_window_end+cur_interval)%SEC;\
clock_gettime(CLOCK_MONOTONIC, &timestamp);\
uint64_t start_sec=timestamp.tv_sec;\
int overlimit=0;\
if(start_nsec>end_nsec) overlimit=1;\
WAIT_LOOP


/*Warten bis Empfänger Window Ende*/
#define WAIT_WINDOW_END \
while(window==1){\
    clock_gettime(CLOCK_MONOTONIC, &timestamp);\
    if(overlimit==0) {\
        if (timestamp.tv_nsec >= end_nsec || timestamp.tv_sec > start_sec) window = 0;\
        }else{\
            if (timestamp.tv_sec > start_sec && timestamp.tv_nsec >= end_nsec) window = 0;\
        }\
        BUSY_WAIT\
    }

 struct conf_data{
     uint64_t interval;
     uint64_t freq;
     uint64_t time_receiver_window_start;
     uint64_t time_receiver_window_end;
     uint64_t time_sender_window_start;
     uint64_t time_sender_window_end;
     uint64_t tests_per_interval;
 };

 struct address{
     void* addr1;
     void* addr2;
 };


#endif
