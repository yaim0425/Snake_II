#include "Menu.h"

#include <stdio.h>

// ========================================================
// Opciones por defecto
// ========================================================

const char* const Menu::DEFAULT_OPTION_TEXT[Menu::DEFAULT_OPTIONS] = {
  "Nuevo",
  "Continuar",
  "Dificultad",
  "Sonido",
  "Creditos"
};

const char* Menu::optionText(int8_t index) const {
  return _optionTexts[index];
}

// ========================================================
// Constructor
// ========================================================

Menu::Menu(Display& display, Buttons& buttons, uint8_t topScore,
           const char* version)
  : _display(display),
    _buttons(buttons),
    _topScore(topScore),
    _version(version),
    _optionCount(DEFAULT_OPTIONS),
    _optionTexts(DEFAULT_OPTION_TEXT),
    _selected(OPC_NUEVO),
    _holdStart(0) {}

// ========================================================
// Inicialización
// ========================================================

void Menu::begin() {
  _holdStart = millis();
}

// ========================================================
// Opciones (cantidad variable)
// ========================================================

void Menu::setOptions(const char* const* texts, uint8_t count) {
  if (texts == nullptr) return;

  if (count < 1) count = 1;
  if (count > MAX_OPTIONS) count = MAX_OPTIONS;

  _optionTexts = texts;
  _optionCount = count;

  if (_selected >= (int8_t)count) _selected = count - 1;

  _holdStart = millis();
}

// ========================================================
// Actualizar
// ========================================================

void Menu::update() {
  _buttons.read();
  navigate();
}

// ========================================================
// Navegación
// ========================================================

void Menu::navigate() {
  int8_t before = _selected;
  bool moved = false;

  // Solo MOVE_LEFT y MOVE_RIGHT (primera y última no conectadas)
  if (_buttons.pressed(Buttons::MOVE_LEFT) && _selected > 0) {
    _selected--;
    moved = true;
  }

  if (_buttons.pressed(Buttons::MOVE_RIGHT) && _selected < _optionCount - 1) {
    _selected++;
    moved = true;
  }

  if (moved) {
    _holdStart = millis();
    Serial.printf("Menu: opcion %d -> %d\n", before, _selected);
  }
}

// ========================================================
// Posición centrada (esquina sup-izquierda) de un texto
// ========================================================

int16_t Menu::textCenterX(const char* text) const {
  return (int16_t)((_display.getWidth() -
                    _display.getTextWidth(text, TEXT_12x16)) / 2);
}

// ========================================================
// Rombos de posición
// ========================================================

void Menu::drawDiamonds() {
  uint8_t n = _optionCount;

  for (uint8_t i = 0; i < n; i++) {
    int16_t cx = (int16_t)((i + 1) * _display.getWidth()) / (n + 1);
    int16_t x = cx - DIA_SIZE / 2;

    if (i == _selected) {
      // Parpadeo al mantener seleccionado
      if (millis() - _holdStart >= BLINK_HOLD &&
          ((millis() / BLINK_TOGGLE) & 1) == 0)
        continue;  // fase apagada: no se dibuja

      // Rombo simétrico de 9 filas (45..53), como el alimento del juego:
      // punta superior 45, hombros 49, punta inferior 53 (visible)
      Adafruit_SSD1306& s = _display.screen();
      s.fillTriangle(x + 4, DIA_TOP, x + 8, DIA_TOP + DIA_SIZE / 2,
                     x + 4, DIA_TOP + DIA_SIZE, SSD1306_WHITE);
      s.fillTriangle(x + 4, DIA_TOP, x, DIA_TOP + DIA_SIZE / 2,
                     x + 4, DIA_TOP + DIA_SIZE, SSD1306_WHITE);
    } else {
      // Solo la punta (triángulo superior), bajada hasta las filas libres del
      // pie: base en la 53 (pegada a las 54..55), vértice en la 50
      int16_t baseY = DIA_TOP + DIA_SIZE;  // 53
      int16_t y = baseY - 3;               // 50
      _display.screen().fillTriangle(x + 4, y, x, baseY, x + 8, baseY,
                                     SSD1306_WHITE);
    }
  }
}

// ========================================================
// Dibujar
// ========================================================

void Menu::print() {

  // Cuadro de selección: fijo, de ancho completo
  _display.screen().fillRect(0, BOX_TOP, _display.getWidth(), BOX_HEIGHT,
                             SSD1306_WHITE);

  // Opción seleccionada, centrada y texto invertido (negro sobre el cuadro)
  _display.drawTextInverted(optionText(_selected),
                            textCenterX(optionText(_selected)), TEXT_SEL_TOP,
                            TEXT_12x16);

  // Limpiar la banda de rombos (45..53)
  _display.screen().fillRect(0, DIA_TOP, _display.getWidth(),
                             DIA_SIZE + 1, SSD1306_BLACK);
  drawDiamonds();

  // Limpiar la banda del Header y redibujar el título
  _display.screen().fillRect(0, 0, _display.getWidth(), BODY_TOP, SSD1306_BLACK);
  _display.drawTextAligned("Snake II", CENTER, TEXT_12x16, REGION_HEADER);

  // Limpiar las 2 filas libres (54..55) + la fila del pie (56..63)
  _display.screen().fillRect(0, 54, _display.getWidth(), 10, SSD1306_BLACK);

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