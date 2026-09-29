#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>   // ver nota: aporta los tipos enteros, y además otras TUs
                      // (Buttons.cpp usa pinMode/digitalRead, Game.cpp usa Serial)
                      // lo heredan de aquí. No quitar sin añadirlo allí.

// ========================================================
// Timer — reloj de 64 bits y cronómetros
//
// Los relojes de milisegundos de 32 bits del core envuelven cada
// ~49,7 días (el uptime del dispositivo) y tienen trampas sutiles:
// comparar contra un instante absoluto (`ahora >= plazo`) o usar
// `elapsed % period` sobre el instante absoluto producen un salto
// de fase una vez por giro. Este reloj usa esp_timer_get_time()
// del ESP32 (microsegundos desde el arranque, 64 bits): no envuelve
// en ~292.000 años y las restas (`ahora - inicio`) y los módulos
// son seguros sin pensar en el giro.
//
// Reglas de uso:
//   - nowMs() : instante actual; medir intervalos siempre con
//               `ahora - inicio`, nunca comparar contra un
//               instante absoluto.
//   - Stopwatch: plazos (TOTAL_MS, COUNTDOWN_MS, BLINK_HOLD...).
//               start() en el begin() de la ventana para que el
//               tiempo inactivo no cuente.
//   - Ticker  : pasos periódicos (ANIM_TICK, HOLD_REPEAT_TICK...);
//               consume() devuelve los pasos completos desde la
//               última consulta y conserva el residuo.
//
// Aquí solo hay declaraciones: las definiciones están en `Timer.cpp`
// (ver la nota de ese archivo sobre por qué nada se declara `inline`).
// ========================================================

// ========================================================
// Reloj global: microsegundos de arranque -> ms en 64 bits
// ========================================================

uint64_t nowMs();

// ========================================================
// Stopwatch — cronómetro: ¿pasaron X ms desde start()?
// ========================================================

class Stopwatch {
public:

  // Reinicia el cronómetro (llamar en begin() de la ventana)
  void start();

  // Milisegundos desde start()
  uint64_t elapsed() const;

  // ¿Ya pasaron `ms` desde start()?
  bool expired(uint32_t ms) const;

  // ¿Es la fase visible del período? (oculto el primer offPct%)
  bool blinkOn(uint32_t period, uint8_t offPct) const;

private:

  uint64_t _start = 0;
};

// ========================================================
// Ticker — acumulador de pasos periódicos de `period` ms
// ========================================================

class Ticker {
public:

  explicit Ticker(uint32_t period);

  // Reinicia el acumulador (llamar en begin() de la ventana)
  void start();

  // Pasos completos de `_period` ms desde la última consulta
  // (conserva el residuo para el siguiente consume())
  uint32_t consume();

private:

  uint32_t _period;
  uint64_t _last = 0;
  uint64_t _accum = 0;
};

#endif

// ====================================================================================
// Fin
// ====================================================================================