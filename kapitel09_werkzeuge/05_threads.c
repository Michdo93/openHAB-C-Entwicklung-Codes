// Kapitel 9: Threads mit pthreads – Mutex und Bedingungsvariable
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

// Ein gemeinsamer Briefkasten: ein Thread legt Zustände hinein, der andere holt sie ab
typedef struct {
    pthread_mutex_t sperre;
    pthread_cond_t neu;
    char state[32];
    int vorhanden, ende;
} briefkasten;

static briefkasten kasten = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, "", 0,
                              0 };

static void* sensor(void* arg) {
    const char* werte[] = { "21.5 °C", "21.7 °C", "22.0 °C" };
    for (int i = 0; i < 3; i++) {
        usleep(200 * 1000);
        pthread_mutex_lock(&kasten.sperre);
        snprintf(kasten.state, sizeof kasten.state, "%s", werte[i]);
        kasten.vorhanden = 1;
        pthread_cond_signal(&kasten.neu); // wartenden Thread wecken
        pthread_mutex_unlock(&kasten.sperre);
    }
    pthread_mutex_lock(&kasten.sperre);
    kasten.ende = 1;
    pthread_cond_signal(&kasten.neu);
    pthread_mutex_unlock(&kasten.sperre);
    printf("Sensor (%s) fertig\n", (const char*)arg);
    return NULL;
}

int main(void) {
    pthread_t t;
    pthread_create(&t, NULL, sensor, "Werkstatt");
    pthread_mutex_lock(&kasten.sperre);
    for (;;) {
        while (!kasten.vorhanden && !kasten.ende) // Schleife gegen falsches Aufwachen
            pthread_cond_wait(&kasten.neu, &kasten.sperre);
        if (kasten.vorhanden) {
            printf("neuer State: %s\n", kasten.state);
            kasten.vorhanden = 0;
        } else if (kasten.ende) {
            break;
        }
    }
    pthread_mutex_unlock(&kasten.sperre);
    pthread_join(t, NULL); // auf das Ende des Threads warten
    return 0;
}
