#include "Buzzer.h"

// ========================================================
// Constructor
// ========================================================

Buzzer::Buzzer(uint8_t pin)
  : _pin(pin),
    _attached(false),
    _busy(false),
    _startMs(0),
    _durationMs(0) {}

// ========================================================
// Inicialización (adjunta el canal LEDC al pin)
// ========================================================

bool Buzzer::begin() {
  // Arduino-ESP32 Core 3.x: ledcAttach(pin, frecuencia, resolución)
  _attached = ledcAttach(_pin, 2000, 10);
  stop();
  return _attached;
}

// ========================================================
// Actualizar: apaga el tono cuando termina su duración
// ========================================================

void Buzzer::update() {
  if (!_busy) return;
  if (_durationMs == 0) return;

  if ((uint32_t)(millis() - _startMs) < _durationMs) return;

  stop();
}

// ========================================================
// Sonido
// ========================================================

void Buzzer::tone(uint16_t frequency, uint32_t durationMs) {
  if (_attached) {
    if (frequency == 0) ledcWriteTone(_pin, 0);
    else                ledcWriteTone(_pin, frequency);
  }

  _startMs    = millis();
  _durationMs = durationMs;
  _busy       = true;
}

void Buzzer::stop() {
  if (_attached) ledcWriteTone(_pin, 0);

  _startMs    = 0;
  _durationMs = 0;
  _busy       = false;
}

// ========================================================
// Estado
// ========================================================

bool Buzzer::busy() const {
  return _busy;
}

bool Buzzer::attached() const {
  return _attached;
}

// ====================================================================================
// Fin
// ====================================================================================