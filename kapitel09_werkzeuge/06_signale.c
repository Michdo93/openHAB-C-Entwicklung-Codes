// Kapitel 9: Signale – ein Dienst beendet sich sauber bei Strg+C oder systemctl stop
#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static volatile sig_atomic_t beenden = 0; // das Einzige, was ein Handler sicher setzen darf

static void handler(int sig) { beenden = sig; }

int main(int argc, char** argv) {
    (void)argv;
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handler;
    sigaction(SIGINT, &sa, NULL);  // Strg+C
    sigaction(SIGTERM, &sa, NULL); // systemctl stop, kill

    int sekunden = argc > 1 ? 0 : 3; // mit Argument: bis zum Signal laufen
    if (sekunden) alarm(sekunden);   // ohne: nach 3 s schickt alarm() SIGALRM
    sigaction(SIGALRM, &sa, NULL);

    printf("Dienst läuft (PID %d) – Strg+C beendet\n", (int)getpid());
    fflush(stdout);
    int runden = 0;
    while (!beenden) {
        pause(); // schläft, bis ein Signal kommt
        runden++;
    }
    printf("Signal %d (%s) nach %d Runde(n): aufräumen und beenden\n", (int)beenden,
           beenden == SIGALRM   ? "SIGALRM"
           : beenden == SIGTERM ? "SIGTERM"
                                : "SIGINT",
           runden);
    return 0;
}
