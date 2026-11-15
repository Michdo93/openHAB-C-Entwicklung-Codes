// Kapitel 20: ohctl – openHAB auf der Kommandozeile, für Menschen und für Skripte
//   ohctl [--json] info | items [Muster] | get Item | send Item Command
//         | update Item State | watch [Item…] | things | rules | run Regel
// Exit-Code: 0 = erledigt, 1 = openHAB meldet einen Fehler, 2 = falscher Aufruf
#define _POSIX_C_SOURCE 200809L
#include <fnmatch.h>
#include <openhab/openhab.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int json = 0; // --json: Antworten unverändert ausgeben
static openhab_sse_t* volatile strom;

static void stopp(int sig) {
    (void)sig;
    if (strom) openhab_sse_interrupt(strom);
}

static int nutzung(void) {
    fprintf(stderr,
            "Aufruf: ohctl [--json] BEFEHL …\n"
            "  info                  openHAB-Version und System\n"
            "  items [Muster]        Items, z. B. 'Wohnzimmer_*'\n"
            "  get ITEM              Zustand lesen\n"
            "  send ITEM COMMAND     Command senden\n"
            "  update ITEM STATE     Zustand melden\n"
            "  watch [ITEM …]        Änderungen verfolgen (Strg+C beendet)\n"
            "  things | rules        Things bzw. Regeln mit Status\n"
            "  run REGEL             Regel sofort ausführen\n"
            "Umgebung: OPENHAB_URL, OPENHAB_TOKEN\n");
    return 2;
}

// Gemeinsamer Ausgang: Fehler nach stderr und Exit-Code 1, sonst JSON auf Wunsch
static int fertig(cJSON* r) {
    int rc = openhab_is_error(r) ? 1 : 0;
    if (rc)
        fprintf(stderr, "ohctl: %s\n", openhab_error(r));
    else if (json) {
        char* text = cJSON_Print(r);
        puts(text);
        cJSON_free(text);
    } else if (cJSON_IsString(r)) {
        puts(r->valuestring);
    }
    cJSON_Delete(r);
    return rc;
}

static int items(openhab_client_t* c, const char* muster) {
    cJSON* r = openhab_items_getItems(c, .fields = "name,type,state,label");
    if (json || openhab_is_error(r)) return fertig(r);
    cJSON* i;
    cJSON_ArrayForEach(
        i, r) if (!muster || fnmatch(muster, openhab_json_str(i, "name"), 0) == 0)
        printf("%-28s %-22s %s\n", openhab_json_str(i, "name"), openhab_json_str(i, "type"),
               openhab_json_str(i, "state"));
    return fertig(r);
}

static int liste(cJSON* r, const char* id, const char* name, const char* status) {
    if (json || openhab_is_error(r)) return fertig(r);
    cJSON* e;
    cJSON_ArrayForEach(e, r) printf("%-24s %-28s %s\n", openhab_json_str(e, id),
                                    openhab_json_str(e, name), openhab_json_str(e, status));
    return fertig(r);
}

static int watch(openhab_client_t* c, int n, char** namen) {
    char topics[2048] = "";
    for (int i = 0; i < n; i++) { // Liste der Topics, durch Komma getrennt
        size_t l = strlen(topics);
        snprintf(topics + l, sizeof topics - l, "%sopenhab/items/%s/statechanged",
                 i ? "," : "", namen[i]);
    }
    strom =
        openhab_events_getEvents(c, .topics = n ? topics : "openhab/items/*/statechanged");
    if (openhab_sse_is_error(strom)) return fertig(openhab_sse_error_result(strom));
    struct sigaction sa = { 0 };
    sa.sa_handler = stopp;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    cJSON* e;
    while ((e = openhab_sse_next_event(strom, -1, 0))) {
        if (json) {
            char* text = openhab_json_print(e);
            puts(text);
            openhab_free(text);
        } else {
            char name[64];
            sscanf(openhab_json_str(e, "topic"), "openhab/items/%63[^/]", name);
            printf("%s %s -> %s\n", name, openhab_json_str(e, "payload.oldValue"),
                   openhab_json_str(e, "payload.value"));
        }
        fflush(stdout); // sofort weiterreichen (Pipes!)
        cJSON_Delete(e);
    }
    openhab_sse_close(strom);
    return 0;
}

int main(int argc, char** argv) {
    int a = 1;
    if (a < argc && !strcmp(argv[a], "--json")) {
        json = 1;
        a++;
    }
    if (a >= argc) return nutzung();
    const char* befehl = argv[a++];
    int rest = argc - a;
    char** arg = argv + a;

    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c =
        openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"), .timeout = 5);
    int rc;
    if (!strcmp(befehl, "info") && rest == 0) {
        cJSON* r = openhab_client_get(c, "/", NULL, NULL);
        if (json || openhab_is_error(r)) return fertig(r);
        printf("openHAB %s (%s), %s, %d Endpunkte\n",
               openhab_json_str(r, "runtimeInfo.version"),
               openhab_json_str(r, "runtimeInfo.buildString"),
               openhab_json_str(r, "locale"),
               cJSON_GetArraySize(openhab_json_get(r, "links")));
        rc = fertig(r);
    } else if (!strcmp(befehl, "items") && rest <= 1) {
        rc = items(c, rest ? arg[0] : NULL);
    } else if (!strcmp(befehl, "get") && rest == 1) {
        rc = fertig(openhab_items_getItemState(c, arg[0]));
    } else if (!strcmp(befehl, "send") && rest == 2) {
        rc = fertig(openhab_items_sendCommand(c, arg[0], arg[1], .source = "ohctl"));
    } else if (!strcmp(befehl, "update") && rest == 2) {
        rc = fertig(openhab_items_postUpdate(c, arg[0], arg[1], .source = "ohctl"));
    } else if (!strcmp(befehl, "watch")) {
        rc = watch(c, rest, arg);
    } else if (!strcmp(befehl, "things") && rest == 0) {
        rc = liste(openhab_things_getThings(c, .summary = OPENHAB_TRUE), "UID", "label",
                   "statusInfo.status");
    } else if (!strcmp(befehl, "rules") && rest == 0) {
        rc = liste(openhab_rules_getRules(c, .summary = OPENHAB_TRUE), "uid", "name",
                   "status.status");
    } else if (!strcmp(befehl, "run") && rest == 1) {
        rc = fertig(openhab_rules_runNow(c, arg[0]));
    } else {
        rc = nutzung();
    }
    openhab_client_destroy(c);
    return rc;
}
