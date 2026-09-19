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

// Valores del canvas del barrido: 0 = transparente, 1 = texto (se dibuja)
static constexpr uint8_t CHIP_TEXT = 1;

// ========================================================
// Geometría
// ========================================================
// El rol (tamaño 2) va SELECCIONADO (cuadro blanco de borde a borde + texto
// invertido), centrado en el alto restante del Body: de la fila 16 (BODY_TOP)
// hasta la fila anterior al pie (PIE_TOP).
// El nombre (tamaño 1) va DES-SELECCIONADO (texto plano blanco) en el pie.
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

void Credits::wipeOld(int16_t offX, int16_t y, uint8_t h, uint16_t color) {
  int16_t w = _display.getWidth();
  uint16_t a = (offX < 0) ? (uint16_t)(-offX) : (uint16_t)offX;
  if (a >= SLIDE_DIST) return;   // aún no arranca el barrido

  // Progreso del barrido (0 al inicio, w al final)
  int16_t front = (int16_t)((uint32_t)(SLIDE_DIST - a) * w / SLIDE_DIST);
  Adafruit_SSD1306& s = _display.screen();

  if (_dir > 0)
    s.fillRect(w - front, y, front, h, color);   // barrido derecha -> izquierda
  else
    s.fillRect(0, y, front, h, color);           // barrido izquierda -> derecha
}

// ========================================================
// Pintar un texto en el canvas (solo la capa de texto)
// ========================================================

void Credits::paintText(const char* text, int16_t x, uint8_t size) {
  _band.fillScreen(0);
  _band.setTextSize(size);
  _band.setTextColor(CHIP_TEXT);
  _band.setCursor(x, 0);
  _band.print(text);
}

// ========================================================
// Vuelcar el canvas a la pantalla en una banda
// ========================================================

void Credits::blitBand(const char* text, int16_t x, uint8_t size, int16_t y,
                       uint16_t color) {
  paintText(text, x, size);

  Adafruit_SSD1306& s = _display.screen();
  uint8_t h = _display.getTextHeight(size);

  for (uint8_t py = 0; py < h; py++) {
    for (uint8_t px = 0; px < _display.getWidth(); px++) {
      if (_band.getPixel(px, py) == CHIP_TEXT)
        s.drawPixel(px, y + py, color);
    }
  }
}

// ========================================================
// Dibujar
// ========================================================

void Credits::print() {
  Adafruit_SSD1306& s = _display.screen();

  // Título de la ventana
  _display.drawTextAligned("Creditos", CENTER, TEXT_12x16, REGION_HEADER);

  // ----- Rol (tamaño 2) seleccionado -----

  // Cuadro blanco de borde a borde, fijo
  int16_t w = _display.getWidth();
  int16_t roleH = _display.getTextHeight(TEXT_12x16);
  int16_t roleY = BODY_TOP + (PIE_TOP - BODY_TOP - roleH) / 2;
  s.fillRect(0, roleY - 1, w, roleH + 2, SSD1306_WHITE);

  // Entrada previa (centrada) se borra desde la dirección contraria
  if (_prev != _entry) {
    int16_t prevX = (w - _display.getTextWidth(ROLE_NAME[_prev][0], TEXT_12x16)) / 2;
    blitBand(ROLE_NAME[_prev][0], prevX, TEXT_12x16, roleY, SSD1306_BLACK);
    wipeOld(_slideX, roleY - 1, roleH + 2, SSD1306_WHITE);
  }

  // Entrada entrante se desliza hasta centrarse
  int16_t curX = (w - _display.getTextWidth(ROLE_NAME[_entry][0], TEXT_12x16)) / 2;
  blitBand(ROLE_NAME[_entry][0], curX + _slideX, TEXT_12x16, roleY, SSD1306_BLACK);

  // ----- Nombre (tamaño 1) des-seleccionado, en el pie -----

  uint8_t nameH = _display.getTextHeight(TEXT_6x8);
  uint8_t nameY = PIE_TOP + 1;  // banda de 10px, texto de 8px, centrado

  if (_prev != _entry) {
    int16_t prevX = (w - _display.getTextWidth(ROLE_NAME[_prev][1], TEXT_6x8)) / 2;
    blitBand(ROLE_NAME[_prev][1], prevX, TEXT_6x8, nameY, SSD1306_WHITE);
    wipeOld(_slideX, PIE_TOP, 10, SSD1306_BLACK);
  }

  int16_t nameX = (w - _display.getTextWidth(ROLE_NAME[_entry][1], TEXT_6x8)) / 2;
  blitBand(ROLE_NAME[_entry][1], nameX + _slideX, TEXT_6x8, nameY, SSD1306_WHITE);
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool Credits::done() const {
  return _exit;
}