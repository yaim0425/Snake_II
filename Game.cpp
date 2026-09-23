#include "Game.h"

#include <stdio.h>

// ========================================================
// Constructor
// ========================================================

Game::Game(Display& display, Buttons& buttons, Sound& sound)
  : _display(display),
    _buttons(buttons),
    _sound(sound),
    _state(State::START),
    _exit(false),
    _redraw(true),
    _redrawHeader(true),
    _dirtyBoard(true),
    _hasGame(false),
    _difficulty(DIFICULTAD_DEFAULT),
    _moveDelay(0),
    _moveLast(0),
    _startMs(0),
    _headIx(0),
    _tailIx(0),
    _length(0),
    _dir(Dir::RIGHT),
    _mouthMoves(0),
    _hasFood(false),
    _score(0),
    _bestScore(0) {}

// ========================================================
// Inicialización (al entrar en la ventana)
//
// `newGame` = true (NUEVO): reinicia la partida y arranca la
// cuenta regresiva "GO !". `false` (CONTINUAR): reanuda la
// partida anterior en pausa (el tablero se conserva) o
// arranca una nueva si no hay partida en curso (por ejemplo
// tras un GAME_OVER).
// ========================================================

void Game::begin(bool newGame) {
  _exit = false;
  _redraw = true;
  _redrawHeader = true;
  _dirtyBoard = true;

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
  if (level < DIFICULTAD_MIN) level = DIFICULTAD_MIN;
  if (level > DIFICULTAD_MAX) level = DIFICULTAD_MAX;
  _difficulty = level;
}

// ========================================================
// Reinicio de partida (tablero, serpiente, alimento y puntaje)
// ========================================================

void Game::reset() {
  randomSeed(micros());

  _hasGame = true;  // arranca una partida en curso (reanudable desde "Continue")
  _score = 0;  // el récord (_bestScore) se conserva entre partidas

  _length = 4;
  _tailIx = 0;
  _headIx = 3;
  _dir = Dir::RIGHT;
  _mouthMoves = 0;

  // Serpiente inicial horizontal: células (1,2)..(4,2), cabeza a la derecha
  for (uint8_t i = 0; i < 4; i++) {
    _body[i].x = i + 1;
    _body[i].y = 2;
  }

  _moveDelay = speedFor(_difficulty);
  _startMs = millis();
  _moveLast = 0;

  spawnFood();
  _sound.play(Sound::SFX_START);
}

// ========================================================
// Entrar a PLAY (fin de la cuenta regresiva o de la pausa)
// ========================================================

void Game::startPlay() {
  _state = State::PLAY;
  _moveLast = millis();
  _dirtyBoard = true;  // borra el overlay "GO !"/"PAUSA" del tablero
}

// ========================================================
// Actualizar (consume los eventos de botones leídos en loop())
// ========================================================

void Game::update() {
  if (_exit) return;

  switch (_state) {

    case State::START: {
      // Cuenta regresiva "GO !": se puede pre-girar la cabeza;
      // se entra a PLAY al confirmar con ACTION_RIGHT o al agotarse GO_MS
      handleTurn();
      if (_buttons.actionRightPressed()) {
        startPlay();
      } else if (millis() - _startMs >= GO_MS) {
        startPlay();
      }
      break;
    }

    case State::PLAY: {
      handleTurn();
      if (millis() - _moveLast >= _moveDelay) {
        _moveLast = millis();
        step();
      }
      break;
    }

    case State::PAUSE: {
      // Reanudar con ACTION_RIGHT o ACTION_LEFT (como el juego original)
      if (_buttons.pressed(Buttons::ACTION_RIGHT) ||
          _buttons.pressed(Buttons::ACTION_LEFT)) {
        startPlay();
      }
      break;
    }

    case State::GAME_OVER: {
      // Cualquier botón ACTION vuelve al menú
      if (_buttons.actionUpPressed() || _buttons.actionRightPressed() ||
          _buttons.actionDownPressed() || _buttons.actionLeftPressed()) {
        _exit = true;
      }
      break;
    }
  }

  if (_exit) return;

  // Botón común "volver al menú" de todas las ventanas (ACTION_UP).
  // En GAME_OVER ya se manejó arriba. La partida NO se pierde:
  // "Continue" la reanuda en pausa.
  if (_state != State::GAME_OVER && _buttons.actionUpPressed()) {
    _exit = true;
  }
}

