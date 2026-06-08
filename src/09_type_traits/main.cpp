// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 09 – Type Traits                                                 ║
// ║  Features: Typ-Kategorien, Typ-Eigenschaften, Typ-Transformationen,     ║
// ║            std::conditional, enable_if / SFINAE, praktische Nutzung     ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// ──── Was sind Type Traits? ───────────────────────────────────────────────
//
//   Type Traits sind Template-Klassen in <type_traits>, die zur Compile-Zeit
//   Informationen über Typen liefern oder Typen transformieren.
//
//   Sie sind das Werkzeug mit dem Templates "klug" werden:
//   "Verhalte dich anders je nachdem ob T ein int, ein Zeiger oder eine Klasse ist."
//
//   Zwei Hauptkategorien:
//   1. Prüfungen (Type Queries): Geben true/false zurück
//        std::is_integral<int>::value  → true
//        std::is_pointer<int*>::value  → true
//
//   2. Transformationen (Type Transformations): Liefern einen neuen Typ
//        std::remove_reference<int&>::type  → int
//        std::add_const<int>::type           → const int
//
//   Moderner Shorthand (C++17):
//        std::is_integral_v<T>         (statt ::value)
//        std::remove_reference_t<T>    (statt ::type)

#include <iostream>
#include <type_traits>
#include <string>
#include <vector>
#include <concepts>   // für Vergleich mit Concepts

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: Primäre Typ-Kategorien (Type Categories)
// ─────────────────────────────────────────────────────────────────────────────
//
// Jeder C++-Typ gehört genau einer primären Kategorie an.
// Diese Traits fragen ab, zu welcher Kategorie ein Typ gehört.

// Hilfsfunktion: Gibt true/false lesbar aus
template<typename T>
void typ_bericht(std::string_view typ_name) {
    std::cout << "  " << typ_name << ":\n";
    std::cout << "    is_void:            " << std::is_void_v<T>             << "\n";
    std::cout << "    is_integral:        " << std::is_integral_v<T>         << "\n";
    std::cout << "    is_floating_point:  " << std::is_floating_point_v<T>   << "\n";
    std::cout << "    is_pointer:         " << std::is_pointer_v<T>          << "\n";
    std::cout << "    is_reference:       " << std::is_reference_v<T>        << "\n";
    std::cout << "    is_lvalue_reference:" << std::is_lvalue_reference_v<T> << "\n";
    std::cout << "    is_class:           " << std::is_class_v<T>            << "\n";
    std::cout << "    is_enum:            " << std::is_enum_v<T>             << "\n";
    std::cout << "    is_const:           " << std::is_const_v<T>            << "\n";
    std::cout << "    is_signed:          " << std::is_signed_v<T>           << "\n";
}

enum class Farbe { Rot, Gruen, Blau };
struct MeinStruct { int x; };

