// Kapitel 20: ohplugin in Unity (C#) – die Datei liegt in Assets/Scripts/,
// libohplugin.so (Linux, Android) bzw. ohplugin.dll (Windows, HoloLens: ARM64)
// in Assets/Plugins/.
using System;
using System.Runtime.InteropServices;
using System.Text;
using UnityEngine;

public class OpenHAB : MonoBehaviour
{
    const string Lib = "ohplugin";
    [DllImport(Lib)] static extern IntPtr oh_verbinden(string url, string token);
    [DllImport(Lib)] static extern int oh_befehl(IntPtr p, string item, string command);
    [DllImport(Lib)] static extern int oh_beobachten(IntPtr p, string items);
    [DllImport(Lib)] static extern int oh_naechste_aenderung(IntPtr p, StringBuilder puffer,
                                                              int groesse);
    [DllImport(Lib)] static extern void oh_trennen(IntPtr p);

    public string url = "http://openhab:8080";
    public string token = "";
    public Light deckenlicht;                // eine Lampe in der Szene
    IntPtr oh;
    readonly StringBuilder puffer = new StringBuilder(128);

    void Start()
    {
        oh = oh_verbinden(url, token);
        if (oh == IntPtr.Zero) { Debug.LogError("openHAB nicht erreichbar"); return; }
        oh_beobachten(oh, "Wohnzimmer_Licht");
    }

    void Update()                            // jeder Frame: gesammelte Änderungen abholen
    {
        if (oh == IntPtr.Zero) return;
        while (oh_naechste_aenderung(oh, puffer, puffer.Capacity) > 0)
        {
            string[] teile = puffer.ToString().Split('=');
            if (teile[0] == "Wohnzimmer_Licht") deckenlicht.enabled = teile[1] == "ON";
        }
    }

    public void Schalten()                   // z. B. an einen Button oder eine Geste binden
    {
        oh_befehl(oh, "Wohnzimmer_Licht", deckenlicht.enabled ? "OFF" : "ON");
    }

    void OnDestroy() { if (oh != IntPtr.Zero) oh_trennen(oh); }
}