// ========================================================
// Cambio de dirección (MOVE), sin reversa directa
// ========================================================

void Game::turn(Dir d) {
  if (d == Dir::NONE || d == _dir) return;

  // Prohibir la reversa directa (la cabeza no puede volver sobre sí misma)
  if ((d == Dir::UP && _dir == Dir::DOWN) ||
      (d == Dir::DOWN && _dir == Dir::UP) ||
      (d == Dir::LEFT && _dir == Dir::RIGHT) ||
      (d == Dir::RIGHT && _dir == Dir::LEFT)) {
    return;
  }

  _dir = d;
}

void Game::handleTurn() {
  if (_buttons.moveUpPressed()) {
    turn(Dir::UP);
  } else if (_buttons.moveRightPressed()) {
    turn(Dir::RIGHT);
  } else if (_buttons.moveDownPressed()) {
    turn(Dir::DOWN);
  } else if (_buttons.moveLeftPressed()) {
    turn(Dir::LEFT);
  }
}

// ========================================================
// Un paso del tablero: mover la cabeza, crecer al comer,
// detectar colisión con el propio cuerpo y tablero lleno
// ========================================================

void Game::step() {
  // La boca abierta tras comer se cierra tras MOUTH_MOVES movimientos
  if (_mouthMoves > 0) _mouthMoves--;

  const Seg& h = _body[_headIx];
  uint8_t nx = h.x;
  uint8_t ny = h.y;

  switch (_dir) {
    case Dir::UP:    ny = (ny == 0) ? (uint8_t)(ROWS - 1) : (uint8_t)(ny - 1); break;
    case Dir::RIGHT: nx = (nx + 1) % COLS; break;
    case Dir::DOWN:  ny = (ny == ROWS - 1) ? 0 : (uint8_t)(ny + 1); break;
    case Dir::LEFT:  nx = (nx == 0) ? (uint8_t)(COLS - 1) : (uint8_t)(nx - 1); break;
    default: break;
  }

  bool eat = _hasFood && nx == _food.x && ny == _food.y;

  // Colisión con el cuerpo. Al comer la cola NO se mueve (es bloqueante);
  // sin comer, la celda de la cola se libera y es legal pisarla.
  uint8_t skip = eat ? 0xFF : _tailIx;
  for (uint8_t i = 0; i < _length; i++) {
    uint8_t s = slot(i);
    if (s == skip) continue;
    if (_body[s].x == nx && _body[s].y == ny) {
      die();
      return;
    }
  }

  // Mover: la nueva cabeza entra en el ring buffer
  uint8_t ni = (_headIx + 1) % MAX_LENGTH;
  _body[ni].x = nx;
  _body[ni].y = ny;
  _headIx = ni;

  if (!eat) {
    _tailIx = (_tailIx + 1) % MAX_LENGTH;
  } else {
    _length++;
    _mouthMoves = MOUTH_MOVES;
    _score++;
if (_score > _bestScore) _bestScore = _score;
    _redrawHeader = true;
    _sound.play(Sound::SFX_EAT);
    spawnFood();  // si el tablero quedó lleno, _hasFood se apaga y se muere abajo
    if (!_hasFood) {
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

  if (_score > _bestScore) _bestScore = _score;
  _redrawHeader = true;
  _sound.play(Sound::SFX_GAME_OVER);
  _dirtyBoard = true;
}

// ========================================================
// Alimento: se ubica en una celda libre cualquiera (al azar).
// Si el tablero está lleno, _hasFood = false (partida ganada).
// ========================================================

void Game::spawnFood() {
  Seg freeCells[MAX_LENGTH];
  uint8_t n = 0;

  for (uint8_t y = 0; y < ROWS; y++) {
    for (uint8_t x = 0; x < COLS; x++) {
      if (!occupied(x, y)) {
        freeCells[n].x = x;
        freeCells[n].y = y;
        n++;
      }
    }
  }

  if (n == 0) {
    _hasFood = false;
    _dirtyBoard = true;
    return;
  }

  Seg pick = freeCells[random(n)];
  _food.x = pick.x;
  _food.y = pick.y;
  _hasFood = true;
  _dirtyBoard = true;
}

// ========================================================
// ¿Una celda está ocupada por la serpiente?
// ========================================================

bool Game::occupied(uint8_t x, uint8_t y) const {
  for (uint8_t i = 0; i < _length; i++) {
    const Seg& s = _body[slot(i)];
    if (s.x == x && s.y == y) return true;
  }
  return false;
}

// ========================================================
// Índice del ring buffer (segmento `index` contado desde la cola)
// ========================================================

uint8_t Game::slot(uint8_t index) const {
  return (_tailIx + index) % MAX_LENGTH;
}

// ========================================================
// Dirección de `a` hacia `b`. Los dos segmentos son siempre
// adyacentes (se mueven 1 celda por paso), con wrap en X y Y.
// Devuelve NONE si no son adyacentes (no debería ocurrir).
// ========================================================

Game::Dir Game::dirToward(const Seg& a, const Seg& b) const {
  if (((uint16_t)a.x + 1) % COLS == b.x) return Dir::RIGHT;
  if (a.x == ((uint16_t)b.x + 1) % COLS) return Dir::LEFT;
  if (((uint16_t)a.y + 1) % ROWS == b.y) return Dir::DOWN;
  if (a.y == ((uint16_t)b.y + 1) % ROWS) return Dir::UP;
  return Dir::NONE;
}

// ========================================================
// Sprite del segmento `index` según la geometría de sus vecinos:
//
//   - cola (índice 0): apunta hacia el siguiente segmento
//   - cabeza (último): dirección actual, boca abierta tras comer
//   - cuerpo: recto (in == out) o curva (in != out); el índice de
//     la curva es CORNER_<horizontal>_<vertical>
//
// El orden del enum Dir (UP=1..LEFT=4) coincide con el orden de
// los sprites por dirección (TAIL_TO_UP..TAIL_TO_LEFT,
// BODY_TO_UP.., HEAD_UP_..): se indexan con (dir - 1).
// ========================================================

SnakeSprites::Part Game::partFor(uint8_t index) const {
  uint8_t s = slot(index);

  if (index == 0) {
    Dir d = dirToward(_body[s], _body[slot(1)]);
    if (d == Dir::NONE) d = _dir;
    return (SnakeSprites::Part)(SnakeSprites::TAIL_TO_UP + ((uint8_t)d - 1));
  }

  if (index == _length - 1) {
    uint8_t off = (uint8_t)_dir - 1;
    if (_mouthMoves > 0)
      return (SnakeSprites::Part)(SnakeSprites::HEAD_UP_OPEN + off);
    return (SnakeSprites::Part)(SnakeSprites::HEAD_UP_CLOSE + off);
  }

  Dir in  = dirToward(_body[s], _body[slot(index - 1)]);
  Dir out = dirToward(_body[s], _body[slot(index + 1)]);
  if (in == Dir::NONE || out == Dir::NONE) return SnakeSprites::EMPTY;

  if (in == out) {
    return (SnakeSprites::Part)(SnakeSprites::BODY_TO_UP + ((uint8_t)out - 1));
  }

  // Curva: lado horizontal y lado vertical del giro.
  // El índice es CORNER_RIGHT_UP(8) + desplazamiento:
  //   horiz RIGHT:  vert UP=+0, DOWN=+1  (RIGHT_UP, RIGHT_DOWN)
  //   horiz LEFT:   vert UP=+2, DOWN=+3  (LEFT_UP,  LEFT_DOWN)
  Dir horiz, vert;
  if (in == Dir::UP || in == Dir::DOWN) {
    horiz = out;
    vert = in;
  } else {
    horiz = in;
    vert = out;
  }

  uint8_t base = SnakeSprites::CORNER_RIGHT_UP;
  if (horiz == Dir::RIGHT) base += (vert == Dir::UP) ? 0 : 1;
  else                     base += (vert == Dir::UP) ? 2 : 3;

  return (SnakeSprites::Part)base;
}

// ========================================================
// Dibujar un sprite en la celda (x, y): cada píxel del sprite
// 4x4 se dibuja como un bloque 2x2 px (completa la celda 8x8).
// ========================================================

void Game::drawSprite(SnakeSprites::Part part, uint8_t x, uint8_t y) {
  Adafruit_SSD1306& s = _display.screen();
  int16_t baseX = (int16_t)x * 8;
  int16_t baseY = BODY_TOP + (int16_t)y * 8;

  for (uint8_t i = 0; i < SnakeSprites::SIZE; i++) {
    for (uint8_t j = 0; j < SnakeSprites::SIZE; j++) {
      if (SnakeSprites::SPRITES[part][i][j])
        s.fillRect(baseX + 2 * j, baseY + 2 * i, 2, 2, SSD1306_WHITE);
    }
  }
}

// ========================================================
// Dibujar toda la serpiente (de la cola a la cabeza: la
// cabeza queda encima)
// ========================================================

void Game::drawSnake() {
  for (uint8_t i = 0; i < _length; i++) {
    const Seg& seg = _body[slot(i)];
    drawSprite(partFor(i), seg.x, seg.y);
  }
}

// ========================================================
// Alimento: rombo simétrico centrado en la celda (como el
// rombo seleccionado del menú)
// ========================================================

void Game::drawFood() {
  Adafruit_SSD1306& s = _display.screen();
  int16_t cx = (int16_t)_food.x * 8 + 4;
  int16_t cy = BODY_TOP + (int16_t)_food.y * 8 + 4;

  s.fillTriangle(cx, cy - 3, cx + 3, cy, cx, cy + 3, SSD1306_WHITE);
  s.fillTriangle(cx, cy - 3, cx - 3, cy, cx, cy + 3, SSD1306_WHITE);
}

// ========================================================
// Header del juego: puntaje (izq., 12x16) y récord (der., 6x8)
// ========================================================

void Game::drawHeader() {
  char buf[8];

  snprintf(buf, sizeof(buf), "%u", (unsigned)_score);
  _display.drawText(buf, 0, 0, TEXT_12x16);

  snprintf(buf, sizeof(buf), "HI %u", (unsigned)_bestScore);
  int16_t w = _display.getTextWidth(buf, TEXT_6x8);
  _display.drawText(buf, _display.getWidth() - w, 4, TEXT_6x8);
}

// ========================================================
// Overlay centrado en el Body (cuadro blanco + texto invertido)
// ========================================================

void Game::drawOverlay(const char* title) {
  Adafruit_SSD1306& s = _display.screen();
  int16_t w = _display.getTextWidth(title, TEXT_12x16);
  int16_t h = _display.getTextHeight(TEXT_12x16);
  int16_t x = (_display.getWidth() - w) / 2;
  int16_t y = BODY_TOP + (_display.getHeight() - BODY_TOP - h) / 2;

  s.fillRoundRect(x - 4, y - 4, w + 6, h + 4, 0, SSD1306_WHITE);
  _display.drawTextInverted(title, x, y, TEXT_12x16);
}

// ========================================================
// Velocidad por dificultad: el nivel 1 es el más lento (1000
// ms por paso) y el 25 el más rápido (88 ms), lineal.
// ========================================================

uint16_t Game::speedFor(uint8_t level) const {
  uint16_t delay = 1000 - (uint16_t)(level - 1) * 38;
  return delay;
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
//   - Overlay "GO !"/"PAUSA"/"GAME OVER" en cada frame
// ========================================================

void Game::print() {
  // Primer frame tras begin(): clear() completo + redibujar todo
  if (_redraw) {
    _display.clear();
    _redraw = false;
    _dirtyBoard = true;
    _redrawHeader = true;
  }

  // Header: solo cuando el puntaje o el récord cambió
  if (_redrawHeader) {
    _display.screen().fillRect(0, 0, _display.getWidth(), BODY_TOP, SSD1306_BLACK);
    drawHeader();
    _redrawHeader = false;
  }

  // Tablero: solo cuando algo cambió
  if (_dirtyBoard) {
    _display.screen().fillRect(0, BODY_TOP, _display.getWidth(),
                               _display.getHeight() - BODY_TOP, SSD1306_BLACK);
    if (_hasFood) drawFood();
    drawSnake();
    _dirtyBoard = false;
  }

  // Overlay según el estado
  switch (_state) {
    case State::START:    drawOverlay("GO !");     break;
    case State::PAUSE:    drawOverlay("PAUSA");    break;
    case State::GAME_OVER: drawOverlay("GAME OVER"); break;
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
// Puntajes
// ========================================================

uint8_t Game::score() const {
  return _score;
}

uint8_t Game::bestScore() const {
  return _bestScore;
}

// ====================================================================================
// Fin
// ====================================================================================