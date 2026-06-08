# Modernes C++ Tutorial (C++20)

Ein strukturiertes Lernprojekt für modernes C++ — von Basics bis Concurrency.  
Jedes Thema ist ein eigenständiges, ausführlich kommentiertes Programm.

---

## Inhaltsverzeichnis

| # | Thema | Wichtigste Konzepte |
|---|-------|---------------------|
| [01](#01-basics) | Basics | `auto`, `constexpr`, Structured Bindings, `[[nodiscard]]`, `nullptr` |
| [02](#02-smart-pointers--raii) | Smart Pointers & RAII | `unique_ptr`, `shared_ptr`, `weak_ptr` |
| [03](#03-move-semantik) | Move-Semantik | lvalue/rvalue, `std::move`, Regel der Fünf, Perfect Forwarding |
| [04](#04-templates--concepts) | Templates & Concepts | Funktions-/Klassen-Templates, Spezialisierung, `if constexpr`, Concepts |
| [05](#05-lambdas--stdffunction) | Lambdas | Captures, STL-Algorithmen, `std::function`, Closures |
| [06](#06-optional-variant--visit) | optional / variant | `std::optional`, `std::variant`, `std::visit`, `std::any` |
| [07](#07-enum-class) | enum class | Scoped Enums, Underlying Type, Bitmasken-Flags |
| [08](#08-stl-container--algorithmen) | STL Container | `vector`, `map`, `set`, Iteratoren, Algorithmen |
| [09](#09-type-traits) | Type Traits | Typ-Abfragen, Transformationen, `std::conditional` |
| [10](#10-stdchrono) | std::chrono | Zeitdauern, Clocks, RAII-Timer, `sleep_for` |
| [11](#11-concurrency) | Concurrency | `thread`, `mutex`, `atomic`, `async`/`future`, `condition_variable` |

---

## Voraussetzungen

```bash
# Compiler (g++ 11+ oder clang++ 13+)
g++ --version      # ≥ 11 für C++20

# Build-System
cmake --version    # ≥ 3.20

# Optional: GitHub CLI
gh --version
```

**Ubuntu/Debian installieren:**
```bash
sudo apt install g++ cmake
```

---

## Projekt bauen & ausführen

```bash
# 1. Repository klonen
git clone https://github.com/tipou82/CPP_Playground.git
cd CPP_Playground

# 2. Build-Verzeichnis konfigurieren (einmalig)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# 3. Alle Themen kompilieren
cmake --build build

# 4. Ein einzelnes Thema ausführen
./build/src/01_basics/01_basics
./build/src/11_concurrency/11_concurrency
# usw.
```

**Alle Themen auf einmal ausführen:**
```bash
for n in 01_basics 02_smart_pointers 03_move_semantik 04_templates \
         05_lambdas 06_optional_variant 07_enum_class 08_stl_containers \
         09_type_traits 10_chrono 11_concurrency; do
  echo "=== $n ===" && ./build/src/$n/$n
done
```

---

## Projektstruktur

```
CPP_Playground/
├── CMakeLists.txt          ← Root: C++20, Warnings, alle Themen
└── src/
    ├── 01_basics/
    │   ├── CMakeLists.txt
    │   └── main.cpp        ← ~200 Zeilen mit Tutorial-Kommentaren
    ├── 02_smart_pointers/
    │   └── ...
    └── ...                 ← Gleiche Struktur für alle 11 Themen
```

---

## 01 Basics

**Datei:** [`src/01_basics/main.cpp`](src/01_basics/main.cpp)

### Was lernst du?

Die grundlegenden modernen C++-Features, die in jedem Programm auftauchen.

### `auto` – Automatische Typableitung

Der Compiler kennt den Typ bereits — warum ihn nochmal schreiben?

```cpp
auto zahl  = 42;               // int
auto text  = std::string{"Hi"};
auto pi    = 3.14;             // double

// In Schleifen: const auto& verhindert unnötiges Kopieren
std::vector<std::string> namen{"Alice", "Bob"};
for (const auto& name : namen) {
    std::cout << name << "\n";
}
```

### `constexpr` – Auswertung zur Compile-Zeit

Berechnungen, die zur Compile-Zeit feststehen, sollten `constexpr` sein.  
Laufzeit-Overhead: null.

```cpp
constexpr long long fakultaet(int n) {
    return n <= 1 ? 1 : n * fakultaet(n - 1);
}

constexpr long long f10 = fakultaet(10);  // Wird vom Compiler berechnet!
// Im compilierten Programm steht nur noch: 3628800
```

### Structured Bindings (C++17)

Mehrere Rückgabewerte direkt auspacken:

```cpp
std::map<std::string, int> scores{{"Alice", 95}, {"Bob", 87}};

for (const auto& [name, punkte] : scores) {   // ← structured binding
    std::cout << name << ": " << punkte << "\n";
}
```

### `if`-Initialisierung (C++17)

Variable lebt nur im `if`-Block — kein "Namespace-Durchsickern":

```cpp
if (auto it = map.find("key"); it != map.end()) {
    std::cout << it->second << "\n";
}
// 'it' existiert hier NICHT mehr
```

### `[[nodiscard]]`

Compiler warnt wenn der Rückgabewert ignoriert wird:

```cpp
[[nodiscard]] int verbinden(const std::string& host);

verbinden("server");           // ⚠️  Warning: Rückgabewert ignoriert!
int status = verbinden("srv"); // ✓ OK
```

---

## 02 Smart Pointers & RAII

**Datei:** [`src/02_smart_pointers/main.cpp`](src/02_smart_pointers/main.cpp)

### Das Problem mit rohen Zeigern

```cpp
// Alte C++-Weise — fehleranfällig!
Ressource* r = new Ressource("DB");
// ... viel Code ...
// Was wenn hier eine Exception geworfen wird?
delete r;  // Wird vielleicht nie ausgeführt → Memory Leak!
```

### RAII – Resource Acquisition Is Initialization

Idee: Ressource im Konstruktor holen, im Destruktor freigeben.  
Der Destruktor wird **immer** aufgerufen — auch bei Exceptions.

### `unique_ptr` – Alleiniger Besitzer

```cpp
// Moderner Weg: kein delete nötig!
auto r = std::make_unique<Ressource>("DB");
r->info();                      // Zugriff wie normaler Zeiger

// Besitz übertragen (Kopieren nicht erlaubt!):
auto neuer = std::move(r);      // r ist jetzt nullptr
```

### `shared_ptr` – Geteilter Besitz

```cpp
auto sp1 = std::make_shared<Ressource>("Cache");
{
    auto sp2 = sp1;              // Kopieren erlaubt → Zähler: 2
    std::cout << sp1.use_count(); // 2
}                                // sp2 zerstört → Zähler: 1
// sp1 zerstört → Zähler: 0 → Ressource wird gelöscht
```

### `weak_ptr` – Beobachten ohne Besitz

Bricht zirkuläre Referenzen (die sonst zu Memory Leaks führen):

```cpp
std::weak_ptr<Ressource> beobachter = shared_ptr_obj;

// Vor Nutzung prüfen ob Objekt noch lebt:
if (auto tmp = beobachter.lock()) {
    tmp->info();  // sicher!
}
```

### Faustregel

| Situation | Smart Pointer |
|-----------|---------------|
| Alleiniger Besitzer (Normalfall) | `unique_ptr` |
| Mehrere Besitzer nötig | `shared_ptr` |
| Zirkuläre Referenz vermeiden | `weak_ptr` |
| Roher Zeiger (Legacy-API) | `T*` (kein Besitz!) |

---

## 03 Move-Semantik

**Datei:** [`src/03_move_semantik/main.cpp`](src/03_move_semantik/main.cpp)

### Das Problem: Kopieren ist teuer

```cpp
std::vector<int> a(1'000'000, 42);
std::vector<int> b = a;           // Kopiert 1 Million Elemente!
```

Wenn `a` danach nicht mehr gebraucht wird: warum nicht einfach die internen Daten "umziehen"?

### lvalue vs. rvalue

```cpp
int x = 42;      // x ist ein lvalue (hat Namen, hat Adresse)
int y = x + 1;   // x+1 ist ein rvalue (temporär, kein Name)

std::string s = "Hallo";         // s = lvalue
std::string t = std::string{"!"} // std::string{"!"} = rvalue (temporär)
```

### Move-Konstruktor: Ressourcen stehlen statt kopieren

```cpp
class Puffer {
    char* daten_;
public:
    // Copy: neuer Speicher + Inhalt kopieren (teuer)
    Puffer(const Puffer& andere) : daten_{new char[...]} {
        std::strcpy(daten_, andere.daten_);
    }

    // Move: Zeiger stehlen (sehr schnell!)
    Puffer(Puffer&& andere) noexcept : daten_{andere.daten_} {
        andere.daten_ = nullptr;  // Quelle neutralisieren
    }
};
```

### `std::move` — was es wirklich tut

`std::move` macht **keinen** Move! Es ist nur ein Cast: lvalue → rvalue-Referenz.  
Erst der Move-Konstruktor macht den eigentlichen Move.

```cpp
std::string a = "Hallo";
std::string b = std::move(a);  // a wird als rvalue behandelt → Move

// a ist jetzt in "valid but unspecified state" (meist leer)
// a NICHT mehr benutzen!
```

### Regel der Fünf

Wenn du eine dieser fünf Methoden selbst definierst, definiere alle fünf:

1. **Destruktor** `~T()`
2. **Copy-Konstruktor** `T(const T&)`
3. **Copy-Assignment** `T& operator=(const T&)`
4. **Move-Konstruktor** `T(T&&) noexcept`
5. **Move-Assignment** `T& operator=(T&&) noexcept`

### NRVO — Kein `std::move` bei `return`!

```cpp
std::string erzeuge() {
    std::string s = "Hallo";
    return s;              // ✓ Compiler optimiert (NRVO)
    // return std::move(s); ← FALSCH! Verhindert NRVO → langsamer!
}
```

---

## 04 Templates & Concepts

**Datei:** [`src/04_templates/main.cpp`](src/04_templates/main.cpp)

### Warum Templates?

Statt für jeden Typ eine eigene Funktion zu schreiben:

```cpp
// Ohne Templates: Code-Duplikation
int    maximum(int a, int b)       { return a > b ? a : b; }
double maximum(double a, double b) { return a > b ? a : b; }

// Mit Templates: ein Code, alle Typen
template<typename T>
T maximum(T a, T b) { return a > b ? a : b; }

maximum(3, 7);          // T = int
maximum(3.14, 2.71);    // T = double
maximum(std::string{"abc"}, std::string{"xyz"});  // T = string
```

### Klassen-Templates

```cpp
template<typename T>
class Stack {
public:
    void push(const T& wert) { daten_.push_back(wert); }
    T    pop()               { auto v = daten_.back(); daten_.pop_back(); return v; }
    bool leer() const        { return daten_.empty(); }
private:
    std::vector<T> daten_;
};

Stack<int>         int_stack;
Stack<std::string> str_stack;   // Gleiche Klasse, anderer Typ!
```

### Template-Spezialisierung

Bestimmte Typen bekommen eine besondere Implementierung:

```cpp
template<typename T>
void ausgabe(const T& v) { std::cout << v; }

template<>              // Vollständige Spezialisierung für bool
void ausgabe<bool>(const bool& v) {
    std::cout << (v ? "ja" : "nein");
}

ausgabe(42);    // → "42"    (allgemein)
ausgabe(true);  // → "ja"    (speziell für bool)
```

### `if constexpr` (C++17)

Compile-Zeit-Verzweigung — der nicht-genommene Zweig wird nicht kompiliert:

```cpp
template<typename T>
std::string beschreibe() {
    if constexpr (std::is_integral_v<T>)
        return "Ganzzahl";
    else if constexpr (std::is_floating_point_v<T>)
        return "Gleitkommazahl";
    else
        return "sonstiger Typ";
}
```

### Concepts (C++20) — Klare Anforderungen an Typen

```cpp
// Eigenes Concept: T muss sortierbar und kopierbar sein
template<typename T>
concept Vergleichbar = std::totally_ordered<T> && std::copy_constructible<T>;

// Nur Typen die Vergleichbar sind, dürfen diese Funktion nutzen
template<Vergleichbar T>
T sicheres_max(T a, T b) { return a > b ? a : b; }

// Vordefinierte Concepts aus <concepts>:
template<std::integral T>       // Nur Ganzzahlen
T verdopple(T x) { return x * 2; }
```

---

## 05 Lambdas & `std::function`

**Datei:** [`src/05_lambdas/main.cpp`](src/05_lambdas/main.cpp)

### Anatomie eines Lambdas

```
[ capture ] ( parameter ) -> rückgabetyp { körper }
```

```cpp
auto addiere = [](int a, int b) { return a + b; };
std::cout << addiere(3, 4);    // 7
```

### Captures — Zugriff auf äußere Variablen

```cpp
int schwelle = 5;
auto filter = [schwelle](int x) { return x > schwelle; };  // Kopie

int zaehler = 0;
auto zaehle = [&zaehler](int x) { if (x % 2 == 0) zaehler++; };  // Referenz!

// mutable: Kopie im Lambda verändern (ohne äußere Variable zu ändern)
auto countdown = [n = 10]() mutable { return --n; };
```

| Capture | Bedeutung |
|---------|-----------|
| `[]` | Kein Zugriff auf äußere Variablen |
| `[x]` | `x` wird **kopiert** (Snapshot zum Zeitpunkt der Lambda-Erstellung) |
| `[&x]` | `x` wird als **Referenz** genutzt |
| `[=]` | Alle genutzten Variablen by value |
| `[&]` | Alle genutzten Variablen by reference |

### Lambdas mit STL-Algorithmen

```cpp
std::vector<int> v{5, 3, 8, 1, 9, 2};

// Sortieren
std::sort(v.begin(), v.end(), [](int a, int b) { return a < b; });

// Filtern (erase-remove, C++20: erase_if)
std::erase_if(v, [](int x) { return x % 2 == 0; });  // Gerade entfernen

// Transformieren
std::transform(v.begin(), v.end(), v.begin(), [](int x) { return x * x; });

// Bedingung prüfen
bool hat_grosse = std::any_of(v.begin(), v.end(), [](int x) { return x > 7; });
```

### `std::function` — Callables speichern

```cpp
// Akzeptiert Lambda, Funktionszeiger, Funktionsobjekt
std::function<void(int)> callback = [](int x) { std::cout << x; };
callback(42);

// Nützlich für Event-Systeme:
std::vector<std::function<void(const std::string&)>> handler;
handler.push_back([](const std::string& msg) { std::cout << "[LOG] " << msg; });
handler.push_back([](const std::string& msg) { /* UI update */ });
```

### Closures — Lambdas mit Zustand

```cpp
// Factory die Lambdas mit eigenem Zustand zurückgibt
auto mache_zaehler = [](int start = 0) {
    return [count = start]() mutable { return ++count; };
};

auto z1 = mache_zaehler(0);
auto z2 = mache_zaehler(100);

z1();  // 1
z1();  // 2
z2();  // 101   ← unabhängig von z1!
```

---

## 06 optional / variant / visit

**Datei:** [`src/06_optional_variant/main.cpp`](src/06_optional_variant/main.cpp)

### `std::optional` — "Wert oder nichts"

Ersetzt gefährliche Magic Numbers (`-1`, `nullptr`) und Out-Parameter:

```cpp
// Ohne optional: -1 als "nicht gefunden"? Und wenn -1 ein gültiger Index ist?
int suche_alt(const std::vector<int>& v, int ziel) { /* return -1 */ }

// Mit optional: klar und typsicher
std::optional<int> suche(const std::vector<int>& v, int ziel) {
    for (size_t i = 0; i < v.size(); ++i)
        if (v[i] == ziel) return static_cast<int>(i);
    return std::nullopt;
}

auto idx = suche(v, 42);
if (idx) std::cout << "Gefunden bei: " << *idx;       // Prüfung + Zugriff
int pos = suche(v, 42).value_or(-1);                  // Fallback-Wert
```

### `std::variant` — Typsichere Union

Ein Wert, der verschiedene Typen haben kann — aber immer nur einen gleichzeitig:

```cpp
using Ergebnis = std::variant<int, std::string>;  // Erfolg oder Fehlermeldung

Ergebnis teile(int a, int b) {
    if (b == 0) return std::string{"Division durch Null!"};
    return a / b;
}

auto r = teile(10, 2);
if (std::holds_alternative<int>(r))
    std::cout << "Ergebnis: " << std::get<int>(r);
```

### `std::visit` — Pattern Matching

Für jede mögliche Typvariante des `variant` einen Handler definieren:

```cpp
// Overloaded-Trick: Mehrere Lambdas zu einem Visitor kombinieren
template<typename... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template<typename... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

using Zellwert = std::variant<int, double, std::string, bool>;

auto formatiere = Overloaded{
    [](int n)               { std::cout << "INT: "    << n    << "\n"; },
    [](double d)            { std::cout << "DOUBLE: " << d    << "\n"; },
    [](const std::string& s){ std::cout << "STRING: " << s    << "\n"; },
    [](bool b)              { std::cout << "BOOL: "   << b    << "\n"; }
};

std::vector<Zellwert> tabelle{42, 3.14, std::string{"Hallo"}, true};
for (const auto& zelle : tabelle)
    std::visit(formatiere, zelle);
```

---

## 07 enum class

**Datei:** [`src/07_enum_class/main.cpp`](src/07_enum_class/main.cpp)

### Das Problem mit C-Enums

```cpp
// C-Enum: global, kein Typschutz
enum Farbe { ROT, GRUEN, BLAU };
enum Ampel { ROT, GELB, GRUEN };  // ❌ FEHLER: ROT und GRUEN doppelt definiert!

int x = ROT;   // ❌ implizite Konvertierung zu int — oft ein Bug!
```

### enum class — sicher und typsicher

```cpp
enum class Farbe { Rot, Gruen, Blau };
enum class Ampel { Rot, Gelb, Gruen };  // ✓ Kein Konflikt (separate Scopes)

Farbe f = Farbe::Rot;     // Muss qualifiziert werden
// int x = f;             // ❌ FEHLER: keine implizite Konvertierung
int x = static_cast<int>(f);  // ✓ Nur explizit möglich
```

### Underlying Type festlegen

```cpp
enum class Richtung : uint8_t { Nord, Sued, Ost, West };  // nur 1 Byte
enum class HttpStatus : uint16_t { OK = 200, NotFound = 404 };

std::cout << sizeof(Richtung);   // 1 (statt 4 für int)
```

### Bitmasken-Flags

```cpp
enum class Recht : uint8_t {
    Lesen     = 1 << 0,   // 0b001
    Schreiben = 1 << 1,   // 0b010
    Ausfuehren = 1 << 2,  // 0b100
};

// Mit Operator-Overloading typsicher kombinierbar:
Recht user = Recht::Lesen | Recht::Schreiben;   // 0b011
bool  kann_lesen = (user & Recht::Lesen) == Recht::Lesen;  // true
```

---

## 08 STL Container & Algorithmen

**Datei:** [`src/08_stl_containers/main.cpp`](src/08_stl_containers/main.cpp)

### Container-Übersicht

```
Sequenz                    Assoziativ (sortiert)     Hash-basiert (O(1))
──────────────────────     ─────────────────────     ───────────────────
vector    dynamisch        map<K,V>  Key→Value        unordered_map
array     statisch (Stack) set       eindeutige Keys  unordered_set
deque     beidseitig       multimap  Duplikat-Keys
list      verknüpft        multiset  Duplikat-Werte
```

### `std::vector` — der Allrounder

```cpp
std::vector<int> v{1, 2, 3};
v.reserve(100);          // Speicher vorallokieren — verhindert Re-Allokationen!
v.push_back(4);          // Kopie anhängen
v.emplace_back(5);       // Direkt konstruieren (effizienter)
v.erase(v.begin() + 1);  // Element an Index 1 entfernen

// C++20: erase_if (ersetzt das umständliche erase-remove-Idiom)
std::erase_if(v, [](int x) { return x % 2 == 0; });
```

### `std::map` vs. `std::unordered_map`

```cpp
std::map<std::string, int> m;       // O(log n), sortiert nach Key
std::unordered_map<std::string, int> um;  // O(1) amortisiert, unsortiert

// Beide haben dieselbe API:
m["Alice"] = 25;
m.insert({"Bob", 30});
m.contains("Alice");                // C++20: boolean check
auto [it, ok] = m.try_emplace("Carol", 28);  // C++17: nur einfügen wenn neu
```

### Wichtige Algorithmen

```cpp
#include <algorithm>
#include <numeric>

std::vector<int> v{5, 3, 8, 1, 9, 2};

// Suchen & Prüfen
std::any_of(v, [](int x){ return x > 7; });        // true
std::min_element(v.begin(), v.end());               // Iterator auf 1

// Umformen
std::sort(v.begin(), v.end());                      // {1,2,3,5,8,9}
std::stable_partition(v.begin(), v.end(),
    [](int x){ return x % 2 == 0; });              // Gerade nach vorne

// Erzeugen
std::vector<int> seq(10);
std::iota(seq.begin(), seq.end(), 1);               // {1,2,3,...,10}

// Reduzieren
int summe = std::accumulate(v.begin(), v.end(), 0); // 28
```

---

## 09 Type Traits

**Datei:** [`src/09_type_traits/main.cpp`](src/09_type_traits/main.cpp)

### Was sind Type Traits?

Template-Klassen in `<type_traits>`, die zur **Compile-Zeit** Typ-Informationen liefern.  
Sie machen Templates "klug" — unterschiedliches Verhalten je nach Typ.

### Typ-Abfragen

```cpp
std::is_integral_v<int>          // true
std::is_floating_point_v<double> // true
std::is_pointer_v<int*>          // true
std::is_const_v<const int>       // true
std::is_same_v<int, long>        // false
std::is_base_of_v<Basis, Kind>   // true (Vererbungsbeziehung)

// C++17 _v Shorthand (statt ::value):
// std::is_integral<int>::value  →  std::is_integral_v<int>
```

### Typ-Transformationen

```cpp
std::remove_reference_t<int&>    // → int     (intern von std::move genutzt!)
std::remove_const_t<const int>   // → int
std::add_pointer_t<int>          // → int*
std::decay_t<int[5]>             // → int*    (Array→Zeiger, wie auto x = arr)
```

### `std::conditional` — Typ zur Compile-Zeit wählen

```cpp
// Wie der ternäre Operator ?: aber für Typen
template<bool BIG>
using ZahlTyp = std::conditional_t<BIG, long long, int>;

ZahlTyp<true>  x = 1'000'000'000'000LL;  // long long
ZahlTyp<false> y = 42;                    // int
```

### Praktische Nutzung mit `if constexpr`

```cpp
template<typename T>
void ausgabe_smart(const T& v) {
    if constexpr (std::is_floating_point_v<T>)
        std::cout << std::fixed << v;
    else if constexpr (std::is_integral_v<T>)
        std::cout << v << " (int)";
    else
        std::cout << v;
}
```

---

## 10 std::chrono

**Datei:** [`src/10_chrono/main.cpp`](src/10_chrono/main.cpp)

### Drei Bausteine

| Baustein | Beschreibung | Beispiel |
|----------|-------------|---------|
| `duration` | Eine Zeitdauer | `500ms`, `2h`, `100us` |
| `clock` | Liefert aktuellen Zeitpunkt | `steady_clock::now()` |
| `time_point` | Ein Zeitpunkt | `now()` |

### Zeitdauern & Literal-Syntax

```cpp
using namespace std::chrono_literals;

auto d1 = 2h;      // 2 Stunden
auto d2 = 30min;   // 30 Minuten
auto d3 = 500ms;   // 500 Millisekunden
auto d4 = 100us;   // 100 Mikrosekunden
auto d5 = 50ns;    // 50 Nanosekunden

// Umrechnen (verlustbehaftet!):
auto sekunden = std::chrono::duration_cast<std::chrono::seconds>(1500ms);
std::cout << sekunden.count();  // 1 (abgeschnitten, nicht gerundet)

// Arithmetik:
auto gesamt = 1h + 30min + 45s;  // 5445 Sekunden
```

### Laufzeit messen

```cpp
auto start = std::chrono::steady_clock::now();

// ... Code der gemessen werden soll ...

auto ende  = std::chrono::steady_clock::now();
auto dauer = std::chrono::duration_cast<std::chrono::milliseconds>(ende - start);
std::cout << "Dauer: " << dauer.count() << " ms\n";
```

### RAII-Timer

```cpp
class Timer {
    std::string label_;
    std::chrono::steady_clock::time_point start_{std::chrono::steady_clock::now()};
public:
    explicit Timer(std::string l) : label_{std::move(l)} {}
    ~Timer() {
        auto ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - start_).count();
        std::cout << "[TIMER] " << label_ << ": " << ms << " ms\n";
    }
};

{
    Timer t("sort von 100k Elementen");
    std::sort(v.begin(), v.end());
}  // ← Destruktor gibt automatisch die Laufzeit aus
```

---

## 11 Concurrency

**Datei:** [`src/11_concurrency/main.cpp`](src/11_concurrency/main.cpp)

### `std::thread` — Threads starten

```cpp
#include <thread>

void arbeit(int id) { std::cout << "Thread " << id << "\n"; }

std::thread t1(arbeit, 1);                      // Freie Funktion
std::thread t2([](){ std::cout << "Lambda\n"; });  // Lambda

t1.join();  // Warten bis Thread fertig — MUSS aufgerufen werden!
t2.join();
```

### Das Data-Race-Problem

```cpp
int zaehler = 0;
// Thread 1 und Thread 2 gleichzeitig:
zaehler++;  // Lesen + Addieren + Schreiben = 3 CPU-Schritte!
            // Dazwischen kann ein anderer Thread eingreifen → falsches Ergebnis!
```

### `std::mutex` — Exklusiver Zugriff

```cpp
std::mutex m;
int zaehler = 0;

void sicher_inkrementieren() {
    std::lock_guard<std::mutex> lock(m);  // Sperrt bei Erzeugung
    zaehler++;
}   // ← lock wird zerstört → mutex automatisch entsperrt (auch bei Exceptions!)
```

### `std::atomic` — Schneller als Mutex für einzelne Variablen

```cpp
std::atomic<int> zaehler{0};

// Alle folgenden Operationen sind atomar (unteilbar):
zaehler++;                       // fetch_add(1)
zaehler.fetch_add(5);           // Addiere 5, gib alten Wert zurück
zaehler.compare_exchange_strong(erwartet, neu);  // CAS-Operation
```

**Wann was?**
- `atomic` → für **einzelne** Variable (Zähler, Flags)
- `mutex` → für **mehrere** Variablen die konsistent sein müssen

### `std::async` & `std::future` — Asynchrone Aufgaben

```cpp
// Startet Funktion in separatem Thread, gibt Future zurück
auto f1 = std::async(std::launch::async, berechne, 1'000'000);
auto f2 = std::async(std::launch::async, berechne, 2'000'000);

// Beide laufen parallel während wir hier weiterarbeiten!
auto r1 = f1.get();  // Wartet und holt Ergebnis
auto r2 = f2.get();
```

### `std::condition_variable` — Producer-Consumer

```cpp
std::queue<int> queue;
std::mutex m;
std::condition_variable cv;

// Producer: Daten erzeugen
std::lock_guard lock(m);
queue.push(wert);
cv.notify_one();  // Einen wartenden Consumer aufwecken

// Consumer: Auf Daten warten (kein Busy-Waiting!)
std::unique_lock lock(m);
cv.wait(lock, []{ return !queue.empty(); });  // Schläft, bis Bedingung true
int wert = queue.front(); queue.pop();
```

---

## Lernpfad

```
Anfänger:    01 → 02 → 07 → 08
Mittel:      03 → 04 → 05 → 06
Fortgeschr.: 09 → 10 → 11
```

---

## Lizenz

Dieses Projekt ist für Lernzwecke frei verwendbar.
