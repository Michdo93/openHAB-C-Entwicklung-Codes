# openHAB & C-Entwicklung – Begleit-Repository

Beispiele zum Buch „openHAB & C-Entwicklung“ (Reihe „openHAB REST API“). Alle Programme
verwenden [c-openhab-rest-client](https://github.com/Michdo93/c-openhab-rest-client) und, für
die Tests, [c-openhab-test-suite](https://github.com/Michdo93/c-openhab-test-suite) in Version
2.0.0 – CMake holt beide beim ersten Übersetzen von GitHub.

## Voraussetzungen

- Linux (Ubuntu 26.04, Debian, Raspberry Pi OS) oder WSL; C11-Compiler, CMake 3.16+, Git
  ```bash
  sudo apt install build-essential cmake git libcurl4-openssl-dev libncurses-dev valgrind
  ```
- openHAB 5 mit der Übungskonfiguration aus `openhab-conf/` (Kapitel 1 des Buchs)
- ein API-Token (Main UI → Profil → API-Tokens)

## Installation

```bash
git clone https://github.com/Michdo93/openHAB-C-Entwicklung-Codes.git
cd openHAB-C-Entwicklung-Codes
cp .env.example .env                 # Adresse und Token eintragen
./starte.sh kapitel10_erste_schritte/01_verbinden.c
```

`starte.sh` übersetzt beim ersten Aufruf alle Beispiele nach `build/`, lädt `.env` und startet
das gewünschte Programm. Von Hand: `cmake -S . -B build && cmake --build build -j`, danach
liegt jedes Beispiel unter `build/<ordner>/<name>`. Die Datei `.env` steht in `.gitignore` –
Tokens gehören nicht ins Repository.

## Inhalt

| Ordner | Inhalt |
|---|---|
| `kapitel08_c/` | Werte und Typen, Kontrollfluss, Funktionen, Zeiger und Arrays, Zeichenketten, Strukturen, benannte Argumente |
| `kapitel09_werkzeuge/` | Speicher, cJSON, libcurl, Fehlersuche (AddressSanitizer, valgrind), Threads, Signale, Dateien und Prozesse |
| `kapitel10_erste_schritte/` | Verbinden, Items lesen, Commands, States, Fehler, Threads, optionale Parameter |
| `kapitel11_items/` | Items anlegen, viele Items, Metadaten, semantisches Modell, Gruppen und Verlauf |
| `kapitel12_things/` | Add-ons, Thing-Typen, Discovery und Inbox, Things, Links, Actions |
| `kapitel13_regeln/` | Regeln lesen, anlegen, steuern, Zeitpläne und Skripte, Modultypen |
| `kapitel14_daten/` | Persistenz, Testdaten, Verlauf als Textdiagramm, Daten löschen, Transformationen, CSV |
| `kapitel15_oberflaechen/` | Sitemaps lesen und anlegen, Sitemap-Ereignisse, Sprache, System |
| `kapitel16_ereignisse/` | SSE roh, Ereignisketten, Ereignisarten, State-Tracker, robuster Listener, Verteiler |
| `kapitel17_dashboard/` | `ohdash`: Terminal-Dashboard nach Model-View-Controller (ncurses und Text) |
| `kapitel18_geraet/` | `ohgeraet`: der Raspberry Pi als Gerät, GPIO mit libgpiod 2, systemd-Unit |
| `kapitel19_tests/` | CTest: Unit-Tests mit Attrappe, Integrationstests mit der Test-Suite, Vorab-Test |
| `kapitel20_projekte/` | `ohctl` für die Kommandozeile, `ohplugin` für Unity/HoloLens, Cross-Kompilierung |
| `openhab-conf/` | Übungskonfiguration (Items, Things, Regeln, Sitemap, Persistenz) |
| `.github/workflows/` | GitHub Actions: Tests mit einem frischen openHAB bei jedem Push |

## Programme aus mehreren Dateien

```bash
build/kapitel17_dashboard/ohdash                       # Vollbild; --text für Zeilen
build/kapitel18_geraet/ohgeraet --intervall=30         # bis Strg+C; --ausgang=gpiochip0:17
build/kapitel20_projekte/ohctl items 'Wohnzimmer_*'
ctest --test-dir build -L unit                         # Tests ohne openHAB
ctest --test-dir build -T memcheck                     # alle Tests unter valgrind
```

| Option (CMake) | Wirkung |
|---|---|
| `-DMIT_ASAN=ON` | AddressSanitizer und UndefinedBehaviorSanitizer (Kapitel 9) |
| `-DMIT_GPIOD=ON` | `ohgeraet` schaltet eine echte GPIO-Leitung (libgpiod 2, `libgpiod-dev`) |
| `-DCMAKE_TOOLCHAIN_FILE=kapitel20_projekte/aarch64-linux-gnu.cmake` | für den Raspberry Pi (64 Bit) übersetzen |

## Zum Buch

„openHAB & C-Entwicklung – Dienste, Steuerungen und Embedded-Programme mit der openHAB REST API,
Server-Sent Events und dem c-openhab-rest-client“, Michael Christian Dörflinger, Reihe
„openHAB REST API“. Jede Datei, die im Buch abgedruckt ist, liegt hier genau so.

> **Achtung:** Die Beispiele schalten Items und legen Items, Things, Regeln und Sitemaps an.
> Nur gegen eine Übungsinstanz mit `openhab-conf/` ausführen, nie gegen das eigene Zuhause.

## Lizenz

MIT – siehe [LICENSE](LICENSE).
