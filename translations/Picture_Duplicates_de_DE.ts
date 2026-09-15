<?xml version='1.0' encoding='UTF-8'?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<!-- ===== MainWindow ===== -->
<context>
    <name></name>
    <message id="mainwindow.title">
        <source/>
        <translation>Bilder Duplikate</translation>
    </message>

    <message id="mainwindow.table.colFolder">
        <source/>
        <translation>Verzeichnis</translation>
    </message>
    <message id="mainwindow.table.colImages">
        <source/>
        <translation>Bilder</translation>
    </message>
    <message id="mainwindow.table.colImages.withNumber">
        <source/>
        <comment>%1 = count</comment>
        <translation>Bilder (%1)</translation>
    </message>
    <message id="mainwindow.table.colStatus">
        <source/>
        <translation>Status</translation>
    </message>
    <message id="mainwindow.table.colStatus.valExcl">
        <source/>
        <translation>Ausgeschlossen</translation>
    </message>
    <message id="mainwindow.table.colStatus.valIncl">
        <source/>
        <translation>Eingeschlossen</translation>
    </message>
    <message id="mainwindow.table.colStatus.valRef">
        <source/>
        <translation>Referenz</translation>
    </message>
    <message id="mainwindow.table.colExclDir">
        <source/>
        <translation>Aus.=Verz.</translation>
    </message>
    <message id="mainwindow.table.colExclDir.Excl">
        <source/>
        <translation>Selb.Verz.ausschl.</translation>
    </message>

    <message id="mainwindow.button.newSet">
        <source/>
        <translation>Neues Set</translation>
    </message>
    <message id="mainwindow.button.duplSet">
        <source/>
        <translation>Set duplizieren</translation>
    </message>
    <message id="mainwindow.button.delSet">
        <source/>
        <translation>Set löschen</translation>
    </message>

    <message id="mainwindow.button.findDupl">
        <source/>
        <translation>Finde Duplikate</translation>
    </message>
    <message id="mainwindow.button.stopScan">
        <source/>
        <translation>Suche beenden</translation>
    </message>
    <message id="mainwindow.button.apply">
        <source/>
        <translation>Anwenden</translation>
    </message>
    <message id="mainwindow.button.reset">
        <source/>
        <translation>Zurücksetzen</translation>
    </message>
    <message id="mainwindow.button.showManual">
        <source/>
        <translation>Manuelle 'Kein-Duplikat' Liste</translation>
    </message>
    <message id="mainwindow.button.cleanDB">
        <source/>
        <translation>DB säubern</translation>
    </message>
    <message id="mainwindow.button.language">
        <source/>
        <translation>Sprache</translation>
    </message>

    <message id="mainwindow.button.addFolder">
        <source/>
        <translation>+</translation>
    </message>
    <message id="mainwindow.button.remFolder">
        <source/>
        <translation>-</translation>
    </message>

    <message id="mainwindow.label.progressTask.Idle">
        <source/>
        <translation>Gerade nichts zu tun</translation>
    </message>
    <message id="mainwindow.label.progressTask.readFileInfoFromDB">
        <source/>
        <translation>Lese Datei Info aus der DB</translation>
    </message>
    <message id="mainwindow.label.progressTask.calculateORBetc">
        <source/>
        <translation>Berechne ORB/pHash/Canny</translation>
    </message>
    <message id="mainwindow.label.progressTask.storeORBetc">
        <source/>
        <translation>Speichere ORB/pHash/Canny in DB</translation>
    </message>
    <message id="mainwindow.label.progressTask.preparePairing">
        <source/>
        <translation>Paar-Liste vorbereiten</translation>
    </message>
    <message id="mainwindow.label.progressTask.readScores">
        <source/>
        <translation>Lese Scores/pHash aus DB</translation>
    </message>
    <message id="mainwindow.label.progressTask.calcScores">
        <source/>
        <translation>Berechne Scores</translation>
    </message>
    <message id="mainwindow.label.progressTask.storeScores">
        <source/>
        <translation>Speichere Scores in DB</translation>
    </message>
    <message id="mainwindow.label.progressTask.removeDBentries">
        <source/>
        <translation>Lösche DB Einträge</translation>
    </message>
    <message id="mainwindow.label.progressTask.removeDBduplicates">
        <source/>
        <translation>Lösche DB Duplikate</translation>
    </message>
    <message id="mainwindow.label.progressTask.shrinkDBfile">
        <source/>
        <translation>DB Datei verkleinern</translation>
    </message>

    <message id="mainwindow.label.progressNoMax">
        <source/>
        <translation>...</translation>
    </message>
    <message id="mainwindow.label.progress.eta">
        <source/>
        <comment>%1 = volume to do in total
                 %2 = remaining time
                 %3 = total needed time
                 (ü) = abbreviation for 'Übrig' / (g) = abbreviation for 'Gesamt'</comment>
        <translation>%1
