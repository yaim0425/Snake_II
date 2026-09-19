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

// ========================================================
// Geometría
// ========================================================
// El rol (tamaño 2) va SELECCIONADO (cuadro blanco + texto invertido),
// centrado en el alto restante del Body: de la fila 16 (BODY_TOP) hasta la
// fila anterior al pie (PIE_TOP).
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
    _exit(false) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Credits::begin() {
  _entry = 1;  // entrada central (Snake II / v0.1)
  _exit = false;
}

// ========================================================
// Actualizar (lee botones, navega entre entradas)
// ========================================================

void Credits::update() {
  _buttons.read();

  // Navegación entre entradas (sin conectar extremos)
  if (_buttons.moveLeftPressed() && _entry > 0) {
    _entry--;
    Serial.println("Credits: izquierda");
  }

  if (_buttons.moveRightPressed() && _entry < NUM_ENTRIES - 1) {
    _entry++;
    Serial.println("Credits: derecha");
  }

  if (_buttons.actionUpPressed()) _exit = true;
}

// ========================================================
// Dibujar
// ========================================================

void Credits::print() {
  Adafruit_SSD1306& s = _display.screen();

  s.fillRect(0, 0, s.width(), s.height(), SSD1306_BLACK);

  // Título de la ventana
  _display.drawTextAligned("Creditos", CENTER, TEXT_12x16, REGION_HEADER);

  // Rol (tamaño 2) seleccionado, centrado en el alto restante [BODY_TOP, PIE_TOP)
  const char* role = ROLE_NAME[_entry][0];
  int16_t w = _display.getWidth();
  int16_t roleH = _display.getTextHeight(TEXT_12x16);
  int16_t roleY = BODY_TOP + (PIE_TOP - BODY_TOP - roleH) / 2;
  uint8_t roleX = (w - _display.getTextWidth(role, TEXT_12x16)) / 2;
  _display.screen().fillRect(0, roleY - 1, w, roleH + 2, SSD1306_WHITE);
  _display.drawTextInverted(role, roleX, roleY, TEXT_12x16);

  // Nombre (tamaño 1) des-seleccionado (plano), centrado en el pie del Body
  const char* name = ROLE_NAME[_entry][1];
  uint8_t nameY = PIE_TOP + 1;  // banda de 10px, texto de 8px, centrado
  uint8_t nameX = (w - _display.getTextWidth(name, TEXT_6x8)) / 2;
  _display.drawText(name, nameX, nameY, TEXT_6x8);
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool Credits::done() const {
  return _exit;
}