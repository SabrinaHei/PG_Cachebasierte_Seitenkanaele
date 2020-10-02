# PG_Cachebasierte_Seitenkanaele
## Inhaltsverzeichnis README
1. Beschreibung
2. Installation und Verwendung
3. Datei- und Ordnerstruktur


### 1. Beschreibung
Diese Arbeit ist im Rahmen einer Projektgruppe an der Universität Bonn im Fachbereich Informatik entstanden.

Es wird die Machbarkeit eines cachebasierten Seitenkanals mit der Strategie
Flush + Flush demonstriert. Es gibt ein Kalibrierungstool, mit dem ein Threshold, der Grenzwert
zwischen Cache Hits und Misses, bestimmt werden kann. Der cachebasierte Seitenkanal findet An-
wendung in einem Chat, sodass zwischen zwei Kommunikationspartnern eine Textkommunikation
stattfinden kann, ohne dafür vom Betriebssystem bereitgestellte Funktionalitäten zu verwenden.

Das Testsystem hat einen Intel Core i7-4800MQ Prozessor mit vier Kernen, 8 GB Hauptspeicher
und die für Intel übliche Cache-Hierarchie. Der Last Level Cache ist ein 12-Way Set-Associative
Cache mit 8192 Sets mit einer Line Size von 64 Byte. Die Tests wurden auf dem Betriebssystem
Ubuntu 20.04 LTS durchgeführt.

Die vorliegende Arbeit wurde für die Verwendung auf einem Linux-Betriebssystem aufbereitet,
sodass für eigene Test ein möglichst zum Testsystem ähnliches Betriebssystem verwendet werden
sollte.

### 2. Installation und Verwendung

Im Ordner **/Code** befindet sich ein Makefile, das alle Sub-Makefiles in Unterordnern ausführt
und somit alle benötigten Programme kompiliert. Das Skript *install.sh* installiert alle benötigten
Programme (make und gnuplot) und führt zuvor genanntes Makefile aus (sudo-Rechte nötig). Nach
dem Ausführen von *install.sh* sind somit alle Programme sofort verwendbar. Es empfiehlt sich die
durch die Makefiles genannten Hinweise zur Ausführung zu verwenden.

1. Zuerst sollte durch das Kalibrierungsprogramm ein geeigneter Threshold gefunden werden.  
**/Code/calibration_fl+fl:** `$ make run`
2. Der neue Threshold und die gewünschte Übertragungsgeschwindigkeit für den Chat kann verändert werden.  
**/Code/chat_mit_fl+fl:** `$ make conf`
3. Chat im Empfängermodus starten.  
**/Code/chat_mit_fl+fl:** `$ make runr`
4. Chat im Sendemodus in neuem Fenster starten.  
**/Code/chat_mit_fl+fl:** `$ make runs`

Optional:
* Überprüfungsmodus beim Chat einschalten ( `$ make runr ARGS=’check’` bzw. `$ make runs ARGS=’check’`)
* Automatische Analyse (`$ make analyse`, `$ make showanalyse`)


### 3. Datei- und Ordnerstruktur

**/Code/calibration_fl+fl:** Automatische Bestimmung des Thresholds

* *calibration.c*:NUMBER_OF_REP wiederholte Threshold Messungen, Bestimmung des Durchschnitts dieser Messungen  
  
    Output: /Testergebnisse/Calibration-Ergebnisse/new-measurement, enthält:  
    * Log-Datei ( _calibration_log.txt_ )
    * Daten zu gemessenen Hits und Misses, die für die Bestimmung eines einzelnen Thresholds und für die Erstellung der Grafiken benötigt werden ( _hit.dat, hitmax.dat, miss.dat,_
    _missmax.dat, threshold.dat_ )
    * Grafiken: Übersicht aller gemessenen Thresholds ( _average_thresholds.pdf_ ), Letzte durch-
    geführte einzelne Threshold Messung ( _single_threshold.pdf_ )
* _libmylibrary.so_ : Verknüpfung zu eigener Shared Library
* _Makefile_ : kompilieren, ausführen, aufräumen

-------------------------------------------------------

**/Code/chat_mit_fl+fl:** Flush + Flush Seitenkanal Anwendung

* *receiver.c*: Empfangsfunktionalitäten (Synchronisation, Empfangen von Bit-Strings, Umwandlung Binär zu ASCII, optionale Überprüfung der Korrektheit der empfangen Daten)
* *sender.c*: Sendefunktionalitäten (Synchronisation, Senden von Bitstrings, Umwandlung ASCII zu Binär, optionale randomisierte Erzeugung von Bit-Strings)
* *receiver.h*: Headerdatei zu receiver.c
* *sender.h*: Headerdatei zu sender.c
* *rcvsnd.h*: Headerdatei mit gemeinsamen Funktionaliäten für receiver.c und sender.c
* *config.c* : Programm zur einfachen Änderung von Übertragungsparametern
* *rcvsnd.conf* : Übertragungsparameter
* *check.txt*: Kopie der gesendeten Daten. Wird vom Empfänger bei Korrektheitsüberprüfung
verglichen.
* *stringlength.txt*: Länge des gerade übertagenen Bitstrings. Statt einer festgelegten Stringlänge
oder der Verwendung von Start-/End-Bitsequenzen wird so für Testzwecke eine flexible
Stringlänge ermöglicht.
* chat_firstreceive.c*: Chatprogramm, das im Empfangsmodus startet. Verwendet receiver.c und
sender.c
* *chat_firstsend.c*: Chatprogramm, das im Sendemodus startet. Verwendet receiver.c und sender.c
* *chat.h*: Headerdatei zu chat_firstreceive.c und chat_firstsend.c
* *analyse.c*: Automatisierte Analyse vonrepwiederholten Übertragungen
* *libmylibrary.so*: Verknüpfung zu eigener Shared Library
* *Makefile*: kompilieren, Chat receiver/sender ausführen, Konfigurationsparameter verändern,
automatisierte Analyse durchführen, Analyseergebnisse anzeigen, aufräumen

-------------------------------

**/Code/library:** Shared Library, die für gemeinsame Cache-Speicherstellen verwendet wird.

-----------------------------

**/Code/obj:** Ausführbare- und Objekt-Dateien

---------------------

**/Code/memoryaccess:** Für Flush + Flush benötigte Speicher-Funktionalitäten
* *memoryaccess.c*: Messung von benötigten CPU-Zyklen, Zugriffauf Speicherstelle, Flushen von
Speicherstelle, Kombifunktionen zum Messen der Flushzeit im allgemeinen (Chat), bei Hit
und bei Miss (Calibration)
* *memoryaccess.h*: Headerdatei zu memoryaccess.c

--------------------------

**/Code/version:** Automatische Versionierung der wichtigsten Programme und Dateien

------------------------

**/Testergebnisse:** Enthält Ergebnisse zu Test- und Analysevorgängen
* **./Calibration-Ergebnisse:** Output Dateien der Calibration-Vorgänge
* **./Chat-Fehleranalyse:** Dateien zur Analyse der Übertragungsqualität
