#include "Game.h"
#include "Globals.h"
#include "Timer.h"

#include "Sprite.h"

#include <stdio.h>

// ========================================================
// Constructor
// ========================================================

Game::Game()
  : _state(State::START),
    _exit(false),
    _redraw(true),
    _redrawHeader(true),
    _dirtyBoard(true),
    _overlayHidden(false),
    _lastCount(0xFF),
    _hasGame(false),
    _difficulty(Config::Difficulty::DEFAULT_LEVEL),
    _moveDelay(0),
    _moveLast(0),
    _startMs(0),
    _gameOverMs(0),
    _newBest(false),
    _celeSfx(0),
    _snake(),
    _food(Snake::COLS, Snake::ROWS, Config::Screen::BODY_TOP),
    _score(0),
    _bestScore(0) {}

// ========================================================
// Inicialización (al entrar en la ventana)
//
// `newGame` = true (NEW): reinicia la partida y arranca el
// conteo regresivo 3-2-1. `false` (CONTINUE): reanuda la
// partida anterior en pausa (el tablero se conserva) o
// arranca una nueva si no hay partida en curso (por ejemplo
// tras un GAME_OVER).
// ========================================================

void Game::begin(bool newGame) {
  _exit = false;
  _redraw = true;
  _redrawHeader = true;
  _dirtyBoard = true;
  _snake.clearPending();  // ningún giro pendiente al entrar
  _newBest = false;      // el festejo de récord lo decide die() en cada muerte
  _celeSfx = 0;
  _gameOverMs = 0;

  if (newGame || !_hasGame) {
    // Nueva partida (velocidad según la dificultad actual del menú)
    _state = State::START;
    reset();
    Serial.println("Snake II: nueva partida");
  } else {
    // Reanudar la partida anterior: queda en pausa, el jugador
    // la retoma con ACTION_RIGHT (o ACTION_LEFT)
    _state = State::PAUSE;
    Serial.println("Snake II: continuar partida");
  }
}

// ========================================================
// Dificultad (la aplica el Engine desde el valor del menú)
// ========================================================

void Game::setDifficulty(uint8_t level) {
  if (level < Config::Difficulty::MIN_LEVEL) level = Config::Difficulty::MIN_LEVEL;
  if (level > Config::Difficulty::MAX_LEVEL) level = Config::Difficulty::MAX_LEVEL;
  _difficulty = level;

  // La dificultad se aplica EN CALIENTE: si hay una partida en curso
  // (PLAY o PAUSE) su velocidad (_moveDelay) se actualiza al instante
  // con el nuevo nivel; la próxima partida nueva la vuelve a derivar
  // en reset().
  _moveDelay = speedFor(_difficulty);
}

// ========================================================
// Reinicio de partida (serpiente, alimento y puntaje)
// ========================================================

void Game::reset() {
  // Semilla del generador aleatorio (lo consume Food::spawn): esp_random()
  // es el RNG de hardware del ESP32 (entropía real, no predecible ni de 32
  // bits fijos como micros()); se combina con el reloj (nowMs) por defensa.
  randomSeed(esp_random() ^ (uint32_t)nowMs());

  _hasGame = true;  // arranca una partida en curso (reanudable desde "Continue")
  _score = 0;  // el récord (_bestScore) se conserva entre partidas
  _overlayHidden = false;  // el conteo arranca con el primer dígito visible
  _lastCount = 0xFF;  // ningún dígito del conteo mostrado aún

  _snake.reset();

  _moveDelay = speedFor(_difficulty);
  _startMs = nowMs();
  _moveLast = 0;

  // Alimento en una celda libre (al arrancar siempre hay celdas)
  _food.spawn(Food::Type::NORMAL, *this);
  // El conteo regresivo 3-2-1 suena solo con sus propios pitidos (SFX_TICK,
  // uno por dígito, en print()): ya no hay jingle SFX_START al entrar a START.
}

// ========================================================
// Entrar a PLAY (fin del conteo regresivo o de la pausa)
// ========================================================

void Game::startPlay() {
  _state = State::PLAY;
  _moveLast = nowMs();
  _dirtyBoard = true;  // borra el overlay "3-2-1"/"PAUSA" del tablero
}

// ========================================================
// Actualizar (consume los eventos de botones leídos en loop())
// ========================================================

