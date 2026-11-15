// Kapitel 17: Der Controller – verbindet Model und openHAB.
// Commands gehen per REST hinaus, States kommen nur über den Ereignisstrom zurück:
// openHAB bleibt die einzige Quelle der Wahrheit.
#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <openhab/openhab.h>

#include "model.h"

typedef struct controller controller;

controller* controller_neu(openhab_client_t* client, model* m);
int controller_laden(controller* c, const char* gruppe); // Anzahl Items oder -1
int controller_starten(controller* c);                   // Ereignisstrom im Hintergrund
void controller_umschalten(controller* c, int index);    // Switch, Dimmer, Rollladen
void controller_aendern(controller* c, int index, int schritt); // Dimmer +/- Prozent
void controller_stoppen(controller* c);
const char* controller_fehler(controller* c); // letzte Fehlermeldung oder NULL

#endif
