// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 04 – Templates & Concepts (C++20)                                ║
// ║  Features: Funktions-Templates, Klassen-Templates,                      ║
// ║            Template-Spezialisierung, if constexpr, Concepts (C++20)     ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// ──── Warum Templates? ────────────────────────────────────────────────────
//
//   Ohne Templates: Code-Duplikation für jeden Typ
//     int    max_int(int a, int b)       { return a > b ? a : b; }
//     double max_double(double a, double b) { return a > b ? a : b; }
//     std::string max_string(...)        { ... }
//
//   Mit Templates: Ein Code, viele Typen
//     template<typename T>
//     T max_wert(T a, T b) { return a > b ? a : b; }
//
//   Templates sind Zero-Cost: Der Compiler generiert zur Compile-Zeit
//   spezialisierte Versionen für jeden genutzten Typ.
//   Laufzeit-Overhead: 0.

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <concepts>    // für std::integral, std::floating_point etc.
#include <type_traits> // für std::is_same_v, std::remove_reference_t etc.

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: Funktions-Templates
// ─────────────────────────────────────────────────────────────────────────────
//
// Syntax: template<typename T>  oder  template<class T>
//         (typename und class sind hier gleichbedeutend)
//
// T ist ein Platzhalter-Typ. Der Compiler ersetzt T durch den tatsächlichen
// Typ wenn die Funktion aufgerufen wird.

// Einfaches Funktions-Template
template<typename T>
T maximum(T a, T b) {
    // Voraussetzung: T muss den Operator > unterstützen
    return a > b ? a : b;
}

// Template mit mehreren Typ-Parametern
template<typename T, typename U>
void typ_info(const T& a, const U& b) {
    // __PRETTY_FUNCTION__ zeigt den vollständigen Funktionsnamen mit Typen
    std::cout << "  T=" << typeid(a).name()
              << ", U=" << typeid(b).name() << "\n";
    std::cout << "  Werte: " << a << " und " << b << "\n";
}

// Template mit Non-Type Parameter: Werte statt Typen als Parameter
// Hier: N ist eine Compile-Zeit-Konstante (kein Typ, sondern ein Wert)
template<typename T, std::size_t N>
T summe(const std::array<T, N>& arr) {
    T ergebnis{};   // Wert-Initialisierung: 0 für int, 0.0 für double, etc.
    for (const auto& x : arr) ergebnis += x;
    return ergebnis;
}

