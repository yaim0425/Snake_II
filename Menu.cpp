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

  // Título (se redibuja después de limpiar la banda)
  _display.drawTextAligned("Snake II", CENTER, TEXT_12x16, REGION_HEADER);

  // Opciones en blanco (todas las visibles)
  for (uint8_t i = 0; i < OPTION_COUNT; i++) {

    int16_t y = _optionY[i];
    if (y + 16 <= BODY_TOP) continue;

    int8_t x = (_display.getWidth() - _display.getTextWidth(OPTION_TEXT[i], TEXT_12x16)) / 2;

    _display.drawText(OPTION_TEXT[i], x, y, TEXT_12x16);
  }

  // Cuadro de selección: fijo, de ancho completo
  _display.screen().fillRect(0, BOX_TOP, _display.getWidth(), BOX_HEIGHT, SSD1306_WHITE);

  // Texto seleccionado: invertido (negro) mientras cruza el cuadro
  int16_t selY = _optionY[_selected];
  if (selY < BOX_TOP + BOX_HEIGHT && selY + 16 > BOX_TOP) {
    int8_t selX = (_display.getWidth() - _display.getTextWidth(OPTION_TEXT[_selected], TEXT_12x16)) / 2;
    _display.drawTextInverted(OPTION_TEXT[_selected], selX, selY, TEXT_12x16);
  }

  // Limpiar la banda del Header y redibujar el título
  _display.screen().fillRect(0, 0, _display.getWidth(), BODY_TOP, SSD1306_BLACK);
  _display.drawTextAligned("Snake II", CENTER, TEXT_12x16, REGION_HEADER);

  // Limpiar dos filas libres sobre el pie + la fila del pie
  _display.screen().fillRect(0, FREE_TOP, _display.getWidth(), PIE_TOP + 8 - FREE_TOP, SSD1306_BLACK);

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