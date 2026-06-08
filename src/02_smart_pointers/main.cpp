// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 02 – Smart Pointers & RAII                                       ║
// ║  Features: unique_ptr, shared_ptr, weak_ptr, make_unique/make_shared    ║
// ╚══════════════════════════════════════════════════════════════════════════╝
// MISRA C++:2023 – Schlüsselregeln in diesem Thema:
//   Rule 21.6.1  (Required) – Dynamic memory should not be used
//                              → kein raw new/delete, smart pointers verwenden
//   Rule 21.6.2  (Required) – Dynamic memory shall be managed automatically
//                              → make_unique / make_shared statt new
//   Rule 23.11.1 (Advisory) – Raw pointer ctors of shared_ptr/unique_ptr
//                              shall not be used → immer make_unique/make_shared
//   Rule 15.1.3  (Required) – Single-argument constructors shall be explicit
//                              → explicit Ressource(std::string n, int w = 0)
//
// ──── Das Problem mit rohen Zeigern (raw pointers) ────────────────────────
//
//   int* p = new int(42);   // Speicher wird auf dem Heap allokiert
//   // ... viel Code ...
//   delete p;               // Muss manuell gelöscht werden!
//
// Was kann schiefgehen?
//   1. Memory Leak:      delete vergessen → Speicher nie freigegeben
//   2. Double Free:      delete zweimal → undefined behavior (Absturz)
//   3. Dangling Pointer: Zugriff nach delete → undefined behavior
//   4. Exception-Safety: Wenn Code vor delete eine Exception wirft →
//                        delete wird nie ausgeführt → Memory Leak
//
// ──── Die Lösung: RAII (Resource Acquisition Is Initialization) ───────────
//
//   Idee: Ressourcen (Speicher, Dateien, Locks...) werden im Konstruktor
//   eines Objekts erworben und im Destruktor automatisch freigegeben.
//   Der Destruktor wird IMMER aufgerufen – auch bei Exceptions!
//
//   Smart Pointer sind RAII-Wrapper um rohe Zeiger.
//   Sie rufen automatisch delete im Destruktor auf.

#include <iostream>
#include <memory>   // für unique_ptr, shared_ptr, weak_ptr, make_unique, make_shared
#include <string>
#include <vector>

// ─────────────────────────────────────────────────────────────────────────────
// Hilfsklasse für Demo-Zwecke
// ─────────────────────────────────────────────────────────────────────────────
// Wir sehen genau, wann ein Objekt erzeugt und zerstört wird – das hilft
// zu verstehen, wann Smart Pointer das delete aufrufen.
struct Ressource {
    std::string name;
    int wert;

    // explicit verhindert implizite Konvertierung:
    // Ressource r = "Hallo"; // Fehler! Muss explizit sein.
    explicit Ressource(std::string n, int w = 0)
        : name{std::move(n)}, wert{w}
    {
        std::cout << "  [ERZEUGT]   Ressource('" << name << "')\n";
    }

    ~Ressource() {
        std::cout << "  [ZERSTÖRT]  Ressource('" << name << "')\n";
    }

