// Kapitel 17: Das Model – der Zustand der Items, die das Dashboard zeigt.
// Es kennt weder openHAB noch das Terminal; wer sich für Änderungen interessiert,
// meldet sich als Beobachter an (Observer-Muster).
#ifndef MODEL_H
#define MODEL_H

typedef struct {
    char name[64];
    char label[64];
    char typ[32];
    char state[64];
} eintrag;

typedef struct model model;

// Beobachter: wird nach jeder Änderung aufgerufen – aus dem Thread, der sie meldet
typedef void (*model_beobachter)(model* m, int index, void* kontext);

model* model_neu(void);
void model_frei(model* m);
int model_hinzufuegen(model* m, const char* name, const char* label, const char* typ,
                      const char* state);
int model_anzahl(model* m);
int model_suche(model* m, const char* name); // Index oder -1
eintrag model_eintrag(model* m, int index);  // Kopie – sicher für jeden Thread
int model_setze_state(model* m, const char* name, const char* state); // 1 = geändert
void model_beobachten(model* m, model_beobachter fn, void* kontext);

#endif