void Game::update() {
  if (_exit) return;

  switch (_state) {

    case State::START: {
      // Conteo regresivo 3-2-1: se puede pre-girar la cabeza;
      // se entra a PLAY al confirmar con ACTION_RIGHT o al
      // agotarse el conteo (COUNTDOWN_MS). Al arrancar la partida
      // (después del "1") suena el jingle GO! (SFX_START).
      handleTurn();
      if (buttons.actionRightPressed()) {
        sound.play(Sound::SFX_START);
        startPlay();
      } else if (nowMs() - _startMs >= COUNTDOWN_MS) {
        sound.play(Sound::SFX_START);
        startPlay();
      }
      break;
    }

    case State::PLAY: {
      handleTurn();
      // Btn2 (ACTION_RIGHT) = Select / Pause: al presionar durante la
      // partida se congela el tablero y se muestra el panel "PAUSA".
      // En PAUSE se retoma con el mismo botón (o ACTION_LEFT).
      if (buttons.actionRightPressed()) {
        sound.play(Sound::SFX_PAUSE);
        _state = State::PAUSE;
        break;
      }
      if (nowMs() - _moveLast >= _moveDelay) {
        _moveLast = nowMs();
        step();
      }
      break;
    }

    case State::PAUSE: {
      // Reanudar con Btn2 (ACTION_RIGHT, "Select / Pause") o ACTION_LEFT
      if (buttons.pressed(Buttons::ACTION_RIGHT) ||
          buttons.pressed(Buttons::ACTION_LEFT)) {
        sound.play(Sound::SFX_RESUME);
        startPlay();
      }
      break;
    }

    case State::GAME_OVER: {
      // Cualquier botón ACTION vuelve al menú
      if (buttons.actionUpPressed() || buttons.actionRightPressed() ||
          buttons.actionDownPressed() || buttons.actionLeftPressed()) {
        _exit = true;
      }
      break;
    }
  }

  if (_exit) return;

  // Botón común "volver al menú" de todas las ventanas (ACTION_UP).
  // En GAME_OVER ya se manejó arriba. La partida NO se pierde:
  // "Continue" la reanuda en pausa.
  if (_state != State::GAME_OVER && buttons.actionUpPressed()) {
    _exit = true;
  }
}

// ========================================================
// Cambio de dirección (MOVE): traduce el botón a Snake::Dir
// y delega en Snake::turn (giro pendiente sin reversa directa).
// El SFX_TURN solo suena si el giro fue aceptado (turn() true).
// ========================================================

void Game::handleTurn() {
  if (buttons.moveUpPressed()) {
    if (_snake.turn(Snake::Dir::UP)) sound.play(Sound::SFX_TURN);
  } else if (buttons.moveRightPressed()) {
    if (_snake.turn(Snake::Dir::RIGHT)) sound.play(Sound::SFX_TURN);
  } else if (buttons.moveDownPressed()) {
    if (_snake.turn(Snake::Dir::DOWN)) sound.play(Sound::SFX_TURN);
  } else if (buttons.moveLeftPressed()) {
    if (_snake.turn(Snake::Dir::LEFT)) sound.play(Sound::SFX_TURN);
  }
}

// ========================================================
// Un paso del tablero (coordinación): la serpiente resuelve
// el movimiento/colisión/comer, Game maneja el resultado:
//   - DIED  -> die() (GAME OVER)
//   - ATE   -> puntaje (+difficultad), SFX_EAT, regenerar alimento
//              (si no hay celda libre: die())
//   - MOVED -> nada, solo repintar
// ========================================================

void Game::step() {
  Snake::Result r = _snake.step(_food.x(), _food.y());

  if (r == Snake::Result::DIED) {
    die();
    return;
  }

  if (r == Snake::Result::ATE) {
    // El valor de la comida es el nivel de dificultad actual: como la
    // dificultad puede cambiar en caliente, el puntaje suma el `_difficulty`
    // del momento de comer (no el de arranque).
    _score += _difficulty;
    _redrawHeader = true;
    sound.play(Sound::SFX_EAT);
    // Si el tablero quedó lleno, spawn devuelve false y se muere abajo
    if (!_food.spawn(Food::Type::NORMAL, *this)) {
      die();
      return;
    }
  }

  _dirtyBoard = true;
}

// ========================================================
// Fin de partida (colisión o tablero lleno)
// ========================================================

void Game::die() {
  _state = State::GAME_OVER;
  _hasGame = false;  // "Continue" ya no reanuda: arranca una nueva

  // El récord solo se verifica/actualiza al terminar en GAME OVER
  // (no durante la partida): el "Best" del menú refleja partidas
  // terminadas, no puntajes en curso. Al establecerse un NUEVO
  // récord se activa el festejo (ciclo "GAME OVER"/"BUT"/"YOU ARE"/
  // "THE BEST" en print(), con SFX_NEW_BEST solo la primera vez
  // que aparece el letrero "THE BEST").
  _newBest = _score > _bestScore;
  if (_newBest) _bestScore = _score;
  _gameOverMs = nowMs();
  _celeSfx = 0;
  _redrawHeader = true;
  sound.play(Sound::SFX_GAME_OVER);
  _dirtyBoard = true;
}

