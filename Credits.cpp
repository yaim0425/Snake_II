#include "Credits.h"

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
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool Credits::done() const {
  return _exit;
}