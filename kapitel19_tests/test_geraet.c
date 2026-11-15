// Kapitel 19: Integrationstest des Geräts (Kapitel 18) mit openHAB und der Test-Suite
#include <openhab/openhab_testsuite.h>
#include <unistd.h>

#include "geraet.h"
#include "testhilfe.h"

static void still(int level, const char* text, void* u) {
    (void)level;
    (void)text;
    (void)u;
}

int main(void) {
    braucht_openhab();
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    openhab_client_t* c = openhab_client_new(url, .token = getenv("OPENHAB_TOKEN"));
    openhab_testlog_set(still, NULL); // Meldungen der Suite unterdrücken
    geraet g = { .client = c,
                 .luefter = ausgang_oeffnen("attrappe"),
                 .intervall = 10,
                 .netz_bytes = -1 };

    // Anmelden legt die Items an – zweimal hintereinander darf nichts schiefgehen
    PRUEFE(geraet_anmelden(&g, "test") > 0);
    PRUEFE(geraet_anmelden(&g, "test") > 0);
    PRUEFE(openhab_item_tester_checkItemIsType(c, "Pi_Temperatur", "Number:Temperature"));
    PRUEFE(openhab_item_tester_doesGroupContainMember(c, "gPi", "Pi_Luefter"));

    // Melden: Werte mit Einheit kommen an, das Lebenszeichen steht auf ON
    messung m = {
        .temperatur = 47.5, .last = 12, .speicher = 40, .netz_kbit = 8, .laufzeit_h = 2.5
    };
    geraet_melden(&g, &m);
    usleep(300 * 1000);
    PRUEFE(openhab_item_tester_checkItemHasState(c, "Pi_Temperatur", "47.5 °C"));
    PRUEFE(openhab_item_tester_checkItemHasState(c, "Pi_Online", "ON"));

    // Command: Der Ausgang schaltet, erst dann bestätigt das Gerät den State
    char antwort[128];
    PRUEFE(geraet_command(&g, "Pi_Luefter", "ON", antwort, sizeof antwort));
    PRUEFE(ausgang_lesen(g.luefter) == 1);
    usleep(300 * 1000);
    PRUEFE(openhab_item_tester_checkItemHasState(c, "Pi_Luefter", "ON"));
    PRUEFE(geraet_command(&g, "Pi_Aktion", "ZEIT", antwort, sizeof antwort));
    PRUEFE(strncmp(antwort, "Gerätezeit", 11) == 0);
    PRUEFE(!geraet_command(&g, "Wohnzimmer_Licht", "ON", antwort, sizeof antwort));

    geraet_command(&g, "Pi_Luefter", "OFF", antwort, sizeof antwort);
    geraet_abmelden(&g);
    ausgang_schliessen(g.luefter);
    openhab_client_destroy(c);
    return test_ende("test_geraet");
}
