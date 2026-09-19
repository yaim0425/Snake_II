// ====================================================================================
// SNAKE II — Despachador de estados
//
// Todas las ventanas (menú, juego, créditos, config) se ejecutan desde loop() y la
// activación depende de una variable global de estado (AppState).
//
// Ventanas:
//   ST_MENU       -> Menu       (opciones: Nuevo, Continuar, Dificultad, Sonido,
//                                Creditos)
//   ST_NUEVO      -> InfoWindow ("Nuevo" - en desarrollo)
//   ST_CONTINUAR  -> InfoWindow ("Continuar" - en desarrollo)
//   ST_DIFICULTAD -> InfoWindow ("Dificultad" - en desarrollo)
//   ST_SONIDO     -> InfoWindow ("Sonido" - en desarrollo)
//   ST_CREDITOS   -> Credits
//
// Cada ventana es una clase con begin()/update()/print(). El despachador decide qué
// ventana corre según el estado; solo el despachador cambia el estado global.
//
// Botones:
//   Menú     : MOVE_RIGHT/MOVE_LEFT navegan; ACTION_RIGHT confirma la opción.
//   Ventanas : ACTION_UP vuelve al menú.
// ====================================================================================

#include "Display.h"
#include "Buttons.h"
#include "Menu.h"
#include "Credits.h"
#include "InfoWindow.h"

// Controlador del OLED
Display display;

// Pines de los botones (orden del enum Button)
const int8_t BUTTON_PINS[Buttons::MAX_BUTTONS] = {
  37, 39, 38, 36,  // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  41, 01, 02, 40   // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};

Buttons buttons(BUTTON_PINS);

// ========================================================
// Ventanas
// ========================================================

Menu menu(display, buttons, 0, "v0.1");
Credits credits(display, buttons, "v0.1");
InfoWindow infoWindow(display, buttons);

// ========================================================
// Estado global de activación
// ========================================================

enum AppState : uint8_t {
  ST_MENU = 0,
  ST_NUEVO,
  ST_CONTINUAR,
  ST_DIFICULTAD,
  ST_SONIDO,
  ST_CREDITOS
};

AppState state = ST_MENU;

// ====================================================================================

void setup() {
  Serial.begin(115200);

  display.begin();
  buttons.begin();

  menu.begin();

  Serial.println("Snake II");
}

// ====================================================================================
// Transición a un nuevo estado (solo el despachador la llama)
// ====================================================================================

void changeState(AppState newState) {
  state = newState;

  switch (state) {
    case ST_MENU:          menu.begin();                                                     break;
    case ST_NUEVO:         infoWindow.begin("Nuevo");                                        break;
    case ST_CONTINUAR:     infoWindow.begin("Continuar");                                    break;
    case ST_DIFICULTAD:    infoWindow.begin("Dificultad");                                   break;
    case ST_SONIDO:        infoWindow.begin("Sonido");                                       break;
    case ST_CREDITOS:      credits.begin();                                                  break;
  }
}

// ====================================================================================
// Despachador: decide qué ventana corre según el estado
// ====================================================================================

void loop() {

  switch (state) {

    // ----------------------------------------------------------
    // Menú principal
    // ----------------------------------------------------------

    case ST_MENU: {

      menu.update();

      display.clear();
      menu.print();
      display.show();

      // Confirmar opción (ACTION_RIGHT) -> cambiar de ventana
      int8_t sel = menu.confirm();
      if (sel >= 0) {
        switch (sel) {
          case Menu::OPC_NUEVO:      changeState(ST_NUEVO);      break;
          case Menu::OPC_CONTINUAR:  changeState(ST_CONTINUAR);  break;
          case Menu::OPC_DIFICULTAD: changeState(ST_DIFICULTAD); break;
          case Menu::OPC_SONIDO:     changeState(ST_SONIDO);     break;
          case Menu::OPC_CREDITOS:   changeState(ST_CREDITOS);   break;
        }
      }
      break;
    }

    // ----------------------------------------------------------
    // Opciones en desarrollo (misma ventana genérica)
    // ----------------------------------------------------------

    case ST_NUEVO:
    case ST_CONTINUAR:
    case ST_DIFICULTAD:
    case ST_SONIDO: {

      infoWindow.update();

      display.clear();
      infoWindow.print();
      display.show();

      if (infoWindow.done()) changeState(ST_MENU);
      break;
    }

    // ----------------------------------------------------------
    // Créditos
    // ----------------------------------------------------------

    case ST_CREDITOS: {

      credits.update();

      display.clear();
      credits.print();
      display.show();

      if (credits.done()) changeState(ST_MENU);
      break;
    }
  }
}

// ====================================================================================
// Fin del despachador
// ====================================================================================