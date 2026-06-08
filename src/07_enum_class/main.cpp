// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 07 – enum class (Scoped Enumerations)                            ║
// ║  Features: C-Enum Probleme, enum class, Underlying Type,                ║
// ║            switch, Bitmasken, Operator-Overloading                      ║
// ╚══════════════════════════════════════════════════════════════════════════╝
// MISRA C++:2023 – Schlüsselregeln in diesem Thema:
//   Rule 10.2.1 (Required) – An enumeration shall be defined with an explicit
//                             underlying type → enum class Status : uint8_t
//   Rule 10.2.2 (Advisory) – Unscoped enumerations should not be declared
//                             → immer enum class / enum struct statt enum
//
// ──── Das Problem mit C-Enums ─────────────────────────────────────────────
//
//   C-Enums (altes C++) haben drei gefährliche Eigenschaften:
//
//   1. Kein eigener Scope (Namen "verschmutzen" den umgebenden Namespace):
//        enum Farbe { ROT, GRUEN, BLAU };
//        enum Ampel { ROT, GELB, GRUEN };  // FEHLER: ROT und GRUEN doppelt!
//
//   2. Implizite Konvertierung zu int (kein Typschutz):
//        enum Richtung { NORD, SUED, OST, WEST };
//        int x = NORD;   // Kompiliert ohne Warnung – oft ein Bug!
//        if (NORD == 0)  // Vergleich mit Ganzzahl – semantisch unsinnig
//
//   3. Kein fester Underlying Type (Größe ist implementierungsdefiniert):
//        sizeof(Richtung) ist nicht garantiert 1, 2, oder 4 Bytes
//
// ──── Die Lösung: enum class (C++11) ─────────────────────────────────────
//
//   enum class Farbe { Rot, Gruen, Blau };
//   enum class Ampel { Rot, Gelb, Gruen };   // OK! Getrennte Scopes
//
//   Farbe f = Farbe::Rot;   // Muss immer qualifiziert werden
//   int x = f;              // FEHLER: keine implizite Konvertierung!
//   if (f == Ampel::Rot)    // FEHLER: verschiedene Typen, kein Vergleich!

#include <iostream>
#include <string>
#include <string_view>
#include <type_traits>   // für std::underlying_type_t
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: Grundlegende enum class Syntax
// ─────────────────────────────────────────────────────────────────────────────

// Kein Namenskonflikt – beide haben "Rot", aber in getrennten Scopes
enum class Farbe { Rot, Gruen, Blau, Gelb, Weiss, Schwarz };
enum class Ampel  { Rot, Gelb, Gruen };   // "Rot" und "Gelb" überschneiden sich nicht!

// Hilfsfunktion: enum class als lesbaren String ausgeben
// (In echtem Code würde man std::format oder eine Lookup-Tabelle nutzen)
std::string_view farbe_als_text(Farbe f) {
    switch (f) {
        case Farbe::Rot:    return "Rot";
        case Farbe::Gruen:  return "Gruen";
        case Farbe::Blau:   return "Blau";
        case Farbe::Gelb:   return "Gelb";
        case Farbe::Weiss:  return "Weiss";
        case Farbe::Schwarz: return "Schwarz";
    }
    return "Unbekannt";
}

std::string_view ampel_als_text(Ampel a) {
    switch (a) {
        case Ampel::Rot:   return "Rot";
        case Ampel::Gelb:  return "Gelb";
        case Ampel::Gruen: return "Gruen";
    }
    return "Unbekannt";
}

