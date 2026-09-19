#include "Credits.h"

// ========================================================
// Entradas de crédito (navegables con MOVE_RIGHT/MOVE_LEFT)
// ========================================================
// Izquierda (0): Programador / opencode.ai
// Derecha (1) : Director / YAIM904

static const char* const ROLE_NAME[Credits::NUM_ENTRIES][2] = {
  { "Programador", "opencode.ai" },
  { "Director",    "YAIM904"     }
};

// ========================================================
// Geometría del resaltado (de lado a lado)
// ========================================================
// Cuadro blanco de ancho completo; el texto (tamaño 1) va invertido (negro)
// centrado dentro del cuadro, con 1 px arriba y 1 px abajo.
static constexpr int16_t HINT_TOP = 40;    // fila superior del cuadro
static constexpr uint8_t HINT_HEIGHT = 10; // 8 (texto) + 1 + 1

// ========================================================
// Constructor
// ========================================================

Credits::Credits(Display& display, Buttons& buttons)
  : _display(display),
    _buttons(buttons),
    _entry(0),
    _exit(false) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Credits::begin() {
  _entry = 0;
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

  s.fillRect(0, 0, _display.getWidth(), 48, SSD1306_BLACK);

  // Título de la ventana
  _display.drawTextAligned("Creditos", CENTER, TEXT_12x16, REGION_HEADER);

  // Rol (tamaño 2), centrado y bajado 4 px del tope del Body
  const char* role = ROLE_NAME[_entry][0];
  TextPos rPos = _display.getTextPos(role, CENTER_UP, TEXT_12x16, REGION_BODY);
  _display.drawText(role, rPos.x, rPos.y + 4, TEXT_12x16);

  // Nombre (tamaño 1) resaltado de lado a lado (cuadro blanco de ancho completo)
  const char* name = ROLE_NAME[_entry][1];
  int16_t w = _display.getWidth();
  int16_t y = HINT_TOP;
  s.fillRect(0, y, w, HINT_HEIGHT, SSD1306_WHITE);
  _display.drawTextInverted(name, (w - _display.getTextWidth(name, TEXT_6x8)) / 2,
                            y + 1, TEXT_6x8);
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool Credits::done() const {
  return _exit;
}