void demo_kategorien() {
    std::cout << "\n=== 1. Typ-Kategorien ===\n";
    std::cout << std::boolalpha;   // true/false statt 1/0

    typ_bericht<int>("int");
    std::cout << "\n";
    typ_bericht<double>("double");
    std::cout << "\n";
    typ_bericht<const int&>("const int&");
    std::cout << "\n";
    typ_bericht<int*>("int*");
    std::cout << "\n";
    typ_bericht<MeinStruct>("MeinStruct");
    std::cout << "\n";
    typ_bericht<Farbe>("enum class Farbe");
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: Typ-Beziehungen (Type Relationships)
// ─────────────────────────────────────────────────────────────────────────────

struct Basis { virtual ~Basis() = default; };
struct Abgeleitet : Basis {};
struct Unverwandt {};

void demo_beziehungen() {
    std::cout << "\n=== 2. Typ-Beziehungen ===\n";
    std::cout << std::boolalpha;

    // is_same: Sind beide Typen exakt gleich?
    std::cout << "  is_same<int, int>:           " << std::is_same_v<int, int>           << "\n";
    std::cout << "  is_same<int, long>:          " << std::is_same_v<int, long>          << "\n";
    std::cout << "  is_same<int, const int>:     " << std::is_same_v<int, const int>     << "\n";
    std::cout << "  is_same<int&, int>:          " << std::is_same_v<int&, int>          << "\n";

    // is_base_of: Ist A eine Basisklasse von B?
    std::cout << "\n  is_base_of<Basis, Abgeleitet>:    "
              << std::is_base_of_v<Basis, Abgeleitet>    << "\n";
    std::cout << "  is_base_of<Abgeleitet, Basis>:    "
              << std::is_base_of_v<Abgeleitet, Basis>    << "\n";
    std::cout << "  is_base_of<Basis, Unverwandt>:    "
              << std::is_base_of_v<Basis, Unverwandt>    << "\n";

    // is_convertible: Kann T nach U konvertiert werden?
    std::cout << "\n  is_convertible<int, double>:      "
              << std::is_convertible_v<int, double>      << "\n";  // ja (implicit)
    std::cout << "  is_convertible<double, int>:      "
              << std::is_convertible_v<double, int>      << "\n";  // ja (narrowing)
    std::cout << "  is_convertible<int, std::string>: "
              << std::is_convertible_v<int, std::string> << "\n";  // nein
    std::cout << "  is_convertible<Abgeleitet*, Basis*>: "
              << std::is_convertible_v<Abgeleitet*, Basis*> << "\n"; // ja (Polymorphie)
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: Typ-Transformationen (Type Transformations)
// ─────────────────────────────────────────────────────────────────────────────
//
// Diese Traits erzeugen neue Typen aus bestehenden.
// Intensiv genutzt in der STL-Implementierung und in Templates.

void demo_transformationen() {
    std::cout << "\n=== 3. Typ-Transformationen ===\n";
    std::cout << std::boolalpha;

    // remove_reference: Entfernt & oder && vom Typ
    // Intern genutzt von std::move und std::forward!
    std::cout << "  remove_reference<int&> == int:   "
              << std::is_same_v<std::remove_reference_t<int&>, int>  << "\n";
    std::cout << "  remove_reference<int&&> == int:  "
              << std::is_same_v<std::remove_reference_t<int&&>, int> << "\n";
    std::cout << "  remove_reference<int> == int:    "
              << std::is_same_v<std::remove_reference_t<int>, int>   << "\n";

    // remove_const / add_const
    std::cout << "\n  remove_const<const int> == int:  "
              << std::is_same_v<std::remove_const_t<const int>, int> << "\n";
    std::cout << "  add_const<int> == const int:     "
              << std::is_same_v<std::add_const_t<int>, const int>    << "\n";

    // remove_pointer / add_pointer
    std::cout << "\n  remove_pointer<int*> == int:     "
              << std::is_same_v<std::remove_pointer_t<int*>, int>    << "\n";
    std::cout << "  add_pointer<int> == int*:        "
              << std::is_same_v<std::add_pointer_t<int>, int*>       << "\n";

    // decay: Wandelt Typ wie bei Übergabe an Funktion by-value um
    //   - Array → Zeiger
    //   - Funktion → Funktionszeiger
    //   - const/volatile / Referenz entfernen
    // Das ist was passiert wenn man auto x = ausdruck; schreibt!
    std::cout << "\n  decay<int&> == int:              "
              << std::is_same_v<std::decay_t<int&>, int>        << "\n";
    std::cout << "  decay<const int> == int:         "
              << std::is_same_v<std::decay_t<const int>, int>   << "\n";
    std::cout << "  decay<int[5]> == int*:           "
              << std::is_same_v<std::decay_t<int[5]>, int*>     << "\n";
    std::cout << "  decay<int(int)> == int(*)(int):  "
              << std::is_same_v<std::decay_t<int(int)>, int(*)(int)> << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: std::conditional – Compile-Zeit-Auswahl zwischen Typen
// ─────────────────────────────────────────────────────────────────────────────
//
// std::conditional<Bedingung, WennTrue, WennFalse>::type
//   → Wenn Bedingung true: WennTrue
//   → Wenn Bedingung false: WennFalse
//
// Analogie: Der ternäre Operator ? : aber für Typen statt Werte.

// Wähle automatisch int oder long long je nach Größe
template<bool BIG>
using GanzzahlTyp = std::conditional_t<BIG, long long, int>;

// Wähle automatisch signed oder unsigned
template<typename T>
using SignedVersion = std::conditional_t<
    std::is_unsigned_v<T>,
    std::make_signed_t<T>,   // unsigned → signed
    T                         // bereits signed: unverändert
>;

void demo_conditional() {
    std::cout << "\n=== 4. std::conditional ===\n";
    std::cout << std::boolalpha;

    // GanzzahlTyp<true>  → long long (8 Bytes)
    // GanzzahlTyp<false> → int       (4 Bytes)
    std::cout << "  GanzzahlTyp<true>  ist long long: "
              << std::is_same_v<GanzzahlTyp<true>,  long long> << "\n";
    std::cout << "  GanzzahlTyp<false> ist int:       "
              << std::is_same_v<GanzzahlTyp<false>, int>       << "\n";

    // Praktisches Beispiel: Typ je nach Plattform wählen
    using IndexTyp = std::conditional_t<sizeof(void*) == 8, uint64_t, uint32_t>;
    std::cout << "  IndexTyp auf dieser Plattform: "
              << sizeof(IndexTyp) << " Bytes\n";

    // Signed/Unsigned Konvertierung
    std::cout << "  SignedVersion<unsigned int> == int: "
              << std::is_same_v<SignedVersion<unsigned int>, int> << "\n";
    std::cout << "  SignedVersion<int> == int:          "
              << std::is_same_v<SignedVersion<int>, int>          << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 5: Praktische Anwendung – Type-Traits in Templates
// ─────────────────────────────────────────────────────────────────────────────
//
// Type Traits werden genutzt um Template-Verhalten zur Compile-Zeit zu steuern.
// In C++20 werden Concepts bevorzugt – aber Type Traits sind die Grundlage!

// Universelle Ausgabefunktion: verhält sich je nach Typ unterschiedlich
template<typename T>
void ausgabe_smart(const T& wert) {
    if constexpr (std::is_floating_point_v<T>) {
        std::cout << "  float(" << sizeof(T)*8 << "bit): " << wert << "\n";
    } else if constexpr (std::is_integral_v<T> && std::is_signed_v<T>) {
        std::cout << "  signed int(" << sizeof(T)*8 << "bit): " << wert << "\n";
    } else if constexpr (std::is_integral_v<T> && std::is_unsigned_v<T>) {
        std::cout << "  unsigned int(" << sizeof(T)*8 << "bit): " << wert << "\n";
    } else if constexpr (std::is_pointer_v<T>) {
        std::cout << "  pointer: " << static_cast<const void*>(wert) << "\n";
    } else if constexpr (std::is_class_v<T>) {
        // Bei Klassen: prüfe ob operator<< definiert ist (rudimentär)
        std::cout << "  class/struct (kein Standardausdruck)\n";
    } else {
        std::cout << "  sonstiger Typ\n";
    }
}

// Sichere Null-Prüfung: funktioniert für Zeiger UND Zahlen
// requires (C++20): Compiler-Fehler mit klarer Meldung bei falschem Typ
template<typename T>
    requires (std::is_pointer_v<T> || std::is_arithmetic_v<T>)
bool ist_null_oder_leer(const T& wert) {
    if constexpr (std::is_pointer_v<T>) {
        return wert == nullptr;
    } else {
        return wert == T{0};   // T{0} = Null für int, 0.0 für double, etc.
    }
}

void demo_praktisch() {
    std::cout << "\n=== 5. Praktische Anwendung ===\n";

    std::cout << "  ausgabe_smart für verschiedene Typen:\n";
    ausgabe_smart(42);
    ausgabe_smart(-7);
    ausgabe_smart(3.14f);
    ausgabe_smart(3.14);
    ausgabe_smart(42u);
    ausgabe_smart((int*)nullptr);
    ausgabe_smart(MeinStruct{5});

    std::cout << "\n  ist_null_oder_leer:\n";
    int  n  = 0;
    int  m  = 5;
    int* p1 = nullptr;
    int* p2 = &m;

    std::cout << "  0:       " << ist_null_oder_leer(n)  << "\n";
    std::cout << "  5:       " << ist_null_oder_leer(m)  << "\n";
    std::cout << "  nullptr: " << ist_null_oder_leer(p1) << "\n";
    std::cout << "  &m:      " << ist_null_oder_leer(p2) << "\n";

    // is_trivially_copyable: Kann memcpy sicher benutzt werden?
    std::cout << "\n  is_trivially_copyable:\n";
    std::cout << "  int:         " << std::is_trivially_copyable_v<int>         << "\n";
    std::cout << "  std::string: " << std::is_trivially_copyable_v<std::string> << "\n";
    std::cout << "  MeinStruct:  " << std::is_trivially_copyable_v<MeinStruct>  << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔══════════════════════════════════════════╗\n"
              << "║  THEMA 09 – Type Traits                  ║\n"
              << "╚══════════════════════════════════════════╝\n";

    demo_kategorien();
    demo_beziehungen();
    demo_transformationen();
    demo_conditional();
    demo_praktisch();

    return 0;
}
