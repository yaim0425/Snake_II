#include "Timer.h"

#include <esp_timer.h>

// ========================================================
// Implementación de Timer (reloj de 64 bits y cronómetros)
//
// Las definiciones viven aquí, no en el header: `Stopwatch` y `Ticker`
// los usan casi todas las ventanas, y como métodos `inline` dentro de la
// clase el compilador repetía el mismo código en cada unidad de
// traducción. Con las definiciones en un único .cpp hay una sola copia y
// el enlace se encarga (con LTO la puede volver a inlinear donde convenga).
//
// Ojo: por eso los métodos NO se declaran `inline` en el header. Una
// función declarada `inline` y definida en otro .cpp no genera símbolo, y
// cualquier .cpp que la use daría "undefined reference" al enlazar.
// ========================================================

// ========================================================
// Reloj global: microsegundos de arranque -> ms en 64 bits
// ========================================================

uint64_t nowMs() {
  return (uint64_t)esp_timer_get_time() / 1000ULL;
}

// ========================================================
// Stopwatch — cronómetro: ¿pasaron X ms desde start()?
// ========================================================

// Reinicia el cronómetro (llamar en begin() de la ventana)
void Stopwatch::start() {
  _start = nowMs();
}

// Milisegundos desde start()
uint64_t Stopwatch::elapsed() const {
  return nowMs() - _start;
}

// ¿Ya pasaron `ms` desde start()?
bool Stopwatch::expired(uint32_t ms) const {
  return elapsed() >= ms;
}

// ¿Es la fase visible del período? (oculto el primer offPct%)
bool Stopwatch::blinkOn(uint32_t period, uint8_t offPct) const {
  return (elapsed() % period) >= (uint64_t)period * offPct / 100;
}

// ========================================================
// Ticker — acumulador de pasos periódicos de `period` ms
// ========================================================

Ticker::Ticker(uint32_t period) : _period(period) {}

// Reinicia el acumulador (llamar en begin() de la ventana)
void Ticker::start() {
  _last = nowMs();
  _accum = 0;
}

// Pasos completos de `_period` ms desde la última consulta
// (conserva el residuo para el siguiente consume())
uint32_t Ticker::consume() {
  uint64_t now = nowMs();
  _accum += now - _last;
  _last = now;
  uint32_t steps = (uint32_t)(_accum / _period);
  _accum %= _period;
  return steps;
}

// ====================================================================================
// Fin
// ====================================================================================
