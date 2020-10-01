#define _GNU_SOURCE

#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "calibration.h"
#include "../version/version.h"
#include "../memoryaccess/memoryaccess.h"
#include "../library/mylibrary.h"


#define MIN(a, b) ((a) > (b)) ? (b) : (a)   //berechnet Minimum aus zwei Werten
#define MAX(a, b) ((a) < (b)) ? (b) : (a)   //berechnet Maximum aus zwei Werten

//Definitionen zu Zeitmessung Hit/Miss -> Thresholdbestimmung einzeln
#define HISTOGRAM_SIZE 500                  //X-Achse, CPU-Zykel
#define NUMBER_OF_TESTS 10000               //Anzahl Tests bzw. Einträge im Histogramm zur Bestimmung eines einzelnen Thresholds

//Definitionen zu Durchschnitt von Thresholds
#define THRESHOLD_HISTOGRAM_SIZE 500        //X-Achse, CPU-Zykel
#define NUMBER_OF_REP 250                 //Anzahl von Threshold Bestimmungen bzw. Wiederholungen

extern inline void flush(void* address);

//Globale Variablen
uint64_t maxhit;
uint64_t maxmiss;

uint64_t calibrate() {
/**********************************************
 Variablen Setup
 **********************************************/

    //X-Achse (Index) von Histogramm ist benötigte Zeit, Y-Achse (Werte) enthält Anzahl von Zugriffen, die so viel Zeit wie Index benötigt haben
    size_t hit_histogram[HISTOGRAM_SIZE] = {0};     //initialisiere Histogramm an allen Stellen mit 0 (0 Zugriffe)
    size_t miss_histogram[HISTOGRAM_SIZE] = {0};    //initialisiere Histogramm an allen Stellen mit 0 (0 Zugriffe)

    void (*addr)(void);
    addr=&func1;

/**********************************************
 Zeitmessung
 **********************************************/

    //Zeitmessung Cache Miss
    for (int i = 0; i < NUMBER_OF_TESTS; i++) {
        __asm__ volatile ("mfence");
        uint64_t time_needed = measuretime_flush_miss(addr);            //gibt benötigte CPU-Zykel für flush bei Miss zurück
        miss_histogram[MIN(HISTOGRAM_SIZE - 1, time_needed)]++;      //trage Zeit in Histogramm ein (erhöhe Treffer um 1): Wenn Zeit > X-Achse, nimm X-Achsen Maximum
        sched_yield();                                                  //signalisiere Bereitschaft für Context Switch, damit Rest danach zeitlang ohne Unterbrechung laufen kann
    }


    //Zeitmessung Cache Hit
    for (int i = 0; i < NUMBER_OF_TESTS; i++) {
        __asm__ volatile ("mfence");
        uint64_t time_needed = measuretime_flush_hit(addr);             //gibt benötigte CPU-Zykel für flush bei Hit zurück
        hit_histogram[MIN(HISTOGRAM_SIZE - 1, time_needed)]++;       //trage Zeit in Hist ein (erhöhe Treffer um 1): Wenn Zeit > X-Achse, nimm X-Achsen Maximum
        sched_yield();                                                  //signalisiere Bereitschaft für Context Switch, damit Rest danach zeitlang ohne Unterbrechung laufen kann
    }


/**********************************************
 Threshold bestimmen
 **********************************************/

    //Cache Hit dauert länger als Miss, da Speicherstelle aus allen Cache Leveln entfernt werden muss
    //=>Berechne Maximum von Cache Miss und Minimum von Cache Hit
    size_t hit_most_index = -1;
    size_t miss_most_index = -1;
    size_t miss_most = 0;
    size_t hit_most = 0;

    for (int i = 0; i < HISTOGRAM_SIZE; i++) {
        //Bestimme Hit Minimum bzw. die am häufigsten benötigte Zeit für einen Hit
        if (hit_most < hit_histogram[i]) {
            hit_most = hit_histogram[i];
            hit_most_index = i;
        }

        //Bestimme Miss Maximum bzw. die am häufigsten benötigte Zeit für einen Miss
        if (miss_most < miss_histogram[i]) {
            miss_most = miss_histogram[i];
            miss_most_index = i;
        }
    }

    maxhit = hit_histogram[hit_most_index];
    maxmiss = miss_histogram[miss_most_index];

    //Annahme: Hit Zeit>Miss Zeit
    uint64_t threshold = hit_most_index - (hit_most_index - miss_most_index)/2;           //Berechne Differenz zwischen Hit und Miss. Nimm davon die Hälfte und zieh sie von Unterem Grenzwert von Hit ab, um Mitte zwischen Miss und Hit zu bestimmen

    //=>Zeit-Achse: Miss obere Grenze| ...|threshold|... |Hit untere Grenze

/**********************************************
 Konsolen Ausgabe und Datenübergabe
 **********************************************/

    //Dateien zum schreiben öffnen
    FILE *log = fopen(F_LOG, "a");
    FILE *miss_dat = fopen(F_MISS_DAT, "w");
    FILE *hit_dat = fopen(F_HIT_DAT, "w");

    //Ergebnisse ausgeben
    for (int i = 0; i < HISTOGRAM_SIZE; i++) {
        printf("%3d Cycles: %6zu Misses %6zu Hits\n", i, miss_histogram[i],hit_histogram[i]);           //Konsolenausgabe
        fprintf(log, "%3d Cycles: %6zu Misses %6zu Hits\n", i, miss_histogram[i],hit_histogram[i]);     //Ausgabe in Log-Datei
        fprintf(hit_dat, "%d %zu\n", i, hit_histogram[i]);          //Hit Ergebnisse in Datei für gnuplot
        fprintf(miss_dat, "%d %zu\n", i, miss_histogram[i]);        //Miss Ergebnisse in Datei für gnuplot
    }
    fclose(miss_dat);
    fclose(hit_dat);

    //Abschließende Ausgabe nach einzelner Threshold Messung
    printf("\n--------------------\n");
    printf("Hit Minimum: %zu\n", hit_most_index);
    printf("Miss Maximum: %zu\n", miss_most_index);
    printf("Threshold: %zu\n", threshold);
    fprintf(log, "\n--------------------\nHit Minimum: %zu\nMiss Maximum: %zu\nThreshold: %zu\n", hit_most_index, miss_most_index, threshold);
    fclose(log);

    //Maximaler Wert Hit in Datei für gnuplot
    FILE *hit_max = fopen(F_HIT_MAX, "w");
    fprintf(hit_max, "%lu %zu", hit_most_index, hit_histogram[hit_most_index]);
    fclose(hit_max);

    //Maximaler Wert Miss in Datei für gnuplot
    FILE *miss_max = fopen(F_MISS_MAX, "w");
    fprintf(miss_max, "%lu %zu", miss_most_index, miss_histogram[miss_most_index]);
    fclose(miss_max);

    return threshold;
}

