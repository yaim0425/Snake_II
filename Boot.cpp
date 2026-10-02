#include "HardwareSerial.h"
#include "Boot.h"
#include "Globals.h"
#include "Sprite.h"

#include <Adafruit_GFX.h>

// ========================================================
// Constructor (usa los servicios globales Display/Buttons)
// ========================================================

const char* const Boot::MESSAGE[MESSAGE_LINES] = {
  "Press any button",
  "to start"
};

Boot::Boot()
  // : _step(0),
  // _prevStep(0),
  // : _ticker(ANIM_TICK),
  : _timer(),
    _blinkMessage(false),
    _visibleMessage(false),
    _holdMessage(true),
    _done(false),
    _clear(true),
    _x(0),
    _y(0),
    _bars(0) {}


// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Boot::begin() {
  // _step = 0;
  // _ticker.start();
  _timer.start();
  _x = 0;
  _y = 0;
  _bars = 0;
  _visibleMessage = false;
  _blinkMessage = false;
  _holdMessage = true;
  _done = false;
  _clear = true;
}

// ========================================================
// Actualizar (procesa eventos de botones ya leídos y avanza el desplazamiento)
// ========================================================

void Boot::update() {
  // Cualquier botón cierra la ventana. El sonido depende del
  // botón presionado (prioridad si se pulsan varios a la vez):
  // MOVE = SFX_CLICK, ACTION_UP (Back) = SFX_BACK,
  // ACTION_RIGHT (Select / Pause) = SFX_CONFIRM,
  // ACTION_DOWN/ACTION_LEFT (None) = SFX_CLICK
  bool exit = false;

  if (buttons.pressed(Buttons::MOVE_UP) || buttons.pressed(Buttons::MOVE_RIGHT) || buttons.pressed(Buttons::MOVE_DOWN) || buttons.pressed(Buttons::MOVE_LEFT)) {
    sound.play(Sound::SFX_CLICK);
    exit = true;
  } else if (buttons.pressed(Buttons::ACTION_UP)) {
    sound.play(Sound::SFX_BACK);
    exit = true;
  } else if (buttons.pressed(Buttons::ACTION_RIGHT)) {
    sound.play(Sound::SFX_CONFIRM);
    exit = true;
  } else if (buttons.pressed(Buttons::ACTION_DOWN) || buttons.pressed(Buttons::ACTION_LEFT)) {
    sound.play(Sound::SFX_CLICK);
    exit = true;
  }

  if (exit) {
    _done = true;
    return;
  }

  // // Duración total
  // if (_stopwatch.expired(TOTAL_MS)) {
  //   _done = true;
  //   return;
  // }

  // // Avance de 1 px cada ANIM_TICK ms (Ticker acumula por tiempo;
  // // consume() devuelve los pasos completos de una vez)
  // uint32_t steps = _ticker.consume();
  // // for (uint8_t i = 0; i < steps - 1; ++i) {
  // //   _step = (_step + 1) % BAR_SPACING;
  // //   drawBars();
  // // }
  // if (steps) _step = (_step + steps) % BAR_SPACING;

  if (_holdMessage && _timer.expired(Config::DefaultTimer::HOLD))
    _holdMessage = !_holdMessage;

  if (!_holdMessage)
    _blinkMessage = _visibleMessage ^ _timer.blinkOn(Config::DefaultTimer::PERIOD, Config::DefaultTimer::OFF);
}

// ========================================================
// Dibujar (bandas de líneas verticales)
//
// Cada franja (BAR_W px) se pinta repartida en dos pasos: aquí
// sus BAR_W-1 columnas de la cola y en drawBars() la de cabeza
// (la primera de la franja según la dirección en la que se
// mueve), que es la única columna nueva en cada avance de 1 px.
// drawBars() borra además, en negro, la columna que cada franja
// deja libre por la cola.
//
// Solo se limpia lo necesario: en el primer frame (y cuando el
// salto es de más de 1 px) se hace un clear() completo; si el
// desplazamiento no cambió, no se toca nada. Todas las columnas
// se pintan con el módulo normalizado a 0..ancho-1 porque
// fillRect() recorta en vez de envolver: la franja que rebalsa
// por el borde se parte en dos tramos (derecho + izquierdo) en
// lugar de cortarse en seco.
// ========================================================

