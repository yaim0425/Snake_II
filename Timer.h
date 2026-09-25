#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>
#include <esp_timer.h>

// ========================================================
// Timer — reloj de 64 bits y cronómetros
//
// millis() (32 bits) da la vuelta cada ~49,7 días y tiene
// trampas sutiles: comparar contra un instante absoluto
// (`millis() >= plazo`) o usar `elapsed % period` sobre el
// instante absoluto producen un salto de fase cada 49,7 días.
// Este reloj usa esp_timer_get_time() del ESP32 (microsegundos
// desde el arranque, 64 bits): no envuelve en ~292.000 años y
// las restas (`ahora - inicio`) y los módulos son seguros sin
// pensar en el giro.
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
// ========================================================

// ========================================================
// Reloj global: microsegundos de arranque -> ms en 64 bits
// ========================================================

inline uint64_t nowMs() {
  return (uint64_t)esp_timer_get_time() / 1000ULL;
}

// ========================================================
// Stopwatch — cronómetro: ¿pasaron X ms desde start()?
// ========================================================

class Stopwatch {
public:

  // Reinicia el cronómetro (llamar en begin() de la ventana)
  void start() { _start = nowMs(); }

  // Milisegundos desde start()
  uint64_t elapsed() const { return nowMs() - _start; }

  // ¿Ya pasaron `ms` desde start()?
  bool expired(uint32_t ms) const { return elapsed() >= ms; }

  // ¿Es la fase visible del período? (oculto el primer offPct%)
  bool blinkOn(uint32_t period, uint8_t offPct) const {
    return (elapsed() % period) >= (uint64_t)period * offPct / 100;
  }

private:

  uint64_t _start = 0;
};

// ========================================================
// Ticker — acumulador de pasos periódicos de `period` ms
// ========================================================

class Ticker {
public:

  explicit Ticker(uint32_t period) : _period(period) {}

  // Reinicia el acumulador (llamar en begin() de la ventana)
  void start() {
    _last = nowMs();
    _accum = 0;
  }

  // Pasos completos de `_period` ms desde la última consulta
  // (conserva el residuo para el siguiente consume())
  uint32_t consume() {
    uint64_t now = nowMs();
    _accum += now - _last;
    _last = now;
    uint32_t steps = (uint32_t)(_accum / _period);
    _accum %= _period;
    return steps;
  }

private:

  uint32_t _period;
  uint64_t _last = 0;
  uint64_t _accum = 0;
};

#endif

// ====================================================================================
// Fin
// ====================================================================================