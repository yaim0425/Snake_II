#include "SoundWindow.h"

// ========================================================
// Constructor
// ========================================================

SoundWindow::SoundWindow(Display& display, Buttons& buttons, Sound& sound)
  : _display(display),
    _buttons(buttons),
    _sound(sound),
    _exit(false) {}

// ========================================================
// Inicialización
// ========================================================

void SoundWindow::begin() {
  _exit = false;
}

// ========================================================
// Actualizar (lee botones)
// ========================================================

void SoundWindow::update() {
  _buttons.read();

  // Volver al menú (botón común de las ventanas)
  if (_buttons.actionUpPressed()) {
    _exit = true;
    return;
  }

  // Alternar sonido On/Off (ACTION_RIGHT)
  if (_buttons.actionRightPressed()) {
    bool wasOn = _sound.enabled();
    _sound.setEnabled(!wasOn);

    // Al encender se oye una confirmación (comprueba que hay audio)
    if (!wasOn) _sound.play(Sound::SFX_CONFIRM);
  }
}

// ========================================================
// Dibujar
// ========================================================

void SoundWindow::print() {

  // Título (Header, como el del menú)
  _display.screen().fillRect(0, 0, _display.getWidth(), 16, SSD1306_BLACK);
  _display.drawTextAligned("Sound", CENTER, TEXT_12x16, REGION_HEADER);

  // On / Off lado a lado, centrados en el Body: el estado activo va
  // resaltado (cuadro blanco + texto negro) y el otro texto plano.
  // El cuadro del resaltado centra el texto (+2*size px por lado en X
  // y +1 arriba/abajo en Y), por eso las posiciones se calculan desde
  // el centro de cada opción.
  const uint8_t size = TEXT_12x16;
  const int16_t half = (int16_t)(_display.getWidth() / 2);

  const int16_t onCx  = half - 20;
  const int16_t offCx = half + 20;

  const int16_t onW  = (int16_t)_display.getTextWidth("On", size);   // 24
  const int16_t offW = (int16_t)_display.getTextWidth("Off", size);  // 36

  const int16_t onX  = onCx - (int16_t)(onW / 2);
  const int16_t offX = offCx - (int16_t)(offW / 2);

  const int16_t y = (int16_t)(16 + ((_display.getHeight() - 16) -
                                    _display.getTextHeight(size)) / 2);

  if (_sound.enabled()) {
    _display.drawHighlight("On", onX, y, size);
    _display.drawText("Off", offX, y, size);
  } else {
    _display.drawText("On", onX, y, size);
    _display.drawHighlight("Off", offX, y, size);
  }
}

// ========================================================
// Salida
// ========================================================

bool SoundWindow::done() const {
  return _exit;
}