%2(ü) | %3 (g)</translation>
    </message>

    <message id="mainwindow.slider.ORB.Title">
        <source/>
        <translation>&lt;center&gt;(3) ORB Score Grenze&lt;br&gt;höheres Ergebnis =&amp;gt; Duplikat&lt;/center&gt;</translation>
    </message>
    <message id="mainwindow.slider.pHashLower.Title">
        <source/>
        <translation>&lt;center&gt;(1a) pHash untere Grenze&lt;br&gt;kleineres Ergebnis =&amp;gt; immer Duplikat&lt;/center&gt;</translation>
    </message>
    <message id="mainwindow.slider.pHashHigher.Title">
        <source/>
        <translation>&lt;center&gt;(1b) pHash obere Grenze&lt;br&gt;höheres Ergebnis =&amp;gt; immer KEIN Duplikat&lt;/center&gt;</translation>
    </message>
    <message id="mainwindow.slider.cannyLower.Title">
        <source/>
        <translation>&lt;center&gt;(2a) Canny untere Grenze&lt;br&gt;kleineres Ergebnis =&amp;gt; immer Duplikat&lt;/center&gt;</translation>
    </message>
    <message id="mainwindow.slider.cannyHigher.Title">
        <source/>
        <translation>&lt;center&gt;(2b) Canny obere Grenze&lt;br&gt;höheres Ergebnis =&amp;gt; immer KEIN Duplikat&lt;/center&gt;</translation>
    </message>

    <message id="mainwindow.combobox.lblMngSameDir">
        <source/>
        <translation>Selbes Verzeichnis: </translation>
    </message>
    <message id="mainwindow.combobox.valOff">
        <source/>
        <translation>Aus</translation>
    </message>
    <message id="mainwindow.combobox.valExcl">
        <source/>
        <translation>Ausschließen</translation>
    </message>
    <message id="mainwindow.combobox.valOnly">
        <source/>
        <translation>Nur selbes</translation>
    </message>

    <message id="mainwindow.dialog.selectFolder.title">
        <source/>
        <translation>Ordner auswählen</translation>
    </message>

    <message id="mainwindow.default.setName">
        <source/>
        <translation>Standard</translation>
    </message>
    <message id="mainwindow.default.newSetName">
        <source/>
        <translation>Neues Set</translation>
    </message>
    <message id="mainwindow.default.newSetName.Count">
        <source/>
        <comment>%1 = count</comment>
        <translation>Neues Set %1</translation>
    </message>
    <message id="mainwindow.default.cloneSet.RegularExpr">
        <source/>
        <translation>\sKopie(\s\d+)?$</translation>
    </message>
    <message id="mainwindow.default.cloneSet.firstCopy">
        <source/>
        <comment>%1 = basename</comment>
        <translation>%1 Kopie</translation>
    </message>
    <message id="mainwindow.default.cloneSet.copyCount">
        <source/>
        <comment>%1 = basename;  %2 = count</comment>
        <translation>%1 Kopie %2</translation>
    </message>

    <message id="mainwindow.output.hint">
        <source/>
        <translation>Hinweis</translation>
    </message>
    <message id="mainwindow.output.hint.folderExists">
        <source/>
        <comment>%1 = folder name</comment>
        <translation>Verzeichnis
"%1"
ist bereits in der Liste enthalten.</translation>
    </message>

    <message id="mainwindow.output.info">
        <source/>
        <translation>Information</translation>
    </message>
    <message id="mainwindow.output.info.noPictures">
        <source/>
        <translation>Keine Bild-Dateien gefunden.</translation>
    </message>
    <message id="mainwindow.output.info.noDupl">
        <source/>
        <translation>Keine Duplikate gefunden.</translation>
    </message>
    <message id="mainwindow.output.info.noDuplShowOldList">
        <source/>
        <translation>Keine Duplikate gefunden. Alte Liste weiterhin anzeigen?</translation>
    </message>
    <message id="mainwindow.output.info.noManualNoDupl">
        <source/>
        <translation>Keine manuellen 'Kein Duplikat' Einträge gefunden.</translation>
    </message>
    <message id="mainwindow.output.info.DBentriesFiles">
        <source/>
        <comment>%1 = count of found DB entries</comment>
        <translation>%1 'Files' Einträge gefunden found, dessen Dateien nicht mehr vorhanden sind.
