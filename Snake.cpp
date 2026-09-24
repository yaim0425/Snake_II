#include "Snake.h"

// ========================================================
// Constructor (sin hardware: el tablero se arma en reset())
// ========================================================

Snake::Snake()
  : _headIx(0),
    _tailIx(0),
    _length(0),
    _dir(Dir::RIGHT),
    _nextDir(Dir::NONE),
    _bellyPending(false) {}

// ========================================================
// Reinicio: serpiente inicial horizontal (1,2)..(4,2),
// cabeza a la derecha
// ========================================================

void Snake::reset() {
  _length = 4;
  _tailIx = 0;
  _headIx = 3;
  _dir = Dir::RIGHT;
  _nextDir = Dir::NONE;
  _bellyPending = false;  // ninguna casilla pendiente de panza

  // Cada segmento guarda su dirección y su sprite persistente
  _body[0] = { 1, 2, Dir::RIGHT, Sprite::TAIL_TO_RIGHT };
  _body[1] = { 2, 2, Dir::RIGHT, Sprite::BODY_TO_RIGHT };
  _body[2] = { 3, 2, Dir::RIGHT, Sprite::BODY_TO_RIGHT };
  _body[3] = { 4, 2, Dir::RIGHT, Sprite::HEAD_RIGHT_CLOSE };
}

// ========================================================
// Cancelar el giro pendiente (al entrar en la ventana)
// ========================================================

void Snake::clearPending() {
  _nextDir = Dir::NONE;
}

// ========================================================
// Giro de la cabeza (MOVE), sin reversa directa
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

bool Snake::turn(Dir d) {
  if (d == Dir::NONE || d == _dir || d == _nextDir) return false;

  // Prohibir la reversa directa contra la dirección COMMITIDA (_dir),
  // no contra un giro pendiente intermedio (la cabeza no puede volver
  // sobre sí misma respecto a la dirección con la que avanzará)
  if ((d == Dir::UP && _dir == Dir::DOWN) ||
      (d == Dir::DOWN && _dir == Dir::UP) ||
      (d == Dir::LEFT && _dir == Dir::RIGHT) ||
      (d == Dir::RIGHT && _dir == Dir::LEFT)) {
    return false;
  }

  // Giro aceptado: queda pendiente
  _nextDir = d;
  return true;
}

// ========================================================
// Un paso del tablero: mover la cabeza (wrap), detectar
// colisión con el propio cuerpo y comer/crecer. `fx`/`fy`
// indican dónde está el alimento (para saber si este paso
// cae sobre él y, por lo tanto, si la cola es bloqueante).
// ========================================================

Snake::Result Snake::step(uint8_t fx, uint8_t fy) {
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

  bool eat = (nx == fx && ny == fy);

  // Colisión con el cuerpo. Al comer la cola NO se mueve (es bloqueante);
  // sin comer, la celda de la cola se libera y es legal pisarla.
  uint8_t skip = eat ? 0xFF : _tailIx;
  for (uint8_t i = 0; i < _length; i++) {
    uint8_t s = slot(i);
    if (s == skip) continue;
    if (_body[s].x == nx && _body[s].y == ny) {
      return Result::DIED;  // la cabeza NO avanza (para GAME OVER lo decide Game)
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
    return Result::MOVED;
  }

  // Comió: crece (la cola NO avanza este paso); la panza queda
  // pendiente para pintarse cuando la cabeza deje la casilla.
  _length++;
  _bellyPending = true;
  return Result::ATE;
}

// ========================================================
// ¿Una celda está ocupada por la serpiente? (lo consulta el
// alimento para generarse en una celda libre)
// ========================================================

bool Snake::occupied(uint8_t x, uint8_t y) const {
  for (uint8_t i = 0; i < _length; i++) {
    const Seg& s = _body[slot(i)];
    if (s.x == x && s.y == y) return true;
  }
  return false;
}

// ========================================================
// Acceso para dibujar (lo consume Game)
// ========================================================

uint8_t Snake::length() const {
  return _length;
}

const Snake::Seg& Snake::segment(uint8_t index) const {
  return _body[slot(index)];
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

Sprite::Part Snake::headPart(bool hasFood, uint8_t fx, uint8_t fy) const {
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

  bool aboutToEat = hasFood && nx == fx && ny == fy;
  uint8_t base = aboutToEat ? Sprite::HEAD_UP_OPEN
                            : Sprite::HEAD_UP_CLOSE;
  return (Sprite::Part)(base + off);
}

// ========================================================
// Índice del ring buffer (segmento `index` contado desde la cola)
// ========================================================

uint8_t Snake::slot(uint8_t index) const {
  return (_tailIx + index) % MAX_LENGTH;
}

// ========================================================
// Dirección opuesta (RIGHT<->LEFT, UP<->DOWN). Se usa para
// hallar el lado de la celda por donde ENTRA la tubería: si la
// cabeza viajaba hacia `d`, el cuerpo anterior viene desde el
// lado opuesto a `d`.
// ========================================================

Snake::Dir Snake::opposite(Dir d) const {
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

Sprite::Part Snake::bodyPartFor(Dir in, Dir out) const {
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

Sprite::Part Snake::bellyPartFor(Dir in, Dir out) const {
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

// ====================================================================================
// Fin
// ====================================================================================