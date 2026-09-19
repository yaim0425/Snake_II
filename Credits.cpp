#include "esp32-hal.h"
#include "Credits.h"

// ========================================================
// Entradas de crédito (navegables con MOVE_RIGHT/MOVE_LEFT)
// ========================================================
// Izquierda (0)     : Dev / opencode.ai
// Centro   (1)      : Snake II / v0.1 (inicial)
// Derecha  (2)      : Director / YAIM904

static const char* const ROLE_NAME[Credits::NUM_ENTRIES][2] = {
  { "Dev",      "opencode.ai" },
  { "Snake II", "v0.1"        },
  { "Director", "YAIM904"     }
};

// Valores del canvas de la banda: 0 = transparente, 1 = texto, 255 = fondo
// (el fondo/bloque cubre el texto viejo al deslizar el nuevo)
static constexpr uint8_t CHIP_BG   = 255;
static constexpr uint8_t CHIP_TEXT = 1;

// ========================================================
// Geometría
// ========================================================
// El rol (tamaño 2) va SELECCIONADO (cuadro blanco de borde a borde + texto
// negro), centrado en el alto restante del Body: de la fila 16 (BODY_TOP)
// hasta la fila anterior al pie (PIE_TOP).
// El nombre (tamaño 1) va DES-SELECCIONADO (texto blanco plano) en el pie.
static constexpr int16_t BODY_TOP = 16;   // fila superior del Body
static constexpr int16_t PIE_TOP = 54;    // fila superior del pie del Body

// ========================================================
// Constructor
// ========================================================

Credits::Credits(Display& display, Buttons& buttons)
  : _display(display),
    _buttons(buttons),
    _entry(1),
    _prev(1),
    _dir(1),
    _slideX(0),
    _animLast(0),
    _exit(false),
    _band(_display.getWidth(), _display.getTextHeight(TEXT_12x16)) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Credits::begin() {
  _entry = 1;  // entrada central (Snake II / v0.1)
  _prev = _entry;
  _slideX = 0;
  _animLast = millis();
  _exit = false;
}

// ========================================================
// Actualizar (lee botones, navega y anima)
// ========================================================

void Credits::update() {
  _buttons.read();
  navigate();
  animate();
}

// ========================================================
// Navegación
// ========================================================

void Credits::navigate() {
  // Solo MOVE_LEFT y MOVE_RIGHT (primera y última no conectadas)
  uint8_t before = _entry;
  bool moved = false;

  if (_buttons.pressed(Buttons::MOVE_LEFT) && _entry > 0) {
    _prev = _entry;
    _entry--;
    moved = true;
  }

  if (_buttons.pressed(Buttons::MOVE_RIGHT) && _entry < NUM_ENTRIES - 1) {
    _prev = _entry;
    _entry++;
    moved = true;
  }

  if (moved) {
    startSlide((_entry > before) ? 1 : -1);
    Serial.printf("Credits: opcion %u -> %u\n", before, _entry);
  }

  if (_buttons.actionUpPressed()) _exit = true;
}

// ========================================================
// Inicio de la transición lateral
// ========================================================

void Credits::startSlide(int8_t dir) {
  _dir = dir;
  _slideX = _dir * SLIDE_DIST;
  _animLast = millis();
}

// ========================================================
// Animación del deslizamiento lateral
// ========================================================

void Credits::animate() {
  uint32_t now = millis();

  if (_slideX == 0) return;
  if (now - _animLast < ANIM_TICK) return;
  _animLast = now;

  _slideX -= _dir * ANIM_STEP;

  if ((_dir > 0 && _slideX < 0) || (_dir < 0 && _slideX > 0))
    _slideX = 0;
}

// ========================================================
// Borrado de la entrada previa (dirección contraria a la entrante)
// ========================================================

void Credits::wipeOld(int16_t offX) {
  int16_t w = _display.getWidth();
  uint16_t a = (offX < 0) ? (uint16_t)(-offX) : (uint16_t)offX;
  if (a >= SLIDE_DIST) return;   // aún no arranca el barrido

  // Progreso del barrido (0 al inicio, w al final), sobre el canvas
  int16_t front = (int16_t)((uint32_t)(SLIDE_DIST - a) * w / SLIDE_DIST);

  if (_dir > 0)
    _band.fillRect(w - front, 0, front, _band.height(), 0);  // dcha -> izda
  else
    _band.fillRect(0, 0, front, _band.height(), 0);          // izda -> dcha
}

// ========================================================
// Pintar un texto en el canvas (bloque de fondo + texto)
// ========================================================

void Credits::paintText(const char* text, int16_t x, uint8_t size) {
  uint8_t tw = _display.getTextWidth(text, size);
  uint8_t th = _display.getTextHeight(size);

  // Bloque de fondo: cubre el texto viejo bajo el entrante
  _band.fillRect(x - 2, 0, tw + 4, th, CHIP_BG);

  _band.setTextSize(size);
  _band.setTextColor(CHIP_TEXT, CHIP_BG);
  _band.setCursor(x, 0);
  _band.print(text);
}

// ========================================================
// Volcar el canvas a la pantalla en una banda
// ========================================================

void Credits::blitBand(int16_t y, uint8_t size, uint16_t fgColor,
                       uint16_t bgColor) {
  Adafruit_SSD1306& s = _display.screen();
  uint8_t h = _display.getTextHeight(size);

  for (uint8_t py = 0; py < h; py++) {
    for (uint8_t px = 0; px < _display.getWidth(); px++) {
      uint8_t v = _band.getPixel(px, py);
      if (v == CHIP_TEXT)      s.drawPixel(px, y + py, fgColor);
      else if (v == CHIP_BG)   s.drawPixel(px, y + py, bgColor);
    }
  }
}

// ========================================================
// Dibujar una banda (rol = 0, nombre = 1) con transición
// ========================================================

void Credits::drawBand(uint8_t slot, int16_t y, uint8_t size, uint16_t fgColor,
                       uint16_t bgColor) {
  int16_t w = _display.getWidth();

  _band.fillScreen(0);

  // La entrada previa (centrada) se pinta y se barre desde la dirección
  // contraria a la entrante
  if (_prev != _entry) {
    int16_t prevX = (w - _display.getTextWidth(ROLE_NAME[_prev][slot], size)) / 2;
    paintText(ROLE_NAME[_prev][slot], prevX, size);
    wipeOld(_slideX);
  }

  // La entrada entrante (bloque de fondo + texto) se desliza hasta centrarse
  int16_t curX = (w - _display.getTextWidth(ROLE_NAME[_entry][slot], size)) / 2;
  paintText(ROLE_NAME[_entry][slot], curX + _slideX, size);

  blitBand(y, size, fgColor, bgColor);
}

// ========================================================
// Dibujar
// ========================================================

void Credits::print() {
  // Título de la ventana
  _display.drawTextAligned("Creditos", CENTER, TEXT_12x16, REGION_HEADER);

  // Rol: cuadro blanco de borde a borde (fijo), texto negro
  int16_t w = _display.getWidth();
  int16_t roleH = _display.getTextHeight(TEXT_12x16);
  int16_t roleY = BODY_TOP + (PIE_TOP - BODY_TOP - roleH) / 2;
  _display.screen().fillRect(0, roleY - 1, w, roleH + 2, SSD1306_WHITE);
  drawBand(0, roleY, TEXT_12x16, SSD1306_BLACK, SSD1306_WHITE);

  // Nombre: texto blanco plano en el pie
  drawBand(1, PIE_TOP + 1, TEXT_6x8, SSD1306_WHITE, SSD1306_BLACK);
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool Credits::done() const {
  return _exit;
}