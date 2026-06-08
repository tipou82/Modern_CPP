# Modernes C++ Tutorial (C++20 / C++23)

Ein strukturiertes Lernprojekt für modernes C++ — von Basics bis Concurrency und C++23.  
Jedes Thema ist ein eigenständiges, ausführlich kommentiertes Programm.

---

## ⚠️ Sicherheits-Cheatsheet

> Modernes C++ hat für fast jeden klassischen Fehler eine sichere Alternative.  
> Nutze immer zuerst die rechte Spalte.

| Klassischer Fehler | Unsicher ❌ | Sicher ✅ (Modernes C++) |
|---|---|---|
| Speicherleck | `new` / `delete` | `unique_ptr`, `shared_ptr` |
| Dangling Pointer | roher `T*` als Besitzer | `unique_ptr` / `shared_ptr` |
| Zirkuläre Referenz | `shared_ptr` in Kreisen | `weak_ptr` |
| Null-Dereferenzierung | `T*` ohne Prüfung | `std::optional<T>` |
| Vergessener Fehlercode | `int` Rückgabewert | `[[nodiscard]]` / `std::expected` |
| Enum-Namenskonflikt | C-`enum` | `enum class` |
| Implizite int-Konversion | C-`enum` | `enum class` (kein impliziter Cast) |
| Unsicherer Union | `union` | `std::variant` |
| Array-Überlauf | `int arr[N]` | `std::array<T,N>` mit `.at()` |
| Data Race | `int` shared | `std::atomic<int>` / `std::mutex` |
| Vergessenes `unlock()` | `mutex.lock()` / `unlock()` | `std::lock_guard` (RAII) |
| Thread ohne `join()` | `std::thread` unverwaltet | immer `.join()` oder `.detach()` |
| Magic Number als Fehler | `return -1;` | `std::optional` / `std::expected` |
| Array zerfällt zu Zeiger | `void f(int arr[])` | `std::span<T>` / `std::array` |
| Integer-Überlauf unbemerkt | signed overflow (UB!) | `std::expected` + Range-Check |

**Faustregel:** Wenn du `new`, `delete`, `NULL`, rohe Arrays, C-Enums oder manuelles `lock()/unlock()` schreibst — gibt es eine sichere moderne Alternative.

---

## MISRA C++ 2023 – Bezüge zu diesem Tutorial

