#ifndef APP_H
#define APP_H

#include "Display.h"
#include "Buttons.h"
#include "Menu.h"
#include "Credits.h"
#include "InfoWindow.h"

// Despachador de ventanas: decide cuál corre según un estado interno y hace la
// transición (con begin() de la ventana entrante) sin depender de loop().
class App {
public:

  // ========================================================
  // Constructor (recibe UNA única instancia de Display y Buttons)
  // ========================================================

  App(Display& display, Buttons& buttons);

  // ========================================================
  // Inicialización (estado inicial: menú)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (ventana activa + transiciones)
  // ========================================================

  void update();

  // ========================================================
  // Dibujar (limpia y dibuja la ventana activa)
  // ========================================================

  void print();

private:

  // ========================================================
  // Estado interno (variable global de activación)
  // ========================================================

  enum class State : uint8_t {
    MENU = 0,
    NUEVO,
    CONTINUAR,
    DIFICULTAD,
    SONIDO,
    CREDITOS
  };

  State _state;

  // ========================================================
  // Transición (fija el estado y llama a begin() de la ventana)
  // ========================================================

  void changeState(State newState);

  // ========================================================
  // Ventanas (comparten la misma Display)
  // ========================================================

  Display& _display;
  Buttons& _buttons;

  Menu _menu;
  Credits _credits;
  InfoWindow _info;
};

#endif