void demo_grundlagen() {
    std::cout << "\n=== 1. Grundlagen ===\n";

    // Deklaration: muss immer den Enum-Namen als Qualifier nutzen
    Farbe lieblingsfarbe = Farbe::Blau;
    Ampel status         = Ampel::Gruen;

    std::cout << "  Lieblingsfarbe: " << farbe_als_text(lieblingsfarbe) << "\n";
    std::cout << "  Ampelstatus:    " << ampel_als_text(status) << "\n";

    // Vergleich nur mit demselben Typ erlaubt:
    if (lieblingsfarbe == Farbe::Blau) {
        std::cout << "  Die Lieblingsfarbe ist Blau!\n";
    }

    // VERHINDERT: Vergleich zwischen verschiedenen Enum-Typen
    // if (lieblingsfarbe == Ampel::Rot) { ... }   // COMPILER-FEHLER!

    // VERHINDERT: Implizite Konvertierung zu int
    // int x = lieblingsfarbe;                     // COMPILER-FEHLER!

    // Explizite Konvertierung ist möglich (wenn wirklich nötig):
    int farb_index = static_cast<int>(lieblingsfarbe);
    std::cout << "  Expliziter int-Cast: " << farb_index << "\n";   // 2 (3. Element = index 2)

    // switch-Statement: Compiler warnt bei fehlenden Fällen!
    std::cout << "\n  Ampel-Simulation:\n";
    for (Ampel phase : {Ampel::Rot, Ampel::Gelb, Ampel::Gruen}) {
        switch (phase) {
            case Ampel::Rot:
                std::cout << "  🔴 STOP\n";
                break;
            case Ampel::Gelb:
                std::cout << "  🟡 VORSICHT\n";
                break;
            case Ampel::Gruen:
                std::cout << "  🟢 FAHREN\n";
                break;
            // Kein default: Compiler warnt bei fehlenden Fällen (-Wswitch)
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: Underlying Type (Zugrunde liegender Typ)
// ─────────────────────────────────────────────────────────────────────────────
//
// Der Underlying Type bestimmt die Speichergröße und den Wertebereich.
// Default: int (4 Bytes auf den meisten Plattformen)
//
// Angabe: enum class Name : Typ { ... }
//
// Sinnvolle Anwendungsfälle:
//   - uint8_t: Sehr viele kleine Enums (z.B. Protokoll-Bytes)
//   - uint32_t: Bitmasken (Flags)
//   - int64_t: Wenn Werte größer als INT_MAX sein könnten
//
// ⚠️  MISRA C++:2023 Rule 10.2.1 (Required):
//     "An enumeration shall be defined with an explicit underlying type."
//     Ohne expliziten Typ ist die Größe implementierungsdefiniert → nicht
//     portabel. In sicherheitskritischem Code (z.B. Protokoll-Bytes über CAN)
//     MUSS die Bitbreite garantiert sein.
//     Compliant: enum class Richtung : uint8_t { ... }
//     Non-compliant: enum class Richtung { ... }  // Größe unbekannt!

// Kleiner Enum: nur 1 Byte nötig (256 mögliche Werte)
enum class Richtung : uint8_t { Nord = 0, Sued, Ost, West };

// Explizite Werte: HTTP-Status-Codes
enum class HttpStatus : uint16_t {
    OK                  = 200,
    Created             = 201,
    NoContent           = 204,
    BadRequest          = 400,
    Unauthorized        = 401,
    Forbidden           = 403,
    NotFound            = 404,
    InternalServerError = 500,
};

void demo_underlying_type() {
    std::cout << "\n=== 2. Underlying Type ===\n";

    // Größe prüfen
    std::cout << "  sizeof(Richtung):  " << sizeof(Richtung)  << " Byte\n";  // 1
    std::cout << "  sizeof(HttpStatus):" << sizeof(HttpStatus) << " Bytes\n"; // 2
    std::cout << "  sizeof(Farbe):     " << sizeof(Farbe)      << " Bytes\n"; // 4 (int)

    // underlying_type_t: Gibt den Basistyp zurück (nützlich für generischen Code)
    using RichtungBasis = std::underlying_type_t<Richtung>;   // uint8_t
    std::cout << "  Underlying type von Richtung: "
              << (std::is_same_v<RichtungBasis, uint8_t> ? "uint8_t" : "?") << "\n";

    // HTTP-Status-Codes mit expliziten Werten
    HttpStatus status = HttpStatus::NotFound;
    std::cout << "  HTTP-Status: " << static_cast<uint16_t>(status) << "\n";  // 404

    // Konvertierung von int zu enum class (nur explizit!)
    auto von_int = static_cast<HttpStatus>(200);
    std::cout << "  200 == OK? " << (von_int == HttpStatus::OK ? "ja" : "nein") << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: Bitmasken-Flags mit enum class
// ─────────────────────────────────────────────────────────────────────────────
//
// Häufiges Pattern: Mehrere unabhängige Flags in einem Wert kombinieren.
// Jedes Flag ist eine Zweierpotenz → einzelne Bits.
//
//   Berechtigung: Lesen=1, Schreiben=2, Ausführen=4
//   Alle Rechte:  Lesen | Schreiben | Ausführen = 7
//
// Mit enum class und Operator-Overloading ist das typsicher!

enum class Berechtigung : uint8_t {
    Keine    = 0,
    Lesen    = 1 << 0,   // 0b00000001 = 1
    Schreiben = 1 << 1,  // 0b00000010 = 2
    Ausfuehren = 1 << 2, // 0b00000100 = 4
    Alle     = Lesen | Schreiben | Ausfuehren   // = 7
    // Das | hier nutzt den Integer-Underlying-Type, NICHT den overloaded operator
    // (Enumerator-Initializer dürfen Arithmetik auf underlying_type nutzen)
};

// Operator-Overloading für bequeme Nutzung von Bitmasken:
constexpr Berechtigung operator|(Berechtigung a, Berechtigung b) {
    return static_cast<Berechtigung>(
        static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}
constexpr Berechtigung operator&(Berechtigung a, Berechtigung b) {
    return static_cast<Berechtigung>(
        static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}
constexpr bool hat_berechtigung(Berechtigung rechte, Berechtigung test) {
    return (rechte & test) == test;
}

void demo_bitmasken() {
    std::cout << "\n=== 3. Bitmasken-Flags ===\n";

    // Kombination von Flags mit | (Oder)
    Berechtigung user_rechte = Berechtigung::Lesen | Berechtigung::Schreiben;
    Berechtigung admin_rechte = Berechtigung::Alle;

    std::cout << "  User-Rechte  (int): "
              << static_cast<int>(user_rechte) << "\n";   // 3 = 1+2
    std::cout << "  Admin-Rechte (int): "
              << static_cast<int>(admin_rechte) << "\n";  // 7 = 1+2+4

    // Prüfen ob bestimmtes Flag gesetzt ist mit & (Und)
    auto check = [](Berechtigung r, std::string_view name) {
        std::cout << "  " << name << ": "
                  << (hat_berechtigung(r, Berechtigung::Lesen)     ? "R" : "-")
                  << (hat_berechtigung(r, Berechtigung::Schreiben)  ? "W" : "-")
                  << (hat_berechtigung(r, Berechtigung::Ausfuehren) ? "X" : "-")
                  << "\n";
    };

    check(user_rechte,   "User ");
    check(admin_rechte,  "Admin");
    check(Berechtigung::Lesen, "Guest");
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: enum class als State-Machine
// ─────────────────────────────────────────────────────────────────────────────
//
// Enum class ist ideal für endliche Zustandsautomaten (State Machines).
// Der Typ macht es unmöglich, einen ungültigen Zustand zu haben.

enum class Bestellstatus {
    Neu,
    Bezahlt,
    InBearbeitung,
    Versandt,
    Zugestellt,
    Storniert
};

std::string_view status_text(Bestellstatus s) {
    switch (s) {
        case Bestellstatus::Neu:           return "Neu";
        case Bestellstatus::Bezahlt:       return "Bezahlt";
        case Bestellstatus::InBearbeitung: return "In Bearbeitung";
        case Bestellstatus::Versandt:      return "Versandt";
        case Bestellstatus::Zugestellt:    return "Zugestellt";
        case Bestellstatus::Storniert:     return "Storniert";
    }
    return "?";
}

// Gültige Übergänge erzwingen
Bestellstatus naechster_status(Bestellstatus aktuell) {
    switch (aktuell) {
        case Bestellstatus::Neu:           return Bestellstatus::Bezahlt;
        case Bestellstatus::Bezahlt:       return Bestellstatus::InBearbeitung;
        case Bestellstatus::InBearbeitung: return Bestellstatus::Versandt;
        case Bestellstatus::Versandt:      return Bestellstatus::Zugestellt;
        case Bestellstatus::Zugestellt:    return Bestellstatus::Zugestellt;  // Endstatus
        case Bestellstatus::Storniert:     return Bestellstatus::Storniert;   // Endstatus
    }
    return aktuell;
}

void demo_state_machine() {
    std::cout << "\n=== 4. State Machine ===\n";

    Bestellstatus status = Bestellstatus::Neu;
    std::cout << "  Bestellung verfolgen:\n";

    for (int schritt = 0; schritt < 5; ++schritt) {
        std::cout << "  Schritt " << schritt << ": " << status_text(status) << "\n";
        status = naechster_status(status);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔══════════════════════════════════════════╗\n"
              << "║  THEMA 07 – enum class                   ║\n"
              << "╚══════════════════════════════════════════╝\n";

    demo_grundlagen();
    demo_underlying_type();
    demo_bitmasken();
    demo_state_machine();

    return 0;
}