void Boot::print() {
  firstPrint();
  blinkMessage();



  // if (_step == _prevStep) return;

  // drawBars();
  // _prevStep = _step;
}

// ========================================================
// Dibujar las franjas iniciales
//
// Cada franja se pinta aquí con sus BAR_W-1 columnas de la cola;
// la de cabeza (la primera según su dirección) la pone
// drawBars(), que es la única que entra al desplazar 1 px. Con el
// desplazamiento 0 (el primer frame) ninguna base cae en la
// última columna, así que el fillRect de BAR_W-1 columnas nunca
// se corta por el borde derecho.
// ========================================================

void Boot::firstPrint() {
  if (!_clear) return;

  // ------------------------------------------------------

  display.clear();
  _clear = false;

  // ------------------------------------------------------

  const int16_t w = Config::Screen::WIDTH;
  // int16_t headerTop = Config::Screen::HEADER_TOP;

  // int16_t footLine = Config::Screen::FOOT_LINE;
  // display.fillRect(0, footLine, w, 1, false);

  const int16_t bodyTop = Config::Screen::BODY_TOP;
  // display.fillRect(0, bodyTop, w, 1, false);
  // display.fillRect(0, bodyTop - 1, w, 1, false);

  // ------------------------------------------------------

  // int16_t top = Config::Scroller::TOP;
  // int16_t h = Config::Screen::HEADER_H;
  // display.fillRect(0, top, w, h, false);
  // display.fillRect(0, 0, w, h, false);

  // ------------------------------------------------------

  // char* name = Config::Version::NAME;
  // int16_t nameWidth = (w - strlen(name) * 12) / 2;
  // display.drawTextInverted(name, nameWidth, top + 1, TEXT_12x16);

  // ------------------------------------------------------

  _visibleMessage = false;
  _blinkMessage = true;
  blinkMessage();

  // for (int8_t i = 0; i < MESSAGE_LINES; i++) {
  //   const char* message = MESSAGE[i];
  //   int16_t x = (w - strlen(message) * 6) / 2;
  //   int16_t y = headerTop + i * 9;
  //   display.drawTextInverted(message, x, y, TEXT_6x8);
  // }

  // const char* message = MESSAGE;
  // _x = (w - strlen(message) * 6) / 2;
  // _y = Config::Screen::HEADER_TOP;
  // display.drawTextInverted(message, _x, _y, TEXT_6x8);
  // display.drawText(message, _x, _y, TEXT_6x8);
  // display.drawText(message, _x, _y, TEXT_12x16);

  // ------------------------------------------------------

  // display.fillRect(0, bodyTop + h, w, h, false);

  // ------------------------------------------------------

  display.fillRect(0, bodyTop, w, Config::Screen::BODY_H, false);

  const int16_t midX = (w - Sprite::LOGO_W) / 2;

  for (int16_t y = 0; y < Sprite::LOGO_H; ++y)
    for (int16_t x = 0; x < Sprite::LOGO_W; ++x)
      if (Sprite::logoPixel(x, y))
        display.drawPixel(midX + x, bodyTop + y, true);

  // ------------------------------------------------------


  // "Press any button..." (centrado en la franja de texto)

  // const int16_t w = display.getWidth();

  // // TITULO: las líneas se mueven de izquierda a derecha, así que la
  // // cabeza es su extremo derecho (y la cola, su extremo izquierdo)
  // for (int16_t x = 0; x < w; x += BAR_SPACING) {
  //   int16_t px = (x + _step) % w;

  //   display.fillRect(px, Config::Screen::HEADER_TOP, BAR_W, Config::Screen::HEADER_H, false);
  // }

  // // CUERPO: las líneas se mueven de derecha a izquierda, así que la
  // // cabeza es su extremo izquierdo (y la cola, su extremo derecho)
  // for (int16_t x = 0; x < w; x += BAR_SPACING) {
  //   int16_t px = ((w - x - _step - BAR_W - 1) % w + w) % w;

  //   display.fillRect(px, Config::Screen::BODY_TOP, BAR_W, Config::Screen::BODY_H, false);
  // }
}

