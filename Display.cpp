#include "Display.h"
#include <string.h>


// ========================================================
// Constructor
// ========================================================

Display::Display(uint8_t sda, uint8_t scl, uint8_t address,
                 uint8_t width, uint8_t height, uint8_t cellSize)
  : _screen(nullptr),
    _sda(sda),
    _scl(scl),
    _address(address),
    _width(width),
    _height(height),
    _cellSize(cellSize),
    _columns(width / cellSize),
    _rows(height / cellSize) {}


// ========================================================
// Inicialización
// ========================================================

void Display::begin() {
  if (_screen != nullptr) return;  // idempotente: ya inicializado (evita filtrar memoria)

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
  _screen->drawPixel(x, y, black ? SSD1306_BLACK : SSD1306_WHITE);
}

void Display::fillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, bool black) {
  if (_screen == nullptr) return;
  _screen->fillRect(x, y, w, h, black ? SSD1306_BLACK : SSD1306_WHITE);
}

void Display::fillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool black) {
  if (_screen == nullptr) return;
  _screen->fillTriangle(x0, y0, x1, y1, x2, y2, black ? SSD1306_BLACK : SSD1306_WHITE);
}

// ========================================================
// Triángulo relleno
// ========================================================

// Ordena dos vértices de arriba (menor y) a abajo (mayor y) sin perder
// el otro eje: es el paso previo a recorrer el triángulo por filas
static void sortByY(int16_t* xa, int16_t* ya, int16_t* xb, int16_t* yb) {
  if (*ya <= *yb) return;
  int16_t t;

  t = *xa;
  *xa = *xb;
  *xb = t;
  t = *ya;
  *ya = *yb;
  *yb = t;
}

// x del corte del lado (xa,ya)-(xb,yb) con la fila y. Solo se llama
// con y dentro del intervalo del lado, así que la división no puede
// ser por cero (si el lado es horizontal, y == ya == yb y devuelve xa).
// Redondea al entero más cercano para que el error no caiga siempre
// hacia el mismo lado: con truncar, el borde izquierdo se queda corto
// 1 px mientras el derecho se pasa.
static int16_t edgeAt(int16_t xa, int16_t ya, int16_t xb, int16_t yb, int16_t y) {
  if (y == yb) return xb;

  int32_t num = (int32_t)(xb - xa) * (y - ya);
  int32_t den = yb - ya;  // > 0: los vertices van ordenados de arriba abajo

  if (num >= 0) return (int16_t)(xa + (num + den / 2) / den);
  return (int16_t)(xa - (-num + den / 2) / den);
}


// ========================================================
// Regiones
// ========================================================

// Devuelve la posición vertical inicial y la altura de la región
static void regionBounds(Region region, uint8_t height, uint8_t* top, uint8_t* regionHeight) {
  *top = 0;
  *regionHeight = height;

  if (region == REGION_HEADER) {
    *regionHeight = Config::Screen::HEADER_H;
  } else if (region == REGION_BODY) {
    *top = Config::Screen::BODY_TOP;
    *regionHeight = height - Config::Screen::BODY_TOP;
  }
}


// ========================================================
// Texto
// ========================================================

TextPos Display::getTextPos(const char* text, TextAlign align, uint8_t size,
                            Region region) const {
  uint8_t top = 0;
  uint8_t regionHeight = 0;
  regionBounds(region, _height, &top, &regionHeight);

  int16_t w = getTextWidth(text, size);
  int16_t h = getTextHeight(size);

  TextPos p = { 0, top };

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
      p.y = top + (regionHeight - h) / 2;
      break;
    case CENTER:
      p.x = (getWidth() - w) / 2;
      p.y = top + (regionHeight - h) / 2;
      break;
    case CENTER_RIGHT:
      p.x = getWidth() - w;
      p.y = top + (regionHeight - h) / 2;
      break;
    case LEFT_DOWN:
      p.y = top + regionHeight - h;
      break;
    case CENTER_DOWN:
      p.x = (getWidth() - w) / 2;
      p.y = top + regionHeight - h;
      break;
    case RIGHT_DOWN:
      p.x = getWidth() - w;
      p.y = top + regionHeight - h;
      break;
  }

  if (p.x < 0) p.x = 0;
  if (p.y < 0) p.y = 0;
  return p;
}

void Display::drawText(const char* text, int16_t x, int16_t y, uint8_t size) {
  if (_screen == nullptr) return;
  _screen->setTextSize(size);
  _screen->setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  _screen->setCursor(x, y);
  _screen->print(text);
}

void Display::drawTextInverted(const char* text, int16_t x, int16_t y, uint8_t size) {
  if (_screen == nullptr) return;
  _screen->setTextSize(size);
  _screen->setTextColor(SSD1306_BLACK);
  _screen->setCursor(x, y);
  _screen->print(text);
}

void Display::drawTextAligned(const char* text, TextAlign align, uint8_t size,
                              Region region) {
  if (_screen == nullptr) return;
  TextPos p = getTextPos(text, align, size, region);
  drawText(text, p.x, p.y, size);
}

void Display::drawHighlight(const char* text, int16_t x, int16_t y, uint8_t size) {
  if (_screen == nullptr) return;

  int16_t bx = x - size;
  int16_t by = y - 1;
  uint16_t bw = getTextWidth(text, size) + 2 * size;
  uint16_t bh = getTextHeight(size) + 2;

  _screen->fillRoundRect(bx, by, bw, bh, 0, SSD1306_WHITE);

  _screen->setTextSize(size);
  _screen->setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  _screen->setCursor(x, y);
  _screen->print(text);
}

void Display::drawHighlightAligned(const char* text, TextAlign align, uint8_t size,
                                   Region region) {
  if (_screen == nullptr) return;
  TextPos p = getTextPos(text, align, size, region);
  drawHighlight(text, p.x, p.y, size);
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
//
// Precondición: display.begin() se llamó antes (construye y pone
// en funcionamiento el objeto OLED).
//
// Si begin() falló (pantalla ausente/no responde por I2C) _screen
// es nullptr y no debe desreferenciarse: se devuelve un fallback
// seguro —un OLED "mudo" en RAM, inicializado la primera vez para
// reservar su buffer—, así ningún dibujo posterior genera
// comportamiento indefinido. isReady() indica si hay pantalla real.
// ========================================================

Adafruit_SSD1306& Display::screen() {
  if (_screen != nullptr) return *_screen;

  static Adafruit_SSD1306 dummy(_width, _height, &Wire);
  static bool dummyReady = false;
  if (!dummyReady) {
    dummy.begin(SSD1306_SWITCHCAPVCC, _address);  // reserva su buffer en RAM
    dummyReady = true;
  }
  return dummy;
}

// ========================================================
// ¿La pantalla quedó operativa? (false si begin() falló)
// ========================================================

bool Display::isReady() const {
  return _screen != nullptr;
}

// ====================================================================================
// Fin
// ====================================================================================