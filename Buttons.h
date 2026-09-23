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
  // Estados (agrupados en bytes: 1 bit por botón, botón 0-7)
  // ========================================================

  // Estado físico sin filtrar (para detectar cambios antes del debounce)
  uint8_t _rawButtons;

  // Estado confirmado actual (debounce aplicado)
  uint8_t _buttons;

  // Se acaba de presionar (evento de un solo ciclo)
  uint8_t _pressed;

  // Se acaba de soltar (evento de un solo ciclo)
  uint8_t _released;

  // ========================================================
  // Helper: verifica un bit (botón 0-7) en un estado agrupado
  // ========================================================

  static inline bool isSet(uint8_t estados, uint8_t boton) {
    return (estados & (1 << boton)) != 0;
  }

  // ========================================================
  // Debounce
  // ========================================================

  uint32_t _buttonLast[MAX_BUTTONS];
  uint32_t _buttonDelay;
};

#endif

// ====================================================================================
// Fin
// ====================================================================================