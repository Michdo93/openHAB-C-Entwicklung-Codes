// Kapitel 19: Unit-Test des Models (Kapitel 17) – Beobachter werden benachrichtigt
#include "model.h"
#include "testhilfe.h"

static int aufrufe = 0, letzter = -1;

static void beobachter(model* m, int index, void* kontext) {
    (void)m;
    aufrufe++;
    letzter = index;
    (*(int*)kontext)++;
}

int main(void) {
    model* m = model_neu();
    int zaehler = 0;
    PRUEFE(model_hinzufuegen(m, "Wohnzimmer_Licht", "Decke", "Switch", "OFF") == 0);
    PRUEFE(model_hinzufuegen(m, "Wohnzimmer_Dimmer", "Lampe", "Dimmer", "0") == 1);
    model_beobachten(m, beobachter, &zaehler);

    PRUEFE(model_suche(m, "Wohnzimmer_Dimmer") == 1);
    PRUEFE(model_suche(m, "Gibtsnicht") == -1);
    PRUEFE(model_setze_state(m, "Wohnzimmer_Dimmer", "40") == 1);
    PRUEFE(aufrufe == 1 && letzter == 1 && zaehler == 1);
    PRUEFE(model_setze_state(m, "Wohnzimmer_Dimmer", "40") == 0); // keine Änderung
    PRUEFE(aufrufe == 1);                                         // kein Aufruf
    PRUEFE(model_setze_state(m, "Gibtsnicht", "ON") == 0);
    PRUEFE_TEXT(model_eintrag(m, 1).state, "40");
    PRUEFE_TEXT(model_eintrag(m, 5).name, ""); // außerhalb: leer

    // Lange Werte werden abgeschnitten, nie über den Puffer hinaus geschrieben
    char lang[200];
    memset(lang, 'x', sizeof lang - 1);
    lang[sizeof lang - 1] = '\0';
    model_setze_state(m, "Wohnzimmer_Licht", lang);
    PRUEFE(strlen(model_eintrag(m, 0).state) == sizeof(((eintrag*)0)->state) - 1);
    model_frei(m);
    return test_ende("test_model");
}