Diese Einträge werden jetzt gelöscht</translation>
    </message>
    <message id="mainwindow.output.info.DBentryDuplFiles">
        <source/>
        <comment>%1 = count of found DB duplicates</comment>
        <translation>%1 'Files' Einträge-Duplikate (bzgl. Datei-Pfad) gefunden.
Diese Einträge werden jetzt gelöscht</translation>
    </message>

    <message id="mainwindow.output.confirm">
        <source/>
        <translation>Bestätigung</translation>
    </message>
    <message id="mainwindow.output.confirm.delFolder">
        <source/>
        <comment>%1 = folder name</comment>
        <translation>Verzeichnis
"%1"
wirklich aus der Liste entfernen?</translation>
    </message>
    <message id="mainwindow.output.confirm.cleanDB">
        <source/>
        <translation>'CleanDB' starten?</translation>
    </message>

    <message id="mainwindow.output.deleteConfirm">
        <source/>
        <translation>Löschen bestätigen</translation>
    </message>
    <message id="mainwindow.output.deleteConfirm.delFile">
        <source/>
        <comment>%1 = picture filename</comment>
        <translation>Soll das Bild "%1" wirklich gelöscht werden?</translation>
    </message>

    <message id="mainwindow.output.noDuplConfirm">
        <source/>
        <translation>'No Duplcate' bestätigen</translation>
    </message>
    <message id="mainwindow.output.noDuplConfirm.duplicates">
        <source/>
        <comment>%1 = picture 1 | %2 = picture 2</comment>
        <translation>Sind die beiden Bilder
%1
und
%2
wirklich keine Duplikate?</translation>
    </message>

    <message id="mainwindow.output.noManNoDuplConfirm">
        <source/>
        <translation>Bestätigen: 'Kein-Duplikat' Indizierung löschen</translation>
    </message>
    <message id="mainwindow.output.noManNoDuplConfirm.duplicates">
        <source/>
        <comment>%1 = picture 1 | %2 = picture 2</comment>
        <translation>Wirklich die Indizierung 'Keine Duplikate' für
%1
und
%2
löschen?</translation>
    </message>

    <message id="mainwindow.output.question">
        <source/>
        <translation>Frage</translation>
    </message>
    <message id="mainwindow.output.question.VacuumUsedSpace">
        <source/>
        <comment>%1 = used space | %2 = free space for shrinking</comment>
        <translation>Gesamte Dateigröße: %1
Freier Platz: %2
DB Datei verkleinern (Vacuum)?</translation>
    </message>
    <message id="mainwindow.output.question.optionYes">
        <source/>
        <translation>Ja, DB Datei verkleinern</translation>
    </message>
    <message id="mainwindow.output.question.optionNo">
        <source/>
        <translation>Nein, keine Verkleinerung der DB Datei</translation>
    </message>

    <message id="mainwindow.output.delete">
        <source/>
        <translation>Löschen</translation>
    </message>
    <message id="mainwindow.output.delete.delSet">
        <source/>
        <translation>Set "%1" wirklich löschen?</translation>
    </message>

    <message id="mainwindow.output.warning">
        <source/>
        <translation>Warnung</translation>
    </message>
    <message id="mainwindow.output.warning.tooManyDupl">
        <source/>
        <comment>%1 = count | %2 = limit</comment>
        <translation>Zuviele Duplikats-Bilder (%1).
Bitte Ordner reduzieren oder schärfere Limits setzen.
Liste wird auf %2 begrenzt.</translation>
    </message>
    <message id="mainwindow.output.warning.tooManyManNoDupl">
        <source/>
        <comment>%1 = count | %2 = limit</comment>
        <translation>Zuviele manuelle Kein-Duplikats-Bilder (%1).
Bitte Ordner reduzieren.
Liste wird auf %2 begrenzt.</translation>
    </message>

    <message id="mainwindow.output.DBwarning">
        <source/>
        <translation>DB Warnung</translation>
    </message>
    <message id="mainwindow.output.DBwarning.tooOld">
        <source/>
        <translation>Die DB Version ist zu alt.
Sie kann nicht auf neuen Stand gebracht werden.</translation>
    </message>
    <message id="mainwindow.output.DBwarning.tooNew">
        <source/>
        <translation>Die DB Version ist zu neu.
