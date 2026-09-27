#pragma once

#include <Arduino.h>

// ========================================================
// Config — constantes compartidas por varias clases o propias
// del hardware/placa. Solo lo que es verdaderamente compartido;
// el resto vive como `static constexpr` dentro de su clase
// (p. ej. ANIM_TICK en Scroller, NEW_BEST_SIGN_MS en Game).
//
//   - Pin       : pines de la placa (botones, buzzer, I2C).
//   - Screen    : geometría del OLED y sus regiones.
//   - Difficulty: límites de la dificultad (compartidos por
//                 Menu y Game).
//   - Version   : versión y fecha de release del firmware
//                 (compartidas por las ventanas que las
//                 muestren).
//
// Al usar constantes con tipo, ámbito (namespace) y constexpr
// no chocan con nombres de librerías ni del core ESP32; por
// eso aquí no hay `#define` para valores (solo serviría para
// lo que necesita el preprocesador, p. ej. #ifdef DEBUG).
// ========================================================

namespace Config {

// ========================================================
// Pines
// ========================================================

namespace Pin {

// Botones (orden del enum Buttons::Button: MOVE_UP..ACTION_LEFT)
constexpr int8_t BUTTONS[8] = {
  2, 1, 42, 41,   // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  38, 40, 39, 47  // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};

constexpr uint8_t BUZZER = 14;   // zumbador
constexpr uint8_t OLED_SDA = 8;  // I2C: datos
constexpr uint8_t OLED_SCL = 9;  // I2C: reloj
}

// ========================================================
// Pantalla OLED (SSD1306)
// ========================================================

namespace Screen {

constexpr uint8_t WIDTH = 128;     // px de ancho
constexpr uint8_t HEIGHT = 64;     // px de alto
constexpr uint8_t CELL = 8;        // px por celda de la rejilla
constexpr uint8_t ADDRESS = 0x3C;  // dirección I2C

// Regiones: Header (0..15) y Body (16..63)
constexpr uint8_t HEADER_TOP = 0;
constexpr uint8_t HEADER_H = 16;

constexpr uint8_t BODY_TOP = 16;
constexpr uint8_t BODY_H = 48;

constexpr uint8_t FOOT_H = 7;
constexpr uint8_t FOOT_TOP = HEIGHT - FOOT_H;             // Punto Y del texto del pie
constexpr uint8_t FOOT_LINE = HEIGHT - (FOOT_H + 1 + 1);  // línea separadora del pie + 2 px de margen;

}

// ========================================================
// Dificultad (nivel 1..10), compartida por Menu y Game
// ========================================================

namespace Difficulty {

constexpr uint8_t MIN_LEVEL = 1;
constexpr uint8_t MAX_LEVEL = 10;
constexpr uint8_t DEFAULT_LEVEL = 5;
}

namespace Version {
constexpr char* NAME = "Snake II";
constexpr char* VERSION = "v1.0.0";
constexpr char* RELEASE_DATE = "2026/12/31";
}
}

// ====================================================================================
// Fin
// ====================================================================================