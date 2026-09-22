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

Credits::Credits(Display& display, Buttons& buttons, Sound& sound)
  : _display(display),
    _buttons(buttons),
    _sound(sound),
    _entry(1),
    _dir(1),
    _slideX(0),
    _animLast(0),
    _colAcc(0),
    _exit(false),
    _chipBoxRole(_display.getWidth(), _display.getTextHeight(TEXT_12x16)),
    _chipBoxName(_display.getWidth(), _display.getTextHeight(TEXT_6x8)),
    _composer(_display.getWidth(), _display.getTextHeight(TEXT_12x16)) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Credits::begin() {
  _entry = 1;  // entrada central (Snake II / v0.1)
  _slideX = 0;
  _colAcc = 0;
  _animLast = millis();
  _exit = false;
}

// ========================================================
// Actualizar (consume eventos de botones, navega y anima)
// ========================================================

void Credits::update() {
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
    _entry--;
    moved = true;
  }

  if (_buttons.pressed(Buttons::MOVE_RIGHT) && _entry < NUM_ENTRIES - 1) {
    _entry++;
    moved = true;
  }

  if (moved) {
    _sound.play(Sound::SFX_CLICK);
    startSlide((_entry > before) ? 1 : -1);
    Serial.printf("Credits: opcion %u -> %u\n", before, _entry);
  }

  if (_buttons.actionUpPressed()) _exit = true;
}

// ========================================================
// Inicio de la transición lateral
// ========================================================

void Credits::startSlide(int8_t dir) {
  // La tira arranca completamente fuera de pantalla (desde el borde)
  // y entra por un lado, igual que el menú
  _dir = dir;
  _slideX = _dir * (int16_t)_display.getWidth();
  _colAcc = 0;
  _animLast = millis();
}

// ========================================================
// Animación del deslizamiento lateral: avanza 1 px por cada
// ANIM_TICK ms acumulado por tiempo (constante aunque el loop
// sea lento), igual que el menú principal
// ========================================================

void Credits::animate() {
  if (_slideX == 0) return;

  uint32_t now = millis();
  uint32_t delta = now - _animLast;
  _animLast = now;

  _colAcc += delta;
  while (_colAcc >= ANIM_TICK) {
    _colAcc -= ANIM_TICK;
    if (_dir > 0) {              // entra por la derecha: se mueve hacia la izquierda
      if (_slideX > 0) _slideX--;
    } else {                     // entra por la izquierda: se mueve hacia la derecha
      if (_slideX < 0) _slideX++;
    }
  }
}

// ========================================================
// Cargar el rol o el nombre en la matriz de 1 bit (128x16,
// centrado); se compone en el canvas auxiliar (sin tocar las
// bandas actuales)
// ========================================================

void Credits::loadEntry(uint8_t slot) {
  uint8_t size = (slot == 0) ? TEXT_12x16 : TEXT_6x8;
  const char* text = ROLE_NAME[_entry][slot];
  int16_t x0 = (int16_t)((_display.getWidth() -
                          _display.getTextWidth(text, size)) / 2);

  // Componer la tira: chip de fondo + texto invertido
  _composer.fillScreen(CHIP_BG);
  _composer.setTextSize(size);
  _composer.setTextColor(CHIP_TEXT, CHIP_BG);
  _composer.setCursor(x0, 0);
  _composer.print(text);

  // Copiar a la matriz de 1 bit: 1 = glifo, 0 = fondo
  for (uint8_t r = 0; r < STRIP_H; r++) {
    for (uint8_t c = 0; c < STRIP_W; c++) {
      uint8_t bit = (uint8_t)(1 << (c % 8));
      if (_composer.getPixel(c, r) == CHIP_TEXT)
        _strip[r][c / 8] |= bit;
      else
        _strip[r][c / 8] &= (uint8_t)~bit;
    }
  }
}

// ========================================================
// Pintar las columnas visibles de la tira sobre su banda
// persistente: cada columna de la tira (incluidos sus espacios
// de fondo) sobrescribe lo que había, así la entrada anterior
// se mantiene hasta ser borrada por la nueva
// ========================================================

void Credits::slideStrip(GFXcanvas8& chipBox, uint8_t h) {
  for (uint8_t r = 0; r < h; r++) {
    for (uint16_t sx = 0; sx < _display.getWidth(); sx++) {
      int16_t sc = (int16_t)sx - _slideX;            // columna de la matriz (borde izq. = _slideX)
      if (sc < 0 || sc >= (int16_t)STRIP_W) continue;

      bool glyph = _strip[r][sc / 8] & (uint8_t)(1 << (sc % 8));
      chipBox.drawPixel(sx, r, glyph ? CHIP_TEXT : CHIP_BG);
    }
  }
}

// ========================================================
// Volcar una banda persistente a la pantalla
// ========================================================

void Credits::blitBand(GFXcanvas8& chipBox, int16_t y, uint8_t h,
                       uint16_t fgColor, uint16_t bgColor) {
  Adafruit_SSD1306& s = _display.screen();

  for (uint8_t r = 0; r < h; r++) {
    for (uint16_t c = 0; c < _display.getWidth(); c++) {
      uint8_t v = chipBox.getPixel(c, r);
      s.drawPixel(c, y + r, (v == CHIP_TEXT) ? fgColor : bgColor);
    }
  }
}

// ========================================================
// Dibujar una banda (rol = 0, nombre = 1) con transición.
// Ambas bandas deslizan a la vez con el mismo _slideX, por lo
// que aparecen sincronizadas (mismo offset horizontal)
// ========================================================

void Credits::drawBand(uint8_t slot, int16_t y, uint8_t size,
                       uint16_t fgColor, uint16_t bgColor) {
  GFXcanvas8& chipBox = (slot == 0) ? _chipBoxRole : _chipBoxName;
  uint8_t h = _display.getTextHeight(size);

  loadEntry(slot);
  slideStrip(chipBox, h);
  blitBand(chipBox, y, h, fgColor, bgColor);
}

// ========================================================
// Dibujar
// ========================================================

void Credits::print() {
  // Título de la ventana
  _display.drawTextAligned("Credits", CENTER, TEXT_12x16, REGION_HEADER);

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