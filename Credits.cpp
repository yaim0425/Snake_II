#include "esp32-hal.h"
#include "Credits.h"
#include "Globals.h"

#include "Config.h"

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

// Alto de cada banda del Scroller: rol 12x16 → 16 px,
// nombre 6x8 → 8 px (coincide con getTextHeight)
static const uint8_t BAND_HEIGHTS[2] = { 16, 8 };

// ========================================================
// Geometría
// ========================================================
// El rol (tamaño 2) va SELECCIONADO (cuadro blanco de borde a borde + texto
// negro), centrado en el alto restante del Body: de la fila 16 (BODY_TOP)
// hasta la fila anterior al pie (PIE_TOP).
// El nombre (tamaño 1) va DES-SELECCIONADO (texto blanco plano) en el pie.
// La fila superior del Body vive en Config::Screen::BODY_TOP (16).
static constexpr int16_t PIE_TOP = 54;    // fila superior del pie del Body

// ========================================================
// Constructor
// ========================================================

Credits::Credits()
  : _entry(1),
    _exit(false),
    _redraw(true),
    _scroller(2, BAND_HEIGHTS) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Credits::begin() {
  _entry = 1;  // entrada central (Snake II / v0.1)
  _scroller.begin();
  _exit = false;
  _redraw = true;
}

// ========================================================
// Actualizar (consume eventos de botones, navega y anima)
// ========================================================

void Credits::update() {
  navigate();
  _scroller.animate();
}

// ========================================================
// Navegación
// ========================================================

void Credits::navigate() {
  // Solo MOVE_LEFT y MOVE_RIGHT (primera y última no conectadas)
  uint8_t before = _entry;
  bool moved = false;

  if (buttons.pressed(Buttons::MOVE_LEFT) && _entry > 0) {
    _entry--;
    moved = true;
  }

  if (buttons.pressed(Buttons::MOVE_RIGHT) && _entry < NUM_ENTRIES - 1) {
    _entry++;
    moved = true;
  }

  if (moved) {
    sound.play(Sound::SFX_CLICK);
    _scroller.startSlide((_entry > before) ? 1 : -1);
    Serial.printf("Credits: opcion %u -> %u\n", before, _entry);
  }

  if (buttons.actionUpPressed()) _exit = true;
}

// ========================================================
// Componer el rol (slot 0) o el nombre (slot 1) en la tira
// ========================================================

void Credits::loadEntry(uint8_t slot) {
  uint8_t size = (slot == 0) ? TEXT_12x16 : TEXT_6x8;
  _scroller.compose(ROLE_NAME[_entry][slot], size);
}

// ========================================================
// Dibujar una banda (rol = 0, nombre = 1): compone su tira y
// vuelca su banda persistente. Ambas bandas deslizan a la vez
// con el mismo _slideX del Scroller, por lo que aparecen
// sincronizadas (mismo offset horizontal)
// ========================================================

void Credits::drawBand(uint8_t slot, int16_t y, uint16_t fgColor,
                       uint16_t bgColor) {
  loadEntry(slot);
  _scroller.blit(slot, y, fgColor, bgColor);
}

// ========================================================
// Dibujar
// ========================================================

void Credits::print() {
  int16_t w = display.getWidth();
  int16_t bodyTop = (int16_t)Config::Screen::BODY_TOP;
  int16_t roleH = display.getTextHeight(TEXT_12x16);
  int16_t roleY = bodyTop + (PIE_TOP - bodyTop - roleH) / 2;

  // Estáticos (solo al entrar, tras el clear() completo): título y el
  // cuadro blanco del rol. Se dibujan UNA sola vez; ya no se redibujan
  // en cada frame.
  if (_redraw) {
    display.clear();

    // Título de la ventana
    display.drawTextAligned("Credits", CENTER, TEXT_12x16, REGION_HEADER);

    // Rol: cuadro blanco de borde a borde (fijo), texto negro
    display.screen().fillRect(0, roleY - 1, w, roleH + 2, SSD1306_WHITE);

    _redraw = false;
  }

  // Dinámicos (cada frame): el rol y el nombre en sus bandas
  // persistente (la tira desliza y sobrescribe columna a columna)
  drawBand(0, roleY, SSD1306_BLACK, SSD1306_WHITE);
  drawBand(1, PIE_TOP + 1, SSD1306_WHITE, SSD1306_BLACK);
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool Credits::done() const {
  return _exit;
}

// ====================================================================================
// Fin
// ====================================================================================
