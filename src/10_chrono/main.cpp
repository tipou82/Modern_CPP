// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 10 – std::chrono                                                 ║
// ║  Features: duration, duration_cast, Clocks, time_point,                 ║
// ║            Laufzeit messen, Literal-Syntax, sleep                       ║
// ╚══════════════════════════════════════════════════════════════════════════╝
//
// ──── Überblick: Drei Hauptbausteine ──────────────────────────────────────
//
//   1. duration<Rep, Period>
//      Eine Zeitdauer: "wie viele Einheiten"
//      Beispiel: 5 Sekunden = duration<int, std::ratio<1>> mit Wert 5
//      Vordefiniert: nanoseconds, microseconds, milliseconds, seconds, minutes, hours
//
//   2. clock (Uhren)
//      Liefert den aktuellen Zeitpunkt. Drei wichtige Uhren:
//        system_clock:            Wanduhrzeit (kann zurückgestellt werden!)
//        steady_clock:            Monoton steigend, ideal zum Messen
//        high_resolution_clock:   Feinste verfügbare Auflösung
//
//   3. time_point<Clock, Duration>
//      Ein spezifischer Zeitpunkt. Differenz zweier time_points = duration.
//
// ──── Zusammenspiel ───────────────────────────────────────────────────────
//
//   auto start = steady_clock::now();   // time_point
//   // ... Code ...
//   auto ende  = steady_clock::now();   // time_point
//   auto dauer = ende - start;          // duration (in nanoseconds intern)
//   auto ms    = duration_cast<milliseconds>(dauer);  // Umrechnen

#include <iostream>
#include <chrono>
#include <thread>       // für std::this_thread::sleep_for
#include <vector>
#include <algorithm>
#include <numeric>
#include <string>
#include <iomanip>

// Bringt Literal-Suffixe in Scope:
// 1s, 500ms, 100us, 1min, 2h usw.
using namespace std::chrono_literals;

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: duration – Zeitdauern rechnen
// ─────────────────────────────────────────────────────────────────────────────

