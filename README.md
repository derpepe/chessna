# CHESSna 2

CHESSna 2 ist eine einfache Schach-Engine, die in C++ geschrieben ist und das Universal Chess Interface (UCI) Protokoll zur Kommunikation verwendet.

Dieses Dokument beschreibt, wie die Engine kompiliert und verwendet wird, und geht auf ihre Besonderheiten ein.

## Bedienung

### Kompilieren

Um die Engine zu kompilieren, wird ein C++-Compiler benötigt, der C++11 unterstützt (z. B. g++). Führen Sie dann einfach den folgenden Befehl im Hauptverzeichnis aus:

```sh
make
```

Dadurch wird die ausführbare Datei `chessna` im Verzeichnis `bin` erstellt.

### Ausführen

Es gibt mehrere Möglichkeiten, die CHESSna-Engine auszuführen:

#### Mit einer UCI-GUI (Empfohlen)

CHESSna ist eine UCI-Engine und sollte daher mit jeder UCI-kompatiblen grafischen Benutzeroberfläche (GUI) funktionieren. Das Repository enthält die bekannte Arena-GUI (im Verzeichnis `prgr/arena_3`), die unter Windows lauffähig ist.

Für Linux- oder macOS-Benutzer gibt es ein Skript, um Arena mit Wine zu starten:

```sh
./bin/arena.command
```

In der GUI können Sie dann die `chessna.exe` (oder die kompilierte `chessna` auf Linux/macOS) als neue UCI-Engine hinzufügen.

#### Direkt auf der Kommandozeile

Für Entwicklungs- oder Testzwecke kann die Engine direkt von der Kommandozeile aus gestartet werden:

```sh
./bin/chessna
```

Die Engine wartet dann auf UCI-Befehle von der Standardeingabe.

#### Test-Modus

Ein spezieller Testmodus führt eine Reihe von internen `perft`-Tests durch, um die Korrektheit der Zuggenerierung zu überprüfen. Dieser Modus kann wie folgt gestartet werden:

```sh
./bin/chessna --test
```

## Besonderheiten

### Benutzerdefinierte Befehle

Zusätzlich zu den Standard-UCI-Befehlen unterstützt CHESSna einige benutzerdefinierte Befehle, wenn sie manuell über die Kommandozeile eingegeben werden:

-   `perft <Tiefe>`
    Führt einen Performance-Test der Zuggenerierung für die aktuelle Stellung bis zur angegebenen Tiefe (`<Tiefe>`) durch und gibt die Anzahl der Knoten aus.

-   `board`
    Gibt eine einfache Textdarstellung des aktuellen Bretts auf der Konsole aus. Nützlich für Debugging-Zwecke.

-   `help`
    Zeigt eine kurze Hilfe-Nachricht für die benutzerdefinierten Befehle an.

### Bekannte Probleme

-   **`stop`-Befehl**: Der `stop`-Befehl beendet zwar die Berechnung, gibt aber derzeit nicht den besten gefundenen Zug zurück, sondern einen fest kodierten Wert (`e2e4`).

### Netzwerk-Spiel (für fortgeschrittene Benutzer)

Die Skripte `bin/listen.sh` und `bin/engine.sh` verwenden `socat`, um die Engine-Kommunikation über ein TCP-Netzwerk zu leiten. Dies ermöglicht es, die GUI auf einem Computer und die CHESSna-Engine auf einem anderen laufen zu lassen. Diese Skripte sind für fortgeschrittene Anwendungsfälle gedacht und erfordern `socat`.
