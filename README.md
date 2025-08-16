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

#### Kommandozeilen-Parameter

Die ausführbare Datei akzeptiert folgende optionale Parameter:

- `--perft [Nr]` führt eine Reihe interner `perft`-Tests bis zur Tiefe 5 durch und beendet die Engine danach. Die Tests werden dabei auf alle verfügbaren CPU-Kerne verteilt, um die Ausführung zu beschleunigen. Wird optional eine Testnummer `Nr` angegeben, so wird nur der entsprechende Test ausgeführt.
- `--full-perft` führt sämtliche eingebauten `perft`-Tests in voller Tiefe aus.
- `--test` führt eine kleine Test-Suite aus (FEN-Ladevorgänge, `perft`-Tests und Bewertungsbeispiele).

Ohne Parameter startet die Engine im normalen UCI-Modus und wartet auf Befehle von der Standardeingabe.

Beispiel:

```sh
./bin/chessna --perft
./bin/chessna --perft 3
./bin/chessna --test
```

#### Als Lichess-Bot

Das Projekt enthält ein Python-Skript, um die CHESSna-Engine als Bot auf [Lichess.org](https://lichess.org) zu betreiben.

**Voraussetzungen:**
- Python 3
- Ein Lichess-Account mit einem API-Token

**1. Installation der Abhängigkeiten:**

Installieren Sie die benötigte Python-Bibliothek mit dem folgenden Befehl:
```sh
cd lichess-bot/
source ./bin/activate
python3.9 -m pip install -r requirements.txt
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
./start-lichess-bot.sh
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

-   `evaluate`
    Bewertet die aktuelle Stellung und gibt den Wert in Centipawns aus.

-   `Zug` (z. B. `e2e4`)
    Ein Zug in UCI-Notation kann direkt eingegeben werden, um ihn auf dem Brett auszuführen.

-   `help`
    Zeigt eine Übersicht aller benutzerdefinierten Befehle an.
