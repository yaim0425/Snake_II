#include "esp32-hal.h"
#include "Menu.h"

#include <stdio.h>

// ========================================================
// Opciones por defecto
// ========================================================

const char* const Menu::DEFAULT_OPTION_TEXT[Menu::DEFAULT_OPTIONS] = {
  "New",
  "Continue",
  "Difficulty",
  "Sound",
  "Credits"
};

const char* Menu::optionText(int8_t index) const {
  return _optionTexts[index];
}

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
    _dir(1),
    _slideX(0),
    _colAcc(0),
    _animLast(0),
    _holdStart(0),
    _chipBox(_display.getWidth(), _display.getTextHeight(TEXT_12x16)),
    _composer(_display.getWidth(), _display.getTextHeight(TEXT_12x16)) {}

// ========================================================
// Inicialización
// ========================================================

void Menu::begin() {
  _slideX = 0;
  _colAcc = 0;
  _holdStart = millis();
  loadOption(_selected);
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

  _slideX = 0;
  _colAcc = 0;
  _holdStart = millis();
  loadOption(_selected);
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
    loadOption(_selected);  // la opción se carga (centrada) antes de mostrarse
    startSlide((_selected > before) ? 1 : -1);
    _holdStart = millis();
    Serial.printf("Menu: opcion %d -> %d\n", before, _selected);
  }
}

// ========================================================
// Inicio de la transición lateral
// ========================================================

void Menu::startSlide(int8_t dir) {
  // La tira arranca completamente fuera de pantalla y entra desde un lado
  _dir = dir;
  _slideX = (_dir > 0) ? (int16_t)_display.getWidth() : -(int16_t)_display.getWidth();
  _colAcc = 0;
  _animLast = millis();
}

// ========================================================
// Animación: la tira avanza una columna por cada ANIM_TICK ms
// (acumulado por tiempo, constante aunque el loop sea lento)
// ========================================================

void Menu::animate() {
  if (_slideX == 0) return;

  uint32_t now = millis();
  uint32_t delta = now - _animLast;
  _animLast = now;

  _colAcc += delta;
  while (_colAcc >= ANIM_TICK) {
    _colAcc -= ANIM_TICK;
    if (_dir > 0) {              // entra por la derecha: se mueve hacia la izquierda
      if (_slideX > 0) _slideX--;
    } else {                     // entra por la izquierda: se mueve hacia la derecha
      if (_slideX < 0) _slideX++;
    }
  }
}

// ========================================================
// Cargar la opción en la matriz de 1 bit (128x16, centrada);
// se compone en el canvas auxiliar (sin tocar la banda actual)
// ========================================================

void Menu::loadOption(int8_t index) {
  const char* text = optionText(index);
  int16_t x0 = textCenterX(text);

  // Componer la tira: chip blanco de fondo + texto negro (invertido)
  _composer.fillScreen(CHIP_WHITE);
  _composer.setTextSize(TEXT_12x16);
  _composer.setTextColor(CHIP_TEXT, CHIP_WHITE);
  _composer.setCursor(x0, 0);
  _composer.print(text);

  // Copiar a la matriz de 1 bit: 1 = glifo (negro), 0 = espacio (blanco)
  for (uint8_t r = 0; r < STRIP_H; r++) {
    for (uint8_t c = 0; c < STRIP_W; c++) {
      uint8_t bit = (uint8_t)(1 << (c % 8));
      if (_composer.getPixel(c, r) == CHIP_TEXT)
        _strip[r][c / 8] |= bit;
      else
        _strip[r][c / 8] &= (uint8_t)~bit;
    }
  }
}

// ========================================================
// Pintar las columnas visibles de la tira sobre la banda
// persistente: cada columna de la tira (incluidos sus espacios
// blancos) sobrescribe lo que había, así la opción anterior se
// mantiene hasta ser borrada por la nueva
// ========================================================

void Menu::slideStrip() {
  for (uint8_t r = 0; r < STRIP_H; r++) {
    for (int16_t sx = 0; sx < (int16_t)_display.getWidth(); sx++) {
      int16_t sc = sx - _slideX;                // columna de la matriz (borde izq. = _slideX)
      if (sc < 0 || sc >= (int16_t)STRIP_W) continue;

      bool glyph = _strip[r][sc / 8] & (uint8_t)(1 << (sc % 8));
      _chipBox.drawPixel(sx, r, glyph ? CHIP_TEXT : CHIP_WHITE);
    }
  }
}

