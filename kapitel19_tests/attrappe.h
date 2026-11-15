// Kapitel 19: Eine Attrappe für openHAB – ein winziger HTTP-Server im Testprogramm.
// Er beantwortet jede Anfrage mit einer vorbereiteten Antwort und merkt sich,
// was der Client geschickt hat. So lässt sich Code testen, der openHAB aufruft.
#ifndef ATTRAPPE_H
#define ATTRAPPE_H

typedef struct attrappe attrappe;

attrappe* attrappe_starten(void);            // lauscht auf 127.0.0.1:<frei>
const char* attrappe_url(const attrappe* a); // "http://127.0.0.1:<port>"
// Antwort für alle Pfade, die mit pfad beginnen (zuletzt festgelegte gewinnt)
void attrappe_antwort(attrappe* a, const char* methode, const char* pfad, int status,
                      const char* body);
int attrappe_anfragen(attrappe* a); // Anzahl bisheriger Anfragen
// Die i-te Anfrage als "METHODE PFAD BODY" (ohne /rest-Präfix zu entfernen)
const char* attrappe_anfrage(attrappe* a, int i);
const char* attrappe_letzte(attrappe* a);
void attrappe_stoppen(attrappe* a);

#endif
