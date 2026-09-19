#include "Credits.h"

// ========================================================
// Geometría del pie (coincide con el menú principal)
// ========================================================
// Línea separadora en la 54, a 2 px sobre el pie (que empieza en la 57).
static constexpr int16_t PIE_LINE_ROW = 54;

// ========================================================
// Constructor
// ========================================================

Credits::Credits(Display& display, Buttons& buttons, const char* version)
  : _display(display),
    _buttons(buttons),
    _version(version),
    _exit(false) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void Credits::begin() {
  _exit = false;
}

// ========================================================
// Actualizar (lee botones y eventos)
// ========================================================

void Credits::update() {
  _buttons.read();
  if (_buttons.actionUpPressed()) _exit = true;
}

// ========================================================
// Dibujar
// ========================================================

void Credits::print() {
  Adafruit_SSD1306& s = _display.screen();

  s.fillRect(0, 0, _display.getWidth(), 48, SSD1306_BLACK);

  _display.drawTextAligned("Creditos", CENTER, TEXT_12x16, REGION_HEADER);

  // Título y versión bajados 4 px
  TextPos tPos = _display.getTextPos("Snake II", CENTER_UP, TEXT_12x16, REGION_BODY);
  _display.drawText("Snake II", tPos.x, tPos.y + 4, TEXT_12x16);

  TextPos vPos = _display.getTextPos(_version, CENTER, TEXT_6x8, REGION_BODY);
  _display.drawText(_version, vPos.x, vPos.y + 4, TEXT_6x8);

  // Limpiar la banda del pie y dibujar la línea separadora de 1 px
  s.fillRect(0, PIE_LINE_ROW, _display.getWidth(), 10, SSD1306_BLACK);
  s.drawFastHLine(0, PIE_LINE_ROW, _display.getWidth(), SSD1306_WHITE);
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool Credits::done() const {
  return _exit;
}