void demo_duration() {
    std::cout << "\n=== 1. duration – Zeitdauern ===\n";

    // Vordefinierte duration-Typen (alle in <chrono>)
    std::chrono::hours       h(2);       // 2 Stunden
    std::chrono::minutes     m(90);      // 90 Minuten
    std::chrono::seconds     s(3600);    // 3600 Sekunden
    std::chrono::milliseconds ms(1500);  // 1500 Millisekunden
    std::chrono::microseconds us(1000);  // 1000 Mikrosekunden
    std::chrono::nanoseconds  ns(500);   // 500 Nanosekunden

    // Mit Literal-Syntax (eleganter):
    auto dauer1 = 2h;          // 2 Stunden
    auto dauer2 = 30min;       // 30 Minuten
    auto dauer3 = 500ms;       // 500 Millisekunden
    auto dauer4 = 100us;       // 100 Mikrosekunden

    // .count() gibt den rohen Zahlenwert zurück
    std::cout << "  2h in Stunden:           " << dauer1.count() << " h\n";
    std::cout << "  30min in Minuten:        " << dauer2.count() << " min\n";
    std::cout << "  500ms in Millisekunden:  " << dauer3.count() << " ms\n";

    // duration_cast: Umrechnen zwischen Einheiten
    // Verlustbehaftet (kein impliziter Cast von grob nach fein)!
    auto stunden_als_sek = std::chrono::duration_cast<std::chrono::seconds>(2h);
    std::cout << "\n  2h → Sekunden:  " << stunden_als_sek.count() << " s\n";

    auto ms_als_sek = std::chrono::duration_cast<std::chrono::seconds>(1500ms);
    std::cout << "  1500ms → Sekunden: " << ms_als_sek.count() << " s (abgeschnitten!)\n";

    // Arithmetik mit durations:
    auto gesamt = 1h + 30min + 45s;
    auto als_sek = std::chrono::duration_cast<std::chrono::seconds>(gesamt);
    std::cout << "\n  1h + 30min + 45s = " << als_sek.count() << " Sekunden\n";

    // Vergleiche
    std::cout << "  500ms < 1s: " << (500ms < 1s) << "\n";
    std::cout << "  1h  > 30min: " << (1h > 30min) << "\n";

    // Gleitkomma-Durations: für nicht-ganzzahlige Werte
    std::chrono::duration<double, std::ratio<1>> halbe_sek(0.5);
    std::cout << "\n  0.5 Sekunden = "
              << std::chrono::duration_cast<std::chrono::milliseconds>(halbe_sek).count()
              << " ms\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: Clocks und time_point
// ─────────────────────────────────────────────────────────────────────────────

void demo_clocks() {
    std::cout << "\n=== 2. Clocks und time_point ===\n";

    // steady_clock: Ideal zum Messen – steigt monoton, nie rückwärts
    std::cout << "  steady_clock::now() aufgerufen (time_point="
              << std::chrono::steady_clock::now().time_since_epoch().count() << " ns)\n";

    // system_clock: Wanduhrzeit – kann verstellt werden (NTP etc.)
    auto wand_uhr = std::chrono::system_clock::now();

    // time_t: Konvertierung für lesbare Ausgabe (system_clock → C-Zeit)
    auto time_t_wert = std::chrono::system_clock::to_time_t(wand_uhr);
    std::cout << "  Aktuelle Systemzeit: " << std::ctime(&time_t_wert);

    // Differenz zweier time_points = duration
    auto t1 = std::chrono::steady_clock::now();
    // Simuliere etwas Arbeit (sleep ist präziser als busy-loop):
    std::this_thread::sleep_for(1ms);
    auto t2 = std::chrono::steady_clock::now();

    auto diff = t2 - t1;   // duration<long long, nano>
    auto diff_us = std::chrono::duration_cast<std::chrono::microseconds>(diff);
    std::cout << "  1.000.000 Additionen dauerten: " << diff_us.count() << " µs\n";

    // Epochen-Zeitstempel (Unix timestamp)
    auto epoch_diff = wand_uhr.time_since_epoch();
    auto sek_seit_epoch = std::chrono::duration_cast<std::chrono::seconds>(epoch_diff);
    std::cout << "  Sekunden seit Unix-Epoche (1970): " << sek_seit_epoch.count() << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: Laufzeit messen – RAII-Timer
// ─────────────────────────────────────────────────────────────────────────────
//
// Klassisches Muster: Start-Zeit merken, am Ende Differenz berechnen.
// RAII-Pattern: Timer startet im Konstruktor, misst im Destruktor automatisch.
// → Laufzeit wird auch bei Exceptions korrekt gemessen!

class Timer {
public:
    explicit Timer(std::string label)
        : label_{std::move(label)}
        , start_{std::chrono::steady_clock::now()}
    {}

    // Manuell ablesen (ohne zu stoppen)
    [[nodiscard]] double millisekunden() const {
        auto jetzt = std::chrono::steady_clock::now();
        return std::chrono::duration<double, std::milli>(jetzt - start_).count();
    }

    // Destruktor: automatische Messung beim Verlassen des Scopes
    ~Timer() {
        auto ende = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::milli> dauer = ende - start_;
        std::cout << "  [TIMER] " << label_ << ": "
                  << std::fixed << std::setprecision(3)
                  << dauer.count() << " ms\n";
    }

private:
    std::string label_;
    std::chrono::steady_clock::time_point start_;
};

void demo_timer() {
    std::cout << "\n=== 3. RAII-Timer ===\n";

    // Blockweise messen:
    {
        Timer t("std::sort von 100.000 Elementen");
        std::vector<int> v(100'000);
        std::iota(v.rbegin(), v.rend(), 1);   // Rückwärts befüllen (schlechter Fall)
        std::sort(v.begin(), v.end());
    }   // Timer-Destruktor gibt Ergebnis aus

    {
        Timer t("std::accumulate von 100.000 Elementen");
        std::vector<int> v(100'000);
        std::iota(v.begin(), v.end(), 1);
        volatile auto sum = std::accumulate(v.begin(), v.end(), 0LL);
        (void)sum;
    }

    // Manuelle Abfrage während der Messung:
    Timer lauf("Gesamtlaufzeit");
    for (int i = 0; i < 3; ++i) {
        std::this_thread::sleep_for(10ms);
        std::cout << "  Nach Schritt " << i+1 << ": "
                  << std::fixed << std::setprecision(1)
                  << lauf.millisekunden() << " ms\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: sleep_for und sleep_until
// ─────────────────────────────────────────────────────────────────────────────
//
// std::this_thread::sleep_for(duration):  Wartet mindestens diese Dauer
// std::this_thread::sleep_until(time_point): Wartet bis zu diesem Zeitpunkt

void demo_sleep() {
    std::cout << "\n=== 4. sleep_for ===\n";

    std::cout << "  Warte 50ms...\n";
    Timer t("sleep_for(50ms)");
    std::this_thread::sleep_for(50ms);
    // Timer-Destruktor misst die tatsächliche Dauer

    // sleep_until: Wartet bis ein bestimmter Zeitpunkt
    auto ziel = std::chrono::steady_clock::now() + 30ms;
    std::cout << "  Warte bis t+30ms (sleep_until)...\n";
    Timer t2("sleep_until(now + 30ms)");
    std::this_thread::sleep_until(ziel);
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔══════════════════════════════════════════╗\n"
              << "║  THEMA 10 – std::chrono                  ║\n"
              << "╚══════════════════════════════════════════╝\n";
    std::cout << std::boolalpha;

    demo_duration();
    demo_clocks();
    demo_timer();
    demo_sleep();

    return 0;
}
