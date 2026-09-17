#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>

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

  TextPos getTextPos(const char* text, TextAlign align, uint8_t size = 1) const;
  void drawTextAligned(const char* text, TextAlign align, uint8_t size = 1);

  void drawButton(const char* text, TextAlign align, uint8_t size = 1, bool selected = false);

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