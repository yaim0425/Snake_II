#include "InfoWindow.h"

// ========================================================
// Constructor
// ========================================================

InfoWindow::InfoWindow(Display& display, Buttons& buttons)
  : _display(display),
    _buttons(buttons),
    _title(nullptr),
    _exit(false) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void InfoWindow::begin(const char* title) {
  _title = title;
  _exit = false;
}

// ========================================================
// Actualizar (lee botones y eventos)
// ========================================================

void InfoWindow::update() {
  _buttons.read();
  if (_buttons.actionUpPressed()) _exit = true;
}

// ========================================================
// Dibujar
// ========================================================

void InfoWindow::print() {
  Adafruit_SSD1306& s = _display.screen();

  s.fillRect(0, 0, _display.getWidth(), 48, SSD1306_BLACK);

  if (_title != nullptr)
    _display.drawTextAligned(_title, CENTER, TEXT_12x16, REGION_HEADER);

  _display.drawTextAligned("En desarrollo", CENTER, TEXT_6x8, REGION_BODY);
  _display.drawTextAligned("ACTION_UP: volver", CENTER_DOWN, TEXT_6x8,
                           REGION_BODY);
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool InfoWindow::done() const {
  return _exit;
}