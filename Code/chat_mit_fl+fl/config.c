#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

//Vereinfachte Anpassung der Konfigurationsparameter für Chat
int main(){
    FILE* fptr;
    FILE* fptr_tmp;
    char* filename="rcvsnd.conf";
    char* filename_tmp="rcvsnd.conf~";
    if ((fptr = fopen(filename, "r+w")) == NULL) {
        printf("\e[31;1Fehler beim Öffnen der Konfigurationsdatei %s\e[39;0m\n",filename);
        exit(1);
    }
    if ((fptr_tmp = fopen(filename_tmp, "w+r")) == NULL) {
        printf("\e[31;1Fehler beim Öffnen der Temporärendatei %s\e[39;0m\n",filename_tmp);
        exit(1);
    }

    printf("Aktuelle Konfiguration:\n");
    printf("(Die Farbkodierungen weisen auf die Unbedenklichkeit der Änderung hin)\n");

    char name[12];
    uint64_t value;

    //Aktuelle Konfiguration auslesen und printen
    while(fscanf(fptr, "%s %lu", name, &value) != EOF) {
        if (strcmp(name, "interval") == 0) printf("\e[31m1. Intervallgröße (in ns):.....................................");
        else if (strcmp(name, "tests") == 0) printf("\e[33m2. Zeitmessungen/Intervall (Tests):............................");
        else if (strcmp(name, "rec_start") == 0) printf("\e[33m3. Empfängerfenster Start (in ns):.............................");
        else if (strcmp(name, "rec_end") == 0) printf("\e[33m4. Empfängerfenster Ende (in ns):..............................");
        else if (strcmp(name, "send_start") == 0)printf("\e[33m5. Senderfenster Start (in ns):................................");
        else if (strcmp(name, "send_end") == 0)printf("\e[33m6. Senderfenster Ende (in ns):.................................");
        else if (strcmp(name, "freq") == 0) printf("\e[32m7. Frequenz (Sende-/Empfangsfenster pro Sekunde):..............");
        else if (strcmp(name, "thresh") == 0) printf("\e[32m8. Threshold:..................................................");
        else printf("\e[31;1mFehler beim Lesen der Konfigurationsdatei: \nParameter %s konnte nicht zugeordnet werden.\n\e[39;0m", name);
        printf("\e[39m%lu\n", value);
    }
    printf("Die jeweiligen Zeiten sind relativ zu 1 Sekunde und werden je nach gewählter Frequenz automatisch skaliert. Die Anzahl der Tests wird entsprechend der Frequenz ebenso automatisch skaliert.\n");

    printf("\nUm einen Wert zu ändern muss die entsprechnde Ziffer (1-8) eingegeben werden:");



    //Wählen des Werts der geändert werden soll
    int input;
    scanf("%d",&input);
    if(input<1||input>8){
        printf("Die Eingabe war leider ungültig. Es sind ausschließlich die Zahlen 1-8 gültig.\n");
        fclose(fptr_tmp);
        fclose(fptr);
        remove(filename_tmp);
        exit(1);
    }
    switch(input){
        case 1: printf("Änderung des Intervalls gewählt. 1 s -> 1000000000 ns.\n Neuer Wert:"); break;
        case 2: printf("Änderung der Zeitmessungen pro Intervall (tests) gewählt. Empfohlener Wert: 7000.\nNeuer Wert:"); break;
        case 3: printf("Änderung der Startzeit des Empfangsfensters gewählt. Empfohlener Wert: 100000000\nNeuer Wert:"); break;
        case 4: printf("Änderung der Endzeit des Empfangsfensters gewählt. Empfohlener Wert: 750000000\nNeuer Wert:"); break;
        case 5: printf("Änderung der Startzeit des Empfangsfensters gewählt. Empfohlener Wert: 0\nNeuer Wert:"); break;
        case 6: printf("Änderung der Startzeit des Empfangsfensters gewählt. Empfohlener Wert: 800000000\nNeuer Wert:"); break;
        case 7: printf("Änderung der Frequenz gewählt. Wähle Werte zwischen 10 bis 7000.\nNeuer Wert:"); break;
        case 8: printf("Änderung des Thresholds gewählt. Empfohlener Wert: 171-176\nNeuer Wert:");
    }


    uint64_t new_val;
    scanf("%lu",&new_val);


    //Neuen Wert in Temporäre Datei schreiben
    fseek(fptr, 0, SEEK_SET);
    while(fscanf(fptr, "%s %lu", name, &value) != EOF) {
        if (input ==1 && strcmp(name, "interval") == 0) fprintf(fptr_tmp, "%s %lu\n", name, new_val);
        else if (input ==2 && strcmp(name, "tests") == 0) fprintf(fptr_tmp, "%s %lu\n", name, new_val);
        else if (input ==3 && strcmp(name, "rec_start") == 0) fprintf(fptr_tmp, "%s %lu\n", name, new_val);
        else if (input ==4 && strcmp(name, "rec_end") == 0) fprintf(fptr_tmp, "%s %lu\n", name, new_val);
        else if (input ==5 && strcmp(name, "send_start") == 0)fprintf(fptr_tmp, "%s %lu\n", name, new_val);
        else if (input ==6 && strcmp(name, "send_end") == 0)fprintf(fptr_tmp, "%s %lu\n", name, new_val);
        else if (input ==7 && strcmp(name, "freq") == 0) fprintf(fptr_tmp, "%s %lu\n", name, new_val);
        else if (input ==8 && strcmp(name, "thresh") == 0) fprintf(fptr_tmp, "%s %lu\n", name, new_val);
        else fprintf(fptr_tmp,"%s %lu\n", name, value);
    }

    //Neue Konfiguration Ausgeben
    fseek(fptr_tmp, 0, SEEK_SET);
    printf("\nNeue Konfiguration:\n");
    while(fscanf(fptr_tmp, "%s %lu", name, &value) != EOF) {
        if (strcmp(name, "interval") == 0) printf("\e[31m1. Intervallgröße (in ns):.....................................");
        else if (strcmp(name, "tests") == 0) printf("\e[33m2. Zeitmessungen/Intervall (Tests):............................");
        else if (strcmp(name, "rec_start") == 0) printf("\e[33m3. Empfängerfenster Start (in ns):.............................");
        else if (strcmp(name, "rec_end") == 0) printf("\e[33m4. Empfängerfenster Ende (in ns):..............................");
        else if (strcmp(name, "send_start") == 0)printf("\e[33m5. Senderfenster Start (in ns):................................");
        else if (strcmp(name, "send_end") == 0)printf("\e[33m6. Senderfenster Ende (in ns):.................................");
        else if (strcmp(name, "freq") == 0) printf("\e[32m7. Frequenz (Sende-/Empfangsfenster pro Sekunde):..............");
        else if (strcmp(name, "thresh") == 0) printf("\e[32m8. Threshold:..................................................");
        else printf("\e[31;1mFehler beim Lesen der Konfigurationsdatei: \nParameter %s konnte nicht zugeordnet werden.\n\e[39;0m", name);
        printf("\e[39m%lu\n", value);
    }


    //Temporäre Datei in Original kopieren
    fseek(fptr, 0, SEEK_SET);
    fseek(fptr_tmp, 0, SEEK_SET);
    while(fscanf(fptr_tmp, "%s %lu", name, &value) != EOF){
        fprintf(fptr,"%s %lu\n", name, value);
    }

    //Dateien schließen und Temporäre Datei Löschen
    fclose(fptr_tmp);
    fclose(fptr);
    remove(filename_tmp);

    return 0;
}

