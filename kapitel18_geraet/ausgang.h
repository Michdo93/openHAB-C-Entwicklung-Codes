// Kapitel 18: Ein Schaltausgang – eine GPIO-Leitung oder eine Attrappe.
//   "gpiochip0:17"  Leitung 17 des Chips gpiochip0 (libgpiod 2, nur mit MIT_GPIOD)
//   "attrappe"      merkt sich den Zustand nur (zum Entwickeln ohne Pi)
#ifndef AUSGANG_H
#define AUSGANG_H

typedef struct ausgang ausgang;

ausgang* ausgang_oeffnen(const char* beschreibung); // NULL bei Fehler
int ausgang_setzen(ausgang* a, int an);             // 0 = ok
int ausgang_lesen(const ausgang* a);
const char* ausgang_name(const ausgang* a);
void ausgang_schliessen(ausgang* a);

#endif
