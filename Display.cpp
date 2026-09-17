#include "Display.h"
#include <string.h>


// ========================================================
// Constructor
// ========================================================

Display::Display(uint8_t sda, uint8_t scl, uint8_t address,
                 uint8_t width, uint8_t height, uint8_t cellSize)
  : _sda(sda),
    _scl(scl),
    _width(width),
    _height(height),
    _screen(nullptr),
    _address(address),
    _cellSize(cellSize),
    _columns(width / cellSize),
    _rows(height / cellSize) {}


// ========================================================
// Inicialización
// ========================================================

void Display::begin() {
  Wire.begin(_sda, _scl);
  _screen = new Adafruit_SSD1306(_width, _height, &Wire, _address, -1);
  if (!_screen->begin(SSD1306_SWITCHCAPVCC, _address)) {
    delete _screen;
    _screen = nullptr;
    return;
  }

  _screen->setRotation(0);
  _screen->clearDisplay();
  _screen->display();
}


// ========================================================
// Pantalla
// ========================================================

void Display::clear() {
  if (_screen == nullptr) return;
  _screen->clearDisplay();
}

void Display::show() {
  if (_screen == nullptr) return;
  _screen->display();
}

void Display::drawPixel(uint8_t x, uint8_t y, bool black) {
  if (_screen == nullptr) return;
  if (black)
    _screen->drawPixel(x, y, SSD1306_BLACK);
  else
    _screen->drawPixel(x, y, SSD1306_WHITE);
}


// ========================================================
// Texto
// ========================================================

TextPos Display::getTextPos(const char* text, TextAlign align, uint8_t size) const {
  int16_t w = getTextWidth(text, size);
  int16_t h = getTextHeight(size);

  TextPos p = { 0, 0 };

  switch (align) {
    case LEFT_UP:
      break;
    case CENTER_UP:
      p.x = (getWidth() - w) / 2;
      break;
    case RIGHT_UP:
      p.x = getWidth() - w;
      break;
    case CENTER_LEFT:
      p.y = (getHeight() - h) / 2;
      break;
    case CENTER:
      p.x = (getWidth() - w) / 2;
      p.y = (getHeight() - h) / 2;
      break;
    case CENTER_RIGHT:
      p.x = getWidth() - w;
      p.y = (getHeight() - h) / 2;
      break;
    case LEFT_DOWN:
      p.y = getHeight() - h;
      break;
    case CENTER_DOWN:
      p.x = (getWidth() - w) / 2;
      p.y = getHeight() - h;
      break;
    case RIGHT_DOWN:
      p.x = getWidth() - w;
      p.y = getHeight() - h;
      break;
  }

  if (p.x < 0) p.x = 0;
  if (p.y < 0) p.y = 0;
  return p;
}

void Display::drawTextAligned(const char* text, TextAlign align, uint8_t size) {
  if (_screen == nullptr) return;
  TextPos p = getTextPos(text, align, size);
  _screen->setTextSize(size);
  _screen->setTextColor(SSD1306_WHITE);
  _screen->setCursor(p.x, p.y);
  _screen->print(text);
}

void Display::drawButton(const char* text, TextAlign align, uint8_t size, bool selected) {
  if (_screen == nullptr) return;

  TextPos p = getTextPos(text, align, size);
  uint16_t tw = getTextWidth(text, size);
  uint16_t th = getTextHeight(size);

  int16_t bx = p.x - 4;
  int16_t by = p.y - 4;
  uint16_t bw = tw + 6;
  uint16_t bh = th + 4;

  _screen->setTextSize(size);

  if (selected) {
    _screen->fillRoundRect(bx, by, bw, bh, 0, SSD1306_WHITE);
    _screen->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  } else {
    _screen->drawRoundRect(bx, by, bw, bh, 0, SSD1306_WHITE);
    _screen->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  }

  _screen->setCursor(p.x, p.y);
  _screen->print(text);
  _screen->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
}

uint8_t Display::getTextWidth(const char* text, uint8_t size) const {
  uint16_t w = strlen(text) * CHAR_W * size;
  return (w > 255) ? 255 : (uint8_t)w;
}

uint8_t Display::getTextHeight(uint8_t size) const {
  uint16_t h = CHAR_H * size;
  return (h > 255) ? 255 : (uint8_t)h;
}


// ========================================================
// Información de pantalla
// ========================================================

uint8_t Display::getWidth() const {
  return _width;
}

uint8_t Display::getHeight() const {
  return _height;
}


uint8_t Display::getCellSize() const {
  return _cellSize;
}


uint8_t Display::getColumns() const {
  return _columns;
}

uint8_t Display::getRows() const {
  return _rows;
}


// ========================================================
// Acceso al objeto Adafruit_SSD1306
// ========================================================

Adafruit_SSD1306& Display::screen() {
  return *_screen;
}
