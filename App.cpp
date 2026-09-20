#include "App.h"

// ========================================================
// Constructor
// ========================================================

App::App(Display& display, Buttons& buttons)
  : _state(State::MENU),
    _display(display),
    _buttons(buttons),
    _menu(display, buttons, 0, "v0.1"),
    _credits(display, buttons),
    _info(display, buttons) {}

// ========================================================
// Inicialización (estado inicial: menú)
// ========================================================

void App::begin() {
  changeState(State::MENU);
}

// ========================================================
// Transición (fija el estado y llama a begin() de la ventana)
// ========================================================

void App::changeState(State newState) {
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
// Actualizar (ventana activa + transiciones)
// ========================================================

void App::update() {

  switch (_state) {

    case State::MENU: {

      _menu.update();

      // Confirmar opción (ACTION_LEFT) -> cambiar de ventana
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

void App::print() {

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