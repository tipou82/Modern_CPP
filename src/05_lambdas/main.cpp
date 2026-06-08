// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 05 – Lambdas & std::function                                     ║
// ║  Features: Lambda-Syntax, Captures, generische Lambdas,                 ║
// ║            std::function, Lambdas in Algorithmen, Closures              ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// ──── Was ist ein Lambda? ─────────────────────────────────────────────────
//
//   Ein Lambda ist eine anonyme Funktion, die direkt an Ort und Stelle
//   definiert wird. Es ist syntaktischer Zucker für ein Funktionsobjekt
//   (eine Klasse mit operator()).
//
//   Ohne Lambda (umständlich):
//     struct Verdoppele {
//         int operator()(int x) const { return x * 2; }
//     };
//     std::transform(v.begin(), v.end(), v.begin(), Verdoppele{});
//
//   Mit Lambda (klar und kompakt):
//     std::transform(v.begin(), v.end(), v.begin(),
//                    [](int x) { return x * 2; });
//
// ──── Anatomie eines Lambdas ──────────────────────────────────────────────
//
//   [ capture ] ( parameter ) -> rückgabetyp { körper }
//   ─────────── ─────────────  ────────────── ─────────
//       |             |              |              |
//   Was aus dem   Wie normale    Optional:       Code
//   umgebenden    Funktions-     Rückgabetyp
//   Scope genutzt  parameter     (meist auto)
//   werden darf

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>  // sort, find_if, transform, for_each, count_if
#include <functional> // std::function
#include <numeric>    // std::accumulate
#include <memory>     // std::make_unique

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: Lambda-Grundsyntax
// ─────────────────────────────────────────────────────────────────────────────

