// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 03 – Move-Semantik & Regel der Fünf                              ║
// ║  Features: lvalue/rvalue, rvalue-Referenzen (&&), std::move,            ║
// ║            Move-Konstruktor, Move-Assignment, Regel der Fünf,           ║
// ║            Perfect Forwarding mit std::forward                          ║
// ╚══════════════════════════════════════════════════════════════════════════╝
// MISRA C++:2023 – Schlüsselregeln in diesem Thema:
//   Rule 15.0.1 (Required) – Special member functions shall be provided
//                             appropriately (Regel der Fünf / Null)
//   Rule 15.0.2 (Advisory) – User-provided copy/move functions should have
//                             appropriate signatures (noexcept bei Move)
//   Rule 18.4.1 (Required) – Exception-unfriendly functions (Move-Ctors,
//                             Move-Assignment, Destruktoren) shall be noexcept
//   Rule 15.1.3 (Required) – Constructors callable with a single argument
//                             shall be explicit
//
// ──── Das Problem: Kopieren ist teuer ─────────────────────────────────────
//
//   std::vector<int> a(1'000'000, 42);  // 1 Million Elemente
//   std::vector<int> b = a;             // Kopiert alle 1 Million Elemente!
//
//   Aber was wenn 'a' danach nicht mehr gebraucht wird?
//   Warum nicht einfach die internen Daten von 'a' nach 'b' "umziehen"?
//   Das ist die Idee hinter Move-Semantik!
//
// ──── lvalue vs. rvalue ───────────────────────────────────────────────────
//
//   lvalue ("left value"):
//     - Hat einen Namen und eine Adresse
//     - Kann links UND rechts vom = stehen
//     - Bleibt nach dem Ausdruck bestehen
//     - Beispiele: Variablen (x, name, vec), *ptr, arr[i]
//
//   rvalue ("right value"):
//     - Temporärer Ausdruck, kein dauerhafter Name
//     - Steht typischerweise NUR rechts vom =
//     - Existiert nach dem Ausdruck nicht mehr
//     - Beispiele: Literale (42, "Hallo"), Ausdrücke (a+b), Funktionsrückgaben

#include <iostream>
#include <string>
#include <vector>
#include <utility>   // für std::move, std::forward
#include <cstring>   // für std::strlen, std::strcpy

// ─────────────────────────────────────────────────────────────────────────────
// Hilfsklasse: String-Puffer (vereinfachtes std::string zum Lernen)
// ─────────────────────────────────────────────────────────────────────────────
// Diese Klasse verwaltet dynamisch allokierten Speicher.
// Wir implementieren alle "Fünf" Spezialmethoden manuell um zu sehen,
// wann welche aufgerufen wird.
class StringPuffer {
public:
    // ── Konstruktor ──────────────────────────────────────────────────────────
    explicit StringPuffer(const char* text = "")
        : groesse_{std::strlen(text)}
        , daten_{new char[groesse_ + 1]}    // +1 für Null-Terminator
    {
        std::strcpy(daten_, text);
        std::cout << "  [CTOR]      StringPuffer(\"" << daten_ << "\")\n";
    }

    // ── 1/5: Copy-Konstruktor ─────────────────────────────────────────────
    // Wird aufgerufen bei: StringPuffer b = a; oder StringPuffer b(a);
    // → Tiefe Kopie: neuer Speicher wird allokiert und Inhalt kopiert
    StringPuffer(const StringPuffer& andere)
        : groesse_{andere.groesse_}
        , daten_{new char[groesse_ + 1]}    // NEUER Speicher!
    {
        std::strcpy(daten_, andere.daten_); // Inhalt kopieren
        std::cout << "  [COPY CTOR] StringPuffer(\"" << daten_
                  << "\") ← kopiert von \"" << andere.daten_ << "\"\n";
    }

    // ── 2/5: Copy-Assignment ──────────────────────────────────────────────
    // Wird aufgerufen bei: b = a; (b existiert bereits)
    StringPuffer& operator=(const StringPuffer& andere) {
        std::cout << "  [COPY =]    \"" << daten_
                  << "\" ← kopiert von \"" << andere.daten_ << "\"\n";

        if (this == &andere) return *this;  // Selbst-Zuweisung: a = a

        delete[] daten_;                    // Alten Speicher freigeben
        groesse_ = andere.groesse_;
        daten_   = new char[groesse_ + 1];  // Neuen Speicher allokieren
        std::strcpy(daten_, andere.daten_); // Inhalt kopieren
        return *this;
    }

