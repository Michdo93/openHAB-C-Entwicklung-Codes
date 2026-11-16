// Kapitel 19: Vorab-Test, bevor ein Dienst startet – oder der Roboter seine Vorführung.
// Prüft in Sekunden alles, was Dashboard (Kapitel 17) und Gerät (Kapitel 18) voraussetzen,
// und endet mit Exit-Code 0 (bereit) oder 1 (nicht starten!).
// Ideal als ExecStartPre= in einer systemd-Unit oder als Schritt einer CI-Pipeline.
#include <openhab/openhab_testsuite.h>
#include <stdio.h>
#include <stdlib.h>

static const char* ITEMS[][2] = {
    { "Wohnzimmer_Licht", "Switch" },
    { "Wohnzimmer_Dimmer", "Dimmer" },
    { "Kueche_Rollladen", "Rollershutter" },
    { "Werkstatt_Luefter", "Switch" },
    { "Werkstatt_Temperatur", "Number:Temperature" },
    { "Haus_Status", "String" },
};
static const char* THINGS[] = { "astro:sun:zuhause" };
static const char* REGELN[] = { "restbuch-1" };
static const char* AUF_DER_SITEMAP[] = { "Werkstatt_Luefter", "Wohnzimmer_Licht" };
#define N(a) (int)(sizeof a / sizeof a[0])

static int fehler = 0;

static void melde(const char* text, const char* was) {
    if (!fehler++) printf("NICHT BEREIT:\n");
    printf("  %s%s\n", text, was);
}

static void still(int level, const char* text, void* u) {
    (void)level;
    (void)text;
    (void)u;
}

int main(void) {
    const char* url =
        getenv("OPENHAB_URL") ? getenv("OPENHAB_URL") : "http://127.0.0.1:8080";
    const char* token = getenv("OPENHAB_TOKEN");
    if (!token || !*token) {
        printf("OPENHAB_TOKEN fehlt – übersprungen\n");
        return 77; // CTest: übersprungen
    }
    openhab_client_t* c = openhab_client_new(url, .token = token, .timeout = 5);
    if (!openhab_client_is_logged_in(c)) {
        printf("NICHT BEREIT: openHAB nicht erreichbar (%s)\n",
               openhab_client_last_error(c));
        return 1;
    }
    openhab_testlog_set(still, NULL); // nur unser Bericht zählt

    for (int i = 0; i < N(ITEMS); i++) {
        if (!openhab_item_tester_doesItemExist(c, ITEMS[i][0]))
            melde("Item fehlt: ", ITEMS[i][0]);
        else if (!openhab_item_tester_checkItemIsType(c, ITEMS[i][0], ITEMS[i][1]))
            melde("falscher Typ: ", ITEMS[i][0]);
    }
    for (int i = 0; i < N(THINGS); i++)
        if (!openhab_thing_tester_isThingOnline(c, THINGS[i]))
            melde("Thing nicht ONLINE: ", THINGS[i]);
    for (int i = 0; i < N(REGELN); i++)
        if (!openhab_rule_tester_isRuleIdle(c, REGELN[i]))
            melde("Regel nicht bereit: ", REGELN[i]);
    for (int i = 0; i < N(AUF_DER_SITEMAP); i++)
        if (!openhab_sitemap_tester_doesSitemapContainItem(c, "restbuch",
                                                           AUF_DER_SITEMAP[i]))
            melde("nicht auf der Sitemap restbuch: ", AUF_DER_SITEMAP[i]);
    // Ein echter Funktionstest: Kommt ein Command an, und wird der Zustand gemeldet?
    if (!openhab_item_tester_testSwitch(c, "Wohnzimmer_Licht", "ON", .expectedState = "ON",
                                        .timeout = 5))
        melde("Command kommt nicht an: ", "Wohnzimmer_Licht");
    // … und landet die Änderung im Persistenzdienst?
    if (!openhab_persistence_tester_isItemPersisted(c, "inmemory", "Wohnzimmer_Licht"))
        melde("nicht in inmemory gespeichert: ", "Wohnzimmer_Licht");

    printf("==================================================\n");
    if (!fehler)
        printf(
            "BEREIT – %d Items, %d Thing, %d Regel, Sitemap, Persistenz und ein Command\n",
            N(ITEMS), N(THINGS), N(REGELN));
    openhab_client_destroy(c);
    return fehler ? 1 : 0;
}
