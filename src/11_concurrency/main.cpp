// ╔══════════════════════════════════════════════════════════════════════════╗
// ║  THEMA 11 – Concurrency: Threads, Mutexe & std::atomic                  ║
// ║  Features: std::thread, std::mutex, std::lock_guard, std::unique_lock,  ║
// ║            std::atomic, std::async, std::future, std::promise,          ║
// ║            std::condition_variable, std::call_once, thread_local         ║
// ╚══════════════════════════════════════════════════════════════════════════╝
// MISRA C++:2023 – Schlüsselregeln in diesem Thema:
//   Rule 18.4.1 (Required) – Exception-unfriendly functions shall be noexcept
//                             → thread-sichere Destruktoren und Move-Ops
//   Rule 21.6.2 (Required) – Dynamic memory shall be managed automatically
//                             → lock_guard / unique_lock (RAII für Mutexe)
//   MISRA Concurrency-Kapitel: std::atomic für lock-free Zugriff, keine
//   direkten .lock()/.unlock()-Aufrufe (nur via RAII-Wrapper).
//
// ──── Warum Concurrency? ──────────────────────────────────────────────────
//
//   Moderne CPUs haben viele Kerne. Sequentieller Code nutzt nur einen.
//   Nebenläufigkeit (Concurrency) erlaubt echte Parallelarbeit:
//     - Thread 1: Daten laden
//     - Thread 2: Daten verarbeiten  } gleichzeitig!
//     - Thread 3: Ergebnisse schreiben
//
// ──── Das zentrale Problem: Data Races ───────────────────────────────────
//
//   Wenn zwei Threads gleichzeitig auf dieselbe Variable zugreifen und
//   mindestens einer schreibt → Data Race → Undefined Behavior!
//
//   int zaehler = 0;
//   Thread 1: zaehler++;   ← Lesen, Inkrementieren, Schreiben (3 Schritte!)
//   Thread 2: zaehler++;   ← gleichzeitig!
//   Ergebnis: kann 1 oder 2 sein, nicht garantiert!
//
//   Lösungen:
//     std::mutex    → Exklusiver Zugriff, nur ein Thread auf einmal
//     std::atomic   → Hardware-Garantie für atomare Operationen (schneller!)

#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <future>     // std::async, std::future, std::promise
#include <condition_variable>
#include <vector>
#include <numeric>
#include <chrono>
#include <string>
#include <queue>
#include <functional>

using namespace std::chrono_literals;

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 1: std::thread – Threads erstellen und verwalten
// ─────────────────────────────────────────────────────────────────────────────
//
// std::thread(callable, args...): Startet neuen Thread
//
// Wichtige Regeln:
//   .join():   Wartet bis Thread fertig ist (MUSS aufgerufen werden!)
//   .detach(): Thread läuft selbstständig weiter (kein join mehr möglich)
//
// ⚠️  Wenn thread-Objekt zerstört wird ohne join/detach → std::terminate!

void thread_arbeit(int id, int iterationen) {
    // Jeder Thread hat seinen eigenen Stack (lokale Variablen)
    int summe = 0;
    for (int i = 0; i < iterationen; ++i) {
        summe += i;
    }
    std::cout << "  Thread " << id << " fertig, summe=" << summe << "\n";
}

