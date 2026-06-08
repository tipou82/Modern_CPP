// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 06 – std::optional, std::variant, std::visit                     ║
// ║  Features: optional (fehlende Werte), variant (typsichere Union),       ║
// ║            visit (Pattern Matching), any (beliebige Typen)              ║
// ╚══════════════════════════════════════════════════════════════════════════╝
// MISRA C++:2023 – Schlüsselregeln in diesem Thema:
//   Rule 18.1.1 (Required) – An exception object shall not have pointer type
//                             → std::optional / std::expected als Alternative
//   std::optional verhindert Null-Dereferenzierung (MISRA-Sicherheitsziel).
//   std::variant ersetzt union: kein undefined behavior, volles RAII.
//
// ──── Das Problem: "Kein Wert" ausdrücken ─────────────────────────────────
//
//   Wie sagt man "diese Funktion gibt manchmal keinen Wert zurück"?
//
//   Alte Lösungen (alle problematisch):
//     int suche(...)  { return -1; }     // Magic Number: Was wenn -1 gültig?
//     bool suche(..., int& ergebnis)     // Out-Parameter: unergonomisch
//     int* suche(...)                    // Zeiger: darf nicht nullptr sein?
//     throw wenn nicht gefunden          // Exception: zu schwer für "normal"
//
//   Moderne Lösung:
//     std::optional<int> suche(...)      // Klar: "gibt int oder nichts zurück"
//
// ──── Das Problem: Heterogene Typen ───────────────────────────────────────
//
//   Manchmal muss ein Wert verschiedene Typen haben können:
//   "Ergebnis ist entweder ein int (Erfolg) oder ein string (Fehlermeldung)"
//
//   Alte Lösung: union – unsicher, kein Destruktor, kein Typtracking
//   Neue Lösung: std::variant<int, std::string> – typsicher, RAII

#include <iostream>
#include <optional>   // std::optional
#include <variant>    // std::variant, std::visit, std::get, std::holds_alternative
#include <any>        // std::any, std::any_cast
#include <string>
#include <vector>
#include <map>

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: std::optional<T>
// ─────────────────────────────────────────────────────────────────────────────
//
// std::optional<T> ist entweder:
//   - "enthält einen T-Wert" (engaged/has_value)
//   - "enthält keinen Wert" (disengaged/empty, wie std::nullopt)
//
// Wichtige Methoden:
//   .has_value()   → bool: enthält Wert?
//   .value()       → T:    Wert (wirft std::bad_optional_access wenn leer)
//   .value_or(x)   → T:    Wert oder Fallback x
//   *opt           → T:    Wert (wie Zeiger-Dereferenzierung, undefined wenn leer!)
//   opt.reset()    → Wert löschen (optional wird leer)

// Funktion die einen optionalen Wert zurückgibt
std::optional<int> finde_in_vektor(const std::vector<int>& v, int ziel) {
    for (std::size_t i = 0; i < v.size(); ++i) {
        if (v[i] == ziel) {
            return static_cast<int>(i);   // Gefunden: Index zurückgeben
        }
    }
    return std::nullopt;   // Nicht gefunden: "kein Wert"
}

// Kette von optionalen Operationen (optional chaining)
std::optional<std::string> hole_stadtteil(
    const std::map<std::string, std::string>& stadtteile,
    const std::string& schluessel)
{
    auto it = stadtteile.find(schluessel);
    if (it == stadtteile.end()) return std::nullopt;
    return it->second;
}

