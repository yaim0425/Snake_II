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

// Valores del canvas del cuadro: 0 = transparente, 1 = texto (negro),
// 255 = chip (blanco)
static constexpr uint8_t CHIP_WHITE = 255;
static constexpr uint8_t CHIP_TEXT  = 1;

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
    _prev(-1),
    _dir(1),
    _slideX(0),
    _animLast(0),
    _holdStart(0),
    _chipBox(_display.getWidth(), BOX_HEIGHT) {}

// ========================================================
// Inicialización
// ========================================================

void Menu::begin() {
  _slideX = 0;
  _prev = -1;
  _animLast = millis();
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

  _prev = -1;
  _slideX = 0;
  _holdStart = millis();
}

// ========================================================
// Actualizar
// ========================================================

void Menu::update() {
  _buttons.read();
  navigate();
  animate();
}

// ========================================================
// Navegación
// ========================================================

void Menu::navigate() {
  int8_t before = _selected;
  bool moved = false;

  // Solo MOVE_UP y MOVE_DOWN (primera y última no conectadas)
  if (_buttons.pressed(Buttons::MOVE_UP) && _selected > 0) {
    _selected--;
    moved = true;
  }

  if (_buttons.pressed(Buttons::MOVE_DOWN) && _selected < _optionCount - 1) {
    _selected++;
    moved = true;
  }

  if (moved) {
    startSlide(before, (_selected > before) ? 1 : -1);
    _holdStart = millis();
    Serial.printf("Menu: opcion %d -> %d\n", before, _selected);
  }
}

// ========================================================
// Inicio de la transición lateral
// ========================================================

void Menu::startSlide(int8_t prev, int8_t dir) {
  // Descarta una transición en curso (salto al estado central)
  if (_prev >= 0 || _slideX != 0) {
    _prev = -1;
    _slideX = 0;
  }

  _prev = prev;
  _dir = dir;
  _slideX = _dir * SLIDE_DIST;  // la entrante arranca desde un lado
  _animLast = millis();
}

// ========================================================
// Animación del deslizamiento lateral
// ========================================================

void Menu::animate() {
  uint32_t now = millis();

  if (_prev < 0) return;
  if (now - _animLast < ANIM_TICK) return;
  _animLast = now;

  _slideX -= _dir * ANIM_STEP;

  if ((_dir > 0 && _slideX < 0) || (_dir < 0 && _slideX > 0))
    _slideX = 0;

  if (_slideX == 0)
    _prev = -1;
}

// ========================================================
// Posición centrada (esquina sup-izquierda) de un texto
// ========================================================

int16_t Menu::textCenterX(const char* text) const {
  return (int16_t)((_display.getWidth() -
                    _display.getTextWidth(text, TEXT_12x16)) / 2);
}

// ========================================================
// Dibujar una opción en el canvas del cuadro
// ========================================================

void Menu::paintOption(int8_t index, int16_t offX) {
  const char* text = optionText(index);
  uint8_t w = _display.getTextWidth(text, TEXT_12x16);
  int16_t x = textCenterX(text) + offX;

  // Totalmente fuera de la pantalla
  if (x + w < 0 || x >= _display.getWidth()) return;

  // Chip blanco (rebasa +2 px en X) + texto invertido (negro)
  _chipBox.fillRect(x - 2, 0, w + 4, BOX_HEIGHT, CHIP_WHITE);
  _chipBox.setTextSize(TEXT_12x16);
  _chipBox.setTextColor(CHIP_TEXT, CHIP_WHITE);
  _chipBox.setCursor(x, TEXT_SEL_TOP - BOX_TOP);
  _chipBox.print(text);
}

// ========================================================
// Volcar el canvas del cuadro a la pantalla
// ========================================================

void Menu::blitChip() {
  Adafruit_SSD1306& s = _display.screen();

  for (uint8_t py = 0; py < BOX_HEIGHT; py++) {
    for (uint8_t px = 0; px < _display.getWidth(); px++) {
      uint8_t v = _chipBox.getPixel(px, py);
      if (v == CHIP_WHITE)
        s.drawPixel(px, BOX_TOP + py, SSD1306_WHITE);
      else if (v == CHIP_TEXT)
        s.drawPixel(px, BOX_TOP + py, SSD1306_BLACK);
    }
  }
}

// ========================================================
// Rombos de posición
// ========================================================

void Menu::drawDiamonds() {
  uint8_t n = _optionCount;

  for (uint8_t i = 0; i < n; i++) {
    int16_t cx = (int16_t)((i + 1) * _display.getWidth()) / (n + 1);
    int16_t x = cx - DIA_SIZE / 2;

    int16_t y = DIA_TOP;
    if (i == _selected) {
      y = DIA_TOP - DIA_RISE;

      // Parpadeo al mantener seleccionado
      if (millis() - _holdStart >= BLINK_HOLD &&
          ((millis() / BLINK_TOGGLE) & 1) == 0)
        continue;  // fase apagada: no se dibuja
    }

    _display.screen().fillTriangle(x + 4, y, x + 8, y + 4,
                                   x + 4, y + 8, SSD1306_WHITE);
    _display.screen().fillTriangle(x + 4, y, x, y + 4,
                                   x + 4, y + 8, SSD1306_WHITE);
  }
}

// ========================================================
// Dibujar
// ========================================================

void Menu::print() {

  // Cuadro de selección: fijo, de ancho completo
  _display.screen().fillRect(0, BOX_TOP, _display.getWidth(), BOX_HEIGHT,
                             SSD1306_WHITE);

  // Opciones deslizantes (la entrante va al centro, la saliente se va)
  _chipBox.fillScreen(0);
  paintOption(_selected, _slideX);
  if (_prev >= 0)
    paintOption(_prev, _slideX - _dir * SLIDE_DIST);
  blitChip();

  // Limpiar la banda de rombos (45..53): 1 px libre + 8 px de rombo
  _display.screen().fillRect(0, DIA_TOP - DIA_RISE, _display.getWidth(),
                             DIA_SIZE + DIA_RISE, SSD1306_BLACK);
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