/**********************************************
 Hilfsfunktionen
 **********************************************/

//Datenverarbeitung mit gnuplot
void plot(char* string) {
    //Graph erstellen mit gnuplot
    FILE *gnuplot = popen("gnuplot -persistent", "w");
    fprintf(gnuplot, "%s", string);             //jeweiliger Plot Befehl
    fprintf(gnuplot, "show output\n");          //Anzeige in Welcher Datei die Daten gespeichert wurden

    //gnuplot schließen
    fprintf(gnuplot, "exit\n");
    fflush(gnuplot);
    pclose(gnuplot);

    return;
}

//Versionierung der aktuellen Datei-Version
void write_version_to_file(int argc, char** argv){
    FILE* fd;
    fd=fopen("../version/calibration_version.txt", "w");
    fprintf(fd, "Versionsinformationen zu Kalibrierung für Fl+Fl cachebasierten Seitenkanal:\n");
    fprintf(fd, "Programmaufruf:\t\t");
    for(int i=0; i<argc;i++){
        fprintf(fd,"%s ", argv[i]);
    }
    fprintf(fd,"%s\n", get_version());
    fprintf(fd, "%s\n", COMMENT_VERSION);
    fprintf(fd, "%s\n", MEMACCES_VERSION);

    fclose(fd);
    return;
}

