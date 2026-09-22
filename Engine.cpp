#include "Engine.h"

// ========================================================
// Constructor: guarda las ventanas ya construidas (no las anida)
// ========================================================

Engine::Engine(Display& display, Buttons& buttons, Boot& boot, Menu& menu,
               Credits& credits, InfoWindow& info, Legend& legend,
               Sound& sound, SoundWindow& soundWindow)
  : _state(State::BOOT),
    _display(display),
    _buttons(buttons),
    _boot(boot),
    _menu(menu),
    _credits(credits),
    _info(info),
    _legend(legend),
    _sound(sound),
    _soundWindow(soundWindow) {}

// ========================================================
// Inicialización (estado inicial: menú). Se llama desde setup().
// ========================================================

void Engine::begin() {
  changeState(State::BOOT);
}

// ========================================================
// Actualizar (solo la ventana activa + transiciones)
// ========================================================

void Engine::update() {

  switch (_state) {

    case State::BOOT: {

      _boot.update();
      if (_boot.done()) changeState(State::LEGEND);
      break;
    }

    case State::MENU: {

      _menu.update();

      // Confirmar opción (ACTION_RIGHT) -> cambiar de ventana
      int8_t sel = _menu.confirm();
      if (sel >= 0) {
        _sound.play(Sound::SFX_CONFIRM);
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
    case State::DIFICULTAD: {

      _info.update();
      if (_info.done()) {
        _sound.play(Sound::SFX_BACK);
        changeState(State::MENU);
      }
      break;
    }

    case State::SONIDO: {

      _soundWindow.update();
      if (_soundWindow.done()) {
        _sound.play(Sound::SFX_BACK);
        changeState(State::MENU);
      }
      break;
    }

    case State::CREDITOS: {

      _credits.update();
      if (_credits.done()) {
        _sound.play(Sound::SFX_BACK);
        changeState(State::MENU);
      }
      break;
    }

    case State::LEGEND: {

      _legend.update();
      if (_legend.done()) {
        // La Legend ya reprodujo el sonido según el botón presionado
        changeState(State::MENU);
      }
      break;
    }
  }
}

// ========================================================
// Dibujar (limpia y dibuja solo la ventana activa)
// ========================================================

void Engine::print() {

  _display.clear();

  switch (_state) {
    case State::BOOT:       _boot.print();                                 break;
    case State::MENU:       _menu.print();                                 break;
    case State::NUEVO:
    case State::CONTINUAR:
    case State::DIFICULTAD:
                             _info.print();                                 break;
    case State::SONIDO:     _soundWindow.print();                          break;
    case State::CREDITOS:   _credits.print();                              break;
    case State::LEGEND:     _legend.print();                               break;
  }
}

// ========================================================
// Puntaje máximo (lo conserva el menú entre sesiones)
// ========================================================

void Engine::setTopScore(uint8_t value) {
  _menu.setTopScore(value);
}

// ========================================================
// Transición (fija el estado y llama al begin() de la ventana entrante)
// ========================================================

void Engine::changeState(State newState) {
  _state = newState;

  switch (_state) {
    case State::BOOT:       _boot.begin();                                 break;
    case State::MENU:       _menu.begin();                                 break;
    case State::NUEVO:      _info.begin("New");                            break;
    case State::CONTINUAR:  _info.begin("Continue");                       break;
    case State::DIFICULTAD: _info.begin("Difficulty");                     break;
    case State::SONIDO:     _soundWindow.begin();                          break;
    case State::CREDITOS:   _credits.begin();                              break;
    case State::LEGEND:     _legend.begin();                               break;
  }
}