    // ── 3/5: Move-Konstruktor ─────────────────────────────────────────────
    // Wird aufgerufen bei: StringPuffer b = std::move(a);
    //                  oder: StringPuffer b = funktion_die_string_zurückgibt();
    //
    // Statt zu KOPIEREN: Ressourcen STEHLEN!
    //   - Wir übernehmen den Zeiger von 'andere'
    //   - 'andere' setzen wir auf einen gültigen aber leeren Zustand
    //   - Kein new[], kein strcpy → sehr schnell!
    //
    // Parameter: StringPuffer&& = rvalue-Referenz (bindet an temporäre Objekte)
    // ⚠️  MISRA C++:2023 Rule 18.4.1 (Required): Move-Konstruktoren müssen
    //     noexcept sein — std::vector fällt sonst auf den langsamen Copy-Pfad
    //     zurück und kann keine starke Exception-Safety mehr garantieren.
    //     Rule 15.0.2 (Advisory): korrekte Signatur mit & und noexcept.
    StringPuffer(StringPuffer&& andere) noexcept
        : groesse_{andere.groesse_}
        , daten_{andere.daten_}   // Zeiger stehlen!
    {
        std::cout << "  [MOVE CTOR] StringPuffer(\"" << daten_
                  << "\") ← gestohlen von rvalue\n";

        // 'andere' in gültigen Zustand versetzen (Destruktor muss funktionieren!)
        andere.daten_   = nullptr;  // andere darf nicht mehr auf unseren Speicher zeigen
        andere.groesse_ = 0;
    }

    // ── 4/5: Move-Assignment ──────────────────────────────────────────────
    // Wird aufgerufen bei: b = std::move(a); (b existiert bereits)
    StringPuffer& operator=(StringPuffer&& andere) noexcept {
        std::cout << "  [MOVE =]    \"" << (daten_ ? daten_ : "nullptr")
                  << "\" ← gestohlen von rvalue\n";

        if (this == &andere) return *this;

        delete[] daten_;            // Alten Speicher freigeben

        // Ressourcen stehlen:
        daten_   = andere.daten_;
        groesse_ = andere.groesse_;

        // 'andere' neutralisieren:
        andere.daten_   = nullptr;
        andere.groesse_ = 0;

        return *this;
    }

    // ── 5/5: Destruktor ───────────────────────────────────────────────────
    ~StringPuffer() {
        std::cout << "  [DTOR]      ~StringPuffer(\""
                  << (daten_ ? daten_ : "nullptr") << "\")\n";
        delete[] daten_;   // nullptr ist OK: delete nullptr ist sicher
    }

    // Zugriffsmethoden
    const char* text() const { return daten_ ? daten_ : ""; }
    std::size_t groesse() const { return groesse_; }

private:
    std::size_t groesse_;
    char* daten_;
};

// ─────────────────────────────────────────────────────────────────────────────
// DEMO 1: Kopieren vs. Verschieben im Vergleich
// ─────────────────────────────────────────────────────────────────────────────

