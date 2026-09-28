#ifndef MENU_SOUND_H
#define MENU_SOUND_H

#include "Timer.h"

// ========================================================
// MenuSound — ventana de edición del sonido
//
// Selector On/Off al que se entra desde la opción "Sound" del
// Menu (Menu::OPT_SOUND). Es una ventana más del Engine, como
// MenuCredits: el Menu deja de editar el sonido inline y se
// limita a devolver la opción elegida.
//
// No borra la pantalla: entra sobre el Menu ya dibujado y solo
// borra la banda de rombos (45..53), la misma franja que el
// selector inline ocupaba, para escribir en ella "ON"/"OFF" con
// su flecha. Al salir es el Menu el que restaura esa franja con
// sus rombos (Menu::restoreDiamondBand), así que el resto de la
// ventana (título, cuadro de la opción, pie) nunca se toca.
//
// El valor NO es propio: el estado real vive en el servicio
// global Sound (`setEnabled`/`enabled`). Esta ventana solo lleva
// el valor en edición y lo aplica al confirmar; por eso no hace
// falta guardarlo entre entradas.
// ========================================================

class MenuSound {
public:

  // ========================================================
  // Constructor (sin parámetros de servicios: usa los globales
  // Display, Buttons y Sound — Globals.h)
  // ========================================================

  MenuSound();

  // ========================================================
  // Inicialización (no dibuja nada: solo copia el valor actual
  // de Sound y ancla el parpadeo; el dibujo es de print())
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (consume los eventos de botones leídos en loop())
  // ========================================================

  void update();

  // ========================================================
  // Dibujar (borra la banda 45..53 y escribe el selector)
  // ========================================================

  void print();

  // ========================================================
  // Estado
  // ========================================================

  bool done() const;

private:

  // ========================================================
  // Geometría
  // ========================================================

  // La banda es la de los rombos del Menu (DIA_TOP/DIA_SIZE de
  // Menu): las filas 45..53. Ojo a la última: la 53 es la línea
  // separadora del pie (Config::Screen::FOOT_LINE), así que el erase
  // se lleva por delante esa fila y el selector escribe encima
  // (46..53). Por eso Menu::restoreDiamondBand() tiene que repintar
  // también la línea al volver.
  static constexpr int16_t SEL_TOP  = 45;
  static constexpr uint8_t SEL_SIZE = 8;

  // Palabra centrada y su flecha, dentro de la banda.
  static constexpr int16_t SEL_TEXT_TOP = SEL_TOP + 1;             // 46
  static constexpr int16_t SEL_TEXT_MID = SEL_TOP + SEL_SIZE / 2;  // 49
  static constexpr int16_t SEL_TEXT_BOT = SEL_TOP + SEL_SIZE;      // 53

  // Flecha: grosor horizontal, hueco con la palabra y período de
  // parpadeo (visible 75%, oculto el 25% inicial).
  static constexpr int16_t ARROW_GAP            = 6;    // hueco (px) entre el texto y la flecha
  static constexpr int16_t ARROW_W              = 6;    // grosor horizontal de la flecha (px)
  static constexpr uint32_t ARROW_BLINK_PERIOD  = 500;  // período del parpadeo de la flecha (ms)
  static constexpr uint8_t  ARROW_BLINK_OFF_PCT = 25;   // % del período en que la flecha está oculta

  // ========================================================
  // Métodos internos
  // ========================================================

  void drawSelector();

  // ========================================================
  // Estado
  // ========================================================

  bool _enabled;   // valor en edición (no aplicado hasta confirmar)
  bool _done;      // confirmó o canceló: el Engine vuelve al Menu

  Stopwatch _blink;  // ancla el parpadeo de la flecha
};

#endif

// ====================================================================================
// Fin
// ====================================================================================