void demo_grundsyntax() {
    std::cout << "\n=== 1. Lambda-Grundsyntax ===\n";

    // Einfachstes Lambda: keine Parameter, keine Captures
    auto hallo = []() {
        std::cout << "  Hallo vom Lambda!\n";
    };
    hallo();   // Lambda aufrufen wie eine Funktion

    // Lambda mit Parametern (wie normale Funktion)
    auto addiere = [](int a, int b) {
        return a + b;   // Rückgabetyp wird abgeleitet (int)
    };
    std::cout << "  3 + 4 = " << addiere(3, 4) << "\n";

    // Expliziter Rückgabetyp (wenn Ableitung nicht eindeutig)
    auto teile = [](double a, double b) -> double {
        if (b == 0.0) return 0.0;
        return a / b;
    };
    std::cout << "  10.0 / 3.0 = " << teile(10.0, 3.0) << "\n";

    // Mehrzeiliges Lambda mit lokalen Variablen
    auto berechne = [](int n) {
        int summe = 0;
        for (int i = 1; i <= n; ++i) {
            summe += i;
        }
        return summe;   // Summe 1+2+...+n = n*(n+1)/2
    };
    std::cout << "  Summe 1..10 = " << berechne(10) << "\n";

    // Sofort ausgeführtes Lambda (IIFE: Immediately Invoked Function Expression)
    // Nützlich um komplexe Initialisierungen in auto zu kapseln
    const int ergebnis = [](int x, int y) {
        return x * x + y * y;
    }(3, 4);   // ← direkt aufgerufen mit (3, 4)
    std::cout << "  3² + 4² = " << ergebnis << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: Captures – Zugriff auf äußere Variablen
// ─────────────────────────────────────────────────────────────────────────────
//
// Ein Lambda kann auf Variablen aus dem umgebenden Scope zugreifen – aber
// nur wenn sie im capture-Abschnitt [] angegeben werden.
//
// Capture-Varianten:
//   []          → kein Zugriff auf äußere Variablen
//   [x]         → x wird BY VALUE kopiert (Lambda hat eigene Kopie)
//   [&x]        → x wird BY REFERENCE genutzt (kein Kopieren, veränderbar)
//   [=]         → alle genutzten Variablen werden by value kopiert
//   [&]         → alle genutzten Variablen werden by reference genutzt
//   [=, &x]     → alles by value, außer x by reference
//   [&, x]      → alles by reference, außer x by value
//   [this]      → Zugriff auf Klassen-Member (in Methoden)
//
// ⚠️  Achtung bei [&]: Lambda darf das umgebende Objekt nicht überleben!
//     (Dangling Reference)

void demo_captures() {
    std::cout << "\n=== 2. Captures ===\n";

    int schwellwert = 5;
    std::vector<int> zahlen{1, 7, 3, 9, 2, 8, 4, 6};

    std::cout << "  Zahlen > " << schwellwert << ":\n  ";

    // [schwellwert]: Kopie von schwellwert zum Zeitpunkt der Lambda-Erstellung
    auto groesser_als = [schwellwert](int x) {
        return x > schwellwert;
    };

    for (int z : zahlen) {
        if (groesser_als(z)) std::cout << z << " ";
    }
    std::cout << "\n";

    schwellwert = 100;  // Änderung hat KEINEN Effekt auf das Lambda!
    std::cout << "  Nach Änderung von schwellwert auf " << schwellwert
              << " – Lambda nutzt immer noch Kopie:\n  ";
    for (int z : zahlen) {
        if (groesser_als(z)) std::cout << z << " ";
    }
    std::cout << "\n";

    // By-Reference Capture: Lambda sieht immer den aktuellen Wert
    int zaehler = 0;
    auto zaehle = [&zaehler](int x) {   // &zaehler: Referenz!
        if (x % 2 == 0) zaehler++;      // modifiziert die äußere Variable
    };
    for (int z : zahlen) zaehle(z);
    std::cout << "  Anzahl gerader Zahlen: " << zaehler << "\n";

    // mutable: Erlaubt Modifikation von by-value kopierten Variablen
    // (ändert NICHT die äußere Variable, nur die Kopie im Lambda)
    int start = 10;
    auto zaehle_hoch = [start]() mutable {   // mutable macht Kopie veränderbar
        return ++start;   // start im Lambda wird verändert, äußeres start nicht
    };
    std::cout << "  zaehle_hoch(): " << zaehle_hoch() << "\n";  // 11
    std::cout << "  zaehle_hoch(): " << zaehle_hoch() << "\n";  // 12
    std::cout << "  äußeres start: " << start << "\n";          // noch 10!
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: Lambdas in STL-Algorithmen
// ─────────────────────────────────────────────────────────────────────────────
//
// Die STL-Algorithmen (<algorithm>) sind der Haupteinsatzbereich für Lambdas.
// Sie ersetzen manuelle Schleifen und machen Code deklarativer:
//   "WAS soll passieren" statt "WIE es passiert".

struct Person {
    std::string name;
    int alter;
};

void demo_algorithmen() {
    std::cout << "\n=== 3. Lambdas mit STL-Algorithmen ===\n";

    std::vector<int> v{5, 3, 8, 1, 9, 2, 7, 4, 6};

    // std::sort: Benutzerdefinierte Sortierung
    auto v_sortiert = v;
    std::sort(v_sortiert.begin(), v_sortiert.end(),
              [](int a, int b) { return a < b; });  // aufsteigend
    std::cout << "  Sortiert (aufst.):  ";
    for (int x : v_sortiert) std::cout << x << " ";
    std::cout << "\n";

    std::sort(v_sortiert.begin(), v_sortiert.end(),
              [](int a, int b) { return a > b; });  // absteigend
    std::cout << "  Sortiert (abst.):   ";
    for (int x : v_sortiert) std::cout << x << " ";
    std::cout << "\n";

    // std::find_if: Erstes Element das Bedingung erfüllt
    auto it = std::find_if(v.begin(), v.end(),
                           [](int x) { return x > 7; });
    if (it != v.end()) {
        std::cout << "  Erstes Element > 7: " << *it << "\n";
    }

    // std::count_if: Anzahl Elemente die Bedingung erfüllen
    int anzahl_gerade = std::count_if(v.begin(), v.end(),
                                       [](int x) { return x % 2 == 0; });
    std::cout << "  Anzahl gerade:      " << anzahl_gerade << "\n";

    // std::transform: Jedes Element transformieren
    std::vector<int> quadrate(v.size());
    std::transform(v.begin(), v.end(), quadrate.begin(),
                   [](int x) { return x * x; });
    std::cout << "  Quadrate:           ";
    for (int x : quadrate) std::cout << x << " ";
    std::cout << "\n";

    // std::for_each: Aktion auf jedes Element
    std::cout << "  Ungerade Zahlen:    ";
    std::for_each(v.begin(), v.end(), [](int x) {
        if (x % 2 != 0) std::cout << x << " ";
    });
    std::cout << "\n";

    // std::accumulate: Elemente "falten" (Summe, Produkt, etc.)
    int summe = std::accumulate(v.begin(), v.end(), 0,
                                [](int acc, int x) { return acc + x; });
    std::cout << "  Summe:              " << summe << "\n";

    // Sortierung von Structs nach benutzerdefiniertem Kriterium
    std::vector<Person> personen{{"Bob", 30}, {"Alice", 25}, {"Carol", 35}};
    std::sort(personen.begin(), personen.end(),
              [](const Person& a, const Person& b) {
                  return a.alter < b.alter;   // nach Alter sortieren
              });
    std::cout << "\n  Personen nach Alter:\n";
    for (const auto& p : personen) {
        std::cout << "    " << p.name << " (" << p.alter << ")\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: Generische Lambdas (C++14) und Template Lambdas (C++20)
// ─────────────────────────────────────────────────────────────────────────────
//
// Generische Lambdas verwenden auto als Parameter-Typ.
// Der Compiler generiert intern ein Template.

void demo_generische_lambdas() {
    std::cout << "\n=== 4. Generische Lambdas ===\n";

    // auto-Parameter: funktioniert für alle Typen mit operator<<
    auto print = [](const auto& wert) {
        std::cout << "  [auto]: " << wert << "\n";
    };
    print(42);
    print(3.14);
    print(std::string{"Hallo"});
    print(true);

    // Generisches Lambda als Komparator
    auto kleiner = [](const auto& a, const auto& b) {
        return a < b;
    };
    std::vector<int> vi{5, 2, 8, 1};
    std::vector<std::string> vs{"Banane", "Apfel", "Kirsche"};
    std::sort(vi.begin(), vi.end(), kleiner);
    std::sort(vs.begin(), vs.end(), kleiner);
    std::cout << "  int-vector sortiert:    ";
    for (int x : vi) std::cout << x << " ";
    std::cout << "\n  string-vector sortiert: ";
    for (const auto& s : vs) std::cout << s << " ";
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 5: std::function – Lambdas speichern und übergeben
// ─────────────────────────────────────────────────────────────────────────────
//
// std::function<Rückgabe(Parameter...)> kann beliebige Callables speichern:
//   - Lambdas
//   - Freie Funktionen (function pointer)
//   - Member-Funktionen (mit std::bind)
//   - Funktionsobjekte (Klassen mit operator())
//
// Wann std::function?
//   → Callbacks, Event-Handler, Strategy-Pattern
//   → Wenn der Callable-Typ zur Compile-Zeit nicht bekannt ist
//   → Als Funktionsparameter der verschiedene Callables akzeptieren soll
//
// ⚠️  std::function hat Overhead (dynamische Allokation, virtuelle Dispatch).
//     Für Performance-kritischen Code: Template-Parameter oder auto bevorzugen.

// Funktion die einen Callback als std::function akzeptiert
void verarbeite_mit_callback(
    const std::vector<int>& daten,
    const std::function<void(int)>& callback   // beliebiger Callable mit int-Parameter
) {
    for (int x : daten) {
        callback(x);
    }
}

// Freie Funktion (normaler Funktions-Zeiger)
void drucke_zahl(int x) {
    std::cout << "  [f-ptr] " << x << "\n";
}

void demo_std_function() {
    std::cout << "\n=== 5. std::function ===\n";

    std::vector<int> zahlen{1, 2, 3, 4, 5};

    // Lambda als std::function
    std::function<void(int)> lambda_callback = [](int x) {
        std::cout << "  [lambda] " << x * x << " (Quadrat)\n";
    };

    std::cout << "  Mit Lambda-Callback:\n";
    verarbeite_mit_callback(zahlen, lambda_callback);

    std::cout << "  Mit Funktions-Zeiger:\n";
    verarbeite_mit_callback(zahlen, drucke_zahl);

    // Callbacks in einem Container speichern (z.B. Event-System)
    std::cout << "\n  Event-System mit vector<std::function>:\n";
    std::vector<std::function<void(const std::string&)>> event_handler;

    event_handler.push_back([](const std::string& msg) {
        std::cout << "  Handler 1: [LOG] " << msg << "\n";
    });
    event_handler.push_back([](const std::string& msg) {
        std::cout << "  Handler 2: [UI]  Zeige an: " << msg << "\n";
    });

    // Alle Handler aufrufen (wie ein Event-System)
    std::string ereignis = "Datei gespeichert";
    for (const auto& handler : event_handler) {
        handler(ereignis);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 6: Closures – Lambdas die Zustand behalten
// ─────────────────────────────────────────────────────────────────────────────
//
// Ein Lambda das Variablen captured ist eine "Closure" – es schließt
// Variablen aus dem umgebenden Scope ein und behält sie über den Aufruf hinaus.
// Das ermöglicht elegante Lösungen für Zähler, Fabriken, etc.

void demo_closures() {
    std::cout << "\n=== 6. Closures ===\n";

    // Zähler-Factory: Gibt Lambdas zurück die eigenen Zähler haben
    auto mache_zaehler = [](int start = 0) {
        int count = start;   // Jedes Lambda hat seinen eigenen count
        return [count]() mutable {
            return ++count;
        };
    };

    auto zaehler_a = mache_zaehler(0);
    auto zaehler_b = mache_zaehler(100);

    std::cout << "  zaehler_a: " << zaehler_a() << ", " << zaehler_a() << ", "
                                 << zaehler_a() << "\n";  // 1, 2, 3
    std::cout << "  zaehler_b: " << zaehler_b() << ", " << zaehler_b() << "\n";  // 101, 102
    std::cout << "  zaehler_a: " << zaehler_a() << "\n";   // 4 (unabhängig von b!)

    // Accumulator: sammelt Werte auf
    std::cout << "\n  Accumulator:\n";
    double gesamt = 0.0;
    auto addiere_zu_gesamt = [&gesamt](double x) {
        gesamt += x;
        std::cout << "    +" << x << " → Gesamt: " << gesamt << "\n";
    };
    addiere_zu_gesamt(10.5);
    addiere_zu_gesamt(3.2);
    addiere_zu_gesamt(7.8);
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔════════════════════════════════════════════╗\n"
              << "║  THEMA 05 – Lambdas & std::function        ║\n"
              << "╚════════════════════════════════════════════╝\n";

    demo_grundsyntax();
    demo_captures();
    demo_algorithmen();
    demo_generische_lambdas();
    demo_std_function();
    demo_closures();

    return 0;
}
