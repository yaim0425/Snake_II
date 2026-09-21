// ====================================================================================
// SNAKE II — Enlace de dependencias
//
// Una única instancia de Display (y de Buttons) es la misma para todo el juego:
// se crean aquí y se comparten por referencia con la clase App.
//
// App es el despachador: decide qué ventana corre según su estado interno y hace
// la transición (con begin() de la ventana entrante) sin que loop() participe.
// loop() solo llama a app.update() y app.print().
//
// La clase App (App.h / App.cpp) está integrada en este archivo.
// ====================================================================================

#include "Display.h"
#include "Buttons.h"
#include "Menu.h"
#include "Credits.h"
#include "InfoWindow.h"

#include <Arduino.h>

// Pines de los botones (orden del enum Button)
const int8_t BUTTON_PINS[Buttons::MAX_BUTTONS] = {
  02, 01, 41, 40,  // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  42, 39, 38, 47   // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};

// Instancias únicas compartidas por todo el juego
Display display;
Buttons buttons(BUTTON_PINS);

// ====================================================================================
// Clase App — despachador de ventanas (integrada en el .ino)
// ====================================================================================

class App {
public:

  // ========================================================
  // Constructor (recibe UNA única instancia de Display y Buttons)
  // ========================================================

  App(Display& display, Buttons& buttons)
    : _state(State::MENU),
      _display(display),
      _buttons(buttons),
      _menu(display, buttons, 0, "v0.1"),
      _credits(display, buttons),
      _info(display, buttons) {}

  // ========================================================
  // Inicialización (estado inicial: menú)
  // ========================================================

  void begin() {
    changeState(State::MENU);
  }

  // ========================================================
  // Actualizar (ventana activa + transiciones)
  // ========================================================

  void update() {

    switch (_state) {

      case State::MENU: {

        _menu.update();

        // Confirmar opción (ACTION_RIGHT) -> cambiar de ventana
        int8_t sel = _menu.confirm();
        if (sel >= 0) {
          switch (sel) {
            case Menu::OPC_NUEVO:      changeState(State::NUEVO);      break;
            case Menu::OPC_CONTINUAR:  changeState(State::CONTINUAR);  break;
            case Menu::OPC_DIFICULTAD: changeState(State::DIFICULTAD); break;
            case Menu::OPC_SONIDO:     changeState(State::SONIDO);     break;
            case Menu::OPC_CREDITOS:   changeState(State::CREDITOS);   break;
          }
        }
        break;
      }

      case State::NUEVO:
      case State::CONTINUAR:
      case State::DIFICULTAD:
      case State::SONIDO: {

        _info.update();
        if (_info.done()) changeState(State::MENU);
        break;
      }

      case State::CREDITOS: {

        _credits.update();
        if (_credits.done()) changeState(State::MENU);
        break;
      }
    }
  }

  // ========================================================
  // Dibujar (limpiar y dibujar la ventana activa)
  // ========================================================

  void print() {

    _display.clear();

    switch (_state) {
      case State::MENU:       _menu.print();                                        break;
      case State::NUEVO:
      case State::CONTINUAR:
      case State::DIFICULTAD:
      case State::SONIDO:     _info.print();                                        break;
      case State::CREDITOS:   _credits.print();                                     break;
    }
  }

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

  void changeState(State newState) {
    _state = newState;

    switch (_state) {
      case State::MENU:       _menu.begin();                                        break;
      case State::NUEVO:      _info.begin("New");                                  break;
      case State::CONTINUAR:  _info.begin("Continue");                             break;
      case State::DIFICULTAD: _info.begin("Difficulty");                           break;
      case State::SONIDO:     _info.begin("Sound");                                break;
      case State::CREDITOS:   _credits.begin();                                     break;
    }
  }

  // ========================================================
  // Ventanas (comparten la misma Display)
  // ========================================================

  Display& _display;
  Buttons& _buttons;

  Menu _menu;
  Credits _credits;
  InfoWindow _info;
};

// ====================================================================================

App app(display, buttons);

// ====================================================================================

void setup() {
  Serial.begin(115200);

  display.begin();
  buttons.begin();

  app.begin();

  Serial.println("Snake II");
}

// ====================================================================================

void loop() {
  app.update();
  app.print();
  display.show();
}

// ====================================================================================
// Fin
// ====================================================================================