// ========================================================
// ¿Una celda está ocupada por la serpiente? Lo consulta Food
// (al colocar el alimento en una celda libre). Delega en Snake.
// ========================================================

bool Game::occupied(uint8_t x, uint8_t y) const {
  return _snake.occupied(x, y);
}

// ========================================================
// Dibujar un sprite en la celda (x, y): cada píxel del sprite
// 4x4 se dibuja como un bloque 2x2 px (completa la celda 8x8).
// ========================================================

void Game::drawSprite(Sprite::Part part, uint8_t x, uint8_t y) {
  Adafruit_SSD1306& s = display.screen();
  int16_t baseX = (int16_t)x * 8;
  int16_t baseY = Config::Screen::BODY_TOP + (int16_t)y * 8;

  for (uint8_t i = 0; i < Sprite::SIZE; i++) {
    for (uint8_t j = 0; j < Sprite::SIZE; j++) {
      if (Sprite::SPRITES[part][i][j])
        s.fillRect(baseX + 2 * j, baseY + 2 * i, 2, 2, SSD1306_WHITE);
    }
  }
}

// ========================================================
// Dibujar toda la serpiente (de la cola a la cabeza: la
// cabeza queda encima). Cada segmento usa su sprite
// persistente; la cabeza calcula el suyo en cada frame.
// ========================================================

void Game::drawSnake() {
  uint8_t n = _snake.length();
  for (uint8_t i = 0; i < n; i++) {
    const Snake::Seg& seg = _snake.segment(i);
    Sprite::Part part = (i == n - 1)
                          ? _snake.headPart(_food.has(), _food.x(), _food.y())
                          : seg.part;
    drawSprite(part, seg.x, seg.y);
  }
}

// ========================================================
// Header del juego: puntuación (izq., 12x16) y segundos
// restantes de la comida especial (der., 12x16; por ahora
// valor fijo solo para el layout, en la clase Food)
// ========================================================

void Game::drawHeader() {
  char buf[8];

  // Puntuación actual, esquina superior izquierda (NO se mueve)
  snprintf(buf, sizeof(buf), "%u", (unsigned)_score);
  display.drawText(buf, 0, 0, TEXT_12x16);

  // Segundos restantes de la comida especial, a la derecha
  snprintf(buf, sizeof(buf), "%u", (unsigned)_food.specialTime());
  int16_t w = display.getTextWidth(buf, TEXT_12x16);
  display.drawText(buf, display.getWidth() - w, 0, TEXT_12x16);
}

// ========================================================
// Overlay en el Body: banda blanca de lado a lado (fullWidth)
// o cuadro centrado alrededor del texto, en ambos casos con
// texto invertido centrado en el rectángulo.
// ========================================================

void Game::drawOverlay(const char* title, bool fullWidth) {
  Adafruit_SSD1306& s = display.screen();
  int16_t w = display.getTextWidth(title, TEXT_12x16);
  int16_t h = display.getTextHeight(TEXT_12x16);
  int16_t x = (display.getWidth() - w) / 2;
  int16_t bandH = h + 2;   // banda: 2 px sobre el texto, 0 debajo
  int16_t y = Config::Screen::BODY_TOP + (display.getHeight() - Config::Screen::BODY_TOP - bandH) / 2 + 2;

  if (fullWidth) {
    s.fillRoundRect(0, y - 2, display.getWidth(), bandH, 0, SSD1306_WHITE);
  } else {
    s.fillRoundRect(x - 4, y - 2, w + 6, bandH, 0, SSD1306_WHITE);
  }
  display.drawTextInverted(title, x, y, TEXT_12x16);
}

// ========================================================
// Velocidad por dificultad: lineal con pasos alternados de
// 101/102 ms (102 en niveles 1, 4 y 7: nivel%3 == 1). Los
// 9 saltos suman 912 ms, de 1000 (nivel 1) a 88 (nivel 10,
// Config::Difficulty::MAX_LEVEL): saltos de 102 = (nivel+1)/3.
// El nivel 1 es el más lento (1000 ms por paso) y el 10 el
// más rápido (88 ms).
// ========================================================

uint16_t Game::speedFor(uint8_t level) const {
  uint8_t jumps = level - 1;                  // saltos entre niveles
  uint8_t wide  = (level + 1) / 3;            // saltos de 102 (nivel%3 == 1)
  return (uint16_t)(1000 - (uint16_t)jumps * 101 - wide);
}

