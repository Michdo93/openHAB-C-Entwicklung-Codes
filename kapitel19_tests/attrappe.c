// Kapitel 19: Die Attrappe – Sockets, ein Thread, ein Mutex
#define _POSIX_C_SOURCE 200809L
#include "attrappe.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_ANTWORTEN 32
#define MAX_ANFRAGEN  128

typedef struct {
    char methode[8], pfad[128];
    int status;
    char* body;
} antwort;

struct attrappe {
    int sock;
    char url[64];
    pthread_t thread;
    pthread_mutex_t sperre;
    antwort antworten[MAX_ANTWORTEN];
    int antwort_anzahl;
    char* anfragen[MAX_ANFRAGEN];
    int anfrage_anzahl;
};

// Eine Anfrage lesen: Kopfzeilen bis zur Leerzeile, dann Content-Length Bytes Body
static char* lese_anfrage(int fd) {
    size_t groesse = 0, kapazitaet = 8192;
    char* p = malloc(kapazitaet);
    long laenge = -1;
    char* ende = NULL;
    for (;;) {
        ssize_t n = recv(fd, p + groesse, kapazitaet - groesse - 1, 0);
        if (n <= 0) break;
        groesse += n;
        p[groesse] = '\0';
        if (!ende && (ende = strstr(p, "\r\n\r\n"))) {
            const char* cl = strstr(p, "Content-Length:"); // so schreibt es libcurl
            laenge = cl && cl < ende ? atol(cl + 15) : 0;
        }
        if (ende && (long)(groesse - (ende + 4 - p)) >= laenge) break;
        if (groesse + 1 >= kapazitaet) break;
    }
    return p;
}

static void* bedienen(void* arg) {
    attrappe* a = arg;
    int fd;
    while ((fd = accept(a->sock, NULL, NULL)) >= 0) {
        char* text = lese_anfrage(fd);
        char methode[8] = "", pfad[256] = "";
        sscanf(text, "%7s %255s", methode, pfad);
        char* body = strstr(text, "\r\n\r\n");
        body = body ? body + 4 : "";

        pthread_mutex_lock(&a->sperre);
        if (a->anfrage_anzahl < MAX_ANFRAGEN) {
            size_t n = strlen(methode) + strlen(pfad) + strlen(body) + 3;
            char* eintrag = malloc(n);
            snprintf(eintrag, n, "%s %s %s", methode, pfad, body);
            a->anfragen[a->anfrage_anzahl++] = eintrag;
        }
        antwort passend = { "", "", 200, "{}" };
        for (int i = 0; i < a->antwort_anzahl; i++) {
            antwort* x = &a->antworten[i];
            if (!strcmp(x->methode, methode) && !strncmp(pfad, x->pfad, strlen(x->pfad)))
                passend = *x;
        }
        char kopf[256];
        int k = snprintf(kopf, sizeof kopf,
                         "HTTP/1.1 %d X\r\nContent-Type: application/json\r\n"
                         "Content-Length: %zu\r\nConnection: close\r\n\r\n",
                         passend.status, strlen(passend.body));
        send(fd, kopf, k, 0);
        send(fd, passend.body, strlen(passend.body), 0);
        pthread_mutex_unlock(&a->sperre);
        free(text);
        close(fd);
    }
    return NULL;
}

attrappe* attrappe_starten(void) {
    attrappe* a = calloc(1, sizeof *a);
    pthread_mutex_init(&a->sperre, NULL);
    a->sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in adr = { .sin_family = AF_INET,
                               .sin_port = 0 }; // Port 0: frei wählen
    adr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    socklen_t laenge = sizeof adr;
    if (bind(a->sock, (struct sockaddr*)&adr, sizeof adr) || listen(a->sock, 16) ||
        getsockname(a->sock, (struct sockaddr*)&adr, &laenge)) {
        perror("attrappe");
        exit(1);
    }
    snprintf(a->url, sizeof a->url, "http://127.0.0.1:%d", ntohs(adr.sin_port));
    pthread_create(&a->thread, NULL, bedienen, a);
    return a;
}

const char* attrappe_url(const attrappe* a) { return a->url; }

void attrappe_antwort(attrappe* a, const char* methode, const char* pfad, int status,
                      const char* body) {
    pthread_mutex_lock(&a->sperre);
    if (a->antwort_anzahl < MAX_ANTWORTEN) {
        antwort* x = &a->antworten[a->antwort_anzahl++];
        snprintf(x->methode, sizeof x->methode, "%s", methode);
        snprintf(x->pfad, sizeof x->pfad, "%s", pfad);
        x->status = status;
        x->body = strdup(body);
    }
    pthread_mutex_unlock(&a->sperre);
}

int attrappe_anfragen(attrappe* a) {
    pthread_mutex_lock(&a->sperre);
    int n = a->anfrage_anzahl;
    pthread_mutex_unlock(&a->sperre);
    return n;
}

const char* attrappe_anfrage(attrappe* a, int i) {
    pthread_mutex_lock(&a->sperre);
    const char* r = i >= 0 && i < a->anfrage_anzahl ? a->anfragen[i] : NULL;
    pthread_mutex_unlock(&a->sperre);
    return r;
}

const char* attrappe_letzte(attrappe* a) {
    return attrappe_anfrage(a, attrappe_anfragen(a) - 1);
}

void attrappe_stoppen(attrappe* a) {
    shutdown(a->sock, SHUT_RDWR); // accept() kehrt mit Fehler zurück
    close(a->sock);
    pthread_join(a->thread, NULL);
    for (int i = 0; i < a->anfrage_anzahl; i++) free(a->anfragen[i]);
    for (int i = 0; i < a->antwort_anzahl; i++) free(a->antworten[i].body);
    pthread_mutex_destroy(&a->sperre);
    free(a);
}
