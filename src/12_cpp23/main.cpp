// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 12 – C++23 Highlights                                            ║
// ║  Compiler: g++-12 oder neuer  (cmake -DCMAKE_CXX_STANDARD=23)          ║
// ║                                                                          ║
// ║  Features: std::expected, monadic optional, std::to_underlying,         ║
// ║            if consteval, static operator(), std::string::contains,      ║
// ║            ranges::zip / enumerate / chunk, auto(x) decay-copy          ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// ──── Was ist neu in C++23? ───────────────────────────────────────────────
//
//   C++23 setzt den Fokus auf drei Bereiche:
//
//   1. SICHERERE FEHLERBEHANDLUNG
//      std::expected<T,E>: Fehler als Typ, kein throw/catch nötig
//      Monadic Optional: Verkettung ohne Null-Prüfungen
//
//   2. KLARERE SYNTAX
//      std::print/println: Typsichere Ausgabe (kein << mehr nötig)
//      std::to_underlying: enum → int ohne static_cast
//      if consteval: Compile-Zeit-Zweige explizit markieren
//
//   3. BESSERE ALGORITHMEN
//      ranges::zip, enumerate, chunk, slide: Moderne Iteration
//
// ──── Compiler-Anforderung ────────────────────────────────────────────────
//
//   g++ 12:  std::expected, monadic optional, to_underlying, if consteval
//   g++ 13:  std::print/println, std::flat_map, std::generator
//   clang 17+: vollständige C++23-Unterstützung

#include <iostream>
#include <expected>       // std::expected (C++23)
#include <optional>       // std::optional + monadic (C++23)
#include <string>
#include <string_view>
#include <vector>
#include <ranges>         // ranges::zip, enumerate, chunk (C++23)
#include <algorithm>
#include <utility>        // std::to_underlying (C++23)
#include <type_traits>
#include <functional>

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: std::expected<T, E> – Fehlerbehandlung ohne Exceptions
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem mit bisherigen Fehlerbehandlungs-Methoden:
//
//   int*  result = allokiere();   // nullptr = Fehler? Kein Info WARUM!
//   int   result = berechne();   // -1 = Fehler? Magic Number!
//   throw FehlerException{...};  // Exceptions: teuer, nicht in jedem Scope sinnvoll
//
// std::expected<T, E> ist:
//   - Entweder: ein gültiger Wert vom Typ T  (success)
//   - Oder:     ein Fehlerwert vom Typ E     (error)
//
// Ähnlich wie Rusts Result<T, E> oder Hasklls Either.
// Kein Overhead, kein Stack-Unwinding, kein try/catch nötig.
//
// ⚠️  SAFETY: Explizite Fehlerbehandlung verhindert vergessene Fehlercodes!
//             Der Compiler erzeugt keine Warnung wenn man Fehler ignoriert,
//             aber der Code macht es sichtbar.

// Verschiedene Fehlertypen mit enum class (sicher, wie in Thema 07 gelernt)
enum class ParseFehler {
    LeerEingabe,
    UngueltigesFormat,
    WertAusserhalbBereich
};

enum class DateiFehler {
    NichtGefunden,
    KeineLeserechte,
    Korrupt
};

// Hilfsfunktion: Fehler als Text
std::string_view fehler_text(ParseFehler f) {
    switch (f) {
        case ParseFehler::LeerEingabe:          return "Leere Eingabe";
        case ParseFehler::UngueltigesFormat:    return "Ungültiges Format";
        case ParseFehler::WertAusserhalbBereich: return "Wert außerhalb des Bereichs";
    }
    return "?";
}

// Funktion gibt expected<int, ParseFehler> zurück:
//   → Entweder der geparste int, oder ein ParseFehler mit Grund
std::expected<int, ParseFehler> parse_int(std::string_view s) {
    if (s.empty())
        return std::unexpected{ParseFehler::LeerEingabe};

    int ergebnis = 0;
    bool negativ = false;
    std::size_t start = 0;

    if (s[0] == '-') { negativ = true; start = 1; }

    for (std::size_t i = start; i < s.size(); ++i) {
        if (s[i] < '0' || s[i] > '9')
            return std::unexpected{ParseFehler::UngueltigesFormat};
        ergebnis = ergebnis * 10 + (s[i] - '0');
    }

    ergebnis = negativ ? -ergebnis : ergebnis;

    if (ergebnis < -999 || ergebnis > 999)
        return std::unexpected{ParseFehler::WertAusserhalbBereich};

    return ergebnis;   // Erfolg: direkt den Wert zurückgeben
}