void demo_optional() {
    std::cout << "\n=== 1. std::optional ===\n";

    std::vector<int> zahlen{10, 20, 30, 40, 50};

    // Gefundener Fall
    auto idx = finde_in_vektor(zahlen, 30);
    if (idx.has_value()) {
        std::cout << "  30 gefunden bei Index: " << idx.value() << "\n";
    }

    // Nicht gefundener Fall
    auto idx2 = finde_in_vektor(zahlen, 99);
    if (!idx2.has_value()) {
        std::cout << "  99 nicht gefunden\n";
    }

    // value_or: Fallback-Wert ohne if-Abfrage
    std::cout << "  30 bei Index: " << finde_in_vektor(zahlen, 30).value_or(-1) << "\n";
    std::cout << "  99 bei Index: " << finde_in_vektor(zahlen, 99).value_or(-1) << "\n";

    // optional als Schnittstelle mit Prüfung
    std::cout << "\n  Optional als Funktionsparameter (optionale Config):\n";

    auto erstelle_verbindung = [](const std::string& host,
                                   std::optional<int> port = std::nullopt) {
        int p = port.value_or(8080);   // Standard-Port falls nicht angegeben
        std::cout << "  Verbinde zu " << host << ":" << p << "\n";
    };
    erstelle_verbindung("server.example.com");          // nutzt Port 8080
    erstelle_verbindung("api.example.com", 443);        // nutzt Port 443

    // optional in Schleife (if mit Initialisierung)
    std::cout << "\n  Suche mehrerer Werte:\n";
    std::vector<int> suchziele{20, 99, 50, 42};
    for (int ziel : suchziele) {
        if (auto pos = finde_in_vektor(zahlen, ziel)) {  // optional ist truthy wenn Wert
            std::cout << "  " << ziel << " → Index " << *pos << "\n";
        } else {
            std::cout << "  " << ziel << " → nicht gefunden\n";
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: std::variant<T1, T2, ...>
// ─────────────────────────────────────────────────────────────────────────────
//
// std::variant<T1, T2, T3> enthält genau EINEN der aufgelisteten Typen.
// Im Gegensatz zu union:
//   - Typsicher: Weiß immer welcher Typ aktiv ist
//   - RAII: Ruft Konstruktor/Destruktor automatisch auf
//   - Kein undefined behavior beim falschen Zugriff (wirft std::bad_variant_access)
//
// ⚠️  MISRA C++:2023 – std::variant ist der MISRA-konforme Ersatz für union.
//     C-unions haben keinen Destruktor, kein Typ-Tracking und erzeugen
//     undefined behavior beim Zugriff auf den falschen Member.
//
// Wichtige Operationen:
//   std::holds_alternative<T>(v)  → bool: enthält v ein T?
//   std::get<T>(v)                → T: Wert als T (wirft wenn falscher Typ)
//   std::get<N>(v)                → Wert am Index N (0-basiert)
//   std::get_if<T>(&v)            → T* oder nullptr (kein throw)
//   v.index()                     → Index des aktiven Typs (0, 1, 2...)

// Variant als Ergebnis-Typ: Entweder Erfolg (int) oder Fehler (string)
using Ergebnis = std::variant<int, std::string>;

Ergebnis teile(int zaehler, int nenner) {
    if (nenner == 0) {
        return std::string{"Fehler: Division durch Null"};  // Fehlerfall
    }
    return zaehler / nenner;   // Erfolgsfall
}

void demo_variant() {
    std::cout << "\n=== 2. std::variant ===\n";

    // Grundlegende Nutzung
    std::variant<int, double, std::string> v;

    v = 42;
    std::cout << "  v enthält int: " << std::holds_alternative<int>(v) << "\n";
    std::cout << "  Wert: " << std::get<int>(v) << "\n";

    v = 3.14;  // Jetzt enthält v double
    std::cout << "  v enthält double: " << std::holds_alternative<double>(v) << "\n";
    std::cout << "  Wert: " << std::get<double>(v) << "\n";

    v = std::string{"Hallo"};
    std::cout << "  v enthält string: " << std::holds_alternative<std::string>(v) << "\n";
    std::cout << "  Wert: \"" << std::get<std::string>(v) << "\"\n";

    // get_if: sicherer Zugriff (kein throw)
    if (auto* s = std::get_if<std::string>(&v)) {
        std::cout << "  get_if<string>: \"" << *s << "\"\n";
    }
    if (auto* n = std::get_if<int>(&v)) {
        std::cout << "  get_if<int>: " << *n << "\n";
    } else {
        std::cout << "  get_if<int>: nullptr (kein int aktiv)\n";
    }

    // Ergebnis-Typ-Pattern
    std::cout << "\n  Ergebnis-Typ (Result<T, E>):\n";
    for (auto [a, b] : std::vector<std::pair<int,int>>{{10,2},{7,0},{15,3}}) {
        auto res = teile(a, b);
        if (std::holds_alternative<int>(res)) {
            std::cout << "  " << a << "/" << b << " = " << std::get<int>(res) << "\n";
        } else {
            std::cout << "  " << a << "/" << b << " → "
                      << std::get<std::string>(res) << "\n";
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: std::visit – Pattern Matching für variant
// ─────────────────────────────────────────────────────────────────────────────
//
// std::visit(visitor, variant) ruft den Visitor mit dem aktiven Typ auf.
// Der Visitor muss alle möglichen Typen des Variants behandeln können.
//
// Pattern Matching-Stil mit überladenen Lambdas:
//   Den Trick "struct Overloaded" nutzen (mehrere Lambdas kombinieren)

// Hilfsstruktur: Kombiniert mehrere Lambdas zu einem Visitor-Objekt
// (Erbt von allen übergebenen Typen und "importiert" deren operator())
template<typename... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
// Deduktionsguide (C++17): ermöglicht Overloaded{lambda1, lambda2} ohne <...>
template<typename... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

// Verschiedene Formen eines "Wertes" in einer Tabellenkalkulationszelle
using Zellwert = std::variant<
    int,          // Ganzzahl
    double,       // Gleitkommazahl
    std::string,  // Text
    bool          // Boolescher Wert
>;

void demo_visit() {
    std::cout << "\n=== 3. std::visit ===\n";

    // Einfacher Visitor mit Lambda (muss alle Typen behandeln!)
    auto beschreibe = [](const auto& wert) {
        std::cout << "  Wert: " << wert
                  << " (Typ: " << typeid(wert).name() << ")\n";
    };

    std::vector<Zellwert> tabelle{42, 3.14, std::string{"Hallo"}, true, -7, std::string{"Ende"}};

    std::cout << "  Einfacher Visitor:\n";
    for (const auto& zelle : tabelle) {
        std::visit(beschreibe, zelle);
    }

    // Overloaded-Pattern: verschiedenes Verhalten je nach Typ
    std::cout << "\n  Overloaded Visitor (Pattern Matching):\n";
    auto formatiere = Overloaded{
        [](int n)               { std::cout << "  INT: " << n << "\n"; },
        [](double d)            { std::cout << "  DBL: " << d << "\n"; },
        [](const std::string& s){ std::cout << "  STR: \"" << s << "\"\n"; },
        [](bool b)              { std::cout << "  BOL: " << (b ? "wahr" : "falsch") << "\n"; }
    };

    for (const auto& zelle : tabelle) {
        std::visit(formatiere, zelle);
    }

    // visit mit Rückgabewert (alle Zweige müssen denselben Typ zurückgeben)
    std::cout << "\n  visit mit Rückgabewert (alles als string):\n";
    auto als_string = Overloaded{
        [](int n)               { return std::to_string(n); },
        [](double d)            { return std::to_string(d); },
        [](const std::string& s){ return s; },
        [](bool b)              { return std::string{b ? "true" : "false"}; }
    };

    for (const auto& zelle : tabelle) {
        std::cout << "  → \"" << std::visit(als_string, zelle) << "\"\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: std::any – Beliebige Typen (flexibel aber unsicher)
// ─────────────────────────────────────────────────────────────────────────────
//
// std::any kann JEDEN kopierbaren Wert speichern.
// Im Gegensatz zu variant muss der Typ nicht zur Compile-Zeit bekannt sein.
//
// Aber: Kein Compile-Zeit-Typschutz, std::any_cast kann fehlschlagen!
// Wann std::any?
//   → Heterogene Sammlungen mit unbekannten Typen (z.B. Plugin-Systeme)
//   → Wenn variant nicht geht weil die Typen nicht bekannt sind
//   → Selten – meistens gibt es bessere Lösungen

void demo_any() {
    std::cout << "\n=== 4. std::any ===\n";

    std::any a;
    std::cout << "  Leer: " << a.has_value() << "\n";

    a = 42;
    std::cout << "  Enthält int: " << (a.type() == typeid(int)) << "\n";
    std::cout << "  Wert: " << std::any_cast<int>(a) << "\n";

    a = std::string{"Hallo"};
    std::cout << "  Jetzt string: \"" << std::any_cast<std::string>(a) << "\"\n";

    // Sicherer Zugriff: any_cast mit Zeiger (gibt nullptr statt Exception)
    if (auto* s = std::any_cast<std::string>(&a)) {
        std::cout << "  any_cast<string>*: \"" << *s << "\"\n";
    }
    if (auto* n = std::any_cast<int>(&a)) {
        std::cout << "  any_cast<int>*: " << *n << "\n";
    } else {
        std::cout << "  any_cast<int>*: nullptr (kein int)\n";
    }

    // Heterogene Sammlung mit any
    std::cout << "\n  Heterogene Sammlung:\n";
    std::vector<std::any> sammlung{42, 3.14, std::string{"Text"}, true};
    for (const auto& elem : sammlung) {
        if (elem.type() == typeid(int))
            std::cout << "  int: " << std::any_cast<int>(elem) << "\n";
        else if (elem.type() == typeid(double))
            std::cout << "  double: " << std::any_cast<double>(elem) << "\n";
        else if (elem.type() == typeid(std::string))
            std::cout << "  string: \"" << std::any_cast<std::string>(elem) << "\"\n";
        else if (elem.type() == typeid(bool))
            std::cout << "  bool: " << std::any_cast<bool>(elem) << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  THEMA 06 – optional, variant, visit, any        ║\n"
              << "╚══════════════════════════════════════════════════╝\n";

    demo_optional();
    demo_variant();
    demo_visit();
    demo_any();

    return 0;
}