// ========================================================
// Dibujar
//
// Renderizado por zonas (sin clear() global): el clear()
// completo solo al entrar; luego por frame se borra/redibuja
// únicamente lo dinámico:
//   - Header (puntaje/récord) solo cuando cambian sus valores
//   - Tablero (alimento + serpiente) solo cuando algo cambió
//     (movimiento, comida, overlay que desaparece)
//   - Overlay (conteo "3-2-1"/"PAUSA"/"GAME OVER") en cada frame
// ========================================================

void Game::print() {
  // Primer frame tras begin(): clear() completo + redibujar todo
  if (_redraw) {
    display.clear();
    _redraw = false;
    _dirtyBoard = true;
    _redrawHeader = true;
  }

  // Header: solo cuando el puntaje o el récord cambió
  if (_redrawHeader) {
    display.screen().fillRect(0, 0, display.getWidth(), Config::Screen::BODY_TOP, SSD1306_BLACK);
    drawHeader();
    _redrawHeader = false;
  }

  // Tablero: solo cuando algo cambió
  if (_dirtyBoard) {
    display.screen().fillRect(0, Config::Screen::BODY_TOP, display.getWidth(),
                               display.getHeight() - Config::Screen::BODY_TOP, SSD1306_BLACK);
    if (_food.has()) _food.draw();
    drawSnake();
    _dirtyBoard = false;
  }

  // Overlay según el estado
  switch (_state) {
    case State::START: {
      // Conteo regresivo 3-2-1: un dígito por segundo (COUNTDOWN_MS/3).
      // Al final de cada fracción el número (y su cuadro) desaparece antes
      // de que aparezca el siguiente: el cambio es como un parpadeo
      // (COUNT_HIDE_MS ocultos por dígito; al ocultarlo se restaura el
      // tablero que hay debajo del cuadro).
      uint32_t elapsed = (uint32_t)(nowMs() - _startMs);
      uint32_t seg = COUNTDOWN_MS / 3;
      uint32_t pos = elapsed % seg;   // posición dentro del dígito actual
      uint32_t done = elapsed / seg;  // cuántos dígitos se completaron
      uint8_t n = (done >= 3) ? 0 : (uint8_t)(3 - done);

      // Pitido del conteo: un SFX_TICK por dígito, al cambiar el mostrado
      // (incluido el "3" inicial: _lastCount arranca en 0xFF)
      if (n != _lastCount) {
        _lastCount = n;
        sound.play(Sound::SFX_TICK);
      }

      if (pos >= seg - COUNT_HIDE_MS) {
        // Fase de parpadeo: el dígito y su cuadro desaparecen. Se marca
        // el tablero para borrar la zona del overlay una sola vez.
        if (!_overlayHidden) {
          _overlayHidden = true;
          _dirtyBoard = true;
        }
      } else {
        _overlayHidden = false;
        char buf[2];
        buf[0] = (char)('0' + n);
        buf[1] = '\0';
        drawOverlay(buf);
      }
      break;
    }
    case State::PAUSE:     drawOverlay("PAUSA", true);   break;
    case State::GAME_OVER: {
      if (_newBest) {
        // Festejo de nuevo récord: ciclo "GAME OVER" -> "BUT" -> "YOU ARE" ->
        // "THE BEST" (NEW_BEST_SIGN_MS cada uno) hasta que se presiona
        // un botón. La fanfarria (SFX_NEW_BEST) suena solo la primera vez
        // que aparece el letrero "THE BEST" (_celeSfx; el "GAME OVER" ya sonó
        // en die() con SFX_GAME_OVER).
        uint8_t phase = (uint8_t)((nowMs() - _gameOverMs) / NEW_BEST_SIGN_MS % 4);
        switch (phase) {
          case 0:
            drawOverlay("GAME OVER", true);
            break;
          case 1:
            drawOverlay("BUT", true);
            break;
          case 2:
            drawOverlay("YOU ARE", true);
            break;
          default:
            if (!(_celeSfx & 0x01)) { _celeSfx |= 0x01; sound.play(Sound::SFX_NEW_BEST); }
            drawOverlay("THE BEST", true);
            break;
        }
        break;
      }
      drawOverlay("GAME OVER", true);
      break;
    }
    default: break;
  }
}

// ========================================================
// Salida (true = pidió volver al menú)
// ========================================================

bool Game::done() const {
  return _exit;
}

// ========================================================
// ¿La partida terminó en GAME OVER? (comparar el estado
// permite distinguir la salida tras un Game Over, que deja
// la selección del menú en "New", de la salida con la
// partida en curso, que la deja en "Continue")
// ========================================================

bool Game::isGameOver() const {
  return _state == State::GAME_OVER;
}

// ========================================================
// Puntajes
// ========================================================

uint16_t Game::score() const {
  return _score;
}

uint16_t Game::bestScore() const {
  return _bestScore;
}

// ====================================================================================
// Fin
// ====================================================================================