Diese Programmversion wird damit nicht klar kommen.</translation>
    </message>

    <message id="mainwindow.output.error">
        <source/>
        <translation>Fehler</translation>
    </message>
    <message id="mainwindow.output.error.setExists">
        <source/>
        <comment>%1 = set name</comment>
        <translation>Name "%1" existiert bereits!</translation>
    </message>
    <message id="mainwindow.output.error.fileNotExists">
        <source/>
        <comment>%1 = picture filename</comment>
        <translation>Das Bild "%1" existiert nicht (mehr).</translation>
    </message>
    <message id="mainwindow.output.error.fileNotDeleted">
        <source/>
        <comment>%1 = picture filename</comment>
        <translation>Das Bild "%1" konnte nicht in den Papierkorb verschoben werden.</translation>
    </message>
    <message id="mainwindow.output.error.readTableFiles">
        <source/>
        <comment>%1 = DB error text</comment>
        <translation>Lesen der DB Tabelle 'Files' nicht erfolgreich.
"%1"</translation>
    </message>
    <message id="mainwindow.output.error.DBTransaction">
        <source/>
        <translation>Konnte DB Transaktion nicht starten</translation>
    </message>
    <message id="mainwindow.output.error.deleteID">
        <source/>
        <comment>%1 = ID | %2 = DB error text</comment>
        <translation>Löschen der ID '%1' war nicht erfolgreich.
"%2"</translation>
    </message>
    <message id="mainwindow.output.error.readTableFeatures">
        <source/>
        <comment>%1 = DB error text</comment>
        <translation>Lesen der DB Tabelle 'Features' nicht erfolgreich.
"%1"</translation>
    </message>
    <message id="mainwindow.output.error.readTableScores">
        <source/>
        <comment>%1 = DB error text</comment>
        <translation>Lesen der DB Tabelle 'Scores' nicht erfolgreich.
"%1"</translation>
    </message>
    <message id="mainwindow.output.error.vacuum">
        <source/>
        <comment>%1 = DB error text</comment>
        <translation>'Vacuum' nicht erfolgreich.
"%1"</translation>
    </message>

    <message id="mainwindow.language.title">
        <source/>
        <translation>Sprache auswählen</translation>
    </message>
</context>


<!-- ===== duplicatemodel ===== -->
<context>
     <message id="duplicatemodel.button.OK">
        <source/>
        <translation>OK</translation>
     </message>
     <message id="duplicatemodel.button.Refresh">
        <source/>
        <translation>Auffrischen</translation>
     </message>
     <message id="duplicatemodel.button.Apply">
        <source/>
        <translation>Anwenden</translation>
    </message>

     <message id="duplicatemodel.title.Duplicates">
        <source/>
        <translation>Liste der Duplikate</translation>
     </message>
     <message id="duplicatemodel.title.Duplicates.count">
        <source/>
        <comment>%1 = size</comment>
        <translation>Liste der Duplikate (%1)</translation>
     </message>
     <message id="duplicatemodel.title.Duplicates.overflow">
        <source/>
        <comment>%1 = Max allowed size</comment>
        <translation>Liste der Duplikate (%1) - Überlauf!!!</translation>
     </message>
     <message id="duplicatemodel.title.manNoDupl">
        <source/>
        <translation>'Manuelles Kein Duplikat' - Liste</translation>
     </message>
     <message id="duplicatemodel.title.manNoDupl.count">
        <source/>
        <comment>%1 = Max size</comment>
        <translation>'Manuelles Kein Duplikat' - Liste (%1)</translation>
     </message>
     <message id="duplicatemodel.title.manNoDupl.overflow">
        <source/>
        <comment>%1 = Max allowed size</comment>
        <translation>'Manuelles Kein Duplikat' - Liste (%1) - Überlauf!!!</translation>
     </message>
</context>