/**********************************************
 main
 **********************************************/

int main(int argc, char** argv){
    //Prozess auf Kern 0 ausführen
    cpu_set_t my_set;                   //Für cpu Bitmaske
    CPU_ZERO(&my_set);          //Bitmaske auf 0 setzen
    CPU_SET(0, &my_set);   //Bit setzen, dass entsprechenden Kern maskiert, Kern 0
    sched_setaffinity(getpid(), sizeof(cpu_set_t), &my_set);   //CPU_affinity für diesen Prozess auf CPU Kern 0 setzen

    write_version_to_file(argc, argv);  //Versionierung

    //Neues Verzeichnis zur Ablage der Ergebnisse erstellen, falls noch nicht vorhanden
    struct stat st = {0};
    if (stat(DIR_RESULT, &st) == -1) {
        mkdir(DIR_RESULT, 0777);
    }else{
        remove(F_LOG);
    }


    //CPU trainieren, dass Fkt wichtig
    void (*addr)(void);
    addr = &func1;
    uint64_t tests = 0;
    uint64_t res[1000];
    while (tests < 100000) {
        res[tests % 1000] = measuretime_flush(addr);
        tests += 1;
        sched_yield();
    }

    //Wiederholte Messung von Thresholds
    size_t threshold_histogram[THRESHOLD_HISTOGRAM_SIZE] = {0};
    uint64_t avg_threshold=0;
    uint64_t last_threshold;
    int failure = 0;
    int max_thresh = 0;
    for (int i = 0; (i - failure) < NUMBER_OF_REP; i++) {
        printf("\n********************************\n*Start measurement of Threshold*\n********************************\nNumber: %d Failure: %d\n",i, failure);
        FILE *log = fopen(F_LOG, "a");
        fprintf(log,"\n********************************\n*Start measurement of Threshold*\n********************************\nNumber: %d Failure: %d\n",i, failure);
        fclose(log);
        sched_yield();
        __asm__ volatile ("mfence");
        __asm__ volatile ("lfence");
        last_threshold = calibrate();       //Threshold messen
        if (last_threshold >= 250) {        //Fehlerbehandlung
            failure++;
            for(int j=0; j<10000000;j++);   //busy-wait
        } else {
            threshold_histogram[MIN(THRESHOLD_HISTOGRAM_SIZE - 1,last_threshold)]++;       //neuen Threshold in Histogramm eintragen
            avg_threshold += last_threshold;                                                  //Zusammenzählen der Thresholds für Durchschnittsberechnung
        }
    }
    avg_threshold /= NUMBER_OF_REP;         //Berechnet durchschnittlichen Threshold


    //Gesammelte Thresholds verarbeiten
    FILE *threshold_dat = fopen(F_THRESHOLDS_DAT, "w");
    for (int i = 0; i < THRESHOLD_HISTOGRAM_SIZE; i++) {
        fprintf(threshold_dat, "%d %zu\n", i, threshold_histogram[i]);          //Ergebnisse in Datei für gnuplot
        if (max_thresh < threshold_histogram[i]) {                                     //Häufigsten Threshold finden -> für gnuplot y-Achse
            max_thresh = threshold_histogram[i];
        }
    }
    fclose(threshold_dat);

    //Finale abschließende Ausgabe
    printf("\n-------------------------------------------------------\n-------------------------------------------------------\n"
           "The middle threshold for %d successful tests was %lu\n", NUMBER_OF_REP, avg_threshold);
    FILE *log = fopen(F_LOG, "a");
    fprintf(log,"\n-------------------------------------------------------\n-------------------------------------------------------\n"
           "The middle threshold for %d successful tests was %lu\n", NUMBER_OF_REP, avg_threshold);
    double errorrate = ((double) failure / (NUMBER_OF_REP + failure)) * 100;
    printf("Failures occured: %d -> Erroneous measurement rate: %.2f%%\n", failure, errorrate);
    fprintf(log,"Failures occured: %d -> Erroneous measurement rate: %.2f%%\n", failure, errorrate);
    fclose(log);


    //Daten an gnuplot geben, um Grafiken zu erstellen
    int yachsis = MAX(maxmiss, maxhit) + (MAX(maxmiss, maxhit) / 5);
    char plot_command[1000] = {0};
    //letzte einzelne Threshold Messung
    snprintf(plot_command, 1000, "set title 'Einzelne Threshold Messung bei %d Tests'\n"
                                 "set xlabel 'benötigte CPU-Zykel'\n"
                                 "set ylabel 'absolute Häufigkeit'\n"
                                 "set grid\n" "set xrange[100:%d]\n"
                                 "set yrange[0:%d]\n"
                                 "set xtics 25\n"
                                 "set arrow from %lu,0 to %lu,%d nohead linecolor '#07519A'\n"             //Berechneten Wert für Threshold als vertikale Linie zeichnen
                                 "set label ' Threshold: %lu' at %lu,%d textcolor '#07519A'\n"
                                 "plot '%s' pointtype 9 linecolor rgb 'red' title 'Cache Miss', "          //Cache Miss Graph
                                 "'%s' pointtype 11 linecolor rgb 'green' title 'Cache Hit', "             //Cache Hit Graph
                                 "'%s' pointtype 6 linecolor rgb 'red' ps 1.4 title 'Häufigster Wert Cache Miss', "     //Miss Zeit-Treffer Maximum
                                 "'%s' pointtype 6 linecolor rgb 'green' ps 1.4 title 'Häufigster Wert Cache Hit'\n"    //Hit Zeit-Treffer Maximum
                                 "set term pdfcairo enhanced\n"
                                 "set output '%s'\n"
                                 "replot\n", NUMBER_OF_TESTS, HISTOGRAM_SIZE*2/3, yachsis+(yachsis/5), last_threshold,
             last_threshold, yachsis+(yachsis/5), last_threshold, last_threshold, yachsis/2, F_MISS_DAT,
             F_HIT_DAT, F_MISS_MAX, F_HIT_MAX, PIC_SNGL_THRESHOLD);
    plot(plot_command);
    //Übersicht aller gemessenen Thresholds
    yachsis = max_thresh + (max_thresh / 5);
    snprintf(plot_command, 1000, "set title 'Übersicht aller %d gemessenen Thresholds'\n"
                                 "set xlabel 'Threshold in CPU-Zykeln'\n"
                                 "set ylabel 'absolute Häufikeit'\n"
                                 "set grid\n"
                                 "set xrange[120:250]\n"
                                 "set yrange[0:%d]\n"
                                 "set arrow from %lu,0 to %lu,%d nohead linecolor 'blue'\n"                 //Berechneten Wert für durchscnittlichen Threshold als vertikale Linie zeichnen
                                 "set label ' Durchschnitt: %lu' at %lu,%d textcolor 'blue'\n"
                                 "plot '%s' pointtype 11 linecolor 0 title 'Thresholds'\n"
                                 "set term pdfcairo enhanced\n"
                                 "set output '%s'\n"
                                 "replot\n", NUMBER_OF_REP,  yachsis, avg_threshold,
             avg_threshold, yachsis, avg_threshold, avg_threshold, yachsis/2, F_THRESHOLDS_DAT,
             PIC_AVG_THRESHOLDS);
    plot(plot_command);

    return 0;
    }



