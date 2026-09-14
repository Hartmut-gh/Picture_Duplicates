# Read_DB_csv_File

Checks pictures of one or more folders to find duplicates.

Überprüft Bilder aus einem oder mehreren Verzeichnissen auf Duplikate.


## Deutsch
### Funktionen

- Es können mehrere "Sets" von Verzeichnissen angelegt werden.

- Zu jedem "Set" können ein oder mehrere Verzeichnisse hinzugefügt werden. Die Unterverzeichnisse von denen werden automatisch auch hinzugefügt.

- Für jedes Verzeichnis kann ausgewählt werden, ob es
    - eingeschlossen ist.
    - eingeschlossen und ein Referenz-Verzeichnis ist.
    - ausgeschlossen ist.

- Zudem kann pro Verzeichnis ausgewählt werden, ob innerhalb des Verzeichnisses nach Duplikaten gesucht wird (beide Dateien sind im selben Verzeichnis).

- Auch für alle Ordner kann generell ausgewählt werden (dann gelten nicht die individuelle Einstellung siehe die Zeile über dieser)
    - Generell keine Duplikats-Sucher im selben Verzeichnis
    - Nur Duplikats-Sucher im selben Verzeichnis

- Zur Duplikatssuche werden bis zu drei Verfahren angewandt. Für jedes Verfahren können Grenzen gesetzt werden.
     1. pHash - Abstand     (unterhalb einer Grenze: Immer ein Duplikat, oberhalb einer Grenze: Niemals ein Duplikat)
     2. canny - Unterschied (unterhalb einer Grenze: Immer ein Duplikat, oberhalb einer Grenze: Niemals ein Duplikat)
     3. ORB   - Unterschied (oberhalb einer Grenze: ein Duplikat)
  Wenn ein Verfahren bereits ein eindeutiges Ergebnis liefert, wird kein weiteres Verfahren mehr verwendet. Dadurch wird viel Rechenleistung gespart.
  pHash geht am schnellsten, canny bereits etwas langsamer und ORB ist sehr rechenintensiv.

- Zur weiteren Beschleunigung werden pHash, canny und ORB Werte von jedem Bild, das schon einmal analysiert wurde, in einer Datenbank gespeichert.
  Auch ORB Differenten werden in der Datenbank gespeichert, weil sie sonst immer sehr zeitaufwendig neu berechnet werden müssten.

- Diese Datenbank lässt sich bereinigen, falls inzwischen viele Bilder gelöscht wurden.

- Das Programm versucht auf mehreren Wegen eine Bilddatei in der Datenbank zu finden:
     - Vollständiger Datei-Pfad  (Dateigröße und Datum der letzten Änderung müssen gleich sein)
     - Nur Dateiname selbst      (Dateigröße und Datum der letzten Änderung müssen gleich sein)
     - Dateiname ohne einen [...] Prefix (Dateigröße und Datum der letzten Änderung müssen gleich sein)
  Nur wenn so keine Dateiinformationen in der Datenbank gefunden werden, wird das Bild als unbekannt gewertet und die Werte berechnet.
  Das Programm erkennt so auch, on eine Datei inzwischen in einem anderen Ordner ist und speichert den neuen Pfad in der Datenbank ab.

- Nach dem Finden der Duplikate öffnet sich ein neues Fenster. In dem kann man nun
    - Bilder im Bildbetrachter des Rechners öffnen
    - Bilder löschen (das 'Master' Bild kann nicht gelöscht werden)
    - Bilder vergleichen (über Differenz-Methode bzw. XOR-Methode)
    - Ein Bild zum 'Master'-Bild machen.
    - Manuell angeben, dass ein Vergleichspaar doch keine Duplikate sind (diese Information wird auch in der Datenbank gespeichert).

- Es lassen sich alle manuellen "Kein-Duplikat" -Bilderpaare anzeigen und diese Indikation wieder entfernen.

- Das Suchen nach Duplikaten benötigt mehrere Programm-Schritte. Das Programm zeigt ganz unten den aktuellen Fortschritt an.


## English
### Features

- You can create several "sets" of folders.

- You can add one or more folders to each set. Subfolders are automatically added as well.

- You can select for each folder, whether
    - to include it.
    - to include it as reference folder.
    - to exclude it.

- You can select as well, whether the search for duplicates should not be done within the same folder (both files are in the same folder).

- There is also a general option (in this case the individual setting in the line above this one is not used)
   - for all folders do not look for duplicates in the same folder.
   - for all folders only look in the same folder for duplicates.

- To find duplicates there are three calculation methods. For each of them you can set limits.
     1. pHash - distance   (below a limit: Always a duplicate, above a limit: Never a duplicate)
     2. canny - difference (below a limit: Always a duplicate, above a limit: Never a duplicate)
     3. ORB   - difference (above a limit: a duplicate)
  If a calculation method already delivers a clear result, no other method will be used. This save a lot of CPU load.
  pHash is the fastest, canny is a bit slower and ORB needs quite some CPU load.

- To speed up even more, pHash, canny and ORB values of each picture, which was analyzed, are stored in a database.
  As well the ORB differences are stored as they need a lot of CPU power.

- The database can be shrinked, of there are many pictures which are deleted meanwhile.

- The program tries to search for picture data in the database:
     - Using the complete file path (file size and file data must be equal)
     - Using the file name only     (file size and file data must be equal)
     - Using the file name stripped from a [...] prefix (file size and file data must be equal)
  Only if no file information was found by this approach, the picture is considered as unknown and the needed values are calculated.
  The program can also recognize whether a file was moved into another folder. In this case the path information is updated in the database.

- After finding of the duplicates there is a new window. Here you can
    - open a picture in the picture viewer of your PC
    - delete a picture (the 'master' picture cannot be deleted)
    - compare pictures (using difference and XOR methods)
    - change a picture to 'master'
    - define manually that a picture pair is not a duplicate

- You can show a list of all the "No Duplicate" pairs where you can remove this indicator.

- The search for duplicates need multiple program steps. The program shows the progress at the bottom of the main window.


## Screenshots

### Hauptfenster
![Hauptfenster](screenshots/01_MainWindow.png)


## Voraussetzungen

- Qt 6.11 oder höher
- CMake 3.16 oder höher
- Ein C++-Compiler


## Build

Das Projekt verwendet CMake und Qt.

Zum Bauen kann das Projekt mit **Qt Creator** geöffnet werden:

1. `CMakeLists.txt` in Qt Creator öffnen.
2. Einen passenden Qt/CMake-Kit auswählen.
3. Projekt konfigurieren lassen.
4. Projekt bauen.

Alternativ kann das Projekt auch direkt mit CMake gebaut werden:
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .

   ```

## Flatpak

Die fertige Version ist als Flatpak unter [Releases](../../releases) verfügbar.


## Lizenz

Dieses Projekt steht unter der **GNU General Public License v3.0 or later (GPL-3.0-or-later)**.

Siehe die Datei `LICENSE` für den vollständigen Lizenztext.
