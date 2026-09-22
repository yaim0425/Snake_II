#include "SoundWindow.h"

// ========================================================
// Constructor
// ========================================================

SoundWindow::SoundWindow(Display& display, Buttons& buttons, Sound& sound)
  : _display(display),
    _buttons(buttons),
    _sound(sound),
    _exit(false),
    _selected(0) {}

// ========================================================
// Inicialización (al entrar, la selección refleja el estado)
// ========================================================

void SoundWindow::begin() {
  _exit = false;
  _selected = _sound.enabled() ? 0 : 1;  // 0 = On, 1 = Off
}

// ========================================================
// Actualizar (lee botones y navega como el menú)
// ========================================================

void SoundWindow::update() {
  _buttons.read();

  // Volver al menú (botón común de las ventanas)
  if (_buttons.actionUpPressed()) {
    _exit = true;
    return;
  }

  // Navegar como el menú: MOVE_LEFT/MOVE_RIGHT cambian la selección
  // (primera y última no conectadas, sin ciclo)
  if (_buttons.pressed(Buttons::MOVE_LEFT) && _selected > 0) {
    _selected--;
    _sound.play(Sound::SFX_CLICK);
  }

  if (_buttons.pressed(Buttons::MOVE_RIGHT) && _selected < 1) {
    _selected++;
    _sound.play(Sound::SFX_CLICK);
  }

  // Confirmar la opción seleccionada (ACTION_RIGHT, como el menú)
  if (_buttons.actionRightPressed()) {
    _sound.setEnabled(_selected == 0);

    // Al dejar el sonido encendido se oye la confirmación
    // (comprueba que hay audio)
    if (_selected == 0) _sound.play(Sound::SFX_CONFIRM);
  }
}

// ========================================================
// Dibujar
// ========================================================

void SoundWindow::print() {

  // Título (Header, como el del menú)
  _display.screen().fillRect(0, 0, _display.getWidth(), 16, SSD1306_BLACK);
  _display.drawTextAligned("Sound", CENTER, TEXT_12x16, REGION_HEADER);

  // On / Off lado a lado, centrados en el Body: la opción seleccionada
  // (como en el menú) va resaltada (cuadro blanco + texto negro) y la
  // otra texto plano. El cuadro del resaltado centra el texto (+2*size px
  // por lado en X y +1 arriba/abajo en Y), por eso las posiciones se
  // calculan desde el centro de cada opción.
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

  // La opción seleccionada (como el menú) va resaltada; la otra plana
  if (_selected == 0) {
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