void demo_thread_grundlagen() {
    std::cout << "\n=== 1. std::thread Grundlagen ===\n";

    // hardware_concurrency: Anzahl logischer Kerne (empfohlen für Thread-Pool-Größe)
    std::cout << "  Logische CPU-Kerne: " << std::thread::hardware_concurrency() << "\n";

    // Thread mit freier Funktion
    std::thread t1(thread_arbeit, 1, 1000);
    std::thread t2(thread_arbeit, 2, 2000);

    // Thread mit Lambda
    std::thread t3([](int id) {
        std::cout << "  Thread " << id << " (Lambda), thread_id="
                  << std::this_thread::get_id() << "\n";
    }, 3);

    // join() MUSS aufgerufen werden bevor thread-Objekt zerstört wird!
    t1.join();
    t2.join();
    t3.join();
    std::cout << "  Alle Threads beendet\n";

    // Argumente werden by-value kopiert. Für Referenzen: std::ref / std::cref
    std::string nachricht = "Hallo aus Thread 4";
    std::thread t4([](const std::string& msg) {
        std::cout << "  " << msg << "\n";
    }, std::cref(nachricht));   // cref = const reference wrapper
    t4.join();
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 2: std::mutex – Exklusiver Zugriff auf gemeinsame Daten
// ─────────────────────────────────────────────────────────────────────────────
//
// mutex (Mutual Exclusion): Ein "Schloss" das nur ein Thread gleichzeitig halten kann.
//
// Nie direkt .lock() / .unlock() aufrufen! → Nicht exception-safe!
// Stattdessen RAII-Wrapper benutzen:
//
//   std::lock_guard<std::mutex>  → Sperrt im Konstruktor, entsperrt im Destruktor
//                                   Kein manuelles unlock möglich (einfachste Variante)
//
//   std::unique_lock<std::mutex> → Flexibler: kann auch manuell unlock/lock
//                                   Nötig für std::condition_variable
//
// ⚠️  MISRA C++:2023 Rule 18.4.1 (Required):
//     Destruktoren und Funktionen die keine Exceptions werfen können (wie
//     lock_guard::~lock_guard) müssen noexcept sein.
//     std::lock_guard garantiert noexcept-Destruktor → RAII-Unlock ist sicher.

// Unsicherer globaler Zähler (ohne Mutex)
int unsicher_zaehler = 0;

// Sicherer Zähler mit Mutex
std::mutex zaehler_mutex;
int sicher_zaehler = 0;

void inkrementiere_unsicher(int n) {
    for (int i = 0; i < n; ++i) {
        // ← Data Race! Drei CPU-Befehle: LOAD, ADD, STORE
        // Zwischen LOAD und STORE kann ein anderer Thread eingreifen!
        unsicher_zaehler++;
    }
}

void inkrementiere_sicher(int n) {
    for (int i = 0; i < n; ++i) {
        // lock_guard: sperrt bei Erzeugung, entsperrt automatisch am Scope-Ende
        std::lock_guard<std::mutex> lock(zaehler_mutex);
        sicher_zaehler++;
    }   // ← lock wird hier zerstört → mutex automatisch entsperrt
}

void demo_mutex() {
    std::cout << "\n=== 2. std::mutex & std::lock_guard ===\n";

    const int THREADS = 4, PRO_THREAD = 10000;

    // Unsicherer Test (Data Race – Ergebnis ist unvorhersehbar)
    unsicher_zaehler = 0;
    std::vector<std::thread> threads_unsicher;
    for (int i = 0; i < THREADS; ++i)
        threads_unsicher.emplace_back(inkrementiere_unsicher, PRO_THREAD);
    for (auto& t : threads_unsicher) t.join();
    std::cout << "  Ohne Mutex: " << unsicher_zaehler
              << " (erwartet: " << THREADS * PRO_THREAD << ")\n";
    std::cout << "  → Verlust durch Data Race!\n";

    // Sicherer Test (mit Mutex – Ergebnis immer korrekt)
    sicher_zaehler = 0;
    std::vector<std::thread> threads_sicher;
    for (int i = 0; i < THREADS; ++i)
        threads_sicher.emplace_back(inkrementiere_sicher, PRO_THREAD);
    for (auto& t : threads_sicher) t.join();
    std::cout << "  Mit Mutex:  " << sicher_zaehler
              << " (erwartet: " << THREADS * PRO_THREAD << ")\n";
    std::cout << "  → Immer korrekt!\n";

    // unique_lock: flexibler (needed for condition_variable, try_lock, etc.)
    std::mutex m;
    std::unique_lock<std::mutex> ul(m);
    // ul.unlock();  // Manuell entsperren möglich
    // ul.lock();    // Manuell sperren möglich
    // ul.try_lock_for(10ms); // Mit Timeout (braucht std::timed_mutex)
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 3: std::atomic – Atomare Operationen (schneller als mutex)
// ─────────────────────────────────────────────────────────────────────────────
//
// std::atomic<T> garantiert, dass Operationen unteilbar (atomar) sind.
// Intern genutzt: CPU-Instruktionen wie LOCK XADD, CMPXCHG etc.
//
// Wann atomic statt mutex?
//   → Für einzelne Variablen (Zähler, Flags, Zeiger)
//   → Wenn nur einfache Lese/Schreib-Operationen nötig sind
//   → Weniger Overhead als mutex (kein Betriebssystem-Aufruf)
//
// Wann mutex statt atomic?
//   → Für komplexe Invarianten (mehrere Variablen müssen konsistent sein)
//   → Für längere kritische Abschnitte

std::atomic<int> atomarer_zaehler{0};
std::atomic<bool> fertig_flag{false};

void inkrementiere_atomar(int n) {
    for (int i = 0; i < n; ++i) {
        atomarer_zaehler++;  // atomic: entspricht atomarer fetch_add(1)
    }
}

void demo_atomic() {
    std::cout << "\n=== 3. std::atomic ===\n";

    // Atomarer Zähler – kein Mutex nötig!
    atomarer_zaehler = 0;
    const int THREADS = 4, PRO_THREAD = 10000;
    std::vector<std::thread> threads;
    for (int i = 0; i < THREADS; ++i)
        threads.emplace_back(inkrementiere_atomar, PRO_THREAD);
    for (auto& t : threads) t.join();
    std::cout << "  Atomarer Zähler: " << atomarer_zaehler
              << " (erwartet: " << THREADS * PRO_THREAD << ")\n";

    // Wichtige atomic-Operationen:
    std::atomic<int> a{10};

    int alt = a.load();                    // Atomares Lesen
    a.store(20);                            // Atomares Schreiben
    int prev = a.exchange(30);              // Schreibe 30, gib alten Wert zurück
    std::cout << "  load: " << alt << ", exchange: " << prev
              << ", jetzt: " << a.load() << "\n";

    // fetch_add / fetch_sub: Atomare Arithmetik
    a.store(0);
    int before = a.fetch_add(5);  // Addiere 5, gib Wert VOR Addition zurück
    std::cout << "  fetch_add(5): vorher=" << before << ", jetzt=" << a.load() << "\n";

    // compare_exchange: Fundamental für Lock-Free-Algorithmen
    // "Wenn a == erwartet, setze a = neu; andernfalls lade aktuellen Wert in erwartet"
    int erwartet = 5;
    bool success = a.compare_exchange_strong(erwartet, 99);
    std::cout << "  CAS(5→99): " << (success ? "erfolgreich" : "fehlgeschlagen")
              << ", a=" << a.load() << "\n";

    // atomic<bool> als Thread-Koordinierungs-Flag
    std::atomic<bool> bereit{false};
    std::thread worker([&bereit]() {
        // Warte bis Haupt-Thread bereit ist (spin-wait – nicht für Produktion!)
        while (!bereit.load()) {}   // busy wait
        std::cout << "  Worker: wurde gestartet nach bereit=true\n";
    });
    std::this_thread::sleep_for(10ms);
    bereit.store(true);   // Signal an Worker
    worker.join();
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 4: std::async und std::future – Asynchrone Aufgaben
// ─────────────────────────────────────────────────────────────────────────────
//
// std::async(policy, callable, args...) startet eine asynchrone Aufgabe.
// Gibt std::future<T> zurück – ein "Versprechen auf einen zukünftigen Wert".
//
// .get()    → Blockiert bis Ergebnis vorliegt, gibt Wert zurück (oder wirft Exception)
// .wait()   → Wartet bis fertig (kein Rückgabewert)
// .wait_for(duration) → Wartet höchstens diese Dauer
//
// launch-Policies:
//   std::launch::async    → Immer in neuem Thread ausführen
//   std::launch::deferred → Lazy: Erst bei .get() im aufrufenden Thread
//   (ohne Policy)         → Implementierungsentscheidung (meist async auf echten Systemen)

long long schwere_berechnung(int n) {
    std::cout << "  [Thread " << std::this_thread::get_id()
              << "] starte Berechnung...\n";
    long long summe = 0;
    for (int i = 0; i <= n; ++i) summe += i;
    return summe;
}

void demo_async_future() {
    std::cout << "\n=== 4. std::async & std::future ===\n";

    // Parallel: Zwei Berechnungen gleichzeitig starten
    std::cout << "  Starte zwei async-Aufgaben parallel:\n";
    auto future1 = std::async(std::launch::async, schwere_berechnung, 1'000'000);
    auto future2 = std::async(std::launch::async, schwere_berechnung, 2'000'000);

    // Beide laufen im Hintergrund während wir hier weitermachen!
    std::cout << "  Haupt-Thread: mache andere Arbeit...\n";

    // .get() blockiert bis Ergebnis da
    long long r1 = future1.get();   // Wartet auf Thread 1
    long long r2 = future2.get();   // Wartet auf Thread 2

    std::cout << "  Ergebnis 1: " << r1 << "\n";
    std::cout << "  Ergebnis 2: " << r2 << "\n";

    // wait_for: Timeout beim Warten
    auto future3 = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(100ms);
        return 42;
    });
    auto status = future3.wait_for(10ms);   // Warte maximal 10ms
    if (status == std::future_status::timeout) {
        std::cout << "  Timeout – noch nicht fertig\n";
    }
    std::cout << "  Schließlich: " << future3.get() << "\n";

    // Exception-Weitergabe: Exceptions im Thread werden bei .get() geworfen
    auto exc_future = std::async(std::launch::async, [] () -> int {
        throw std::runtime_error("Fehler im Thread!");
    });
    try {
        exc_future.get();
    } catch (const std::exception& e) {
        std::cout << "  Exception aus Thread: " << e.what() << "\n";
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 5: std::condition_variable – Producer-Consumer Pattern
// ─────────────────────────────────────────────────────────────────────────────
//
// condition_variable: Erlaubt Threads effizient auf eine Bedingung zu warten.
// Statt Busy-Waiting ("spin") schläft der Thread und wird geweckt wenn nötig.
//
// Immer mit unique_lock<mutex> benutzen!
// Immer mit Predicate (Lambda) .wait() aufrufen → Schutz vor "spurious wakeups"
//
// Typisches Muster (Producer-Consumer):
//   Produzent: Daten in Queue, notify_one/all
//   Konsument: wait(lock, [&]{ return !queue.empty(); }), Daten verarbeiten

std::queue<int> aufgaben_queue;
std::mutex queue_mutex;
std::condition_variable queue_cv;
bool alle_aufgaben_fertig = false;

void produzent(int anzahl) {
    for (int i = 1; i <= anzahl; ++i) {
        std::this_thread::sleep_for(5ms);  // Simuliert Arbeit
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            aufgaben_queue.push(i);
            std::cout << "  [Prod] Aufgabe " << i << " erstellt\n";
        }
        queue_cv.notify_one();  // Weckt einen wartenden Konsumenten
    }
    {
        std::lock_guard<std::mutex> lock(queue_mutex);
        alle_aufgaben_fertig = true;
    }
    queue_cv.notify_all();  // Weckt alle Konsumenten (damit sie terminieren)
}

void konsument(int id) {
    while (true) {
        int aufgabe;
        {
            // unique_lock nötig für condition_variable!
            std::unique_lock<std::mutex> lock(queue_mutex);

            // wait(): Entsperrt mutex, schläft, wacht auf wenn notify kommt,
            // prüft Bedingung (Predicate), sperrt mutex wieder wenn true
            queue_cv.wait(lock, [] {
                return !aufgaben_queue.empty() || alle_aufgaben_fertig;
            });

            if (aufgaben_queue.empty()) return;  // Keine Aufgaben mehr → fertig

            aufgabe = aufgaben_queue.front();
            aufgaben_queue.pop();
        }   // lock wird hier freigegeben

        // Verarbeitung außerhalb des locks (wichtig für Performance!)
        std::cout << "  [Kons" << id << "] verarbeite Aufgabe " << aufgabe << "\n";
        std::this_thread::sleep_for(15ms);  // Simuliert Verarbeitung
    }
}

void demo_condition_variable() {
    std::cout << "\n=== 5. condition_variable (Producer-Consumer) ===\n";

    alle_aufgaben_fertig = false;
    while (!aufgaben_queue.empty()) aufgaben_queue.pop();  // Reset

    std::thread prod(produzent, 5);
    std::thread cons1(konsument, 1);
    std::thread cons2(konsument, 2);

    prod.join();
    cons1.join();
    cons2.join();
    std::cout << "  Alle Aufgaben verarbeitet\n";
}

// ─────────────────────────────────────────────────────────────────────────────
// KONZEPT 6: std::call_once und thread_local
// ─────────────────────────────────────────────────────────────────────────────

std::once_flag init_flag;
std::string shared_resource;

void initialisiere() {
    // Wird garantiert genau EINMAL aufgerufen – egal wie viele Threads!
    shared_resource = "Initialisiert!";
    std::cout << "  [call_once] Initialisierung (thread_id="
              << std::this_thread::get_id() << ")\n";
}

// thread_local: Jeder Thread hat seine eigene Instanz dieser Variable
thread_local int thread_zaehler = 0;

void demo_call_once_thread_local() {
    std::cout << "\n=== 6. call_once & thread_local ===\n";

    // call_once: Für Singleton-Pattern, einmalige Ressource-Initialisierung
    std::cout << "  call_once – 4 Threads versuchen zu initialisieren:\n";
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([] {
            std::call_once(init_flag, initialisiere);   // Nur einer kommt durch!
        });
    }
    for (auto& t : threads) t.join();
    std::cout << "  shared_resource: \"" << shared_resource << "\"\n";

    // thread_local: Jeder Thread hat seine eigene Kopie
    std::cout << "\n  thread_local – jeder Thread hat eigenen Zähler:\n";
    std::vector<std::thread> tl_threads;
    for (int i = 0; i < 3; ++i) {
        tl_threads.emplace_back([i] {
            thread_zaehler = (i + 1) * 10;  // Jeder setzt seinen EIGENEN Zähler
            std::this_thread::sleep_for(5ms);
            // thread_zaehler von anderen Threads wird nicht beeinflusst!
            std::cout << "  Thread " << i << ": thread_zaehler = "
                      << thread_zaehler << "\n";
        });
    }
    for (auto& t : tl_threads) t.join();
}

// ─────────────────────────────────────────────────────────────────────────────
// MAIN
// ─────────────────────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔═════════════════════════════════════════════════╗\n"
              << "║  THEMA 11 – Concurrency: Threads & std::atomic  ║\n"
              << "╚═════════════════════════════════════════════════╝\n";

    demo_thread_grundlagen();
    demo_mutex();
    demo_atomic();
    demo_async_future();
    demo_condition_variable();
    demo_call_once_thread_local();

    return 0;
}