void demo_copy_vs_move() {
    std::cout << "\n=== 1. Kopieren vs. Verschieben ===\n";

    std::cout << "\n-- Kopieren (teuer bei großen Objekten) --\n";
    StringPuffer original("Hallo, Welt!");
    StringPuffer kopie = original;          // Copy-Konstruktor
    std::cout << "  original: \"" << original.text() << "\"\n";
    std::cout << "  kopie:    \"" << kopie.text() << "\"\n";

    std::cout << "\n-- Verschieben (kostengünstig!) --\n";
    StringPuffer temp("Temporärer Wert");
    // std::move: Sagt dem Compiler "ich brauche 'temp' nicht mehr,
    //            behandle ihn wie ein temporäres Objekt"
    // WICHTIG: Nach std::move ist 'temp' in einem gültigen aber undefinierten
    //          Zustand – nicht mehr benutzen!
    StringPuffer verschoben = std::move(temp);
    std::cout << "  verschoben: \"" << verschoben.text() << "\"\n";
    std::cout << "  temp nach move: \"" << temp.text() << "\" (leer!)\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// DEMO 2: std::move – Was es wirklich macht (und was nicht!)
// ─────────────────────────────────────────────────────────────────────────────
//
// std::move macht KEINEN Move!
// Es ist nur ein Cast: lvalue → rvalue-Referenz
// Erst der Move-Konstruktor/Move-Assignment macht den eigentlichen Move.
//
// Analogie: std::move ist das Schild "Zum Verkauf". Es verkauft das Haus
// nicht, sondern signalisiert nur, dass es verkauft werden darf.

void demo_std_move() {
    std::cout << "\n=== 2. std::move verstehen ===\n";

    std::string a = "Ich bin ein lvalue";
    std::string b = "Ich bin auch ein lvalue";

    std::cout << "  a: \"" << a << "\"\n";
    std::cout << "  b: \"" << b << "\"\n";

    // std::move(a) wandelt a in rvalue um → move assignment wird aufgerufen
    b = std::move(a);

    // Nach dem Move: a ist in "valid but unspecified state" (meist leer)
    std::cout << "  Nach b = std::move(a):\n";
    std::cout << "  b: \"" << b << "\"\n";
    std::cout << "  a: \"" << a << "\" (unspezifiziert – nicht nutzen!)\n";

    // std::swap nutzt intern move (deswegen ist es für komplexe Typen schnell)
    std::string x = "Aaaa", y = "Bbbb";
    std::swap(x, y);   // Intern: move x→tmp, move y→x, move tmp→y
    std::cout << "  Nach swap: x=\"" << x << "\", y=\"" << y << "\"\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// DEMO 3: Rückgabewerte und NRVO
// ─────────────────────────────────────────────────────────────────────────────
//
// NRVO (Named Return Value Optimization):
//   Der Compiler kann den Move/Copy bei return meist komplett weglassen
//   (Objekt wird direkt am Zielort konstruiert). Das nennt sich "Elision".
//
//   Faustregel: Bei return von lokalen Variablen KEIN std::move verwenden!
//   std::move verhindert NRVO und kann langsamer sein als ohne.

StringPuffer erzeuge_puffer(const char* text) {
    StringPuffer result(text);
    return result;   // NRVO: Compiler optimiert den Copy/Move weg
    // return std::move(result);  ← FALSCH! Verhindert NRVO!
}

void demo_return_values() {
    std::cout << "\n=== 3. Rückgabewerte und NRVO ===\n";
    std::cout << "  Erzeuge Puffer via Funktion:\n";
    StringPuffer p = erzeuge_puffer("Von Funktion");
    std::cout << "  Ergebnis: \"" << p.text() << "\"\n";
    // Dank NRVO sehen wir möglicherweise keinen Copy/Move-Konstruktor-Aufruf!
}

// ─────────────────────────────────────────────────────────────────────────────
// DEMO 4: Move-Semantik mit STL-Containern
// ─────────────────────────────────────────────────────────────────────────────

void demo_stl_moves() {
    std::cout << "\n=== 4. Move mit STL-Containern ===\n";

    std::vector<std::string> v1{"eins", "zwei", "drei", "vier", "fünf"};
    std::cout << "  v1.size() = " << v1.size() << "\n";

    // move übernimmt internen Speicher – kein Kopieren der Strings!
    std::vector<std::string> v2 = std::move(v1);
    std::cout << "  Nach move: v1.size()=" << v1.size()
              << ", v2.size()=" << v2.size() << "\n";

    // emplace_back konstruiert Objekt direkt im Vector (kein Kopieren/Moven!)
    std::vector<StringPuffer> puffer;
    puffer.reserve(3);  // Speicher vorallokieren (verhindert Reallocations)
    std::cout << "\n  emplace_back (konstruiert in-place):\n";
    puffer.emplace_back("Alpha");
    puffer.emplace_back("Beta");
    puffer.emplace_back("Gamma");
    std::cout << "  Puffer im Vector:\n";
    for (const auto& p : puffer) {
        std::cout << "    \"" << p.text() << "\"\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// DEMO 5: Perfect Forwarding mit std::forward
// ─────────────────────────────────────────────────────────────────────────────
//
// Problem: Eine Template-Funktion soll Argumente "perfekt" weitergeben:
//   - lvalue → als lvalue weitergeben (→ Copy-Konstruktor)
//   - rvalue → als rvalue weitergeben (→ Move-Konstruktor)
//
// Lösung: Universal Reference (T&&) + std::forward<T>()
//
// Wann wird das gebraucht?
//   - Wrapper-Funktionen (z.B. make_unique, emplace_back intern)
//   - Generische Hilfsfunktionen die Argumente weiterdelegieren

// T&& ist hier eine UNIVERSAL REFERENCE (nicht rvalue-Referenz!)
// weil T ein Template-Parameter ist. Kann sowohl an lvalue als auch rvalue binden.
template<typename T>
StringPuffer erstelle_mit_forward(T&& arg) {
    // std::forward<T>: Behält die Wertekategorie von 'arg' bei
    //   - war arg ein lvalue → forward gibt lvalue zurück → Copy
    //   - war arg ein rvalue → forward gibt rvalue zurück → Move
    std::cout << "  erstelle_mit_forward aufgerufen\n";
    return StringPuffer(std::forward<T>(arg));
}

void demo_perfect_forwarding() {
    std::cout << "\n=== 5. Perfect Forwarding ===\n";

    const char* text = "Forwarded String";

    std::cout << "\n  Mit lvalue:\n";
    StringPuffer quelle("Quelle für Forward");
    auto result1 = erstelle_mit_forward(quelle);         // lvalue → Copy
    std::cout << "  result1: \"" << result1.text() << "\"\n";

    std::cout << "\n  Mit rvalue (std::move):\n";
    auto result2 = erstelle_mit_forward(std::move(quelle)); // rvalue → Move
    std::cout << "  result2: \"" << result2.text() << "\"\n";

    std::cout << "\n  Mit Literal (rvalue):\n";
    auto result3 = erstelle_mit_forward(text);            // const char* → Copy
    std::cout << "  result3: \"" << result3.text() << "\"\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔══════════════════════════════════════════╗\n"
              << "║  THEMA 03 – Move-Semantik & Regel der 5 ║\n"
              << "╚══════════════════════════════════════════╝\n";

    demo_copy_vs_move();
    demo_std_move();
    demo_return_values();
    demo_stl_moves();
    demo_perfect_forwarding();

    std::cout << "\n[main() endet]\n";
    return 0;
}
