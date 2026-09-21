#ifndef LEGEND_H
#define LEGEND_H

#include "Display.h"
#include "Buttons.h"

// ========================================================
// Legend — panel de botones (leyenda de flechas)
//
// Muestra dos pad direccionales con la disposición de los
// botones:
//   - Izquierda: MOVE, un rombo de 4 flechas.
//   - Derecha:   ACTION, un rombo de 4 flechas con sus
//     etiquetas Btn1 (arriba = ACTION_UP), Btn2 (derecha =
//     ACTION_RIGHT), Btn3 (abajo = ACTION_DOWN), Btn4
//     (izquierda = ACTION_LEFT) en una columna a la derecha.
//
// Se muestra al arranque (después de la animación Boot) y al
// volver al menú desde cualquier ventana. Cualquier botón la
// cierra (done() = true) y Engine pasa al menú.
// ========================================================

class Legend {
public:
  // ========================================================
  // Constructor
  // ========================================================

  Legend(Display& display, Buttons& buttons);

  // ========================================================
  // Inicialización (al entrar en la ventana)
  // ========================================================

  void begin();

  // ========================================================
  // Actualizar (lee botones y espera a que cualquier botón salga)
  // ========================================================

  void update();

  // ========================================================
  // Dibujar (dos pad direccionales + rótulos)
  // ========================================================

  void print();

  // ========================================================
  // Salida (true = se pidió ir al menú)
  // ========================================================

  bool done() const;

private:
  // ========================================================
  // Geometría de los pads (rombo de 4 flechas)
  // ========================================================

  static constexpr int16_t CY = 32;        // centro vertical de ambos pads
  static constexpr int16_t R  = 12;        // radio del rombo (centro-flecha)

  static constexpr int16_t PAD_MOVE_X   = 30;  // centro del pad MOVE
  static constexpr int16_t PAD_ACTION_X = 82;  // centro del pad ACTION

  // Rótulos: "Move"/"Action" arriba y columna BtnN a la derecha del pad ACTION
  static constexpr int16_t SIGN_Y  = 0;
  static constexpr int16_t LABEL_X = 104;
  static constexpr int16_t Btn_Y[4] = { 9, 25, 41, 56 };  // Btn1 → Btn4

  // ========================================================
  // Dependencias y estado
  // ========================================================

  Display& _display;
  Buttons& _buttons;

  bool _exit;

  // ========================================================
  // Helpers de dibujo
  // ========================================================

  // Flecha sólida (0 = ↑, 1 = →, 2 = ↓, 3 = ←), centrada en (cx, cy)
  void drawArrow(uint8_t dir, int16_t cx, int16_t cy);

  // Rombo de 4 flechas centrado en (cx, cy)
  void drawPad(int16_t cx);
};

#endif