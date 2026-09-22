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

  // Estado actual, resaltado y centrado en el Body
  const char* state = _sound.enabled() ? "On" : "Off";
  _display.drawHighlightAligned(state, CENTER, TEXT_12x16, REGION_BODY);
}

// ========================================================
// Salida
// ========================================================

bool SoundWindow::done() const {
  return _exit;
}