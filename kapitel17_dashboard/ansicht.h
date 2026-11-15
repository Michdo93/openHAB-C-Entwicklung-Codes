// Kapitel 17: Zwei Views für dasselbe Model und denselben Controller
#ifndef ANSICHT_H
#define ANSICHT_H

#include "controller.h"
#include "model.h"

int ansicht_text(model* m, controller* c, int dauer); // Zeilen, Befehle über stdin
int ansicht_ncurses(model* m, controller* c);         // Vollbild im Terminal

#endif