// ========================================================
// Volcar la banda persistente al cuadro de la pantalla
// (la banda guarda la opción anterior + la parcial de la nueva:
// sin borrado, la vieja queda hasta que la nueva la cubre)
// ========================================================

void Menu::blitBand() {
  Adafruit_SSD1306& s = _display.screen();

  for (uint8_t r = 0; r < STRIP_H; r++) {
    for (uint16_t c = 0; c < _display.getWidth(); c++) {
      uint8_t v = _chipBox.getPixel(c, r);
      s.drawPixel(c, TEXT_SEL_TOP + r,
                  (v == CHIP_TEXT) ? SSD1306_BLACK : SSD1306_WHITE);
    }
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
      // Parpadeo: visible 75% del período, oculto 25%
      if (millis() - _holdStart >= BLINK_HOLD &&
          (millis() % BLINK_PERIOD) < (uint32_t)BLINK_PERIOD * BLINK_OFF_PCT / 100)
        continue;  // fase oculta (25%): no se dibuja

      // Rombo simétrico de 9 filas (45..53), como el alimento del juego:
      // punta superior 45, hombros 49, punta inferior 53 (visible)
      Adafruit_SSD1306& s = _display.screen();
      s.fillTriangle(x + 4, DIA_TOP, x + 8, DIA_TOP + DIA_SIZE / 2,
                     x + 4, DIA_TOP + DIA_SIZE, SSD1306_WHITE);
      s.fillTriangle(x + 4, DIA_TOP, x, DIA_TOP + DIA_SIZE / 2,
                     x + 4, DIA_TOP + DIA_SIZE, SSD1306_WHITE);
    } else {
      // Solo la punta (triángulo superior), apoyada sobre la línea separadora:
      // base en la 53 (sobre la línea de la 54), vértice en la 50
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

  // Opciones: la banda persistente (lo que ya está en pantalla) se mantiene
  // intacta; la tira nueva desliza y sus columnas sobrescriben la banda hasta
  // reemplazarla por completo. La opción anterior permanece hasta ser borrada
  slideStrip();
  blitBand();

  // Limpiar la banda de rombos (45..53)
  _display.screen().fillRect(0, DIA_TOP, _display.getWidth(),
                             DIA_SIZE + 1, SSD1306_BLACK);
  drawDiamonds();

  // Limpiar la banda del Header y redibujar el título
  _display.screen().fillRect(0, 0, _display.getWidth(), BODY_TOP, SSD1306_BLACK);
  _display.drawTextAligned("Snake II", CENTER, TEXT_12x16, REGION_HEADER);

  // Limpiar la banda del pie: línea (54) + filas libres (55..56) + texto (57..63)
  _display.screen().fillRect(0, PIE_LINE_ROW, _display.getWidth(), 10, SSD1306_BLACK);

  // Línea horizontal de 1 px, a 2 px sobre el pie
  _display.screen().drawFastHLine(0, PIE_LINE_ROW, _display.getWidth(), SSD1306_WHITE);

  // Pie: Top y versión, bajados 1 px (fila 57)
  char buf[16];
  sprintf(buf, "Top: %u pts", _topScore);

  TextPos tPos = _display.getTextPos(buf, LEFT_DOWN, TEXT_6x8, REGION_BODY);
  _display.drawText(buf, tPos.x, PIE_TOP, TEXT_6x8);

  TextPos vPos = _display.getTextPos(_version, RIGHT_DOWN, TEXT_6x8, REGION_BODY);
  _display.drawText(_version, vPos.x, PIE_TOP, TEXT_6x8);
}

// ========================================================
// Accesos
// ========================================================

int8_t Menu::selected() const {
  return _selected;
}

int8_t Menu::confirm() const {
  if (_buttons.actionRightPressed()) return _selected;
  return -1;
}

void Menu::setTopScore(uint8_t value) {
  _topScore = value;
}