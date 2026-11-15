// Kapitel 15: Text wie einen Sprachbefehl auswerten lassen – ohne Mikrofon
//   ./starte.sh kapitel15_oberflaechen/04_sprache.c "Schalte die Leselampe ein"
#include <openhab/openhab.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static openhab_client_t* c;

// Der Interpreter erkennt Items an ihrem Label und antwortet in der gewählten Sprache
static void sage(const char* satz) {
    cJSON* antwort = openhab_voice_interpretText(c, satz, .language = "de");
    const char* fehler = openhab_error(antwort);
    printf("  »%s« -> %s\n", satz, fehler ? strrchr(fehler, '(') : antwort->valuestring);
    cJSON_Delete(antwort);
}

int main(int argc, char** argv) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));

    cJSON* interpreter = openhab_voice_getInterpreters(c, .language = "de");
    cJSON* i;
    cJSON_ArrayForEach(i, interpreter) {
        char* sprachen = openhab_json_print(openhab_json_get(i, "locales"));
        printf("Interpreter %-8s %-26s Sprachen %.30s…\n", openhab_json_str(i, "id"),
               openhab_json_str(i, "label"), sprachen);
        openhab_free(sprachen);
    }
    cJSON_Delete(interpreter);
    cJSON* sinks = openhab_audio_getSinks(c);
    printf("Audio-Ausgänge:");
    cJSON_ArrayForEach(i, sinks) printf(" %s", openhab_json_str(i, "id"));
    printf("\n");
    cJSON_Delete(sinks);

    // Den eingebauten Interpreter zum Standard machen – eine Systemeinstellung
    cJSON* config = openhab_services_getServiceConfig(c, "org.openhab.voice");
    cJSON_DeleteItemFromObject(config, "defaultHLI");
    cJSON_AddStringToObject(config, "defaultHLI", "system");
    cJSON_Delete(openhab_services_updateServiceConfig(c, "org.openhab.voice", config));
    cJSON_Delete(config);
    config = openhab_services_getServiceConfig(c, "org.openhab.voice");
    printf("Standard-Interpreter: %s\n", openhab_json_str(config, "defaultHLI"));
    cJSON_Delete(config);

    cJSON_Delete(openhab_items_sendCommand(c, "Wohnzimmer_Licht", "OFF"));
    cJSON_Delete(openhab_items_sendCommand(c, "Wohnzimmer_Dimmer", "60"));
    const char* saetze[] = { "Schalte das Deckenlicht ein", "Schalte das Deckenlicht ein",
                             "Schalte die Leselampe aus", "Mach mir einen Kaffee" };
    for (int k = 0; k < 4; k++) sage(saetze[k]);
    usleep(500 * 1000);
    cJSON* l = openhab_items_getItemState(c, "Wohnzimmer_Licht");
    cJSON* d = openhab_items_getItemState(c, "Wohnzimmer_Dimmer");
    printf("Deckenlicht: %s | Leselampe: %s\n", l->valuestring, d->valuestring);
    cJSON_Delete(l);
    cJSON_Delete(d);

    // Einen bestimmten Interpreter wählen, unabhängig vom Standard
    cJSON* ids = cJSON_CreateStringArray((const char*[]){ "system" }, 1);
    cJSON* r = openhab_voice_interpretTextBatch(c, "turn off the Deckenlicht", ids,
                                                .language = "en");
    printf("%s\n", cJSON_IsString(r) ? r->valuestring : openhab_error(r));
    cJSON_Delete(r);
    cJSON_Delete(ids);

    for (int k = 1; k < argc; k++) sage(argv[k]); // eigene Sätze aus dem Terminal
    openhab_client_destroy(c);
    return 0;
}
