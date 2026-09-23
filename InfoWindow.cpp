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
    _exit(false),
    _redraw(true) {}

// ========================================================
// Inicialización (al entrar en la ventana)
// ========================================================

void InfoWindow::begin(const char* title) {
  _title = title;
  _exit = false;
  _redraw = true;
}

// ========================================================
// Actualizar (consume los eventos de botones leídos en loop())
// ========================================================

void InfoWindow::update() {
  if (_buttons.actionUpPressed()) _exit = true;
}

// ========================================================
// Dibujar (todo es estático: solo se dibuja al entrar)
// ========================================================

void InfoWindow::print() {
  if (!_redraw) return;

  _display.clear();

  if (_title != nullptr)
    _display.drawTextAligned(_title, CENTER, TEXT_12x16, REGION_HEADER);

  _display.drawTextAligned("In development", CENTER, TEXT_6x8, REGION_BODY);

  // Línea separadora del pie, de 1 px
  _display.screen().drawFastHLine(0, PIE_LINE_ROW, _display.getWidth(),
                                  SSD1306_WHITE);

  _redraw = false;
}

// ========================================================
// Salida (true = ACTION_UP pidió volver al menú)
// ========================================================

bool InfoWindow::done() const {
  return _exit;
}

// ====================================================================================
// Fin
// ====================================================================================