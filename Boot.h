#ifndef BOOT_H
#define BOOT_H

#include "Config.h"
#include "Timer.h"

// ========================================================
// Boot — animación de arranque (franjas verticales)
//
// La pantalla se divide en dos bandas completas: TITULO
// (filas 0..15) y CUERPO (filas 16..63). Cada banda se llena
// con líneas verticales de 3 px de grosor, separadas 8 px,
// que se desplazan:
//   - TITULO: de izquierda a derecha.
//   - CUERPO: de derecha a izquierda.
//
// Las líneas avanzan 1 px cada ANIM_TICK ms (con acumulador
// por tiempo, igual que el menú). Al llegar al borde derecho,
// la línea se parte en dos tramos (derecho + izquierdo) para
// no cortarse en seco. La animación dura en total TOTAL_MS;
// cualquier botón la termina antes. Al terminar, done() devuelve
// true y Engine pasa al menú.
// ========================================================

class Boot {
public:
  // ========================================================
  // Constructor (sin parámetros: usa los servicios globales
  // Display y Buttons, declarados en Globals.h)
  // ========================================================

  Boot();

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (consume eventos de botones ya leídos y avanza el desplazamiento)
  // ========================================================

  void update();

  // ========================================================
  // Dibujar (bandas de líneas verticales)
  // ========================================================

  void print();

  // ========================================================
  // Salida (true = la animación terminó)
  // ========================================================

  bool done() const;

private:
  // ========================================================
  // Geometría
  // ========================================================

  // Grosor de cada línea vertical
  static constexpr uint8_t BAR_W = 3;

  // Separación entre líneas (el desplazamiento repite cada traslación)
  static constexpr uint8_t BAR_SPACING = 8;

  // Bandas completas (regiones de la pantalla en Config::Screen):
  // TITULO = Header (Config::Screen::HEADER_TOP/H), CUERPO = Body
  // (Config::Screen::BODY_TOP/H).

  // ========================================================
  // Tiempo
  // ========================================================

  // Avance de 1 px cada ANIM_TICK ms
  static constexpr uint32_t ANIM_TICK = 30;

  // Duración total de la animación
  static constexpr uint32_t TOTAL_MS = 4000;

  // ========================================================
  // Estado interno
  // ========================================================

  uint8_t _shift;       // desplazamiento actual (0..BAR_SPACING-1)
  uint8_t _prevShift;   // desplazamiento que se dibujó en pantalla
  Ticker _ticker;       // avance de 1 px cada ANIM_TICK ms (acumulador por tiempo)
  Stopwatch _total;     // duración total desde el begin() (TOTAL_MS)
  bool _done;
  bool _redraw;         // primer frame: clear() completo + dibujar todo

  // ========================================================
  // Helpers
  // ========================================================

  // Dibuja una línea vertical de BAR_W px (con rebalse por el borde derecho)
  void drawBar(int16_t x, uint8_t top, uint8_t height, int16_t width);

  void drawFirstBars();

  // Dibuja todas las franjas (TITULO y CUERPO) en el desplazamiento actual
  void drawBars();

  // Borra SOLO las columnas de las franjas anteriores que no coinciden con
  // las nuevas (las que coinciden se mantienen; el resto de la pantalla no
  // se toca)
  void eraseOldBars();

  // Borra 1 px de cada columna de la franja vieja que no está en la nueva
  void eraseBarDiff(int16_t oldX, int16_t newX, uint8_t top, uint8_t height,
                    int16_t width);
};

#endif

// ====================================================================================
// Fin
// ====================================================================================