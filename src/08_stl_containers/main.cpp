// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 08 – STL: Container & Algorithmen                                ║
// ║  Features: vector, array, map, unordered_map, set, deque,               ║
// ║            Iteratoren, Erase-Remove, partition, iota, ranges            ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// ──── Überblick: Container-Typen ─────────────────────────────────────────
//
//  Sequence Containers (Elemente in definierter Reihenfolge):
//    std::vector<T>        Dynamisches Array, O(1) Zugriff, O(n) Insert mitte
//    std::array<T, N>      Festes Array (Stack), Zero-Overhead, N zur Compile-Zeit
//    std::deque<T>         Doppelseiting-Queue, O(1) vorne/hinten, O(n) mitte
//    std::list<T>          Doppelt verknüpfte Liste, O(1) Insert/Erase überall
//    std::forward_list<T>  Einfach verknüpfte Liste, minimaler Overhead
//
//  Associative Containers (Elemente nach Key sortiert, O(log n)):
//    std::map<K,V>         Geordnete Key-Value Paare, Keys eindeutig
//    std::multimap<K,V>    Wie map, Keys dürfen doppelt vorkommen
//    std::set<T>           Geordnete Menge einzigartiger Werte
//    std::multiset<T>      Wie set, Duplikate erlaubt
//
//  Unordered Containers (Hash-basiert, O(1) amortisiert):
//    std::unordered_map<K,V>   Hash-Map, Keys eindeutig
//    std::unordered_set<T>     Hash-Menge, Elemente eindeutig
//
//  Container Adaptors (bauen auf anderen Containern auf):
//    std::stack<T>         LIFO (Last In, First Out)
//    std::queue<T>         FIFO (First In, First Out)
//    std::priority_queue<T> Heap: größtes Element immer oben

#include <iostream>
#include <vector>
#include <array>
#include <deque>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <stack>
#include <queue>
#include <algorithm>   // sort, find, count_if, remove_if, partition, unique, etc.
#include <numeric>     // iota, accumulate, inner_product
#include <string>
#include <iterator>    // back_inserter, inserter, ostream_iterator

