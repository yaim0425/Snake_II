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
    _overlayHidden(false),
    _lastCount(0xFF),
    _hasGame(false),
    _difficulty(DIFICULTAD_DEFAULT),
    _moveDelay(0),
    _moveLast(0),
    _startMs(0),
    _headIx(0),
    _tailIx(0),
    _length(0),
    _dir(Dir::RIGHT),
    _nextDir(Dir::NONE),
    _bellyPending(false),
    _food(display, COLS, ROWS, BODY_TOP),
    _score(0),
    _bestScore(0) {}

// ========================================================
// Inicialización (al entrar en la ventana)
//
// `newGame` = true (NUEVO): reinicia la partida y arranca el
// conteo regresivo 3-2-1. `false` (CONTINUAR): reanuda la
// partida anterior en pausa (el tablero se conserva) o
// arranca una nueva si no hay partida en curso (por ejemplo
// tras un GAME_OVER).
// ========================================================

void Game::begin(bool newGame) {
  _exit = false;
  _redraw = true;
  _redrawHeader = true;
  _dirtyBoard = true;
  _nextDir = Dir::NONE;  // ningún giro pendiente al entrar
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
  if (level < DIFICULTAD_MIN) level = DIFICULTAD_MIN;
  if (level > DIFICULTAD_MAX) level = DIFICULTAD_MAX;
  _difficulty = level;

  // La dificultad se aplica EN CALIENTE: si hay una partida en curso
  // (PLAY o PAUSE) su velocidad (_moveDelay) se actualiza al instante
  // con el nuevo nivel; la próxima partida nueva la vuelve a derivar
  // en reset().
  _moveDelay = speedFor(_difficulty);
}

// ========================================================
// Reinicio de partida (tablero, serpiente, alimento y puntaje)
// ========================================================

