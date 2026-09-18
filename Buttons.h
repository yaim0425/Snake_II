#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

class Buttons {
public:

  // ========================================================
  // Configuración
  // ========================================================

  static constexpr uint8_t MAX_BUTTONS = 8;

  enum Button : uint8_t {
    MOVE_UP = 0,
    MOVE_RIGHT,
    MOVE_DOWN,
    MOVE_LEFT,

    ACTION_UP,
    ACTION_RIGHT,
    ACTION_DOWN,
    ACTION_LEFT
  };

  // ========================================================
  // Constructor
  // ========================================================

  Buttons(
    const int8_t* pins,
    uint32_t buttonDelay = 30
  );

  // ========================================================
  // Inicialización
  // ========================================================

  void begin();

  // ========================================================
  // Lectura
  // ========================================================

  void read();

  // ========================================================
  // Estado actual
  // ========================================================

  bool state(uint8_t index) const;
  bool pressed(uint8_t index) const;
  bool released(uint8_t index) const;

  bool moveUp() const;
  bool moveRight() const;
  bool moveDown() const;
  bool moveLeft() const;

  bool actionUp() const;
  bool actionRight() const;
  bool actionDown() const;
  bool actionLeft() const;

  // ========================================================
  // Pressed
  // ========================================================

  bool moveUpPressed() const;
  bool moveRightPressed() const;
  bool moveDownPressed() const;
  bool moveLeftPressed() const;

  bool actionUpPressed() const;
  bool actionRightPressed() const;
  bool actionDownPressed() const;
  bool actionLeftPressed() const;

  // ========================================================
  // Released
  // ========================================================

  bool moveUpReleased() const;
  bool moveRightReleased() const;
  bool moveDownReleased() const;
  bool moveLeftReleased() const;

  bool actionUpReleased() const;
  bool actionRightReleased() const;
  bool actionDownReleased() const;
  bool actionLeftReleased() const;

private:

  // ========================================================
  // Pines
  // ========================================================

  int8_t _pins[MAX_BUTTONS];

  // ========================================================
  // Estados
  // ========================================================

  // Estado confirmado actual
  bool _buttons[MAX_BUTTONS];

  // Estado confirmado anterior
  bool _lastButtons[MAX_BUTTONS];

  // Estado físico sin filtrar
  bool _rawButtons[MAX_BUTTONS];

  // Se acaba de presionar
  bool _pressed[MAX_BUTTONS];

  // Se acaba de soltar
  bool _released[MAX_BUTTONS];

  // ========================================================
  // Debounce
  // ========================================================

  uint32_t _buttonLast[MAX_BUTTONS];
  uint32_t _buttonDelay;
};

#endif