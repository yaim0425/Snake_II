#include "Buttons.h"
#include "Timer.h"

// ========================================================
// Constructor
// ========================================================

Buttons::Buttons(
  const int8_t* pins,
  uint32_t buttonDelay)
  : _buttonDelay(buttonDelay) {

  _rawButtons = 0;
  _buttons = 0;
  _pressed = 0;
  _released = 0;

  for (uint8_t i = 0; i < MAX_BUTTONS; i++) {

    _pins[i] = pins[i];
    _buttonLast[i] = 0;
  }
}

// ========================================================
// Inicialización
// ========================================================

void Buttons::begin() {

  for (uint8_t i = 0; i < MAX_BUTTONS; i++) {

    pinMode(
      _pins[i],
      INPUT_PULLDOWN);

    _buttonLast[i] = nowMs();
  }

  // Leer estado inicial (bits agrupados en un byte)
  _buttons = 0;

  for (uint8_t i = 0; i < MAX_BUTTONS; i++) {

    if (digitalRead(_pins[i]) == HIGH)
      _buttons |= (uint8_t)(1 << i);
  }

  _rawButtons = _buttons;
}

// ========================================================
// Leer botones
// ========================================================

void Buttons::read() {

  uint64_t now = nowMs();

  // Limpiar eventos
  _pressed = 0;
  _released = 0;

  // --------------------------------------------
  // Leer los 8 botones agrupados en un byte
  // (bit i = botón i, HIGH = presionado)
  // --------------------------------------------

  uint8_t raw = 0;

  for (uint8_t i = 0; i < MAX_BUTTONS; i++) {

    if (digitalRead(_pins[i]) == HIGH)
      raw |= (uint8_t)(1 << i);
  }

  // --------------------------------------------
  // Bits que cambiaron físicamente (arrancan su
  // propio temporizador de debounce)
  // --------------------------------------------

  uint8_t changed = raw ^ _rawButtons;
  _rawButtons = raw;

  // --------------------------------------------
  // Debounce por botón sobre el byte
  // --------------------------------------------

  for (uint8_t i = 0; i < MAX_BUTTONS; i++) {

    uint8_t bit = (uint8_t)(1 << i);

    if (changed & bit) _buttonLast[i] = now;

    if ((now - _buttonLast[i]) < _buttonDelay) continue;

    bool state = (raw & bit) != 0;   // estado físico deseado
    bool cur   = (_buttons & bit) != 0;  // estado confirmado anterior

    if (cur == state) continue;      // estable, sin cambio

    if (state) _buttons |= bit;
    else       _buttons &= (uint8_t)~bit;

    // Pressed (0 -> 1) / Released (1 -> 0)
    if (state && !cur) _pressed |= bit;
    else               _released |= bit;
  }

  // --------------------------------------------
  // Anticonflicto MOVE: si hay 2 o más botones
  // MOVE confirmados a la vez, se anula la
  // activación de todos (estado y eventos)
  // --------------------------------------------

  const uint8_t moves = _buttons & MOVE_MASK;

  if (moves & (moves - 1)) {

    _buttons  &= (uint8_t)~MOVE_MASK;
    _pressed  &= (uint8_t)~MOVE_MASK;
    _released &= (uint8_t)~MOVE_MASK;
  }
}

// ========================================================
// Estado actual
// ========================================================

bool Buttons::state(uint8_t index) const {
  return isSet(_buttons, index);
}

bool Buttons::pressed(uint8_t index) const {
  return isSet(_pressed, index);
}

bool Buttons::released(uint8_t index) const {
  return isSet(_released, index);
}

// ====================================================================================
// Fin
// ====================================================================================