void demo_funktions_templates() {
    std::cout << "\n=== 1. Funktions-Templates ===\n";

    // Typableitung: Compiler ermittelt T aus den Argumenten
    std::cout << "  maximum(3, 7) = " << maximum(3, 7) << "\n";         // T=int
    std::cout << "  maximum(3.14, 2.71) = " << maximum(3.14, 2.71) << "\n"; // T=double
    std::cout << "  maximum(\"abc\", \"xyz\") = "
              << maximum(std::string{"abc"}, std::string{"xyz"}) << "\n"; // T=string

    // Explizite Typangabe (wenn Ableitung nicht funktioniert)
    std::cout << "  maximum<double>(3, 3.14) = "
              << maximum<double>(3, 3.14) << "\n";   // int wird zu double konvertiert

    std::cout << "\n  Mehrere Typ-Parameter:\n";
    typ_info(42, 3.14);
    typ_info(std::string{"Hallo"}, true);

    std::cout << "\n  Non-Type Template Parameter:\n";
    std::array<int, 5> zahlen{1, 2, 3, 4, 5};
    std::cout << "  Summe von {1,2,3,4,5} = " << summe(zahlen) << "\n";

    std::array<double, 3> komma{1.1, 2.2, 3.3};
    std::cout << "  Summe von {1.1,2.2,3.3} = " << summe(komma) << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: Klassen-Templates
// ─────────────────────────────────────────────────────────────────────────────
//
// Klassen-Templates erlauben generische Datenstrukturen.
// Die STL (Standard Template Library) nutzt sie überall:
//   std::vector<T>, std::map<K,V>, std::pair<T,U>, std::optional<T> ...

// Einfacher typsicherer Stack (LIFO-Datenstruktur)
template<typename T>
class Stack {
public:
    // Elemente hinzufügen
    void push(const T& wert) {
        daten_.push_back(wert);
    }

    // Mit Move (effizienter wenn rvalue übergeben wird)
    void push(T&& wert) {
        daten_.push_back(std::move(wert));
    }

    // Oberstes Element entfernen und zurückgeben
    T pop() {
        if (leer()) throw std::runtime_error("Stack ist leer!");
        T oben = std::move(daten_.back());
        daten_.pop_back();
        return oben;
    }

    // Oberstes Element ansehen (ohne zu entfernen)
    const T& peek() const {
        if (leer()) throw std::runtime_error("Stack ist leer!");
        return daten_.back();
    }

    bool leer() const { return daten_.empty(); }
    std::size_t groesse() const { return daten_.size(); }

private:
    std::vector<T> daten_;  // T wird erst bei der Instanziierung eingesetzt
};

void demo_klassen_templates() {
    std::cout << "\n=== 2. Klassen-Templates ===\n";

    // int-Stack
    Stack<int> int_stack;
    int_stack.push(10);
    int_stack.push(20);
    int_stack.push(30);
    std::cout << "  int-Stack: " << int_stack.groesse() << " Elemente\n";
    std::cout << "  peek: " << int_stack.peek() << "\n";
    std::cout << "  pop: " << int_stack.pop() << "\n";
    std::cout << "  pop: " << int_stack.pop() << "\n";

    // string-Stack – dieselbe Klasse, anderer Typ!
    Stack<std::string> str_stack;
    str_stack.push("Erster");
    str_stack.push("Zweiter");
    str_stack.push("Dritter");
    std::cout << "\n  string-Stack: " << str_stack.groesse() << " Elemente\n";
    while (!str_stack.leer()) {
        std::cout << "  pop: \"" << str_stack.pop() << "\"\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: Template-Spezialisierung
// ─────────────────────────────────────────────────────────────────────────────
//
// Manchmal braucht ein bestimmter Typ eine besondere Implementierung.
// Beispiel: void ausgabe(T x) soll für bool "ja/nein" statt "1/0" ausgeben.
//
// Vollständige Spezialisierung: template<>  (alle Typ-Parameter festgelegt)
// Partielle Spezialisierung:    template<typename T>  (einige festgelegt)

// Primäre Template-Definition (für alle Typen)
template<typename T>
void ausgabe(const T& wert) {
    std::cout << "  [allgemein] " << wert << "\n";
}

// Vollständige Spezialisierung für bool
template<>
void ausgabe<bool>(const bool& wert) {
    std::cout << "  [bool]      " << (wert ? "ja (true)" : "nein (false)") << "\n";
}

// Vollständige Spezialisierung für std::string
template<>
void ausgabe<std::string>(const std::string& wert) {
    std::cout << "  [string]    \"" << wert << "\" (Länge: " << wert.size() << ")\n";
}

// Partielle Spezialisierung für std::vector<T> (T bleibt variabel)
template<typename T>
void ausgabe(const std::vector<T>& vec) {
    std::cout << "  [vector]    [";
    for (std::size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << vec[i];
    }
    std::cout << "]\n";
}

void demo_spezialisierung() {
    std::cout << "\n=== 3. Template-Spezialisierung ===\n";

    ausgabe(42);                              // allgemein (int)
    ausgabe(3.14);                            // allgemein (double)
    ausgabe(true);                            // speziell (bool)
    ausgabe(false);                           // speziell (bool)
    ausgabe(std::string{"Hallo"});            // speziell (string)
    ausgabe(std::vector<int>{1, 2, 3, 4});   // partiell (vector<int>)
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: if constexpr (C++17) – Compile-Zeit-Verzweigung
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem: In Templates wollen wir manchmal verschiedene Code-Pfade
//          je nach Typ haben. Normale if-Bedingungen werden zur Laufzeit
//          ausgewertet – aber beide Zweige müssen für alle Typen kompilierbar sein!
//
// Lösung: if constexpr – der nicht-genommene Zweig wird gar nicht kompiliert.
//
// Vergleich:
//   if (bedingung) { ... }           → Laufzeit, beide Zweige müssen kompilieren
//   if constexpr (bedingung) { ... } → Compile-Zeit, nur genommener Zweig kompiliert

template<typename T>
std::string typ_beschreibung() {
    // std::is_integral_v<T>      = true für int, char, bool, long, ...
    // std::is_floating_point_v<T> = true für float, double, long double
    // std::is_same_v<T, U>       = true wenn T und U identisch sind

    if constexpr (std::is_same_v<T, bool>) {
        return "Boolescher Wert (true/false)";
    } else if constexpr (std::is_integral_v<T>) {
        return "Ganzzahl (" + std::to_string(sizeof(T)) + " Bytes)";
    } else if constexpr (std::is_floating_point_v<T>) {
        return "Gleitkommazahl (" + std::to_string(sizeof(T)) + " Bytes)";
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "std::string";
    } else {
        return "Unbekannter Typ";
    }
    // Nur der passende Zweig wird kompiliert!
    // Der Rest wird vom Compiler ignoriert.
}

void demo_if_constexpr() {
    std::cout << "\n=== 4. if constexpr ===\n";

    std::cout << "  bool:   " << typ_beschreibung<bool>()   << "\n";
    std::cout << "  int:    " << typ_beschreibung<int>()    << "\n";
    std::cout << "  long:   " << typ_beschreibung<long>()   << "\n";
    std::cout << "  float:  " << typ_beschreibung<float>()  << "\n";
    std::cout << "  double: " << typ_beschreibung<double>() << "\n";
    std::cout << "  string: " << typ_beschreibung<std::string>() << "\n";
    std::cout << "  char*:  " << typ_beschreibung<char*>()  << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 5: Concepts (C++20) – Anforderungen an Template-Parameter
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem mit klassischen Templates:
//   template<typename T>
//   T maximum(T a, T b) { return a > b ? a : b; }
//
//   maximum(Foo{}, Bar{});   // Kompiliert? Fehlermeldung = Zeichensalat!
//   Error: no match for 'operator>' in ... (seitenweise Fehlermeldungen)
//
// Lösung: Concepts definieren explizit, welche Typen erlaubt sind.
//   Fehlermeldungen werden klar und lesbar.
//
// C++20 bringt vordefinierte Concepts in <concepts>:
//   std::integral, std::floating_point, std::totally_ordered,
//   std::copy_constructible, std::invocable, ...

// ── Eigenes Concept definieren ────────────────────────────────────────────
// Syntax: concept Name = Anforderungs-Ausdruck;
// Der Ausdruck muss zur Compile-Zeit true oder false ergeben.

// Concept: T muss addierbar sein (T + T muss funktionieren)
template<typename T>
concept Addierbar = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;  // a+b muss zu T konvertierbar sein
};

// Concept: T muss vergleichbar und kopierbar sein
template<typename T>
concept Vergleichbar = std::totally_ordered<T> && std::copy_constructible<T>;

// Concept für numerische Typen (Ganzzahl oder Gleitkomma)
template<typename T>
concept Numerisch = std::integral<T> || std::floating_point<T>;

// ── Concepts in Funktionen verwenden ─────────────────────────────────────

// Variante 1: requires-Klausel (explizit)
template<typename T>
    requires Vergleichbar<T>
T sicheres_maximum(T a, T b) {
    return a > b ? a : b;
}

// Variante 2: Concept direkt als Typ-Parameter (kompakter)
template<Numerisch T>
T quadrat(T x) {
    return x * x;
}

// Variante 3: auto mit Concept (kürzeste Schreibweise, C++20 abbreviated templates)
auto addiere(Addierbar auto a, Addierbar auto b) {
    return a + b;
}

void demo_concepts() {
    std::cout << "\n=== 5. Concepts (C++20) ===\n";

    // Vergleichbar: int, double, string ✓
    std::cout << "  sicheres_maximum(5, 3) = "
              << sicheres_maximum(5, 3) << "\n";
    std::cout << "  sicheres_maximum(2.5, 1.7) = "
              << sicheres_maximum(2.5, 1.7) << "\n";
    std::cout << "  sicheres_maximum(\"abc\", \"xyz\") = "
              << sicheres_maximum(std::string{"abc"}, std::string{"xyz"}) << "\n";

    // Numerisch: int, double ✓
    std::cout << "  quadrat(7) = " << quadrat(7) << "\n";
    std::cout << "  quadrat(2.5) = " << quadrat(2.5) << "\n";
    // quadrat(std::string{"x"});  ← Compiler-Fehler mit klarer Meldung!

    // Addierbar: int, double, string ✓
    std::cout << "  addiere(3, 4) = " << addiere(3, 4) << "\n";
    std::cout << "  addiere(1.5, 2.5) = " << addiere(1.5, 2.5) << "\n";
    std::cout << "  addiere(string+string) = "
              << addiere(std::string{"Hallo "}, std::string{"Welt"}) << "\n";

    // Vordefinierte Concepts aus <concepts>:
    std::cout << "\n  Vordefinierte Concepts (Beispiele):\n";
    std::cout << "  std::integral<int>:          "
              << std::integral<int> << "\n";
    std::cout << "  std::floating_point<double>: "
              << std::floating_point<double> << "\n";
    std::cout << "  std::integral<double>:       "
              << std::integral<double> << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔══════════════════════════════════════════╗\n"
              << "║  THEMA 04 – Templates & Concepts (C++20) ║\n"
              << "╚══════════════════════════════════════════╝\n";

    demo_funktions_templates();
    demo_klassen_templates();
    demo_spezialisierung();
    demo_if_constexpr();
    demo_concepts();

    return 0;
}