[MISRA C++:2023](https://misra.org.uk/misra-cpp/) (veröffentlicht März 2023, Basis: ISO C++17) ist der Industriestandard für sicherheitskritisches C++ in Automotive, Avionik und Medizintechnik. Die Tabelle zeigt, welche Regeln die hier gelernten Features direkt adressieren.

| Regel | Typ | Feature in diesem Tutorial |
|-------|-----|---------------------------|
| **0.0.1** No unreachable code | Required | Alle Themen: kein toter Code |
| **0.1.2** Return value shall be used | Required | Thema 01: `[[nodiscard]]` erzwingt diese Regel |
| **7.11.2** Array shall not decay to pointer as function argument | Required | Thema 08: `std::span` / `std::array` statt `T[]` |
| **8.1.1** Non-transient lambda shall not implicitly capture `this` | Required | Thema 05: explizite Capture-Liste |
| **8.1.2** Variables should be captured explicitly in non-transient lambda | Advisory | Thema 05: `[x]` statt `[&]` / `[=]` |
| **8.2.2** C-style casts shall not be used | Required | Alle Themen: nur `static_cast`, `const_cast` etc. |
| **10.2.1** Enumeration shall be defined with an explicit underlying type | Required | Thema 07: `enum class Status : uint8_t` |
| **10.2.2** Unscoped enumerations should not be declared | Advisory | Thema 07: `enum class` statt `enum` |
| **11.3.1** Variables of array type should not be declared | Advisory | Thema 08: `std::array<T,N>` statt `int arr[N]` |
| **15.0.1** Special member functions shall be provided appropriately | Required | Thema 03: Regel der Fünf / Regel der Null |
| **15.0.2** User-provided copy/move functions should have appropriate signatures | Advisory | Thema 03: korrekte Signaturen mit `noexcept` |
| **15.1.3** Single-argument constructors shall be `explicit` | Required | Themen 02 / 03: kein impliziter Einzel-Arg-Konstruktor |
| **17.8.1** Function templates shall not be explicitly specialized | Required | Thema 04: Concepts statt expliziter Spezialisierung |
| **18.1.1** Exception object shall not have pointer type | Required | Thema 12: `std::expected` als Exception-Alternative |
| **18.4.1** Exception-unfriendly functions shall be `noexcept` | Required | Themen 03 / 11 / 12: `noexcept` bei Move, Destruktoren |
| **21.6.1** Dynamic memory should not be used | Required | Thema 02: kein `new` / `delete` → smart pointers |
| **21.6.2** Dynamic memory shall be managed automatically | Required | Thema 02: `make_unique`, `make_shared` |
| **23.11.1** Raw pointer ctors of `shared_ptr` / `unique_ptr` should not be used | Advisory | Thema 02: immer `make_unique` / `make_shared` |

> **Hinweis:** MISRA C++:2023 basiert auf C++17. `std::expected` (C++23) und `std::to_underlying` (C++23) sind noch nicht direkt erfasst, füllen aber die in den Regeln adressierten Lücken (explizite Fehlerbehandlung, sichere Enum-Konvertierung).

---

## Inhaltsverzeichnis

| # | Thema | Safety-Features | C++-Version |
|---|-------|-----------------|-------------|
| [01](#01-basics) | Basics | `[[nodiscard]]`, `nullptr`, `constexpr` | C++11–17 |
| [02](#02-smart-pointers--raii) | Smart Pointers & RAII | `unique_ptr`, `shared_ptr`, `weak_ptr` | C++11 |
| [03](#03-move-semantik) | Move-Semantik | Regel der Fünf, `noexcept` | C++11 |
| [04](#04-templates--concepts) | Templates & Concepts | Concepts, `static_assert` | C++11–20 |
| [05](#05-lambdas--stdffunction) | Lambdas | explizite Captures, `std::function` | C++11–14 |
| [06](#06-optional-variant--visit) | optional / variant | `std::optional`, `std::variant` | C++17 |
| [07](#07-enum-class) | enum class | Scoped Enums, Bitmasken | C++11 |
| [08](#08-stl-container--algorithmen) | STL Container | `.at()`, `erase_if`, Algorithmen | C++11–20 |
| [09](#09-type-traits) | Type Traits | `requires`, `static_assert` | C++11–20 |
| [10](#10-stdchrono) | std::chrono | RAII-Timer, typsichere Zeitdauern | C++11 |
| [11](#11-concurrency) | Concurrency | `atomic`, `lock_guard`, `call_once` | C++11 |
| [12](#12-c23-highlights) | **C++23 Highlights** | **`std::expected`**, monadic optional | **C++23** |

---

## Voraussetzungen

```bash
# Compiler
g++ --version     # ≥ 11 für Themen 01–11 (C++20)
g++-12 --version  # ≥ 12 für Thema 12 (C++23)

# Build-System
cmake --version   # ≥ 3.20
```

**Ubuntu/Debian installieren:**
```bash
sudo apt install g++ cmake        # Für Themen 01–11
sudo apt install g++-12           # Zusätzlich für Thema 12 (C++23)
```

---

## Projekt bauen & ausführen

```bash
# 1. Repository klonen
git clone https://github.com/tipou82/Modern_CPP.git
cd Modern_CPP

# 2. Build-Verzeichnis konfigurieren (einmalig)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# 3. Alle Themen kompilieren
cmake --build build

# 4. Einzelnes Thema ausführen
./build/src/01_basics/01_basics
./build/src/12_cpp23/12_cpp23
```

**Alle Themen auf einmal:**
```bash
for n in 01_basics 02_smart_pointers 03_move_semantik 04_templates \
         05_lambdas 06_optional_variant 07_enum_class 08_stl_containers \
         09_type_traits 10_chrono 11_concurrency 12_cpp23; do
  echo "=== $n ===" && ./build/src/$n/$n
done
```

---

## Projektstruktur

```
Modern_CPP/
├── CMakeLists.txt
└── src/
    ├── 01_basics/          main.cpp  (~250 Zeilen)
    ├── 02_smart_pointers/  main.cpp
    ├── ...
    └── 12_cpp23/           main.cpp  (benötigt g++-12)
```

---

## 01 Basics

**Datei:** [`src/01_basics/main.cpp`](src/01_basics/main.cpp)

### `auto` – Automatische Typableitung

```cpp
auto zahl = 42;              // int
auto text = std::string{"Hi"};

// In Schleifen: const auto& verhindert unnötiges Kopieren
for (const auto& name : namen) { ... }
```

### `constexpr` – Auswertung zur Compile-Zeit

```cpp
constexpr long long fakultaet(int n) {
    return n <= 1 ? 1 : n * fakultaet(n - 1);
}
constexpr long long f10 = fakultaet(10);  // Vom Compiler berechnet, kein Overhead
```

### Structured Bindings (C++17)

```cpp
for (const auto& [name, punkte] : score_map) {
    std::cout << name << ": " << punkte << "\n";
}
```

### `if`-Initialisierung (C++17)

```cpp
if (auto it = map.find("key"); it != map.end()) {
    std::cout << it->second;
}
// 'it' existiert hier NICHT mehr → kein versehentlicher Zugriff
```

### ⚠️ Safety: `[[nodiscard]]`

```cpp
[[nodiscard]] int verbinden(const std::string& host);

verbinden("server");           // ⚠️ Compiler-Warnung: Rückgabewert ignoriert!
int status = verbinden("srv"); // ✓ Fehlercode wird behandelt
```

> **Wann nutzen:** Immer wenn der Rückgabewert ein Fehlercode, allokierter  
> Speicher oder ein Status ist, der nie ignoriert werden darf.

> **MISRA C++:2023** – `[[nodiscard]]` ist die syntaktische Umsetzung von Regel **0.1.2** (Required): Rückgabewerte von Funktionen müssen verwendet werden.

### ⚠️ Safety: `nullptr` statt `NULL`

```cpp
void f(int x);
void f(int* p);

f(NULL);    // ❌ Ruft f(int) auf! NULL ist nur eine 0.
f(nullptr); // ✓ Ruft f(int*) auf. Eindeutig typsicher.
```

---

## 02 Smart Pointers & RAII

**Datei:** [`src/02_smart_pointers/main.cpp`](src/02_smart_pointers/main.cpp)

### ⚠️ Safety: Das Problem mit rohen Zeigern

```cpp
// ❌ Fehleranfällig – 4 mögliche Bugs:
Ressource* r = new Ressource();
// 1. Memory Leak wenn delete vergessen
// 2. Double-Free wenn delete zweimal
// 3. Dangling Pointer nach delete
// 4. Exception vor delete → Leak!
delete r;
```

### ⚠️ Safety: RAII – Automatische Ressourcenverwaltung

```cpp
// ✓ Sicher – Destruktor wird IMMER aufgerufen (auch bei Exceptions!)
auto r = std::make_unique<Ressource>();
// Kein delete nötig – kein Leak möglich
```

### `unique_ptr` – Alleiniger Besitzer

```cpp
auto r = std::make_unique<Ressource>("DB");
auto neuer = std::move(r);  // Besitz übertragen (r ist jetzt nullptr)
// Kopieren ist verboten → kein versehentliches Teilen des Besitzes
```

### `shared_ptr` – Geteilter Besitz

```cpp
auto sp = std::make_shared<Ressource>("Cache");
auto sp2 = sp;               // Zähler: 2
// Wenn letzter shared_ptr zerstört wird → Objekt gelöscht
```

### ⚠️ Safety: `weak_ptr` verhindert Memory Leaks

```cpp
// ❌ Zirkuläre Referenz → Memory Leak!
struct Node { std::shared_ptr<Node> next; };

// ✓ weak_ptr bricht den Kreis
struct Node { std::weak_ptr<Node> next; };
```

### Faustregel

| Situation | Typ |
|-----------|-----|
| Alleiniger Besitzer | `unique_ptr` |
| Geteilter Besitz | `shared_ptr` |
| Beobachten ohne Besitz | `weak_ptr` |
| Kein Heap, lokale Variable | Stack-Objekt (kein Pointer!) |

> **MISRA C++:2023** – Direkte Umsetzung von Regel **21.6.1** (Required: kein dynamisches Speicher direkt), **21.6.2** (Required: automatische Verwaltung) und **23.11.1** (Advisory: kein `new` in smart-pointer-Konstruktoren → immer `make_unique` / `make_shared`). `explicit` Konstruktoren: Regel **15.1.3** (Required).

---

## 03 Move-Semantik

**Datei:** [`src/03_move_semantik/main.cpp`](src/03_move_semantik/main.cpp)

### lvalue vs. rvalue

```cpp
int x = 42;       // x ist lvalue (hat Name, hat Adresse)
int y = x + 1;    // x+1 ist rvalue (temporär, kein Name)
```

### Move statt Copy

```cpp
std::vector<int> a(1'000'000, 42);
std::vector<int> b = std::move(a);  // Kein Kopieren! Zeiger wird "gestohlen".
// a ist danach leer – nicht mehr benutzen!
```

### ⚠️ Safety: Regel der Fünf

Wenn du **eine** dieser Methoden selbst definierst, musst du **alle fünf** definieren:

```cpp
class MeineKlasse {
    ~MeineKlasse();                            // 1. Destruktor
    MeineKlasse(const MeineKlasse&);           // 2. Copy-Konstruktor
    MeineKlasse& operator=(const MeineKlasse&);// 3. Copy-Assignment
    MeineKlasse(MeineKlasse&&) noexcept;       // 4. Move-Konstruktor
    MeineKlasse& operator=(MeineKlasse&&) noexcept; // 5. Move-Assignment
};
```

> **Warum:** Wenn du einen Destruktor hast, verwaltest du Ressourcen manuell.  
> Dann muss auch Kopieren und Verschieben korrekt implementiert sein —  
> sonst entstehen Doppel-Frees oder Leaks.

### ⚠️ Safety: `noexcept` bei Move-Operationen

```cpp
MeineKlasse(MeineKlasse&&) noexcept { ... }  // ← noexcept IMMER hinzufügen!
```

> `noexcept` auf Move-Operationen erlaubt `std::vector` starke Exception-Safety  
> zu garantieren. Ohne `noexcept` fällt vector auf den Copy-Pfad zurück.

> **MISRA C++:2023** – Regel **15.0.1** (Required: Regel der Fünf – alle fünf Spezialmethoden konsistent definieren), **15.0.2** (Advisory: korrekte Signaturen), **18.4.1** (Required: `noexcept` bei Move-Konstruktoren und Move-Assignment), **15.1.3** (Required: `explicit` für Einzel-Argument-Konstruktoren).

---

## 04 Templates & Concepts

**Datei:** [`src/04_templates/main.cpp`](src/04_templates/main.cpp)

### Funktions-Templates

```cpp
template<typename T>
T maximum(T a, T b) { return a > b ? a : b; }

maximum(3, 7);       // T = int
maximum(3.14, 2.71); // T = double
```

### ⚠️ Safety: Concepts (C++20) – Klare Anforderungen

```cpp
// ❌ Ohne Concepts: seitenlange, unlesbare Fehlermeldungen
template<typename T>
T unsicher(T a, T b) { return a > b ? a : b; }

// ✓ Mit Concepts: Klare Fehlermeldung "T erfüllt Concept Vergleichbar nicht"
template<typename T>
    requires std::totally_ordered<T>
T sicher(T a, T b) { return a > b ? a : b; }

// Noch kürzer:
template<std::totally_ordered T>
T sicher(T a, T b) { return a > b ? a : b; }
```

### ⚠️ Safety: `if constexpr` statt Spezialisierungen

```cpp
template<typename T>
void verarbeite(const T& v) {
    if constexpr (std::is_pointer_v<T>) {
        if (v == nullptr) return;  // Null-Check nur für Zeiger
        // ...
    } else {
        // Anderer Code für Nicht-Zeiger
    }
}
```

> **MISRA C++:2023** – Regel **17.8.1** (Required: Funktions-Templates dürfen nicht explizit spezialisiert werden → Concepts oder Overloads bevorzugen). Regel **8.2.2** (Required: kein C-Cast) – alle Typkonvertierungen nur mit `static_cast` etc.

---

## 05 Lambdas & `std::function`

**Datei:** [`src/05_lambdas/main.cpp`](src/05_lambdas/main.cpp)

### Lambda-Anatomie

```
[ capture ] ( parameter ) -> rückgabetyp { körper }
```

### ⚠️ Safety: Capture-Regeln

```cpp
int x = 42;

// ❌ [&] in asynchronem Code → Dangling Reference!
auto future = std::async([&]{ return x * 2; });  // x könnte weg sein!

// ✓ [=] oder explizites [x] kopiert den Wert sicher
auto future = std::async([x]{ return x * 2; });  // sicher
```

| Capture | Safety-Hinweis |
|---------|---------------|
| `[=]` | Sicher, aber kopiert alles |
| `[x]` | ✓ Empfohlen: nur was gebraucht wird |
| `[&x]` | Nur wenn Lambda kürzer lebt als `x` |
| `[&]` | ⚠️ Gefährlich in async/gespeicherten Lambdas |

### Lambdas mit STL-Algorithmen

```cpp
// C++20: erase_if (sicherer als erase-remove-Idiom)
std::erase_if(v, [](int x) { return x < 0; });  // Alle Negativen entfernen

std::sort(v.begin(), v.end(), [](int a, int b) { return a < b; });
bool hat_grosse = std::any_of(v.begin(), v.end(), [](int x) { return x > 100; });
```

> **MISRA C++:2023** – Regel **8.1.1** (Required: non-transient Lambdas dürfen `this` nicht implizit capturen) und **8.1.2** (Advisory: Variablen sollen explizit gecaptured werden, kein `[&]` / `[=]` für gespeicherte/asynchrone Lambdas). Begründung: implizite Captures verschleiern Abhängigkeiten und provozieren Dangling References.

---

## 06 optional / variant / visit

**Datei:** [`src/06_optional_variant/main.cpp`](src/06_optional_variant/main.cpp)

### ⚠️ Safety: `std::optional` – kein Null-Pointer, kein Magic Number

```cpp
// ❌ Unsicher: -1 als "nicht gefunden" – was wenn -1 gültig ist?
int suche(std::vector<int>& v, int ziel);  // return -1 bei Fehler

// ✓ Sicher: Typ macht "kein Wert" explizit
std::optional<int> suche(std::vector<int>& v, int ziel);

auto idx = suche(v, 42);
if (idx)  std::cout << "Gefunden: " << *idx;  // nur wenn Wert da!
int pos = suche(v, 42).value_or(-1);           // sicherer Fallback
```

### ⚠️ Safety: `std::variant` statt `union`

```cpp
// ❌ C-Union: kein Typtracking, kein Destruktor, undefined behavior
union Wert { int i; double d; std::string s; };  // s wird nie zerstört!

// ✓ std::variant: typsicher, RAII, kein UB
std::variant<int, double, std::string> wert;
wert = std::string{"Hallo"};
// Destruktor wird immer korrekt aufgerufen!
```

### `std::visit` – Pattern Matching

```cpp
template<typename... Ts> struct Overloaded : Ts... { using Ts::operator()...; };
template<typename... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

std::visit(Overloaded{
    [](int n)  { std::cout << "int: "    << n; },
    [](double d){ std::cout << "double: " << d; },
    [](auto&)  { std::cout << "sonstig"; }   // Catch-all
}, mein_variant);
```

> **MISRA C++:2023** – `std::optional` verhindert Null-Dereferenzierung (MISRA-Sicherheitsziel). `std::variant` ersetzt `union` (kein undefined behavior bei Typ-Mismatch). Regel **18.1.1** (Required: Exception-Objekte dürfen keine Zeiger sein) – `std::optional` / `std::expected` sind typsichere Alternativen zu Ausnahmen für erwartbare Fehler.

---

## 07 enum class

**Datei:** [`src/07_enum_class/main.cpp`](src/07_enum_class/main.cpp)

### ⚠️ Safety: enum class vs. C-enum

```cpp
// ❌ C-enum: Namenskonflikte + implizite int-Konvertierung
enum Farbe { ROT, GRUEN, BLAU };
enum Ampel { ROT, GELB, GRUEN };  // FEHLER: ROT doppelt!
int x = ROT;                       // implizit zu int → oft ein Bug

// ✓ enum class: sicher, typsicher, kein Konflikt
enum class Farbe { Rot, Gruen, Blau };
enum class Ampel { Rot, Gelb, Gruen };
Farbe f = Farbe::Rot;              // Muss qualifiziert werden
// int x = f;                      // FEHLER: kein impliziter Cast
int x = static_cast<int>(f);       // Nur explizit möglich
```

### Underlying Type für Protokolle / Serialisierung

```cpp
enum class Status : uint8_t { OK = 0, Warnung = 1, Fehler = 2 };
// Größe garantiert 1 Byte → sicher für Netzwerkprotokolle
```

> **MISRA C++:2023** – Direkte Umsetzung von Regel **10.2.1** (Required: jede Enumeration muss einen expliziten Underlying Type deklarieren) und **10.2.2** (Advisory: keine unscoped `enum`, immer `enum class` / `enum struct`). Beide Regeln verhindern implizite Konvertierungen zu `int` und unspezifizierte Bitbreiten.

---

## 08 STL Container & Algorithmen

**Datei:** [`src/08_stl_containers/main.cpp`](src/08_stl_containers/main.cpp)

### ⚠️ Safety: `at()` vs. `operator[]`

```cpp
std::vector<int> v{1, 2, 3};

v[10];       // ❌ Undefined Behavior! Kein Check.
v.at(10);    // ✓ Wirft std::out_of_range → fangbar, kein UB

std::array<int, 3> arr{1, 2, 3};
arr.at(5);   // ✓ Wirft std::out_of_range
```

### ⚠️ Safety: `std::array` statt C-Array

```cpp
// ❌ C-Array: zerfällt zu Zeiger, keine Größe, kein bounds-check
void f(int arr[], int n) { arr[n+1]; }  // UB, kein Fehler

// ✓ std::array: hat .size(), .at(), funktioniert mit Algorithmen
void f(std::array<int, 5>& arr) { arr.at(3); }
```

### Wichtige Algorithmen

```cpp
std::erase_if(v, [](int x){ return x < 0; });  // C++20: sicheres Löschen

std::any_of(v.begin(), v.end(), pred);   // Gibt es ein Element das pred erfüllt?
std::all_of(v.begin(), v.end(), pred);   // Erfüllen ALLE Elemente pred?
std::none_of(v.begin(), v.end(), pred);  // Kein Element erfüllt pred?
```

> **MISRA C++:2023** – Regel **11.3.1** (Advisory: keine C-Array-Variablen, `std::array<T,N>` verwenden) und **7.11.2** (Required: Arrays dürfen bei Funktionsübergabe nicht zu Zeigern zerfallen → `std::array` / `std::span` übergeben). `.at()` statt `operator[]` für Bounds-Checking aligns with MISRA's no-UB-goal.

---

## 09 Type Traits

**Datei:** [`src/09_type_traits/main.cpp`](src/09_type_traits/main.cpp)

### Was sind Type Traits?

Template-Klassen die zur **Compile-Zeit** Typ-Informationen liefern — das Fundament von Concepts.

```cpp
std::is_integral_v<int>       // true
std::is_pointer_v<int*>       // true
std::is_same_v<int, long>     // false (!)
std::is_base_of_v<Basis, Kind>// true
```

### ⚠️ Safety: Typ-Transformationen

```cpp
// Intern genutzt von std::move und std::forward:
std::remove_reference_t<int&>   // → int
std::remove_reference_t<int&&>  // → int

// decay: Was passiert wenn Typ by-value übergeben wird
std::decay_t<int[5]>   // → int* (Array → Zeiger!)
std::decay_t<int&>     // → int  (Referenz → Wert)
```

### `std::conditional` – Typ zur Compile-Zeit wählen

```cpp
// Automatisch passenden Ganzzahl-Typ wählen:
template<bool BIG>
using ZahlTyp = std::conditional_t<BIG, long long, int>;

ZahlTyp<true>  grosse_zahl = 1'000'000'000'000LL;
ZahlTyp<false> kleine_zahl = 42;
```

> **MISRA C++:2023** – Type Traits ermöglichen präzise Typ-Kontrolle zur Compile-Zeit und sind die Grundlage von Concepts. `static_assert` macht MISRA-relevante Typ-Invarianten zur Compile-Zeit prüfbar. Unterstützen Regel **8.2.2** (kein C-Cast) durch exakte Typ-Informationen.

---

## 10 std::chrono

**Datei:** [`src/10_chrono/main.cpp`](src/10_chrono/main.cpp)

### Typsichere Zeitdauern

```cpp
using namespace std::chrono_literals;

auto timeout  = 5s;      // std::chrono::seconds
auto interval = 500ms;   // std::chrono::milliseconds

// ⚠️ Safety: Keine versehentliche Einheitenverwechslung!
// timeout + interval ist möglich (automatische Konvertierung)
// timeout == 5000 würde nicht kompilieren (int ≠ duration)
```

### ⚠️ Safety: `steady_clock` statt `system_clock` für Messungen

```cpp
// ❌ system_clock kann zurückgestellt werden (NTP, Sommer-/Winterzeit)
auto start = std::chrono::system_clock::now();

// ✓ steady_clock steigt immer monoton → zuverlässig für Laufzeit-Messung
auto start = std::chrono::steady_clock::now();
```

### RAII-Timer

```cpp
class Timer {
    std::chrono::steady_clock::time_point start_{std::chrono::steady_clock::now()};
public:
    ~Timer() {
        auto ms = std::chrono::duration<double, std::milli>(
            std::chrono::steady_clock::now() - start_).count();
        std::cout << ms << " ms\n";
    }
};
{ Timer t; std::sort(v.begin(), v.end()); }  // Automatisch gemessen
```

> **MISRA C++:2023** – `std::chrono::duration` macht Einheitenverwechslungen (Sekunden vs. Millisekunden) zur Compile-Zeit unmöglich — der Compiler erzwingt explizite Konvertierung (`duration_cast`). Das entspricht MISRA's Typ-Sicherheitsziel und verhindert eine ganze Klasse von Laufzeit-Bugs.

---

## 11 Concurrency

**Datei:** [`src/11_concurrency/main.cpp`](src/11_concurrency/main.cpp)

### ⚠️ Safety: Data Race — das häufigste Concurrency-Problem

```cpp
int zaehler = 0;
// Thread 1 und Thread 2 gleichzeitig:
zaehler++;  // Lesen + Addieren + Schreiben = 3 Schritte → Race Condition!
// Ergebnis: unvorhersehbar, kann 1 oder 2 oder Datenmüll sein
```

### ⚠️ Safety: `lock_guard` statt manuell lock/unlock

```cpp
// ❌ Manuell: nicht exception-safe!
mutex.lock();
tuewas();       // Wenn Exception → unlock wird nie aufgerufen → Deadlock!
mutex.unlock();

// ✓ RAII: unlock passiert automatisch, auch bei Exceptions
{
    std::lock_guard<std::mutex> guard(mutex);
    tuewas();
}  // ← guard wird zerstört → unlock sicher
```

### ⚠️ Safety: `std::atomic` für einzelne Variablen

```cpp
// ❌ Race Condition
int zaehler = 0;
zaehler++;  // Nicht atomar!

// ✓ Atomar: Hardware-Garantie, kein Mutex nötig
std::atomic<int> zaehler{0};
zaehler++;  // Unteilbar, kein Race
```

### ⚠️ Safety: `condition_variable` immer mit Predicate

```cpp
// ❌ Spurious Wakeup möglich!
cv.wait(lock);

// ✓ Predicate schützt vor Spurious Wakeups
cv.wait(lock, []{ return !queue.empty(); });
```

### ⚠️ Safety: `call_once` für thread-sichere Initialisierung

```cpp
// ✓ Garantiert genau einmal aufgerufen, egal wie viele Threads
std::once_flag flag;
std::call_once(flag, initialisiere);
```

> **MISRA C++:2023** – `std::lock_guard` RAII entspricht dem Automatismus von Regel **21.6.2** auf Mutex-Ebene (kein manuelles unlock). `std::atomic` vermeidet Data Races (MISRA Concurrency-Kapitel). Regel **18.4.1** (Required): thread-sichere Funktionen — insbesondere Destruktoren und Move-Operatoren — müssen `noexcept` sein.

---

## 12 C++23 Highlights

**Datei:** [`src/12_cpp23/main.cpp`](src/12_cpp23/main.cpp)  
**Benötigt:** `g++-12` oder neuer (`sudo apt install g++-12`)

### ⚠️ Safety: `std::expected<T, E>` – Fehler als Typ

Das wichtigste Safety-Feature von C++23: Fehlerbehandlung ohne Exceptions,  
ohne Magic Numbers, ohne vergessbare Fehlercodes.

```cpp
// ❌ Altes C++: Fehler unsichtbar im Rückgabewert
int parse(std::string_view s);  // -1 bei Fehler? Was bedeutet -1?

// ✓ C++23: Erfolg oder Fehler explizit im Typ
std::expected<int, ParseFehler> parse(std::string_view s) {
    if (s.empty()) return std::unexpected{ParseFehler::LeerEingabe};
    // ...
    return ergebnis;
}

auto r = parse("42");
if (r)      std::cout << "OK: "     << *r;
else        std::cout << "Fehler: " << r.error();
int wert  = r.value_or(0);          // Sicherer Fallback
```

### ⚠️ Safety: Monadic Optional – keine Null-Dereferenzierung

```cpp
// ❌ Klassisch: Verschachtelte if-Ketten → leicht vergessen
optional<User> user = hole_user(id);
if (user) {
    if (user->email) {
        auto dom = extrahiere_domain(*user->email);
        if (dom) std::cout << *dom;
    }
}

// ✓ C++23: Pipeline – schlägt fehl ohne UB, kein if nötig
auto domain = hole_user(id)
    .and_then([](const User& u) { return u.email; })
    .and_then(extrahiere_domain)
    .transform([](std::string d) { /* formatieren */ return d; })
    .value_or("unbekannt");
```

### ⚠️ Safety: `std::to_underlying` – Enum sicher konvertieren

```cpp
enum class Status : uint8_t { OK = 0, Fehler = 1 };

// ❌ static_cast: Fehler wenn Underlying Type geändert wird
uint8_t code = static_cast<uint8_t>(Status::OK);

// ✓ to_underlying: immer korrekt, typ-unabhängig
auto code = std::to_underlying(Status::OK);  // automatisch uint8_t
```

### `if consteval` – Compile-Zeit-Zweige

```cpp
constexpr double wurzel(double x) {
    if consteval {
        // Läuft NUR zur Compile-Zeit (Newton-Raphson)
        double r = x;
        for (int i = 0; i < 20; ++i) r = (r + x/r) / 2.0;
        return r;
    } else {
        return __builtin_sqrt(x);  // Hardware-optimiert zur Laufzeit
    }
}
```

### `static operator()` – Zustandslose Lambdas

```cpp
// Kein implizites 'this' → besser optimierbar, kein versehentlicher Capture
auto filter = [](int x) static { return x > 0; };
```

### Ranges-Erweiterungen

```cpp
// views::enumerate: Index mitliefern ohne manuellen Zähler
for (auto [i, val] : std::views::enumerate(v))
    std::println("  [{}] {}", i, val);

// views::zip: Zwei Ranges koppeln
for (auto [name, wert] : std::views::zip(namen, werte))
    std::println("  {}: {}", name, wert);

// views::chunk: In Blöcke aufteilen
for (auto block : v | std::views::chunk(3)) { ... }
```

> **MISRA C++:2023** – `std::expected` schließt die Lücke von Regel **18.1.1** (Required: Exception-Objekte nicht als Zeiger) und macht Fehlerbehandlung explizit ohne Exceptions. `std::to_underlying` ist der typsichere Weg für Enum-Konvertierungen (Kontext: Regeln **10.2.1** / **10.2.2**). Regel **18.4.1** (Required): `noexcept` bei `if consteval`-Funktionen und constexpr-Kontexten.  
> *Hinweis: MISRA C++:2023 basiert auf C++17 – C++23-Features werden in künftigen Revisionen abgedeckt.*

---

## Lernpfad

```
Stufe 1 – Einsteiger:     01 → 07 → 08 → 02
Stufe 2 – Fortgeschritten: 03 → 04 → 05 → 06
Stufe 3 – Experte:         09 → 10 → 11 → 12
```

---

## Safety-Zusammenfassung pro Thema

```
01 Basics          → [[nodiscard]], nullptr, constexpr
02 Smart Pointers  → RAII, unique_ptr, shared_ptr, weak_ptr (kein new/delete!)
03 Move-Semantik   → Regel der Fünf, noexcept
04 Templates       → Concepts (klare Fehlermeldungen), static_assert
05 Lambdas         → Explizite Captures (kein [&] in async!)
06 optional/variant→ Kein nullptr, kein union, kein Magic-Number
07 enum class      → Kein impliziter int-Cast, kein Namenskonflikt
08 STL             → .at() (bounds-check), std::array, erase_if
09 Type Traits     → Typ-Garantien zur Compile-Zeit
10 chrono          → steady_clock, typsichere Dauern
11 Concurrency     → lock_guard, atomic, call_once (kein Data Race)
12 C++23           → std::expected, monadic optional, to_underlying
```

---

## Lizenz

Dieses Projekt ist für Lernzwecke frei verwendbar.
