#include "Buttons.h"

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

    _buttonLast[i] = millis();
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

  uint32_t now = millis();

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

bool Buttons::moveUp() const {
  return isSet(_buttons, MOVE_UP);
}

bool Buttons::moveRight() const {
  return isSet(_buttons, MOVE_RIGHT);
}

bool Buttons::moveDown() const {
  return isSet(_buttons, MOVE_DOWN);
}

bool Buttons::moveLeft() const {
  return isSet(_buttons, MOVE_LEFT);
}

bool Buttons::actionUp() const {
  return isSet(_buttons, ACTION_UP);
}

bool Buttons::actionRight() const {
  return isSet(_buttons, ACTION_RIGHT);
}

bool Buttons::actionDown() const {
  return isSet(_buttons, ACTION_DOWN);
}

bool Buttons::actionLeft() const {
  return isSet(_buttons, ACTION_LEFT);
}

// ========================================================
// Pressed
// ========================================================

bool Buttons::moveUpPressed() const {
  return isSet(_pressed, MOVE_UP);
}

bool Buttons::moveRightPressed() const {
  return isSet(_pressed, MOVE_RIGHT);
}

bool Buttons::moveDownPressed() const {
  return isSet(_pressed, MOVE_DOWN);
}

bool Buttons::moveLeftPressed() const {
  return isSet(_pressed, MOVE_LEFT);
}

bool Buttons::actionUpPressed() const {
  return isSet(_pressed, ACTION_UP);
}

bool Buttons::actionRightPressed() const {
  return isSet(_pressed, ACTION_RIGHT);
}

bool Buttons::actionDownPressed() const {
  return isSet(_pressed, ACTION_DOWN);
}

bool Buttons::actionLeftPressed() const {
  return isSet(_pressed, ACTION_LEFT);
}

// ========================================================
// Released
// ========================================================

bool Buttons::moveUpReleased() const {
  return isSet(_released, MOVE_UP);
}

bool Buttons::moveRightReleased() const {
  return isSet(_released, MOVE_RIGHT);
}

bool Buttons::moveDownReleased() const {
  return isSet(_released, MOVE_DOWN);
}

bool Buttons::moveLeftReleased() const {
  return isSet(_released, MOVE_LEFT);
}

bool Buttons::actionUpReleased() const {
  return isSet(_released, ACTION_UP);
}

bool Buttons::actionRightReleased() const {
  return isSet(_released, ACTION_RIGHT);
}

bool Buttons::actionDownReleased() const {
  return isSet(_released, ACTION_DOWN);
}

bool Buttons::actionLeftReleased() const {
  return isSet(_released, ACTION_LEFT);
}