<!-- ===== imagebox ===== -->
<context>
     <message id="imagebox.formatting.date">
        <source/>
        <translation>dd.MM.yyyy hh:mm:ss</translation>
     </message>

     <message id="imagebox.label.type">
        <source/>
        <translation>Typ</translation>
     </message>
     <message id="imagebox.label.number">
        <source/>
        <translation>Nummer</translation>
     </message>
     <message id="imagebox.label.score">
        <source/>
        <translation>Score</translation>
     </message>
     <message id="imagebox.label.filename">
        <source/>
        <translation>Dateiname</translation>
     </message>
     <message id="imagebox.label.folder">
        <source/>
        <translation>Verzeichnis</translation>
     </message>
     <message id="imagebox.label.filedate">
        <source/>
        <translation>Datei Datum</translation>
     </message>
     <message id="imagebox.label.filesize">
        <source/>
        <translation>Datei Größe</translation>
     </message>
     <message id="imagebox.label.imageSize">
        <source/>
        <translation>Bild Größe</translation>
     </message>

     <message id="imagebox.data.root">
        <source/>
        <translation>WURZEL</translation>
     </message>
     <message id="imagebox.data.copy">
        <source/>
        <translation>Kopie</translation>
     </message>
     <message id="imagebox.data.copy.extra">
        <source/>
        <comment>%1 = extra text to note the comparison algorithm</comment>
        <translation>Kopie%1</translation>
     </message>
     <message id="imagebox.data.duplicate">
        <source/>
        <translation>DUPLIKAT</translation>
     </message>
     <message id="imagebox.data.duplicate.extra">
        <source/>
        <comment>%1 = extra text to note the comparison algorithm</comment>
        <translation>DUPLIKAT%1</translation>
     </message>
     <message id="imagebox.data.deleted">
        <source/>
        <translation>GELÖSCHT</translation>
     </message>
     <message id="imagebox.data.manNuDupl">
        <source/>
        <comment>%1 = extra text to note the comparison algorithm</comment>
        <translation>Manuell: KEIN DUPLIKAT%1</translation>
     </message>
     <message id="imagebox.data.delManNuDupl">
        <source/>
        <translation>'KEIN DUPLIKAT' Indizierung gelöscht</translation>
     </message>
     <message id="imagebox.data.first">
        <source/>
        <translation>Erstes</translation>
     </message>
     <message id="imagebox.data.second">
        <source/>
        <translation>Zweites</translation>
     </message>
     <message id="imagebox.data.scoreData">
        <source/>
        <comment>%2 = pHash value | %3 = canny value | %1 = ORB score value
                 The programm checks pHash first, then canny then ORB</comment>
        <translation>pHash: %2, canny: %3, orb: %1</translation>
     </message>

     <message id="imagebox.button.open">
        <source/>
        <translation>Öffnen</translation>
     </message>
     <message id="imagebox.button.delete">
        <source/>
        <translation>Löschen</translation>
     </message>
     <message id="imagebox.button.master">
        <source/>
        <translation>-&gt; Master</translation>
     </message>
     <message id="imagebox.button.compare">
        <source/>
        <translation>Vergleichen</translation>
     </message>
     <message id="imagebox.button.noDupl">
        <source/>
        <translation>Kein Duplikat</translation>
     </message>
     <message id="imagebox.button.manNoDuplDel">
        <source/>
        <translation>Manuelles &lt;Kein Duplikat&gt; löschen</translation>
     </message>

     <message id="imagebox.text.compareDiff">
        <source/>
        <translation> (Vergleich durch DIFF)</translation>
     </message>
     <message id="imagebox.text.compareXOR">
        <source/>
        <translation> (Vergleich durch XOR)</translation>
     </message>
</context>


<!-- ===== nomanualparametersdialog ===== -->
<context>
     <message id="nomanualparametersdialog.title.noManualParametersDialog">
        <source/>
        <translation>Parameter für die manuelle 'Keine Duplikate' Liste</translation>
     </message>

     <message id="nomanualparametersdialog.groupBox.gBSortType">
        <source/>
        <translation>Sortierungs-Typ</translation>
     </message>
     <message id="nomanualparametersdialog.groupBox.gBSortDir">
        <source/>
        <translation>Sortierungs-Richtung</translation>
     </message>

     <message id="nomanualparametersdialog.label.min">
        <source/>
        <translation>Min</translation>
     </message>
     <message id="nomanualparametersdialog.label.max">
        <source/>
        <translation>Max</translation>
     </message>
     <message id="nomanualparametersdialog.label.lblListFrom">
        <source/>
        <translation>Liste von ... </translation>
     </message>
     <message id="nomanualparametersdialog.label.lblListTo">
        <source/>
        <translation>bis ...</translation>
     </message>

     <message id="nomanualparametersdialog.radioButton.rbAdded">
        <source/>
        <translation>Zuletzt hinzugefügt ganz oben</translation>
     </message>
     <message id="nomanualparametersdialog.radioButton.rbAlpha">
        <source/>
        <translation>Alphabetisch</translation>
     </message>
     <message id="nomanualparametersdialog.radioButton.rbSize">
        <source/>
        <translation>Datei Größe</translation>
     </message>
     <message id="nomanualparametersdialog.radioButton.rbLastMod">
        <source/>
        <translation>Datei zuletzt geändert</translation>
     </message>
     <message id="nomanualparametersdialog.radioButton.rbImageSize">
        <source/>
        <translation>Bild Größe</translation>
     </message>
     <message id="nomanualparametersdialog.radioButton.rbAsc">
        <source/>
        <translation>Aufsteigend</translation>
     </message>
     <message id="nomanualparametersdialog.radioButton.rbDesc">
        <source/>
        <translation>Absteigend</translation>
     </message>
</context>


</TS>