void Game::reset() {
  randomSeed(micros());

  _hasGame = true;  // arranca una partida en curso (reanudable desde "Continue")
  _score = 0;  // el récord (_bestScore) se conserva entre partidas
  _overlayHidden = false;  // el conteo arranca con el primer dígito visible
  _lastCount = 0xFF;  // ningún dígito del conteo mostrado aún

  _length = 4;
  _tailIx = 0;
  _headIx = 3;
  _dir = Dir::RIGHT;
  _nextDir = Dir::NONE;
  _bellyPending = false;  // ninguna casilla pendiente de panza

  // Serpiente inicial horizontal: células (1,2)..(4,2), cabeza a la derecha.
  // Cada segmento guarda su dirección y su sprite persistente.
  _body[0] = { 1, 2, Dir::RIGHT, Sprite::TAIL_TO_RIGHT };
  _body[1] = { 2, 2, Dir::RIGHT, Sprite::BODY_TO_RIGHT };
  _body[2] = { 3, 2, Dir::RIGHT, Sprite::BODY_TO_RIGHT };
  _body[3] = { 4, 2, Dir::RIGHT, Sprite::HEAD_RIGHT_CLOSE };

  _moveDelay = speedFor(_difficulty);
  _startMs = millis();
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
  _moveLast = millis();
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
      if (_buttons.actionRightPressed()) {
        _sound.play(Sound::SFX_START);
        startPlay();
      } else if (millis() - _startMs >= COUNTDOWN_MS) {
        _sound.play(Sound::SFX_START);
        startPlay();
      }
      break;
    }

    case State::PLAY: {
      handleTurn();
      // Btn2 (ACTION_RIGHT) = Select / Pause: al presionar durante la
      // partida se congela el tablero y se muestra el panel "PAUSA".
      // En PAUSE se retoma con el mismo botón (o ACTION_LEFT).
      if (_buttons.actionRightPressed()) {
        _sound.play(Sound::SFX_PAUSE);
        _state = State::PAUSE;
        break;
      }
      if (millis() - _moveLast >= _moveDelay) {
        _moveLast = millis();
        step();
      }
      break;
    }

    case State::PAUSE: {
      // Reanudar con Btn2 (ACTION_RIGHT, "Select / Pause") o ACTION_LEFT
      if (_buttons.pressed(Buttons::ACTION_RIGHT) ||
          _buttons.pressed(Buttons::ACTION_LEFT)) {
        _sound.play(Sound::SFX_RESUME);
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
//
// Solo hay "estado actual" (`_dir`) y "siguiente" (`_nextDir`):
// un único giro pendiente, sin cola ni buffer. Un MOVE se
// evalúa SIEMPRE desde la dirección actual de la cabeza
// (`_dir`, la COMMITIDA, la que usará en el próximo paso): desde
// ella solo hay 3 posibilidades —seguir, girar a la izquierda o
// girar a la derecha— y la contraria (180°) se ignora. Si llega
// un MOVE válido, queda como PENDIENTE (`_nextDir`, el último
// válido pisa al anterior) y se aplica recién en el próximo
// `step()`. Así no se produce un GAME OVER espurio por una
// reversa falsa del último MOVE contra la dirección con la que
// la cabeza avanzará realmente.
// ========================================================

void Game::turn(Dir d) {
  if (d == Dir::NONE || d == _dir || d == _nextDir) return;

  // Prohibir la reversa directa contra la dirección COMMITIDA (_dir),
  // no contra un giro pendiente intermedio (la cabeza no puede volver
  // sobre sí misma respecto a la dirección con la que avanzará)
  if ((d == Dir::UP && _dir == Dir::DOWN) ||
      (d == Dir::DOWN && _dir == Dir::UP) ||
      (d == Dir::LEFT && _dir == Dir::RIGHT) ||
      (d == Dir::RIGHT && _dir == Dir::LEFT)) {
    return;
  }

  // Giro aceptado: queda pendiente y suena el blip de dirección
  _nextDir = d;
  _sound.play(Sound::SFX_TURN);
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
  // Dirección real de este paso: el giro pendiente del último MOVE
  // válido (ya validado contra la dirección COMMITIDA en turn()).
  // Se trabaja con una copia local `dir`: `_dir` (la dirección
  // COMMITIDA) SOLO se actualiza si el destino resulta legal, así al
  // colisionar la cabeza conserva su orientación real de movimiento
  // y no aparece dibujada "volteada" hacia el choque (headPart()
  // dibuja la cabeza según `_dir`).
  Dir dir = _dir;
  if (_nextDir != Dir::NONE) {
    dir = _nextDir;
    _nextDir = Dir::NONE;
  }

  const Seg& h = _body[_headIx];
  uint8_t nx = h.x;
  uint8_t ny = h.y;

  switch (dir) {
    case Dir::UP:    ny = (ny == 0) ? (uint8_t)(ROWS - 1) : (uint8_t)(ny - 1); break;
    case Dir::RIGHT: nx = (nx + 1) % COLS; break;
    case Dir::DOWN:  ny = (ny == ROWS - 1) ? 0 : (uint8_t)(ny + 1); break;
    case Dir::LEFT:  nx = (nx == 0) ? (uint8_t)(COLS - 1) : (uint8_t)(nx - 1); break;
    default: break;
  }

  bool eat = _food.has() && nx == _food.x() && ny == _food.y();

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

  // El destino es legal: la cabeza "commitea" la nueva dirección tras
  // el giro (si no se superó la validación, `_dir` quedó intacto y la
  // cabeza sigue apuntando hacia donde realmente viajaba).
  _dir = dir;

  // El cuerpo NO se mueve: la casilla que la cabeza deja pasa a ser
  // cuerpo nuevo con su sprite persistente (BODY recto, CORNER si
  // giró, BELLY si era la casilla de la comida recién comida).
  const Seg& oldHead = _body[_headIx];
  Dir in  = oldHead.dir;   // con qué dirección llegó la cabeza a esta casilla
  Dir out = dir;           // con qué dirección se va hacia la nueva casilla

  // La cabeza avanza a la nueva casilla (se agrega la nueva parte)
  uint8_t ni = (_headIx + 1) % MAX_LENGTH;
  _body[ni] = { nx, ny, dir, Sprite::HEAD_RIGHT_CLOSE };
  _headIx = ni;

  // La casilla que dejó la cabeza se convierte en cuerpo (parte persistente)
  Seg& newBody = _body[(_headIx + MAX_LENGTH - 1) % MAX_LENGTH];

  if (_bellyPending) {
    // Estaba sobre la comida (recién comida): al dejarla se pinta la panza
    newBody.part = bellyPartFor(in, out);
    _bellyPending = false;
  } else {
    newBody.part = bodyPartFor(in, out);
  }
  newBody.dir = out;

  if (!eat) {
    // Se elimina la última parte (cola): el segmento que queda último
    // pasa a ser cola y recibe su sprite de cola apuntando como su dir
    _tailIx = (_tailIx + 1) % MAX_LENGTH;
    Seg& tail = _body[_tailIx];
    tail.part = (Sprite::Part)(Sprite::TAIL_TO_UP +
                  ((uint8_t)tail.dir - 1));
  } else {
    _length++;
    // El valor de la comida es el nivel de dificultad actual: como la
    // dificultad puede cambiar en caliente, el puntaje suma el `_difficulty`
    // del momento de comer (no el de arranque).
    _score += _difficulty;
    _bellyPending = true;  // la cabeza quedó sobre la comida: al moverse pintará BELLY
    _redrawHeader = true;
    _sound.play(Sound::SFX_EAT);
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
  _gameOverMs = millis();
  _celeSfx = 0;
  _redrawHeader = true;
  _sound.play(Sound::SFX_GAME_OVER);
  _dirtyBoard = true;
}

// ========================================================
// ¿Una celda está ocupada por la serpiente? Lo consulta Food
// (al colocar el alimento en una celda libre).
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
// Sprite de la cabeza. La boca se abre una casilla antes de
// llegar a la comida (el alimento está en la próxima celda
// según _dir) y se cierra al colisionar con ella (cuando la
// cabeza está sobre el alimento, ya no apunta a ninguna).
// El orden del enum Dir (UP=1..LEFT=4) coincide con el orden
// de los sprites por dirección (HEAD_UP_..): se indexan con
// (dir - 1).
// ========================================================

Sprite::Part Game::headPart() const {
  uint8_t off = (uint8_t)_dir - 1;

  // Próxima celda según la dirección actual
  const Seg& head = _body[_headIx];
  uint8_t nx = head.x;
  uint8_t ny = head.y;
  switch (_dir) {
    case Dir::UP:    ny = (ny == 0) ? (uint8_t)(ROWS - 1) : (uint8_t)(ny - 1); break;
    case Dir::RIGHT: nx = (nx + 1) % COLS; break;
    case Dir::DOWN:  ny = (ny == ROWS - 1) ? 0 : (uint8_t)(ny + 1); break;
    case Dir::LEFT:  nx = (nx == 0) ? (uint8_t)(COLS - 1) : (uint8_t)(nx - 1); break;
    default: break;
  }

  bool aboutToEat = _food.has() && nx == _food.x() && ny == _food.y();
  uint8_t base = aboutToEat ? Sprite::HEAD_UP_OPEN
                            : Sprite::HEAD_UP_CLOSE;
  return (Sprite::Part)(base + off);
}

// ========================================================
// Dirección opuesta (RIGHT<->LEFT, UP<->DOWN). Se usa para
// hallar el lado de la celda por donde ENTRA la tubería: si la
// cabeza viajaba hacia `d`, el cuerpo anterior viene desde el
// lado opuesto a `d`.
// ========================================================

Game::Dir Game::opposite(Dir d) const {
  switch (d) {
    case Dir::UP:    return Dir::DOWN;
    case Dir::DOWN:  return Dir::UP;
    case Dir::RIGHT: return Dir::LEFT;
    case Dir::LEFT:  return Dir::RIGHT;
    default:         return Dir::NONE;
  }
}

// ========================================================
// Sprite del cuerpo según las direcciones de entrada (in) y
// salida (out): recto (in == out) -> BODY_TO_<dir>; giro
// (in != out) -> CORNER_<lado horizontal>_<lado vertical>
// (el índice de la curva es CORNER_RIGHT_UP(8) +
// desplazamiento: horiz RIGHT -> vert UP +0 / DOWN +1;
// horiz LEFT -> vert UP +2 / DOWN +3).
// ========================================================

Sprite::Part Game::bodyPartFor(Dir in, Dir out) const {
  if (in == out) {
    return (Sprite::Part)(Sprite::BODY_TO_UP + ((uint8_t)out - 1));
  }

  // La esquina conecta el lado por el que la tubería ENTRA a la
  // celda (opuesto a la dirección de llegada `in`; p. ej. *iba a
  // la izquierda* -> entra por la derecha) y el lado por el que
  // SALE (`out`; p. ej. *ahora va a arriba*). Los nombres son
  // esos dos lados: CORNER_<horizontal>_<vertical> (RIGHT_UP si
  // entra por la derecha y sale arriba, o entra por arriba y sale
  // a la derecha).
  Dir entry = opposite(in);
  Dir horiz = (entry == Dir::RIGHT || entry == Dir::LEFT) ? entry : out;
  Dir vert  = (entry == Dir::RIGHT || entry == Dir::LEFT) ? out   : entry;

  uint8_t base = Sprite::CORNER_RIGHT_UP;
  if (horiz == Dir::RIGHT) base += (vert == Dir::UP) ? 0 : 1;
  else                     base += (vert == Dir::UP) ? 2 : 3;

  return (Sprite::Part)base;
}

// ========================================================
// Sprite de la panza (cuando la cabeza deja la casilla donde
// estaba la comida). Recta (in == out) -> BELLY_TO_RIGHT (sirve
// para UP) o BELLY_TO_LEFT (sirve para DOWN); giro (in != out)
// -> BELLY_RIGHT_UP(22) + desplazamiento (igual que CORNER).
// ========================================================

Sprite::Part Game::bellyPartFor(Dir in, Dir out) const {
  if (in == out) {
    // Panza recta: comparte sprite por par de direcciones
    if (out == Dir::UP || out == Dir::RIGHT)
      return Sprite::BELLY_TO_RIGHT;
    return Sprite::BELLY_TO_LEFT;
  }

  // Giro (igual que CORNER): la esquina conecta el lado por el que
  // la tubería ENTRA (opuesto a `in`) y el lado por el que SALE
  // (`out`). El nombre es BELLY_<horizontal>_<vertical>.
  Dir entry = opposite(in);
  Dir horiz = (entry == Dir::RIGHT || entry == Dir::LEFT) ? entry : out;
  Dir vert  = (entry == Dir::RIGHT || entry == Dir::LEFT) ? out   : entry;

  uint8_t base = Sprite::BELLY_RIGHT_UP;
  if (horiz == Dir::RIGHT) base += (vert == Dir::UP) ? 0 : 1;
  else                     base += (vert == Dir::UP) ? 2 : 3;

  return (Sprite::Part)base;
}

// ========================================================
// Dibujar un sprite en la celda (x, y): cada píxel del sprite
// 4x4 se dibuja como un bloque 2x2 px (completa la celda 8x8).
// ========================================================

void Game::drawSprite(Sprite::Part part, uint8_t x, uint8_t y) {
  Adafruit_SSD1306& s = _display.screen();
  int16_t baseX = (int16_t)x * 8;
  int16_t baseY = BODY_TOP + (int16_t)y * 8;

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
  for (uint8_t i = 0; i < _length; i++) {
    const Seg& seg = _body[slot(i)];
    Sprite::Part part = (i == _length - 1) ? headPart() : seg.part;
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
  _display.drawText(buf, 0, 0, TEXT_12x16);

  // Segundos restantes de la comida especial, a la derecha
  snprintf(buf, sizeof(buf), "%u", (unsigned)_food.specialTime());
  int16_t w = _display.getTextWidth(buf, TEXT_12x16);
  _display.drawText(buf, _display.getWidth() - w, 0, TEXT_12x16);
}

// ========================================================
// Overlay en el Body: banda blanca de lado a lado (fullWidth)
// o cuadro centrado alrededor del texto, en ambos casos con
// texto invertido centrado en el rectángulo.
// ========================================================

void Game::drawOverlay(const char* title, bool fullWidth) {
  Adafruit_SSD1306& s = _display.screen();
  int16_t w = _display.getTextWidth(title, TEXT_12x16);
  int16_t h = _display.getTextHeight(TEXT_12x16);
  int16_t x = (_display.getWidth() - w) / 2;
  int16_t bandH = h + 2;   // banda: 2 px sobre el texto, 0 debajo
  int16_t y = BODY_TOP + (_display.getHeight() - BODY_TOP - bandH) / 2 + 2;

  if (fullWidth) {
    s.fillRoundRect(0, y - 2, _display.getWidth(), bandH, 0, SSD1306_WHITE);
  } else {
    s.fillRoundRect(x - 4, y - 2, w + 6, bandH, 0, SSD1306_WHITE);
  }
  _display.drawTextInverted(title, x, y, TEXT_12x16);
}

// ========================================================
// Velocidad por dificultad: lineal con pasos alternados de
// 101/102 ms (102 en niveles 1, 4 y 7: nivel%3 == 1). Los
// 9 saltos suman 912 ms, de 1000 (nivel 1) a 88 (nivel 10,
// DIFICULTAD_MAX): saltos de 102 = (nivel+1)/3.
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
      uint32_t elapsed = millis() - _startMs;
      uint32_t seg = COUNTDOWN_MS / 3;
      uint32_t pos = elapsed % seg;   // posición dentro del dígito actual
      uint32_t done = elapsed / seg;  // cuántos dígitos se completaron
      uint8_t n = (done >= 3) ? 0 : (uint8_t)(3 - done);

      // Pitido del conteo: un SFX_TICK por dígito, al cambiar el mostrado
      // (incluido el "3" inicial: _lastCount arranca en 0xFF)
      if (n != _lastCount) {
        _lastCount = n;
        _sound.play(Sound::SFX_TICK);
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
        uint8_t phase = (uint8_t)((millis() - _gameOverMs) / NEW_BEST_SIGN_MS % 4);
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
            if (!(_celeSfx & 0x01)) { _celeSfx |= 0x01; _sound.play(Sound::SFX_NEW_BEST); }
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