#include "Menu.h"

#include <stdio.h>

// ========================================================
// Opciones
// ========================================================

const char* const Menu::OPTION_TEXT[Menu::OPTION_COUNT] = {
  "Nuevo",
  "Continuar",
  "Dificultad",
  "Sonido",
  "Creditos"
};

// ========================================================
// Constructor
// ========================================================

Menu::Menu(Display& display, Buttons& buttons, uint8_t topScore,
           const char* version)
  : _display(display),
    _buttons(buttons),
    _topScore(topScore),
    _version(version),
    _selected(OPC_NUEVO),
    _animLast(0) {
  for (uint8_t i = 0; i < OPTION_COUNT; i++)
    _optionY[i] = 0;
}

// ========================================================
// Inicialización
// ========================================================

void Menu::begin() {
  for (uint8_t i = 0; i < OPTION_COUNT; i++)
    _optionY[i] = optionTarget(i);
  _animLast = millis();
}

// ========================================================
// Actualizar
// ========================================================

void Menu::update() {

  _buttons.read();

  // Navegación (primera y última no conectadas)
  if (_buttons.pressed(Buttons::MOVE_UP) || _buttons.pressed(Buttons::ACTION_UP))
    if (_selected > 0) _selected--;
  if (_buttons.pressed(Buttons::MOVE_DOWN) || _buttons.pressed(Buttons::ACTION_DOWN))
    if (_selected < OPTION_COUNT - 1) _selected++;

  // Animar
  animate();
}

// ========================================================
// Animación de las opciones
// ========================================================

void Menu::animate() {

  uint32_t now = millis();
  if (now - _animLast < ANIM_TICK) return;
  _animLast = now;

  for (uint8_t i = 0; i < OPTION_COUNT; i++) {

    int16_t t = optionTarget(i);

    if (_optionY[i] < t)
      _optionY[i]++;
    else if (_optionY[i] > t)
      _optionY[i]--;
  }
}

// Posición objetivo (parte superior del texto) de la opción i
int16_t Menu::optionTarget(int8_t i) const {
  return TEXT_SEL_TOP + (i - _selected) * OPTION_STEP;
}

// ========================================================
// Dibujar
// ========================================================

void Menu::print() {

  // Título
  _display.drawTextAligned("Snake II", CENTER, TEXT_12x16, REGION_HEADER);

  // Opciones
  for (uint8_t i = 0; i < OPTION_COUNT; i++) {

    int16_t y = _optionY[i];

    // No dibujar si la opción sale por arriba del Body
    if (y < BODY_TOP) continue;

    int8_t x = (_display.getWidth() - _display.getTextWidth(OPTION_TEXT[i], TEXT_12x16)) / 2;

    if (i == _selected)
      _display.drawHighlight(OPTION_TEXT[i], x, y, TEXT_12x16);
    else
      _display.drawText(OPTION_TEXT[i], x, y, TEXT_12x16);
  }

  // Limpiar la fila del pie antes de escribir (para no mezclarla con opciones)
  _display.screen().fillRect(0, 56, _display.getWidth(), 8, SSD1306_BLACK);

  // Pie: Top y versión
  char buf[16];
  sprintf(buf, "Top: %u pts", _topScore);

  _display.drawTextAligned(buf, LEFT_DOWN, TEXT_6x8, REGION_BODY);
  _display.drawTextAligned(_version, RIGHT_DOWN, TEXT_6x8, REGION_BODY);
}

// ========================================================
// Accesos
// ========================================================

int8_t Menu::selected() const {
  return _selected;
}

void Menu::setTopScore(uint8_t value) {
  _topScore = value;
}