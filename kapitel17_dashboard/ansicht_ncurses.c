// Kapitel 17: Die ncurses-View – Vollbild, Pfeiltasten, Neuzeichnen bei Änderungen.
// ncurses ist nicht thread-sicher: Der Beobachter setzt nur ein Flag, gezeichnet wird
// ausschließlich im Hauptthread.
#include <locale.h>
#include <ncurses.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include "ansicht.h"

static atomic_int neu_zeichnen = 1;

static void geaendert(model* m, int index, void* kontext) {
    (void)m;
    (void)index;
    (void)kontext;
    atomic_store(&neu_zeichnen, 1);
}

static void zeichnen(model* m, controller* c, int auswahl) {
    erase();
    attron(A_BOLD);
    mvprintw(0, 0, " ohdash – openHAB im Terminal");
    attroff(A_BOLD);
    mvprintw(1, 0, " Pfeile: wählen  Enter: schalten  +/-: dimmen  q: Ende");
    for (int i = 0; i < model_anzahl(m); i++) {
        eintrag e = model_eintrag(m, i);
        if (i == auswahl) attron(A_REVERSE);
        mvprintw(3 + i, 1, " %-20.20s %-18.18s %-14.14s ", e.label, e.typ, e.state);
        if (i == auswahl) attroff(A_REVERSE);
        int prozent = 0;
        if (!strcmp(e.typ, "Dimmer") || !strcmp(e.typ, "Rollershutter"))
            prozent = atoi(e.state);
        else if (!strcmp(e.typ, "Switch"))
            prozent = strcmp(e.state, "ON") ? 0 : 100;
        mvhline(3 + i, 60, '#', prozent / 10); // kleiner Balken
    }
    if (controller_fehler(c))
        mvprintw(4 + model_anzahl(m), 1, "Fehler: %s", controller_fehler(c));
    refresh();
}

int ansicht_ncurses(model* m, controller* c) {
    setlocale(LC_ALL, ""); // UTF-8 (Umlaute, °C) richtig darstellen
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(200); // getch wartet höchstens 200 ms
    model_beobachten(m, geaendert, NULL);
    int auswahl = 0, taste;
    while ((taste = getch()) != 'q') {
        int n = model_anzahl(m);
        switch (taste) {
        case KEY_UP:
            if (auswahl > 0) auswahl--;
            break;
        case KEY_DOWN:
            if (auswahl < n - 1) auswahl++;
            break;
        case '\n':
        case '\r':
        case KEY_ENTER:
            controller_umschalten(c, auswahl);
            break;
        case '+':
            controller_aendern(c, auswahl, 10);
            break;
        case '-':
            controller_aendern(c, auswahl, -10);
            break;
        case ERR:
            if (!atomic_exchange(&neu_zeichnen, 0)) continue;
            break;
        }
        zeichnen(m, c, auswahl);
    }
    endwin();
    return 0;
}
