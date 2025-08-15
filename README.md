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

#### Kommandozeilen-Parameter

Die ausführbare Datei akzeptiert folgende optionale Parameter:

- `--test [Nr]` führt eine Reihe interner `perft`-Tests bis zur Tiefe 5 durch und beendet die Engine danach. Wird optional eine Testnummer `Nr` angegeben, so wird nur der entsprechende Test ausgeführt.
- `--deep-test` führt sämtliche eingebauten `perft`-Tests in voller Tiefe aus.

Ohne Parameter startet die Engine im normalen UCI-Modus und wartet auf Befehle von der Standardeingabe.

Beispiel:

```sh
./bin/chessna --test
./bin/chessna --test 3
```

### Als Lichess-Bot

Das Projekt enthält ein Python-Skript, um die CHESSna-Engine als Bot auf [Lichess.org](https://lichess.org) zu betreiben.

**Voraussetzungen:**
- Python 3
- Ein Lichess-Account mit einem API-Token

**1. Installation der Abhängigkeiten:**

Installieren Sie die benötigte Python-Bibliothek mit dem folgenden Befehl:
```sh
cd lichess-bot/
source ./bin/activate
python3 -m pip install -r requirements.txt
```

**2. Konfiguration:**

Öffnen Sie die Datei `lichess_bot.py` und tragen Sie Ihren Lichess API-Token in die Variable `LICHESS_API_TOKEN` ein. Sie können einen Token unter [lichess.org/account/oauth/token/create](https://lichess.org/account/oauth/token/create) erstellen. Stellen Sie sicher, dass der Token die Berechtigung "Spiele als Bot spielen" (`bot:play`) hat.

```python
# lichess_bot.py

# ...
LICHESS_API_TOKEN = "DEIN_API_TOKEN_HIER_EINFÜGEN"
# ...
```

Passen Sie bei Bedarf auch den Pfad zur Engine-Executable in der Variable `ENGINE_PATH` an.

**3. Bot starten:**

Führen Sie das Skript aus, um den Bot zu starten:
```sh
python lichess_bot.py
```

Der Bot verbindet sich dann mit Lichess, wartet auf Herausforderungen und spielt automatisch Partien.

## Besonderheiten

### Benutzerdefinierte Befehle

Zusätzlich zu den Standard-UCI-Befehlen unterstützt CHESSna einige benutzerdefinierte Befehle, wenn sie manuell über die Kommandozeile eingegeben werden:

-   `perft <Tiefe>`
    Führt einen Performance-Test der Zuggenerierung für die aktuelle Stellung bis zur angegebenen Tiefe (`<Tiefe>`) durch und gibt die Anzahl der Knoten aus.

-   `board`
    Gibt eine einfache Textdarstellung des aktuellen Bretts auf der Konsole aus. Nützlich für Debugging-Zwecke.

-   `perftdiv <Tiefe>`
    Führt einen `perft`-Test mit zusätzlicher Aufschlüsselung nach einzelnen Zügen durch.

-   `help`
    Zeigt eine Übersicht aller benutzerdefinierten Befehle an.

### Bekannte Probleme

-   **`stop`-Befehl**: Der `stop`-Befehl beendet zwar die Berechnung, gibt aber derzeit nicht den besten gefundenen Zug zurück, sondern einen fest kodierten Wert (`e2e4`).

### Netzwerk-Spiel (für fortgeschrittene Benutzer)

Die Skripte `bin/listen.sh` und `bin/engine.sh` verwenden `socat`, um die Engine-Kommunikation über ein TCP-Netzwerk zu leiten. Dies ermöglicht es, die GUI auf einem Computer und die CHESSna-Engine auf einem anderen laufen zu lassen. Diese Skripte sind für fortgeschrittene Anwendungsfälle gedacht und erfordern `socat`.
