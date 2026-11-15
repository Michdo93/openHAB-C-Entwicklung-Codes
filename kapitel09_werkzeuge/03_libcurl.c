// Kapitel 9: HTTP mit libcurl – was die Bibliothek unter der Haube tut
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* daten;
    size_t laenge;
} puffer;

// libcurl liefert die Antwort stückweise; die Rückruffunktion hängt sie an
static size_t sammeln(char* stueck, size_t groesse, size_t anzahl, void* ziel) {
    puffer* p = ziel;
    size_t n = groesse * anzahl;
    char* neu = realloc(p->daten, p->laenge + n + 1);
    if (!neu) return 0; // 0 bricht die Übertragung ab
    memcpy(neu + p->laenge, stueck, n);
    p->daten = neu;
    p->laenge += n;
    p->daten[p->laenge] = '\0';
    return n;
}

static long anfrage(const char* methode, const char* url, const char* token,
                    const char* body, puffer* antwort) {
    CURL* curl = curl_easy_init();
    struct curl_slist* kopf = NULL;
    char auth[512];
    snprintf(auth, sizeof auth, "Authorization: Bearer %s", token ? token : "");
    kopf = curl_slist_append(kopf, auth);
    kopf = curl_slist_append(kopf, "Content-Type: text/plain");
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, methode);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, kopf);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, sammeln);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, antwort);
    if (body) curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    long status = 0;
    CURLcode rc = curl_easy_perform(curl);
    if (rc == CURLE_OK)
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    else
        fprintf(stderr, "libcurl: %s\n", curl_easy_strerror(rc));
    curl_slist_free_all(kopf);
    curl_easy_cleanup(curl);
    return status;
}

int main(void) {
    const char* basis =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    const char* token = getenv("OPENHAB_TOKEN");
    char url[256];
    curl_global_init(CURL_GLOBAL_DEFAULT); // einmal je Programm

    snprintf(url, sizeof url, "%s/rest/items/Wohnzimmer_Licht", basis);
    puffer p = { 0 };
    long status = anfrage("POST", url, token, "ON", &p); // Command = POST mit Text
    printf("POST %s -> HTTP %ld\n", url + strlen(basis), status);
    free(p.daten);

    snprintf(url, sizeof url, "%s/rest/items/Wohnzimmer_Licht/state", basis);
    p = (puffer){ 0 };
    status = anfrage("GET", url, token, NULL, &p);
    printf("GET  %s -> HTTP %ld: %s\n", url + strlen(basis), status,
           p.daten ? p.daten : "");
    free(p.daten);

    curl_global_cleanup();
    return status == 200 ? 0 : 1;
}
