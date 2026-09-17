#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>

// ========================================================
// Regiones de la pantalla
// ========================================================
//
// Header  : (0,0)   - (128,15)
// Body    : (0,16)  - (128,64)
// Full    : (0,0)   - (128,64)

enum Region {
  REGION_FULL = 0,
  REGION_HEADER,
  REGION_BODY
};

enum TextSize {
  TEXT_6x8 = 1,
  TEXT_12x16 = 2,
  TEXT_18x24 = 3
};

enum TextAlign {
  LEFT_UP,
  CENTER_UP,
  RIGHT_UP,
  CENTER_LEFT,
  CENTER,
  CENTER_RIGHT,
  LEFT_DOWN,
  CENTER_DOWN,
  RIGHT_DOWN
};

struct TextPos {
  int16_t x;
  int16_t y;
};

class Display {
public:
  // ========================================================
  // Constructor
  // ========================================================

  Display(uint8_t sda = 8, uint8_t scl = 9, uint8_t address = 0x3C,
          uint8_t width = 128, uint8_t height = 64, uint8_t cellSize = 8);


  // ========================================================
  // Inicialización
  // ========================================================

  void begin();


  // ========================================================
  // Pantalla
  // ========================================================

  void clear();
  void show();

  void drawPixel(uint8_t x, uint8_t y, bool black = false);

  // ========================================================
  // Texto
  // ========================================================

  // Posición de la esquina sup-izquierda del texto según alineación y región
  TextPos getTextPos(const char* text, TextAlign align, uint8_t size = 1,
                     Region region = REGION_FULL) const;

  // Imprimir texto en una posición píxel exacta
  void drawText(const char* text, int16_t x, int16_t y, uint8_t size = 1);

  // Imprimir texto según alineación dentro de la región indicada
  void drawTextAligned(const char* text, TextAlign align, uint8_t size = 1,
                       Region region = REGION_FULL);

  // Texto resaltado (cuadro blanco + texto invertido) en posición exacta
  // El cuadro rebasa al texto: +2*size px en X, +1 px en Y
  void drawHighlight(const char* text, int16_t x, int16_t y, uint8_t size = 1);

  // Texto resaltado según alineación dentro de la región indicada
  void drawHighlightAligned(const char* text, TextAlign align, uint8_t size = 1,
                            Region region = REGION_FULL);

  uint8_t getTextWidth(const char* text, uint8_t size = 1) const;
  uint8_t getTextHeight(uint8_t size = 1) const;

  // ========================================================
  // Información de pantalla
  // ========================================================

  uint8_t getWidth() const;
  uint8_t getHeight() const;

  uint8_t getCellSize() const;

  uint8_t getColumns() const;
  uint8_t getRows() const;

  // ========================================================
  // Acceso al OLED
  // ========================================================

  Adafruit_SSD1306& screen();

private:
  Adafruit_SSD1306* _screen;

  static constexpr uint8_t CHAR_W = 6;
  static constexpr uint8_t CHAR_H = 8;

  uint8_t _sda;
  uint8_t _scl;
  uint8_t _address;

  uint8_t _width;
  uint8_t _height;

  uint8_t _cellSize;

  uint8_t _columns;
  uint8_t _rows;
};

#endif