    void info() const {
        std::cout << "  Ressource: name='" << name << "', wert=" << wert << "\n";
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: unique_ptr – Alleiniger Eigentümer
// ─────────────────────────────────────────────────────────────────────────────
//
// unique_ptr<T>:
//   - Genau EIN unique_ptr "besitzt" das Objekt
//   - Kein Kopieren erlaubt (kein copy constructor / copy assignment)
//   - Verschieben (move) ist erlaubt → Besitz wird übertragen
//   - Destruktor ruft automatisch delete auf
//
// Analogie: Haustürschlüssel – es gibt nur einen. Wenn du ihn weitergibst,
//           hast du ihn nicht mehr. Du kannst keine Kopie machen.
//
// Wann unique_ptr?
//   → Fast immer, wenn man einen rohen Zeiger gehabt hätte.
//   → Ausnahme: Wenn mehrere Objekte denselben Besitz teilen müssen.

void demo_unique_ptr() {
    std::cout << "\n=== 1. unique_ptr ===\n";

    std::cout << "-- Erzeugung mit make_unique --\n";
    // make_unique<T>(args...) ist sicherer als new!
    // Grund: unique_ptr<T>(new T(a), new T(b)) kann bei einer Exception leaken,
    //        make_unique nicht.
    // ⚠️  MISRA C++:2023 Rule 23.11.1 (Advisory): Raw pointer constructors of
    //     std::unique_ptr and std::shared_ptr should not be used.
    //     → Immer make_unique<T>() / make_shared<T>() statt unique_ptr<T>(new T())
    auto ressource = std::make_unique<Ressource>("Datenbank", 100);

    // Zugriff über -> (wie bei rohem Zeiger)
    ressource->info();

    // Oder über * (Dereferenzierung)
    std::cout << "  Name via *: " << (*ressource).name << "\n";

    // .get() gibt den rohen Zeiger zurück (Besitz bleibt beim unique_ptr!)
    // Nur benutzen wenn eine Funktion rohen Zeiger erwartet (legacy code)
    Ressource* roh = ressource.get();
    std::cout << "  Roher Zeiger (kein Besitz): " << roh->name << "\n";

    std::cout << "\n-- Besitz übertragen (move) --\n";
    // unique_ptr kann NICHT kopiert werden:
    // auto kopie = ressource;   // FEHLER: copy is deleted!

    // Aber VERSCHIEBEN ist erlaubt (Besitz geht über):
    auto neuer_besitzer = std::move(ressource);
    // ressource ist jetzt "leer" (nullptr), neuer_besitzer hat den Besitz

    std::cout << "  Original leer? " << (ressource == nullptr ? "ja" : "nein") << "\n";
    neuer_besitzer->info();

    std::cout << "\n-- release() und reset() --\n";
    // release(): Gibt Besitz ab, gibt rohen Zeiger zurück, smart ptr wird nullptr
    Ressource* roh2 = neuer_besitzer.release();
    std::cout << "  Nach release(): smart ptr leer? "
              << (neuer_besitzer == nullptr ? "ja" : "nein") << "\n";
    // Jetzt sind wir SELBST für delete verantwortlich!
    delete roh2;   // Muss manuell gelöscht werden!

    std::cout << "\n-- Automatisches Löschen am Scope-Ende --\n";
    {
        auto r1 = std::make_unique<Ressource>("Temporär", 42);
        auto r2 = std::make_unique<Ressource>("Ebenfalls temporär", 99);
        std::cout << "  Beide Ressourcen aktiv\n";
    }   // ← Hier endet der Scope: r1 und r2 werden automatisch gelöscht!
    std::cout << "  Scope beendet – beide Ressourcen gelöscht\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: shared_ptr – Geteilter Besitz (Reference Counting)
// ─────────────────────────────────────────────────────────────────────────────
//
// shared_ptr<T>:
//   - Mehrere shared_ptr können dasselbe Objekt "besitzen"
//   - Intern: Referenzzähler (wie viele shared_ptr zeigen auf das Objekt?)
//   - Wenn Zähler auf 0 fällt → Objekt wird gelöscht
//   - Kopieren ist erlaubt → Zähler wird erhöht
//
// Analogie: Bibliotheksbuch – mehrere Personen können es "haben" (ausleihen).
//           Wenn alle es zurückgegeben haben, kommt es ins Regal zurück.
//
// Wann shared_ptr?
//   → Wenn mehrere Objekte denselben Besitz teilen müssen.
//   → Cache-Einträge, Observer-Patterns, geteilte Konfigurationen.
//
// Achtung: shared_ptr hat Overhead (Heap-Allokation für Kontrollblock,
//          atomarer Referenzzähler). Nicht unnötig einsetzen!

void demo_shared_ptr() {
    std::cout << "\n=== 2. shared_ptr ===\n";

    std::cout << "-- Grundlegende Nutzung --\n";
    auto sp1 = std::make_shared<Ressource>("Geteilte Ressource", 50);
    // use_count() gibt den aktuellen Referenzzähler zurück
    std::cout << "  Referenzzähler nach Erzeugung: " << sp1.use_count() << "\n"; // 1

    {
        auto sp2 = sp1;   // Kopieren erlaubt! Zähler steigt.
        std::cout << "  Referenzzähler nach Kopie: " << sp1.use_count() << "\n"; // 2

        auto sp3 = sp1;   // Noch eine Kopie
        std::cout << "  Referenzzähler nach 2. Kopie: " << sp1.use_count() << "\n"; // 3

        sp2->info();      // Alle drei zeigen auf dasselbe Objekt
        sp3->info();
    }   // sp2 und sp3 werden hier zerstört → Zähler sinkt auf 1
    std::cout << "  Referenzzähler nach Block: " << sp1.use_count() << "\n"; // 1

    std::cout << "\n-- shared_ptr in Funktionen --\n";
    // Wenn eine Funktion den Besitz teilen will, nimmt sie shared_ptr by value
    // (das erhöht den Zähler für die Dauer des Funktionsaufruf):
    auto teile_besitz = [](std::shared_ptr<Ressource> sp) {
        std::cout << "  In Funktion, Zähler: " << sp.use_count() << "\n"; // 2
        sp->info();
    };  // sp wird hier zerstört, Zähler sinkt wieder

    teile_besitz(sp1);
    std::cout << "  Nach Funktion, Zähler: " << sp1.use_count() << "\n"; // 1
    // sp1 wird am Ende dieser Funktion zerstört → Ressource gelöscht
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: weak_ptr – Beobachten ohne Besitz
// ─────────────────────────────────────────────────────────────────────────────
//
// weak_ptr<T>:
//   - "Beobachtet" ein shared_ptr, besitzt das Objekt aber NICHT
//   - Erhöht den Referenzzähler NICHT
//   - Schützt vor zirkulären Referenzen (circular references)
//   - Vor Benutzung muss geprüft werden, ob das Objekt noch lebt
//
// Was ist eine zirkuläre Referenz?
//   shared_ptr<A> → shared_ptr<B> → shared_ptr<A>
//   Beide haben Zähler ≥ 1, aber kein äußerer Code zeigt mehr drauf.
//   → Keiner löscht den anderen → Memory Leak!
//
// Lösung: Eine der Referenzen als weak_ptr → kein Kreis mehr.
//
// Analogie: Beobachter/Zeuge – kann sehen ob jemand noch lebt,
//           aber kein "Recht" auf das Objekt.

struct Knoten {
    std::string name;
    // shared_ptr würde zirkuläre Referenz erzeugen!
    // weak_ptr bricht den Kreis:
    std::weak_ptr<Knoten> naechster;

    explicit Knoten(std::string n) : name{std::move(n)} {
        std::cout << "  [+] Knoten '" << name << "'\n";
    }
    ~Knoten() {
        std::cout << "  [-] Knoten '" << name << "'\n";
    }
};

void demo_weak_ptr() {
    std::cout << "\n=== 3. weak_ptr ===\n";

    std::cout << "-- Grundlegende Nutzung --\n";
    std::weak_ptr<Ressource> beobachter;  // Noch kein Objekt

    {
        auto besitzer = std::make_shared<Ressource>("Beobachtetes Objekt", 77);
        beobachter = besitzer;   // Kein Besitz! Zähler bleibt 1.

        std::cout << "  Zähler (nur besitzer): " << besitzer.use_count() << "\n"; // 1
        std::cout << "  Objekt abgelaufen? " << (beobachter.expired() ? "ja" : "nein") << "\n";

        // weak_ptr kann NICHT direkt benutzt werden!
        // Erst lock() aufrufen → gibt shared_ptr zurück (oder nullptr)
        if (auto tmp = beobachter.lock()) {   // tmp ist shared_ptr<Ressource>
            // Jetzt Zähler temporär 2:
            std::cout << "  Zähler mit lock(): " << besitzer.use_count() << "\n"; // 2
            tmp->info();
        }   // tmp wird zerstört, Zähler zurück auf 1
    }   // besitzer wird zerstört, Ressource gelöscht

    // Nach Zerstörung des besitzers ist weak_ptr "abgelaufen"
    std::cout << "  Objekt abgelaufen? " << (beobachter.expired() ? "ja" : "nein") << "\n";
    if (auto tmp = beobachter.lock()) {
        std::cout << "  Objekt noch da!\n";     // wird nicht ausgeführt
    } else {
        std::cout << "  lock() gab nullptr – Objekt ist weg\n";
    }

    std::cout << "\n-- Zirkuläre Referenz vermeiden --\n";
    {
        auto k1 = std::make_shared<Knoten>("K1");
        auto k2 = std::make_shared<Knoten>("K2");
        // k1 → k2 (weak, kein Besitz) → wenn k1 zerstört wird, kein Problem
        k1->naechster = k2;
        k2->naechster = k1;   // weak_ptr → kein Kreis!

        std::cout << "  Beide Knoten erstellt\n";
    }   // k1 und k2 werden korrekt zerstört (wäre mit shared_ptr ein Leak!)
    std::cout << "  Beide Knoten korrekt gelöscht (kein Memory Leak)\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: Smart Pointer in Containern und als Funktionsparameter
// ─────────────────────────────────────────────────────────────────────────────
//
// Wichtige Faustregel für Funktionsparameter:
//
//   void f(Ressource& r)              → Funktion nutzt r nur, kein Besitz
//   void f(const Ressource& r)        → Funktion liest r nur
//   void f(unique_ptr<Ressource> up)  → Funktion übernimmt Besitz
//   void f(unique_ptr<Ressource>& up) → Funktion verändert Besitz (selten)
//   void f(shared_ptr<Ressource> sp)  → Funktion teilt Besitz
//
// Für Algorithmen/Container: smart pointer move-fähig → gut in vector<unique_ptr>

void demo_container() {
    std::cout << "\n=== 4. Smart Pointer in Containern ===\n";

    // vector von unique_ptr: gut für heterogene Sammlungen (z.B. Polymorphie)
    std::vector<std::unique_ptr<Ressource>> pool;

    // push_back mit move (da unique_ptr nicht kopierbar)
    pool.push_back(std::make_unique<Ressource>("CPU",  1));
    pool.push_back(std::make_unique<Ressource>("RAM",  2));
    pool.push_back(std::make_unique<Ressource>("Disk", 3));

    std::cout << "Pool-Inhalt:\n";
    for (const auto& r : pool) {
        std::cout << "  - ";
        r->info();
    }

    std::cout << "Pool wird am Ende des Scopes automatisch gelöscht:\n";
    // Alle unique_ptr im vector werden beim Destruktor des vectors gelöscht
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔═══════════════════════════════════════╗\n"
              << "║  THEMA 02 – Smart Pointer & RAII      ║\n"
              << "╚═══════════════════════════════════════╝\n";

    demo_unique_ptr();
    demo_shared_ptr();
    demo_weak_ptr();
    demo_container();

    std::cout << "\n[main() endet – keine Ressourcen werden geleakt]\n";
    return 0;
}
