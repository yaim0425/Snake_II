#include "Engine.h"

// ========================================================
// Constructor: guarda las ventanas ya construidas (no las anida)
// ========================================================

Engine::Engine(Display& display, Buttons& buttons, Boot& boot, Menu& menu,
               Credits& credits, Game& game, Legend& legend,
               Sound& sound)
  : _state(State::BOOT),
    _display(display),
    _buttons(buttons),
    _boot(boot),
    _menu(menu),
    _credits(credits),
    _game(game),
    _legend(legend),
    _sound(sound) {}

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
          case Menu::OPT_NEW:      changeState(State::NEW);      break;
          case Menu::OPT_CONTINUE:  changeState(State::CONTINUE);  break;
          case Menu::OPT_CREDITS:   changeState(State::CREDITS);   break;
        }
      }
      break;
    }

    case State::NEW:
    case State::CONTINUE: {

      _game.update();
      if (_game.done()) {
        _sound.play(Sound::SFX_BACK);
        _menu.setBestScore(_game.bestScore());
        // "Continue" solo se habilita si la partida sigue en curso Y el
        // jugador tiene puntos (score > 0): al salir sin haber comido no
        // hay nada que reanudar. Con GAME OVER o sin puntos las opciones
        // son 4 y la selección queda en "New".
        bool gameOver = _game.isGameOver();
        bool resumable = !gameOver && _game.score() > 0;
        _menu.setContinueAvailable(resumable);
        _menu.setSelected(resumable ? Menu::OPT_CONTINUE : Menu::OPT_NEW);
        changeState(State::MENU);
      }
      break;
    }

    case State::CREDITS: {

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

  // NO hay clear() global: cada ventana limpia la pantalla completa
  // solo la primera vez que se dibuja (tras su begin()) y luego solo
  // borra/redibuja sus zonas dinámicas (ver cada ventana).

  switch (_state) {
    case State::BOOT:       _boot.print();                                 break;
    case State::MENU:       _menu.print();                                 break;
    case State::NEW:
    case State::CONTINUE:
                             _game.print();                                 break;
    case State::CREDITS:   _credits.print();                              break;
    case State::LEGEND:     _legend.print();                               break;
  }
}

// ========================================================
// Puntaje máximo (lo conserva el menú entre sesiones)
// ========================================================

void Engine::setBestScore(uint16_t value) {
  _menu.setBestScore(value);
}

// ========================================================
// Transición (fija el estado y llama al begin() de la ventana entrante)
// ========================================================

void Engine::changeState(State newState) {
  _state = newState;

  switch (_state) {
    case State::BOOT:       _boot.begin();                                 break;
    case State::MENU:       _menu.begin();                                 break;
    case State::NEW:      _game.setDifficulty(_menu.difficulty());
                            _game.begin(true);                              break;
    case State::CONTINUE:  _game.setDifficulty(_menu.difficulty());
                            _game.begin(false);                             break;
    case State::CREDITS:   _credits.begin();                              break;
    case State::LEGEND:     _legend.begin();                               break;
  }
}

// ====================================================================================
// Fin
// ====================================================================================