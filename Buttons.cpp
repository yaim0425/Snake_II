#include "Buttons.h"

// ========================================================
// Constructor
// ========================================================

Buttons::Buttons(
  const int8_t* pins,
  uint32_t buttonDelay)
  : _buttonDelay(buttonDelay) {

  _rawButtons = 0;

  for (uint8_t i = 0; i < MAX_BUTTONS; i++) {

    _pins[i] = pins[i];

    _buttons[i] = false;
    _lastButtons[i] = false;

    _pressed[i] = false;
    _released[i] = false;

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

    _buttonLast[i] = millis();
  }

  // Leer estado inicial (bits agrupados en un byte)
  for (uint8_t i = 0; i < MAX_BUTTONS; i++) {

    bool state =
      digitalRead(_pins[i]) == HIGH;

    _buttons[i] = state;
    _lastButtons[i] = state;

    if (state) _rawButtons |= (uint8_t)(1 << i);
    else       _rawButtons &= (uint8_t)~(1 << i);
  }
}

// ========================================================
// Leer botones
// ========================================================

void Buttons::read() {

  uint32_t now = millis();

  // Limpiar eventos
  for (uint8_t i = 0; i < MAX_BUTTONS; i++) {

    _pressed[i] = false;
    _released[i] = false;
  }

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
  // Debounce por botón sobre el byte
  // --------------------------------------------

  for (uint8_t i = 0; i < MAX_BUTTONS; i++) {

    bool state = (raw >> i) & 1;

    // --------------------------------------------
    // Cambio físico detectado
    // --------------------------------------------

    if (state != ((_rawButtons >> i) & 1)) {

      if (state) _rawButtons |= (uint8_t)(1 << i);
      else       _rawButtons &= (uint8_t)~(1 << i);

      _buttonLast[i] = now;
    }

    // --------------------------------------------
    // Debounce
    // --------------------------------------------

    if ((now - _buttonLast[i]) >= _buttonDelay) {

      if (_buttons[i] != state) {

        // Guardar estado anterior
        _lastButtons[i] = _buttons[i];

        // Aceptar nuevo estado
        _buttons[i] = state;

        // ----------------------------------------
        // Pressed
        // ----------------------------------------

        if (
          _buttons[i] && !_lastButtons[i]) {
          _pressed[i] = true;
        }

        // ----------------------------------------
        // Released
        // ----------------------------------------

        if (
          !_buttons[i] && _lastButtons[i]) {
          _released[i] = true;
        }
      }
    }
  }
}

// ========================================================
// Estado actual
// ========================================================

bool Buttons::state(uint8_t index) const {
  return _buttons[index];
}

bool Buttons::pressed(uint8_t index) const {
  return _pressed[index];
}

bool Buttons::released(uint8_t index) const {
  return _released[index];
}

bool Buttons::moveUp() const {
  return _buttons[MOVE_UP];
}

bool Buttons::moveRight() const {
  return _buttons[MOVE_RIGHT];
}

bool Buttons::moveDown() const {
  return _buttons[MOVE_DOWN];
}

bool Buttons::moveLeft() const {
  return _buttons[MOVE_LEFT];
}

bool Buttons::actionUp() const {
  return _buttons[ACTION_UP];
}

bool Buttons::actionRight() const {
  return _buttons[ACTION_RIGHT];
}

bool Buttons::actionDown() const {
  return _buttons[ACTION_DOWN];
}

bool Buttons::actionLeft() const {
  return _buttons[ACTION_LEFT];
}

// ========================================================
// Pressed
// ========================================================

bool Buttons::moveUpPressed() const {
  return _pressed[MOVE_UP];
}

bool Buttons::moveRightPressed() const {
  return _pressed[MOVE_RIGHT];
}

bool Buttons::moveDownPressed() const {
  return _pressed[MOVE_DOWN];
}

bool Buttons::moveLeftPressed() const {
  return _pressed[MOVE_LEFT];
}

bool Buttons::actionUpPressed() const {
  return _pressed[ACTION_UP];
}

bool Buttons::actionRightPressed() const {
  return _pressed[ACTION_RIGHT];
}

bool Buttons::actionDownPressed() const {
  return _pressed[ACTION_DOWN];
}

bool Buttons::actionLeftPressed() const {
  return _pressed[ACTION_LEFT];
}

// ========================================================
// Released
// ========================================================

bool Buttons::moveUpReleased() const {
  return _released[MOVE_UP];
}

bool Buttons::moveRightReleased() const {
  return _released[MOVE_RIGHT];
}

bool Buttons::moveDownReleased() const {
  return _released[MOVE_DOWN];
}

bool Buttons::moveLeftReleased() const {
  return _released[MOVE_LEFT];
}

bool Buttons::actionUpReleased() const {
  return _released[ACTION_UP];
}

bool Buttons::actionRightReleased() const {
  return _released[ACTION_RIGHT];
}

bool Buttons::actionDownReleased() const {
  return _released[ACTION_DOWN];
}

bool Buttons::actionLeftReleased() const {
  return _released[ACTION_LEFT];
}