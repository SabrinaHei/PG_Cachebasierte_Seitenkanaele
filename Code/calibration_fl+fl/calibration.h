#ifndef MINE_CALIBRATION_H
#define MINE_CALIBRATION_H

#define COMMENT_VERSION "Versions Kommentar:\tAusgabe von Threshold Median + Grafik in eigenem Ordner"          //Versions Kommentar

//Output-Dateien
#define DIR_RESULT "../../Testergebnisse/Calibration-Ergebnisse/new-measurement/"
#define F_MISS_DAT "../../Testergebnisse/Calibration-Ergebnisse/new-measurement/miss.dat"                       //Daten zu Cache Misses bei einzelner Threshold Bestimmung
#define F_HIT_DAT "../../Testergebnisse/Calibration-Ergebnisse/new-measurement/hit.dat"                         //Daten zu Cache Hits bei einzelner Threshold Bestimmung
#define F_MISS_MAX "../../Testergebnisse/Calibration-Ergebnisse/new-measurement/missmax.dat"                    //Häufigste benötigte Zeit für Cache Miss bei einzelner Threshold Bestimmung
#define F_HIT_MAX "../../Testergebnisse/Calibration-Ergebnisse/new-measurement/hitmax.dat"                      //Häufigste benötigte Zeit für Cache Hit bei einzelner Threshold Bestimmung
#define F_THRESHOLDS_DAT "../../Testergebnisse/Calibration-Ergebnisse/new-measurement/threshold.dat"            //Alle gesammelten Thresholds
#define PIC_SNGL_THRESHOLD "../../Testergebnisse/Calibration-Ergebnisse/new-measurement/single_threshold.pdf"   //Output Bild: Plot der letzten einzelnen Threshold Bestimmung
#define PIC_AVG_THRESHOLDS "../../Testergebnisse/Calibration-Ergebnisse/new-measurement/average_thresholds.pdf" //Output Bild: Plot aller gesammelten Thresholds
#define F_LOG "../../Testergebnisse/Calibration-Ergebnisse/new-measurement/calibration_log.txt"                 //Log-Datei mit allen Programmausgaben

uint64_t calibrate();
void plot(char*);
void write_version_to_file(int, char**);

#endif
