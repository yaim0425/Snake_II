#include "InfoWindow.h"

// ========================================================
// Geometría del pie (coincide con el menú principal)
// ========================================================
// Línea separadora en la 54, a 2 px sobre el pie (que empieza en la 57).
static constexpr int16_t PIE_LINE_ROW = 54;

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

  _display.drawTextAligned("In development", CENTER, TEXT_6x8, REGION_BODY);

  // Limpiar la banda del pie y dibujar la línea separadora de 1 px
  s.fillRect(0, PIE_LINE_ROW, _display.getWidth(), 10, SSD1306_BLACK);
  s.drawFastHLine(0, PIE_LINE_ROW, _display.getWidth(), SSD1306_WHITE);
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool InfoWindow::done() const {
  return _exit;
}