// Verkettung von expected (ähnlich wie monadic optional):
// Wenn parse_int scheitert, wird die Division nie versucht
std::expected<double, ParseFehler> parse_und_halbiere(std::string_view s) {
    return parse_int(s)
        .and_then([](int n) -> std::expected<double, ParseFehler> {
            if (n == 0)
                return std::unexpected{ParseFehler::WertAusserhalbBereich};
            return static_cast<double>(n) / 2.0;
        });
}

void demo_expected() {
    std::cout << "\n=== 1. std::expected<T, E> ===\n";
    std::cout << "  (C++23 – sicherste Methode für erwartbare Fehler)\n\n";

    // Erfolgsfall
    auto r1 = parse_int("42");
    if (r1) {
        std::cout << "  parse_int(\"42\")  → Erfolg: " << *r1 << "\n";
    }

    // Fehlerfälle
    std::vector<std::string_view> eingaben{"", "abc", "99999", "-7", "100"};
    for (auto s : eingaben) {
        auto r = parse_int(s);
        if (r.has_value()) {
            std::cout << "  parse_int(\"" << s << "\") → " << r.value() << "\n";
        } else {
            std::cout << "  parse_int(\"" << s << "\") → Fehler: "
                      << fehler_text(r.error()) << "\n";
        }
    }

    // value_or: Fallback bei Fehler (wie optional)
    int sicher = parse_int("kaputt").value_or(0);
    std::cout << "\n  value_or(0) bei Fehler: " << sicher << "\n";

    // Verkettung mit and_then
    std::cout << "\n  Verkettung (parse + halbiere):\n";
    for (auto s : {"20", "0", "xyz"}) {
        auto r = parse_und_halbiere(s);
        if (r) std::cout << "  \"" << s << "\" → " << *r << "\n";
        else   std::cout << "  \"" << s << "\" → Fehler: " << fehler_text(r.error()) << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: Monadic Operations für std::optional (C++23)
// ─────────────────────────────────────────────────────────────────────────────
//
// C++23 fügt drei Methoden zu std::optional hinzu:
//
//   .transform(f)   → Wendet f auf den Wert an (wenn vorhanden)
//                     optional<T> → optional<U>
//
//   .and_then(f)    → f muss selbst optional<U> zurückgeben (Verkettung)
//                     Wie flatMap / bind in funktionaler Programmierung
//
//   .or_else(f)     → Wenn leer: f() aufrufen (Fallback-Funktion)
//
// ⚠️  SAFETY: Verhindert Null-Pointer-Dereferenzierung durch
//             explizite Verkettung statt manueller if-Prüfungen.

struct Benutzer {
    std::string name;
    std::optional<std::string> email;
    std::optional<int>         alter;
};

std::optional<Benutzer> hole_benutzer(int id) {
    if (id == 1) return Benutzer{"Alice", "alice@example.com", 30};
    if (id == 2) return Benutzer{"Bob",   std::nullopt,        25};
    return std::nullopt;   // Kein Benutzer gefunden
}

std::optional<std::string> extrahiere_domain(const std::string& email) {
    auto at = email.find('@');
    if (at == std::string::npos) return std::nullopt;
    return email.substr(at + 1);
}

void demo_monadic_optional() {
    std::cout << "\n=== 2. Monadic Optional (C++23) ===\n";

    // Ohne C++23: verschachtelte if-Ketten
    std::cout << "  Ohne C++23 (viele if-Prüfungen):\n";
    auto benutzer_alt = hole_benutzer(1);
    if (benutzer_alt.has_value()) {
        if (benutzer_alt->email.has_value()) {
            auto domain = extrahiere_domain(*benutzer_alt->email);
            if (domain.has_value()) {
                std::cout << "  Domain: " << *domain << "\n";
            }
        }
    }

    // Mit C++23: elegante Pipeline (kein if nötig!)
    std::cout << "\n  Mit C++23 (Pipeline, keine if-Kette):\n";

    auto hole_domain = [](int id) {
        return hole_benutzer(id)
            // transform: Benutzer → optional<string> (Email)
            .and_then([](const Benutzer& b) { return b.email; })
            // and_then: string → optional<string> (Domain)
            .and_then(extrahiere_domain)
            // transform: string → string (Großbuchstaben)
            .transform([](std::string d) {
                std::transform(d.begin(), d.end(), d.begin(), ::toupper);
                return d;
            })
            // or_else: Fallback wenn irgendein Schritt fehlschlägt
            .or_else([]() -> std::optional<std::string> {
                return "UNBEKANNT";
            });
    };

    for (int id : {1, 2, 3}) {
        auto domain = hole_domain(id);
        std::cout << "  ID " << id << ": " << domain.value_or("(leer)") << "\n";
    }

    // transform: Wert transformieren ohne has_value() zu prüfen
    std::optional<int> n = 21;
    auto verdoppelt  = n.transform([](int x) { return x * 2; });
    auto als_string  = verdoppelt.transform([](int x) { return std::to_string(x); });
    std::cout << "\n  transform-Kette: 21 → *2 → string: \""
              << als_string.value_or("?") << "\"\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: std::to_underlying() – enum class sicher konvertieren
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem (C++20 und früher):
//   enum class Status : uint8_t { OK = 0, Fehler = 1 };
//   uint8_t wert = static_cast<uint8_t>(Status::OK);  // umständlich, fehleranfällig
//   // Was wenn jemand den Underlying Type ändert? Falscher Cast, kein Compiler-Fehler!
//
// C++23 Lösung:
//   uint8_t wert = std::to_underlying(Status::OK);
//   // Immer korrekt, egal welcher Underlying Type!
//
// ⚠️  SAFETY: Verhindert Fehler wenn Enum-Underlying-Type geändert wird.

enum class Prioritaet : uint8_t  { Niedrig = 1, Mittel = 2, Hoch = 3, Kritisch = 4 };
enum class Fehlercode : int32_t  { OK = 0, Warnung = 1, Fehler = -1, Fatal = -99 };
enum class Flags      : uint32_t { Keine = 0, LesenF = 1, SchreibenF = 2, AlleF = 3 };

void demo_to_underlying() {
    std::cout << "\n=== 3. std::to_underlying() (C++23) ===\n";

    // Früher: static_cast (umständlich und fehleranfällig)
    // Jetzt: std::to_underlying (klar, sicher, typ-unabhängig)
    std::cout << "  Prioritaet::Hoch    = "
              << static_cast<int>(std::to_underlying(Prioritaet::Hoch)) << "\n";
    std::cout << "  Fehlercode::Fatal   = "
              << std::to_underlying(Fehlercode::Fatal) << "\n";
    std::cout << "  Flags::AlleF        = "
              << std::to_underlying(Flags::AlleF) << "\n";

    // Nützlich beim Logging / Serialisieren von Enums:
    auto serialisiere = [](Prioritaet p) {
        return std::to_underlying(p);   // Kein static_cast<uint8_t> nötig!
    };
    std::cout << "\n  Serialisiert: Prioritaet::Mittel = "
              << static_cast<int>(serialisiere(Prioritaet::Mittel)) << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: if consteval – Compile-Zeit-Zweige explizit markieren
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem (C++20):
//   constexpr bool ist_konstant = std::is_constant_evaluated();
//   Das ist verwirrend und fehleranfällig zu schreiben.
//
// C++23 Lösung:
//   if consteval { ... }
//   → Dieser Block wird NUR ausgeführt wenn die Funktion zur Compile-Zeit läuft
//   → Der else-Block wird NUR zur Laufzeit ausgeführt
//
// ⚠️  SAFETY: Verhindert, dass nicht-constexpr-Code in Compile-Zeit-Pfaden landet.

constexpr double wurzel(double x) {
    if consteval {
        // Compile-Zeit: Newton-Raphson (reine constexpr-Arithmetik)
        // (std::sqrt ist in constexpr-Kontext nicht immer verfügbar)
        double r = x;
        for (int i = 0; i < 20; ++i)
            r = (r + x / r) / 2.0;
        return r;
    } else {
        // Laufzeit: Hardware-optimierte Version (schneller)
        return __builtin_sqrt(x);  // oder: #include <cmath> → std::sqrt(x)
    }
}

void demo_if_consteval() {
    std::cout << "\n=== 4. if consteval (C++23) ===\n";

    // Compile-Zeit: Ergebnis wird vom Compiler berechnet
    constexpr double wurzel_2 = wurzel(2.0);
    std::cout << "  Compile-Zeit: sqrt(2) = " << wurzel_2 << "\n";

    // Laufzeit: Hardware-optimierte Version
    double x = 3.0;
    double wurzel_3 = wurzel(x);
    std::cout << "  Laufzeit:     sqrt(3) = " << wurzel_3 << "\n";

    // Nachweis: constexpr zur Compile-Zeit (kein Laufzeit-Overhead)
    static_assert(wurzel(4.0) == 2.0, "Compile-Zeit sqrt(4) == 2");
    std::cout << "  static_assert(sqrt(4.0) == 2.0) bestanden!\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 5: static operator() – Lambdas ohne Closure-Overhead
// ─────────────────────────────────────────────────────────────────────────────
//
// Bisher: Lambdas ohne Captures haben immer noch einen impliziten this-Zeiger.
//         Das verhindert manche Compiler-Optimierungen.
//
// C++23: static operator() markiert den Aufruf-Operator als statisch.
//        → Kein this-Zeiger → bessere Optimierbarkeit
//        → Nur für Lambdas ohne Captures sinnvoll!
//
// ⚠️  SAFETY: Verhindert versehentliche Captures in "reinen" Funktionen.

void demo_static_lambda() {
    std::cout << "\n=== 5. static operator() (C++23) ===\n";

    // C++23: 'static' kennzeichnet das Lambda als zustandslos
    auto quadrat = [](int x) static { return x * x; };
    auto ist_gerade = [](int x) static -> bool { return x % 2 == 0; };

    std::cout << "  quadrat(7) = " << quadrat(7) << "\n";
    std::cout << "  ist_gerade(4) = " << ist_gerade(4) << "\n";
    std::cout << "  ist_gerade(7) = " << ist_gerade(7) << "\n";

    // Praktisch: Als Callback übergeben (Funktionszeiger-Kompatibilität)
    std::vector<int> v{3, 1, 4, 1, 5, 9, 2, 6};
    std::sort(v.begin(), v.end(), [](int a, int b) static { return a < b; });
    std::cout << "  Sortiert: ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 6: std::string::contains (C++23)
// ─────────────────────────────────────────────────────────────────────────────
//
// Endlich: eine intuitive contains()-Methode für std::string und std::string_view!
// Vorher: s.find(sub) != std::string::npos  ← unlesbar!
// Jetzt:  s.contains(sub)                   ← klar!
//
// ⚠️  SAFETY: Verhindert den häufigen Bug:
//     if (s.find(sub))         ← FALSCH! find() gibt 0 zurück wenn am Anfang!
//     if (s.contains(sub))    ← RICHTIG! Kein "!= npos" vergessen möglich.

void demo_string_contains() {
    std::cout << "\n=== 6. std::string::contains (C++23) ===\n";

    std::string text = "Modernes C++ macht Spaß!";

    // contains: prüft ob Substring vorhanden
    std::cout << "  contains(\"C++\"):    " << text.contains("C++")    << "\n";
    std::cout << "  contains(\"Java\"):   " << text.contains("Java")   << "\n";
    std::cout << "  contains('S'):      " << text.contains('S')      << "\n";

    // starts_with / ends_with (bereits C++20, aber gut zu wissen):
    std::cout << "\n  starts_with(\"Mod\"): " << text.starts_with("Mod") << "\n";
    std::cout << "  ends_with(\"!\"):     " << text.ends_with('!')     << "\n";

    // Vergleich: alter vs. neuer Stil
    // ALT:  text.find("C++") != std::string::npos  ← fehleranfällig!
    // NEU:  text.contains("C++")                   ← sicher und lesbar!
    std::cout << "\n  Sicherer Test (contains statt find):\n";
    std::vector<std::string> woerter{"C++", "Python", "Spaß", "Rust"};
    for (const auto& w : woerter) {
        std::cout << "  \"" << w << "\": " << text.contains(w) << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 7: Ranges-Erweiterungen (C++23)
// ─────────────────────────────────────────────────────────────────────────────
//
// C++23 erweitert die Ranges-Bibliothek aus C++20 um sehr nützliche Views:
//
//   views::enumerate  → Gibt (Index, Wert) Paare zurück
//   views::zip        → Koppelt zwei Ranges Element-für-Element
//   views::chunk      → Teilt in Blöcke fester Größe auf
//   views::slide      → Gleitendes Fenster über die Range
//   views::repeat     → Wiederholt einen Wert n-mal
//   views::iota       → Aufsteigende Zahlen (schon C++20, aber erweitert)

void demo_ranges() {
    std::cout << "\n=== 7. Ranges C++23 ===\n";

    std::vector<std::string> sprachen{"C++", "Rust", "Go", "Python"};
    std::vector<int>         bewertungen{95, 90, 75, 85};

    // views::enumerate: Index + Wert ohne manuellen Zähler
    std::cout << "  views::enumerate:\n";
    for (auto [i, s] : std::views::enumerate(sprachen)) {
        std::cout << "    [" << i << "] " << s << "\n";
    }

    // views::zip: Zwei Ranges koppeln (wie Python's zip)
    std::cout << "\n  views::zip (Sprache + Bewertung):\n";
    for (auto [spr, wert] : std::views::zip(sprachen, bewertungen)) {
        std::cout << "    " << spr << ": " << wert << "/100\n";
    }

    // views::chunk: In Blöcke aufteilen
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8};
    std::cout << "\n  views::chunk(3) von {1..8}:\n";
    for (auto block : v | std::views::chunk(3)) {
        std::cout << "    Block: ";
        for (int x : block) std::cout << x << " ";
        std::cout << "\n";
    }

    // views::slide: Gleitendes Fenster
    std::cout << "\n  views::slide(3) – gleitendes Fenster:\n";
    for (auto fenster : v | std::views::slide(3)) {
        std::cout << "    Fenster: ";
        for (int x : fenster) std::cout << x << " ";
        std::cout << "\n";
    }

    // views::repeat: Wert wiederholen
    std::cout << "\n  views::repeat(42, 4): ";
    for (int x : std::views::repeat(42, 4)) std::cout << x << " ";
    std::cout << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 8: auto(x) – Explizite Decay-Copy (C++23)
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem: In manchen Kontexten ist unklar ob eine Kopie oder Referenz entsteht.
//          Besonders in Range-for-Schleifen mit Modifikation der Range.
//
// C++23: auto(ausdruck) erzeugt IMMER eine Kopie (decay-copy).
//        Macht die Absicht explizit und verhindert Dangling References.
//
// ⚠️  SAFETY: Schützt vor dem klassischen "Range-for modifiziert Container"-Bug.

void demo_auto_decay_copy() {
    std::cout << "\n=== 8. auto(x) Decay-Copy (C++23) ===\n";

    std::vector<std::string> namen{"Alice", "Bob", "Carol"};

    // Sicheres Iterieren und Einfügen (kein Dangling-Reference-Problem):
    std::vector<std::string> kopien;
    for (const auto& name : namen) {
        kopien.push_back(auto(name));  // C++23: garantiert eine Kopie
    }
    std::cout << "  Kopien: ";
    for (const auto& k : kopien) std::cout << "\"" << k << "\" ";
    std::cout << "\n";

    // Typischer Anwendungsfall: Lambda das immer eine frische Kopie erstellt
    auto mache_kopie = [](const auto& ref) {
        return auto(ref);  // Explizite decay-copy → Typ ist immer "sauber"
    };

    std::string original = "Hallo";
    auto kopie = mache_kopie(original);
    kopie += "!";
    std::cout << "  Original: \"" << original << "\"\n";
    std::cout << "  Kopie:    \"" << kopie    << "\"\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔══════════════════════════════════════════════════╗\n"
              << "║  THEMA 12 – C++23 Highlights                     ║\n"
              << "║  Compiler: g++-12 oder neuer                     ║\n"
              << "╚══════════════════════════════════════════════════╝\n";
    std::cout << std::boolalpha;

    demo_expected();
    demo_monadic_optional();
    demo_to_underlying();
    demo_if_consteval();
    demo_static_lambda();
    demo_string_contains();
    demo_ranges();
    demo_auto_decay_copy();

    std::cout << "\n✓ Alle C++23-Demos erfolgreich!\n";
    return 0;
}
