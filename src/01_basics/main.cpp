// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 01 – Moderne C++ Basics                                          ║
// ║  Features: auto, constexpr, range-for, structured bindings,             ║
// ║            if-init, [[nodiscard]], nullptr, Type Aliases                ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// Ziel dieses Tutorials:
//   Moderne C++ (ab C++11/17) hat viele Features eingeführt, die Code
//   kürzer, sicherer und ausdrucksstärker machen. Wir schauen uns die
//   wichtigsten "Quality-of-Life"-Features an.

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <array>

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: auto – Automatische Typableitung
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem (altes C++):
//   std::map<std::string, std::vector<int>>::const_iterator it = m.begin();
//   Das ist fehleranfällig und schwer lesbar.
//
// Lösung (modernes C++):
//   auto it = m.begin();
//   Der Compiler kennt den Typ bereits – warum ihn nochmal schreiben?
//
// Wichtige Regeln:
//   auto x = 42;         → x ist int (Kopie)
//   auto& x = obj;       → x ist Referenz auf obj (kein Kopieren!)
//   const auto& x = obj; → read-only Referenz (sicherste Variante in for-loops)
//   auto* p = ptr;       → Zeiger (selten nötig, auto alleine reicht auch)
void demo_auto() {
    std::cout << "\n--- auto ---\n";

    // Einfache Typen: auto leitet int, double, string ab
    auto ganze_zahl  = 42;              // int
    auto kommazahl   = 3.14;            // double
    auto text        = std::string{"Hallo, C++20!"};
    auto wahrheit    = true;            // bool

    std::cout << "int:    " << ganze_zahl  << "\n"
              << "double: " << kommazahl   << "\n"
              << "string: " << text        << "\n"
              << "bool:   " << std::boolalpha << wahrheit << "\n";

    // auto in range-based for loops: IMMER const auto& verwenden
    // wenn wir die Elemente nur lesen wollen!
    //   auto x      → kopiert jedes Element (teuer bei großen Objekten)
    //   auto& x     → Referenz, kein Kopieren, aber veränderbar
    //   const auto& → Referenz, kein Kopieren, read-only (empfohlen!)
    std::vector<std::string> namen{"Alice", "Bob", "Carol"};
    std::cout << "Namen: ";
    for (const auto& name : namen) {   // kein Kopieren des Strings!
        std::cout << name << " ";
    }
    std::cout << "\n";

    // auto bei Iteratoren: spart viel Tipp-Arbeit
    auto it = namen.begin();   // statt: std::vector<std::string>::iterator it
    std::cout << "Erster Name: " << *it << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: constexpr – Auswertung zur Compile-Zeit
// ─────────────────────────────────────────────────────────────────────────────
//
// const     → Wert wird einmal gesetzt und kann nicht geändert werden.
//             Auswertung kann zur Laufzeit passieren.
//
// constexpr → Ausdruck MUSS zur Compile-Zeit berechenbar sein.
//             Ergebnis wird direkt ins Programm eingebaut.
//             Laufzeit-Overhead: null.
//
// Wann constexpr?
//   - Mathematische Konstanten (pi, konversions-Faktoren)
//   - Array-Größen
//   - Kleine, reine Berechnungsfunktionen
//
// constexpr-Funktionen in C++14+ können auch Schleifen und Verzweigungen
// enthalten – nicht nur einfache Ausdrücke!

constexpr double PI = 3.14159265358979;   // Konstante – zur Compile-Zeit bekannt

// Rekursive constexpr-Funktion – wird vollständig vom Compiler berechnet
constexpr long long fakultaet(int n) {
    // In einer constexpr-Funktion ist alles erlaubt, was zur Compile-Zeit
    // berechenbar ist (ab C++14: if, for, while, lokale Variablen)
    if (n <= 1) return 1;
    return n * fakultaet(n - 1);
}

// Iterative Version (C++14+) – oft verständlicher als Rekursion
constexpr long long potenz(long long basis, int exp) {
    long long ergebnis = 1;
    for (int i = 0; i < exp; ++i) {   // Schleife in constexpr erlaubt (C++14+)
        ergebnis *= basis;
    }
    return ergebnis;
}

void demo_constexpr() {
    std::cout << "\n--- constexpr ---\n";

    // Diese Berechnungen finden zur COMPILE-ZEIT statt!
    // Im compilierten Programm steht nur noch die Zahl, keine Berechnung.
    constexpr long long f10  = fakultaet(10);   // 3628800
    constexpr long long p2_8 = potenz(2, 8);    // 256

    std::cout << "10! = " << f10  << "\n";
    std::cout << "2^8 = " << p2_8 << "\n";
    std::cout << "PI  = " << PI   << "\n";

    // constexpr als Array-Größe: Das war früher (pre-C++11) umständlich.
    // #define SIZE 10 → böse (kein Typ, kein Scope)
    // const int SIZE = 10 → manchmal nicht als Compile-Zeit-Konstante erlaubt
    constexpr int ARRAY_GROESSE = 5;
    std::array<int, ARRAY_GROESSE> arr{1, 2, 3, 4, 5};  // Größe muss Compile-Zeit-Konstante sein
    std::cout << "Array: ";
    for (const auto& x : arr) std::cout << x << " ";
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: Structured Bindings (C++17)
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem (altes C++):
//   auto paar = std::make_pair(42, "Hallo");
//   int zahl = paar.first;        // umständlich
//   std::string wort = paar.second;
//
// Lösung (C++17):
//   auto [zahl, wort] = paar;     // direkt auspacken!
//
// Funktioniert mit:
//   - std::pair / std::tuple
//   - std::map (gibt std::pair<const Key, Value> zurück)
//   - Eigene Structs mit public-Membern
//   - std::array und C-Arrays

void demo_structured_bindings() {
    std::cout << "\n--- Structured Bindings ---\n";

    // 1) std::pair auspacken
    auto koordinate = std::make_pair(3.0, 7.5);
    auto [x, y] = koordinate;   // x = 3.0, y = 7.5
    std::cout << "Punkt: (" << x << ", " << y << ")\n";

    // 2) std::map iterieren – ohne structured bindings sehr umständlich:
    //    for (auto& eintrag : karte) { eintrag.first; eintrag.second; }
    // Mit structured bindings viel lesbarer:
    std::map<std::string, int> highscores{
        {"Alice", 95}, {"Bob", 87}, {"Carol", 92}
    };
    std::cout << "Highscores:\n";
    for (const auto& [name, punkte] : highscores) {
        // name  = eintrag.first  (der Schlüssel)
        // punkte = eintrag.second (der Wert)
        std::cout << "  " << name << ": " << punkte << "\n";
    }

    // 3) Eigene Struct – funktioniert automatisch für public-Member
    struct Punkt3D { double x, y, z; };
    Punkt3D p3{1.0, 2.0, 3.0};
    auto [px, py, pz] = p3;
    std::cout << "3D-Punkt: " << px << ", " << py << ", " << pz << "\n";

    // 4) Mit Referenz: & verhindert Kopieren und erlaubt Verändern
    std::map<std::string, int> alter{{"Max", 25}, {"Lena", 30}};
    for (auto& [name, jahre] : alter) {
        jahre += 1;   // alle ein Jahr älter machen (kein Kopieren!)
    }
    std::cout << "Nach Geburtstag: Max=" << alter["Max"]
              << ", Lena=" << alter["Lena"] << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: if/switch mit Initialisierung (C++17)
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem (altes C++):
//   auto it = container.find(key);  // it "verunreinigt" den äußeren Scope
//   if (it != container.end()) { ... }
//   // it existiert hier noch, obwohl wir es nicht mehr brauchen
//
// Lösung (C++17): Variable direkt im if-Statement deklarieren
//   if (auto it = container.find(key); it != container.end()) { ... }
//   // it existiert nur innerhalb des if-Blocks!
//
// Vorteile:
//   - Scope-Kontrolle: Variable lebt nur so lang wie nötig
//   - Keine versehentliche Nutzung nach dem if
//   - Klarere Intention des Codes

void demo_if_init() {
    std::cout << "\n--- if/switch mit Initialisierung ---\n";

    std::map<std::string, int> inventar{{"Schwert", 1}, {"Schild", 2}};

    // Syntax: if (Initialisierung; Bedingung) { ... }
    if (auto it = inventar.find("Schwert"); it != inventar.end()) {
        // 'it' ist nur in diesem Block gültig
        std::cout << "Schwert gefunden: Anzahl = " << it->second << "\n";
    } else {
        // 'it' ist auch im else-Block gültig (zeigt auf end())
        std::cout << "Schwert nicht gefunden\n";
    }
    // Hier existiert 'it' NICHT mehr → kein versehentlicher Zugriff möglich!

    // Auch in switch-Statements möglich:
    int code = 2;
    switch (auto status = code * 10; status) {  // status = 20
        case 10: std::cout << "Status: OK\n"; break;
        case 20: std::cout << "Status: Warnung (code=" << code << ")\n"; break;
        default: std::cout << "Status: Fehler\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 5: [[nodiscard]] – Compiler-Warnung bei ignoriertem Rückgabewert
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem (altes C++):
//   int fehlercode = datei_speichern();  // was wenn wir das vergessen?
//   datei_speichern();  // kein Fehler, kein Compiler-Hinweis → stille Bugs!
//
// Lösung: [[nodiscard]] am Funktionskopf oder am Return-Typ
//   Der Compiler erzeugt eine Warnung wenn der Rückgabewert ignoriert wird.
//
// Typische Anwendungsfälle:
//   - Fehlercodes / Status-Werte
//   - Allokations-Ergebnisse (nullptr wäre fatal)
//   - Factory-Funktionen (Objekt wird sonst sofort zerstört)

[[nodiscard]] int verbindung_herstellen(const std::string& host) {
    // In echtem Code: Verbindungsaufbau, Fehlerbehandlung etc.
    std::cout << "  Verbinde zu: " << host << "\n";
    return 0;   // 0 = Erfolg (andere Werte = Fehlercodes)
}

// Ab C++20: Nachricht dazu angeben
[[nodiscard("Fehlercode muss geprüft werden!")]]
int datei_oeffnen(const std::string& pfad) {
    std::cout << "  Öffne Datei: " << pfad << "\n";
    return 0;
}

void demo_nodiscard() {
    std::cout << "\n--- [[nodiscard]] ---\n";

    // RICHTIG: Rückgabewert wird geprüft
    int status = verbindung_herstellen("server.example.com");
    if (status != 0) {
        std::cout << "Fehler beim Verbinden!\n";
    } else {
        std::cout << "Verbindung erfolgreich\n";
    }

    // RICHTIG: [[nodiscard]] mit C++20-Nachricht
    auto err = datei_oeffnen("daten.txt");
    std::cout << "Datei-Status: " << err << "\n";

    // FALSCH (würde Compiler-Warnung erzeugen):
    // verbindung_herstellen("host");  ← warning: ignoring return value
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 6: nullptr statt NULL / 0
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem (altes C++):
//   NULL ist nur ein Makro für 0 (eine Ganzzahl, kein Zeiger-Typ!)
//   Das führt zu mehrdeutigen Funktionsaufrufen:
//
//   void f(int x);
//   void f(int* p);
//   f(NULL);   // Ruft f(int) auf – überraschend!
//   f(nullptr); // Ruft f(int*) auf – eindeutig!
//
// nullptr hat den Typ std::nullptr_t – eindeutig ein Null-Zeiger.
// Es kann niemals als Ganzzahl missverstanden werden.

void verarbeite_int(int x) {
    std::cout << "  int-Überladung: " << x << "\n";
}
void verarbeite_int(int* p) {
    if (p == nullptr)
        std::cout << "  Zeiger-Überladung: Nullzeiger\n";
    else
        std::cout << "  Zeiger-Überladung: *p = " << *p << "\n";
}

void demo_nullptr() {
    std::cout << "\n--- nullptr ---\n";

    // NULL würde die int-Überladung aufrufen (unerwartetes Verhalten!)
    // nullptr ruft korrekt die Zeiger-Überladung auf:
    verarbeite_int(nullptr);   // → "Zeiger-Überladung: Nullzeiger"

    int zahl = 100;
    verarbeite_int(&zahl);     // → "Zeiger-Überladung: *p = 100"
    verarbeite_int(42);        // → "int-Überladung: 42"

    // nullptr in Bedingungen nutzen
    int* p = nullptr;
    if (p != nullptr) {
        std::cout << "Hat Wert\n";
    } else {
        std::cout << "p ist nullptr – kein Zugriff!\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 7: using – Typ-Aliase (moderner Ersatz für typedef)
// ─────────────────────────────────────────────────────────────────────────────
//
// Alt (C-Stil): typedef std::vector<std::string> StringVektor;
// Neu (C++11): using StringVektor = std::vector<std::string>;
//
// Vorteil von using:
//   - Lesbarerer Syntax (Name zuerst, dann Typ)
//   - Kann mit Templates kombiniert werden (template aliases)
//   - Konsistent mit dem Rest der modernen C++ Syntax

using StringVektor   = std::vector<std::string>;
using IntMap         = std::map<std::string, int>;
using GanzzahlMatrix = std::vector<std::vector<int>>;

// Template-Alias: typedef konnte das nicht!
template<typename T>
using Paar = std::pair<T, T>;   // Paar<int> = std::pair<int, int>

void demo_type_aliases() {
    std::cout << "\n--- Type Aliases (using) ---\n";

    StringVektor sprachen{"C++", "Python", "Rust", "Go"};
    std::cout << "Sprachen: ";
    for (const auto& s : sprachen) std::cout << s << " ";
    std::cout << "\n";

    // Template-Alias in Aktion
    Paar<double> koordinaten{3.14, 2.71};
    std::cout << "Koordinaten: (" << koordinaten.first
              << ", " << koordinaten.second << ")\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔════════════════════════════════════╗\n"
              << "║  THEMA 01 – Moderne C++ Basics     ║\n"
              << "╚════════════════════════════════════╝\n";

    demo_auto();
    demo_constexpr();
    demo_structured_bindings();
    demo_if_init();
    demo_nodiscard();
    demo_nullptr();
    demo_type_aliases();

    return 0;
}
