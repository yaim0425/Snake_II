#ifndef SCROLLER_H
#define SCROLLER_H

#include "Config.h"
#include "Timer.h"

// ========================================================
// Scroller — animación "scroller de 1 bit"
//
// Compone un texto en un array de int8_t (cada byte = 1
// columna de 8 px: bit 0 = fila 0, bit 7 = fila 7) y lo
// desliza lateralmente. Una sola banda por instancia.
//
// La franja es siempre fondo blanco y texto negro (sin
// parámetros de color). El texto se compone centrado en un
// canvas auxiliar y luego se extraen las columnas.
//
// Usa la Display global (Globals.h) para el ancho de la
// franja y el cálculo del límite de caracteres.
// ========================================================

class Scroller {
public:
  // ========================================================
  // Constructor (sin parámetros: usa la Display global)
  // ========================================================

  Scroller();

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Componer el texto en la franja (texto centrado)
  //
  // Calcula el límite de caracteres según el tamaño
  // (ancho / (6 * size)) y trunca silenciosamente si el
  // texto excede el límite. El texto se compone en un
  // canvas auxiliar y luego se extraen las columnas.
  // ========================================================

  void setTexto(const char* text, uint8_t height = Config::Screen::BODY_TOP + 3, uint8_t size = 2);

  // ========================================================
  // Animación lateral (arranca desde el borde, ±ancho)
  // ========================================================

  void startSlide(int8_t dir);

  // ========================================================
  // Avanzar la animación (1 px por ANIM_TICK ms)
  // ========================================================

  void animate();

  // ========================================================
  // Forzar el volcado en el próximo frame (la ventana lo
  // llama tras su display.clear())
  // ========================================================

  void redraw();

  // ========================================================
  // Volcar la franja a la pantalla en la fila y (fondo
  // blanco, texto negro). Es un no-op si no hay nada nuevo
  // que volcar (dirty flag a 0).
  // ========================================================

  void blit(int16_t y);

private:
  // ========================================================
  // Geometría y tiempos
  // ========================================================

  // Ancho de la franja (ancho de pantalla)
  static constexpr uint8_t STRIP_W = Config::Screen::WIDTH;

  // Alto máximo de la franja (texto 18x24)
  static constexpr uint8_t STRIP_H = 24;

  // Avance de 1 px cada ANIM_TICK ms (acumulador por tiempo)
  static constexpr uint32_t ANIM_TICK = 4;

  // ========================================================
  // Estado interno
  // ========================================================

  int8_t _strip[STRIP_H / 8][STRIP_W];  // franja: cada byte = 1 columna de 8 px
  uint8_t _height;                      // alto actual de la franja en px
  int8_t _dir;                          // +1 entra por la derecha, -1 por la izquierda
  int16_t _slideX;                      // borde izquierdo de la franja en pantalla
  Ticker _ticker;                       // avance de 1 px por ANIM_TICK ms
  bool _dirty;                          // hay algo nuevo que volcar a pantalla
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