// Hilfsfunktion: Container-Inhalt ausgeben
template<typename Container>
void print(std::string_view label, const Container& c) {
    std::cout << "  " << label << ": [";
    bool first = true;
    for (const auto& x : c) {
        if (!first) std::cout << ", ";
        std::cout << x;
        first = false;
    }
    std::cout << "]\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: std::vector – der meistgenutzte Container
// ─────────────────────────────────────────────────────────────────────────────
//
// Interner Aufbau: Heap-allokiertes Array.
// Wenn capacity erschöpft: neuer Speicher (~doppelt so groß), alle Elemente kopiert/bewegt.
//
// Wichtige Methoden:
//   .push_back(x)       → hinten anfügen (kopiert)
//   .emplace_back(args) → hinten konstruieren (effizienter, kein Copy/Move)
//   .pop_back()         → letztes Element entfernen
//   .insert(it, x)      → vor Iterator einfügen (verschiebt Elemente!)
//   .erase(it)          → an Iterator löschen (verschiebt Elemente!)
//   .reserve(n)         → Speicher für n Elemente vorallokieren (kein Realloc!)
//   .resize(n)          → Größe auf n setzen (auffüllen oder abschneiden)
//   .size()             → Anzahl aktueller Elemente
//   .capacity()         → Anzahl belegter Speicherplätze
//   .clear()            → Alle Elemente löschen (capacity bleibt!)
//   .at(i)              → Zugriff mit Bounds-Check (wirft std::out_of_range)
//   [i]                 → Zugriff ohne Bounds-Check (schneller, undefined bei Fehler)

void demo_vector() {
    std::cout << "\n=== 1. std::vector ===\n";

    // Verschiedene Konstruktions-Arten
    std::vector<int> v1;             // leerer Vektor
    std::vector<int> v2(5, 42);     // 5 Elemente, alle = 42
    std::vector<int> v3{1,2,3,4,5}; // Initialisierungsliste
    std::vector<int> v4(v3);        // Kopie von v3

    print("v2 (5 × 42)", v2);
    print("v3 {1..5}",   v3);

    // capacity vs. size: wichtig für Performance!
    std::vector<int> perf;
    std::cout << "\n  Ohne reserve – Reallokationen:\n";
    for (int i = 0; i < 8; ++i) {
        perf.push_back(i);
        std::cout << "    size=" << perf.size() << " capacity=" << perf.capacity() << "\n";
    }

    std::vector<int> fast;
    fast.reserve(8);  // Ein Mal allokieren, kein Realloc!
    std::cout << "  Mit reserve(8) – capacity von Anfang: " << fast.capacity() << "\n";
    for (int i = 0; i < 8; ++i) fast.push_back(i);
    std::cout << "  Nach 8 push_back – capacity: " << fast.capacity() << "\n";

    // Einfügen und Löschen
    std::vector<int> v{10, 20, 30, 40, 50};
    print("\n  Vor erase(index 2)", v);
    v.erase(v.begin() + 2);          // 30 löschen
    print("  Nach erase",            v);

    v.insert(v.begin() + 1, 99);     // 99 an Index 1 einfügen
    print("  Nach insert(1, 99)",    v);

    // emplace_back: konstruiert direkt im vector (kein Move/Copy nötig)
    struct Punkt { int x, y; };
    std::vector<Punkt> punkte;
    punkte.emplace_back(1, 2);  // Punkt direkt konstruiert – kein Punkt{1,2} erstellt
    punkte.emplace_back(3, 4);
    std::cout << "\n  emplace_back Punkte: ";
    for (const auto& p : punkte) std::cout << "(" << p.x << "," << p.y << ") ";
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: std::array – festes Array, Zero-Overhead
// ─────────────────────────────────────────────────────────────────────────────
//
// Im Gegensatz zu vector:
//   - Größe N ist Compile-Zeit-Konstante → kein Heap, kein Overhead
//   - Kann nicht verkleinert oder vergrößert werden
//   - Ideal für kleine, bekannte Datenmengen (z.B. Koordinaten, Farben)
//
// Im Gegensatz zu C-Array (int arr[5]):
//   - Hat .size(), .begin(), .end() → funktioniert mit allen Algorithmen
//   - Hat .at() für bounds-checked Zugriff
//   - Kann by-value kopiert werden (kein Decay zu Zeiger!)

void demo_array() {
    std::cout << "\n=== 2. std::array ===\n";

    // Größe MUSS zur Compile-Zeit bekannt sein
    std::array<int, 5> arr{1, 2, 3, 4, 5};
    std::array<double, 3> rgb{0.5, 0.8, 1.0};  // R, G, B (0.0–1.0)
    std::cout << "  RGB: " << rgb[0] << ", " << rgb[1] << ", " << rgb[2] << "\n";

    std::cout << "  arr.size() = " << arr.size() << "\n";
    std::cout << "  arr[2]     = " << arr[2]     << "\n";
    std::cout << "  arr.front()= " << arr.front() << "\n";
    std::cout << "  arr.back() = " << arr.back()  << "\n";

    // Funktioniert mit allen STL-Algorithmen
    std::sort(arr.begin(), arr.end(), std::greater<int>{});   // absteigend
    print("  Sortiert abst.", arr);

    // Aggregat-Initialisierung + CTAD (Class Template Argument Deduction, C++17)
    // Typ und Größe werden automatisch abgeleitet:
    auto farbe = std::array{255, 128, 0};   // std::array<int, 3>
    std::cout << "  RGB: " << farbe[0] << ", " << farbe[1] << ", " << farbe[2] << "\n";

    // Stack-Vergleich: array liegt auf dem Stack, vector auf dem Heap
    std::cout << "  sizeof(arr):                 " << sizeof(arr) << " Bytes (Stack)\n";
    std::vector<int> vgl{1,2,3,4,5};
    std::cout << "  sizeof(vector<int>):         " << sizeof(vgl) << " Bytes (Header, Daten auf Heap)\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: std::map und std::unordered_map
// ─────────────────────────────────────────────────────────────────────────────
//
// std::map<K, V>:
//   - Intern: Rot-Schwarz-Baum → immer sortiert nach Key
//   - O(log n) für insert, find, erase
//   - Keys müssen vergleichbar sein (operator< oder Comparator)
//   - Iteration in Key-Reihenfolge
//
// std::unordered_map<K, V>:
//   - Intern: Hash-Tabelle
//   - O(1) amortisiert für insert, find, erase (O(n) worst case)
//   - Keys müssen hashbar sein (std::hash<K> spezialisiert)
//   - Iteration in undefinierten Reihenfolge
//
// Wann welchen?
//   → Brauchst du sortierte Reihenfolge?          → map
//   → Brauchst du maximale Performance?           → unordered_map
//   → Key ist ein benutzerdefinierter Typ?        → map (einfacher)

void demo_map() {
    std::cout << "\n=== 3. map & unordered_map ===\n";

    // std::map: Automatisch nach Key sortiert!
    std::map<std::string, int> alter{
        {"Zara", 28}, {"Alice", 25}, {"Bob", 30}, {"Mia", 22}
    };

    std::cout << "  map (nach Key sortiert):\n";
    for (const auto& [name, jahre] : alter) {   // structured binding
        std::cout << "    " << name << ": " << jahre << "\n";
    }

    // Verschiedene Wege um einzufügen:
    alter["Daniel"] = 35;                        // operator[]: erstellt bei Bedarf
    alter.insert({"Emma", 27});                  // insert: nur wenn Key NEU
    alter.emplace("Felix", 31);                  // emplace: konstruiert in-place
    alter.insert_or_assign("Alice", 26);         // Einfügen oder Überschreiben (C++17)

    std::cout << "  Nach Einfügungen: " << alter.size() << " Einträge\n";

    // Suchen: .find() gibt Iterator zurück (kein throw bei nicht gefunden)
    if (auto it = alter.find("Bob"); it != alter.end()) {
        std::cout << "  Bob gefunden: " << it->second << "\n";
    }
    if (alter.count("Zara")) {   // count: 0 oder 1 bei map
        std::cout << "  Zara ist vorhanden\n";
    }
    // contains (C++20): eleganter als count
    std::cout << "  'Xaver' vorhanden: " << alter.contains("Xaver") << "\n";

    // std::unordered_map: Schneller, keine Reihenfolge
    std::cout << "\n  unordered_map (keine Reihenfolge):\n";
    std::unordered_map<std::string, double> preise{
        {"Apfel", 0.99}, {"Birne", 1.29}, {"Banane", 0.49}
    };
    for (const auto& [name, preis] : preise) {
        std::cout << "    " << name << ": " << preis << "€\n";
    }

    // try_emplace (C++17): Effizientes bedingtes Einfügen
    // Konstruiert Wert NUR wenn Key nicht vorhanden (keine unnötige Objekt-Erzeugung)
    auto [it, inserted] = preise.try_emplace("Mango", 2.49);
    std::cout << "  Mango eingefügt: " << inserted
              << ", Preis: " << it->second << "€\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: std::set und std::unordered_set
// ─────────────────────────────────────────────────────────────────────────────
//
// set/unordered_set: Wie map/unordered_map, aber nur Keys, kein Value.
// Haupteigenschaft: Jeder Wert ist EINZIGARTIG. Duplikate werden ignoriert.
//
// Anwendungsfälle:
//   - Duplikate entfernen
//   - Schnell prüfen ob ein Wert bereits gesehen wurde
//   - Mengenoperationen (Schnittmenge, Vereinigung)

void demo_set() {
    std::cout << "\n=== 4. set & unordered_set ===\n";

    // set: Automatisch sortiert, Duplikate werden ignoriert
    std::set<int> s{5, 3, 8, 1, 3, 5, 9, 2};  // Duplikate: 3 und 5
    print("  set (sortiert, eindeutig)", s);

    s.insert(7);
    s.insert(3);   // Duplikat: wird ignoriert
    print("  Nach insert(7, 3)", s);

    // Schnelle Suche
    std::cout << "  8 vorhanden: " << s.contains(8) << "\n";
    std::cout << "  6 vorhanden: " << s.contains(6) << "\n";

    // Duplikate aus vector entfernen (klassischer Trick)
    std::vector<int> v{1, 4, 2, 4, 3, 1, 5, 2, 3};
    std::set<int> unique_set(v.begin(), v.end());
    std::vector<int> dedupliziert(unique_set.begin(), unique_set.end());
    print("  Original",      v);
    print("  Dedupliziert",  dedupliziert);

    // Schnittmenge zweier sets
    std::set<int> a{1, 2, 3, 4, 5};
    std::set<int> b{3, 4, 5, 6, 7};
    std::vector<int> schnitt;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
                          std::back_inserter(schnitt));
    print("  Schnittmenge {1..5} ∩ {3..7}", schnitt);
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 5: Wichtige Algorithmen (über Lambdas hinaus)
// ─────────────────────────────────────────────────────────────────────────────

void demo_algorithmen() {
    std::cout << "\n=== 5. Wichtige Algorithmen ===\n";

    std::vector<int> v{1, 5, 2, 8, 3, 7, 4, 6, 9};
    print("  Original", v);

    // std::iota: Füllt mit aufsteigenden Werten (1, 2, 3, ...)
    std::vector<int> seq(10);
    std::iota(seq.begin(), seq.end(), 1);   // 1, 2, 3, ..., 10
    print("  iota(1..10)", seq);

    // std::any_of / all_of / none_of
    bool hat_gerade = std::any_of(v.begin(), v.end(),
                                   [](int x){ return x % 2 == 0; });
    bool alle_positiv = std::all_of(v.begin(), v.end(),
                                     [](int x){ return x > 0; });
    bool kein_negativ = std::none_of(v.begin(), v.end(),
                                      [](int x){ return x < 0; });
    std::cout << "\n  any_of(gerade):     " << hat_gerade   << "\n";
    std::cout << "  all_of(>0):         " << alle_positiv << "\n";
    std::cout << "  none_of(<0):        " << kein_negativ << "\n";

    // std::min_element / max_element
    auto min_it = std::min_element(v.begin(), v.end());
    auto max_it = std::max_element(v.begin(), v.end());
    std::cout << "\n  min=" << *min_it << ", max=" << *max_it << "\n";

    // Erase-Remove-Idiom: Alle geraden Zahlen entfernen
    // (std::remove_if "schiebt" Elemente vor, erase kürzt den vector)
    std::vector<int> gefiltert = v;
    gefiltert.erase(
        std::remove_if(gefiltert.begin(), gefiltert.end(),
                       [](int x){ return x % 2 == 0; }),
        gefiltert.end()
    );
    print("\n  Nach Erase-Remove (ungerade only)", gefiltert);

    // C++20: std::erase_if (eleganter!)
    std::vector<int> v2 = v;
    std::erase_if(v2, [](int x){ return x % 2 == 0; });
    print("  C++20 erase_if (gleiches Ergebnis)", v2);

    // std::partition: Elemente aufteilen (stabile Reihenfolge innerhalb Gruppen)
    std::vector<int> geteilt = v;
    auto trennpunkt = std::stable_partition(
        geteilt.begin(), geteilt.end(),
        [](int x){ return x % 2 == 0; }   // Gerade zuerst
    );
    print("\n  Nach partition (gerade | ungerade)", geteilt);
    std::cout << "  Trennpunkt (erstes Ungerades): " << *trennpunkt << "\n";

    // std::unique: Doppelte aufeinanderfolgende Elemente entfernen
    // (Erst sortieren, dann unique + erase!)
    std::vector<int> mit_duplik{1,1,2,3,3,3,4,5,5};
    auto new_end = std::unique(mit_duplik.begin(), mit_duplik.end());
    mit_duplik.erase(new_end, mit_duplik.end());
    print("\n  Nach sort+unique", mit_duplik);

    // std::partial_sort: Nur die k kleinsten Elemente sortiert vorne
    std::vector<int> ps{9,3,7,1,5,8,2,6,4};
    std::partial_sort(ps.begin(), ps.begin() + 3, ps.end());  // 3 kleinste vorne
    print("\n  partial_sort (3 kleinste vorne)", ps);
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 6: Container Adaptors – Stack, Queue, Priority Queue
// ─────────────────────────────────────────────────────────────────────────────

void demo_adaptors() {
    std::cout << "\n=== 6. Container Adaptors ===\n";

    // stack<T>: LIFO – letztes Element raus zuerst
    std::cout << "  stack (LIFO):\n";
    std::stack<int> stk;
    for (int i : {1, 2, 3, 4, 5}) stk.push(i);
    std::cout << "    Reihenfolge beim Entnehmen: ";
    while (!stk.empty()) {
        std::cout << stk.top() << " ";
        stk.pop();
    }
    std::cout << "\n";

    // queue<T>: FIFO – erstes Element raus zuerst
    std::cout << "  queue (FIFO):\n";
    std::queue<std::string> q;
    q.push("Erster");
    q.push("Zweiter");
    q.push("Dritter");
    std::cout << "    Reihenfolge beim Entnehmen: ";
    while (!q.empty()) {
        std::cout << q.front() << " ";
        q.pop();
    }
    std::cout << "\n";

    // priority_queue<T>: Größtes Element immer oben (Heap)
    std::cout << "  priority_queue (größtes zuerst):\n";
    std::priority_queue<int> pq;
    for (int i : {3, 1, 4, 1, 5, 9, 2, 6}) pq.push(i);
    std::cout << "    Reihenfolge beim Entnehmen: ";
    while (!pq.empty()) {
        std::cout << pq.top() << " ";
        pq.pop();
    }
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔══════════════════════════════════════════╗\n"
              << "║  THEMA 08 – STL: Container & Algorithmen ║\n"
              << "╚══════════════════════════════════════════╝\n";
    std::cout << std::boolalpha;

    demo_vector();
    demo_array();
    demo_map();
    demo_set();
    demo_algorithmen();
    demo_adaptors();

    return 0;
}