void Boot::blinkMessage() {
  if (!_blinkMessage) return;

  const int16_t w = Config::Screen::WIDTH;
  const int16_t headerTop = Config::Screen::HEADER_TOP;

  for (int8_t line = 0; line < MESSAGE_LINES; line++) {
    const char* message = MESSAGE[line];
    int16_t x = (w - strlen(message) * 6) / 2;
    int16_t y = headerTop + line * 8;

    if (_visibleMessage)
      display.fillRect(x, y, strlen(message) * 6, 8, true);
    else
      display.drawText(message, x, y, TEXT_6x8);
  }

  _visibleMessage = !_visibleMessage;
  _blinkMessage = false;
}

// ========================================================
// Dibujar todas las franjas en el desplazamiento actual
//
// Por cada franja: se imprime su columna de cabeza (la primera
// según la dirección) y se borra en negro la que deja libre por
// la cola; el resto de la franja no cambia, así que no se toca.
// La base de cada franja es la misma que en drawFirstBars().
// ========================================================

void Boot::drawBars() {
  const int16_t w = display.getWidth();

  // TITULO: de izquierda a derecha (cabeza a la derecha, se borra la izquierda)
  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t px = x + _step;

    int16_t deleteBar = px - 1;
    int16_t createBar = px + BAR_W - 1;

    drawBar(deleteBar, Config::Screen::HEADER_TOP, Config::Screen::HEADER_H, true);
    drawBar(createBar, Config::Screen::HEADER_TOP, Config::Screen::HEADER_H, false);
  }

  // CUERPO: de derecha a izquierda (cabeza a la izquierda, se borra la derecha)
  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t px = w - x - _step - BAR_W;

    int16_t deleteBar = px + BAR_W - 1;
    int16_t createBar = px - 1;

    drawBar(deleteBar, Config::Screen::BODY_TOP, Config::Screen::BODY_H, true);
    drawBar(createBar, Config::Screen::BODY_TOP, Config::Screen::BODY_H, false);
  }
}

// ========================================================
// Borrar solo lo que dejan de ocupar las franjas: cada columna
// de la franja anterior que no pertenece a la franja nueva se
// pinta de negro; el resto de la pantalla no se toca
// ========================================================

void Boot::eraseOldBars() {
  const int16_t w = display.getWidth();

  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t oldX = (x + _prevStep) % w;
    int16_t newX = (x + _step) % w;
    eraseBarDiff(oldX, newX, Config::Screen::HEADER_TOP, Config::Screen::HEADER_H, w);
  }

  for (int16_t x = 0; x < w; x += BAR_SPACING) {
    int16_t oldX = ((x - _prevStep) % w + w) % w;
    int16_t newX = ((x - _step) % w + w) % w;
    eraseBarDiff(oldX, newX, Config::Screen::BODY_TOP, Config::Screen::BODY_H, w);
  }
}

// ========================================================
// Borra 1 px de las columnas de la franja vieja que no están en la nueva
// ========================================================

void Boot::eraseBarDiff(int16_t oldX, int16_t newX, uint8_t top,
                        uint8_t height, int16_t width) {
  Adafruit_SSD1306& s = display.screen();

  for (uint8_t i = 0; i < BAR_W; i++) {
    int16_t oc = (oldX + i) % width;
    bool shared = false;
    for (uint8_t j = 0; j < BAR_W; j++) {
      if (((newX + j) % width) == oc) {
        shared = true;
        break;
      }
    }
    if (!shared) s.fillRect(oc, top, 1, height, SSD1306_BLACK);
  }
}

// ========================================================
// Dibujar una columna vertical de 1 px con el módulo normalizado
// a 0..ancho-1: fillRect() recorta en vez de envolver, así que la
// columna que rebalsa por el borde se pinta en el tramo que le
// toca (la parte de la franja que se va de la pantalla la repone
// el borrado de la columna que queda libre)
// ========================================================

void Boot::drawBar(int16_t x, uint8_t y, uint8_t height, bool black) {
  const int16_t w = display.getWidth();

  display.fillRect(((x % w) + w) % w, y, 1, height, black);
}

// ========================================================
// Salida (true = la animación terminó)
// ========================================================

bool Boot::done() const {
  return _done;
}

// ====================================================================================
// Fin
// ====================================================================================
