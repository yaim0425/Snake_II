// ====================================================================================
// SNAKE II — Enlace de dependencias (wiring)
//
// Este archivo construye TODAS las clases y las inicia en setup():
//   - Display y Buttons: hardware (I2C del OLED y pines de los botones).
//   - Boot, Menu, Credits, InfoWindow, Legend: ventanas independientes
//     (hermanas, no anidadas), instancias únicas para todo el juego. Sus
//     begin() los llama Engine al entrar en cada estado, y entre transiciones
//     sus valores se conservan.
//   - Engine: despachador puro. Su estado interno decide qué ventana se ve;
//     loop() solo llama engine.update() y engine.print().
//
// La clase Engine está en Engine.h / Engine.cpp.
// ====================================================================================

#include "Display.h"
#include "Buttons.h"
#include "Boot.h"
#include "Menu.h"
#include "Credits.h"
#include "InfoWindow.h"
#include "Legend.h"
#include "Buzzer.h"
#include "Sound.h"
#include "SoundWindow.h"
#include "Engine.h"

#include <Arduino.h>

// Pines de los botones (orden del enum Button)
const int8_t BUTTON_PINS[Buttons::MAX_BUTTONS] = {
  02, 01, 42, 41,  // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  38, 40, 39, 47   // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};

// ====================================================================================
// Instancias únicas (construidas antes de setup(), iniciadas en setup())
// ====================================================================================

// Hardware
Display display;

// Entrada
Buttons buttons(BUTTON_PINS);

// Sonido: Buzzer (hardware, un tono no bloqueante) + Sound
// (secuencias de los efectos del juego). SoundWindow es la
// opción "Sound" del menú (On/Off) y Sound se usa en el menú
// y en las transiciones del Engine.
Buzzer buzzer;
Sound sound(buzzer);
SoundWindow soundWindow(display, buttons, sound);

// Ventanas: clases independientes (hermanas, compartidas por referencia).
// Persisten entre estados: sus valores se conservan.
Boot boot(display, buttons);
Menu menu(display, buttons, sound, 0, "v0.1");
Credits credits(display, buttons);
InfoWindow info(display, buttons);
Legend legend(display, buttons, sound);

// Despachador: recibe las ventanas y decide cuál se ve según su estado.
Engine engine(display, buttons, boot, menu, credits, info, legend,
              sound, soundWindow);

// ====================================================================================

void setup() {
  Serial.begin(115200);

  // Iniciar hardware y entrar al primer estado (menú -> menu.begin())
  display.begin();
  buttons.begin();
  buzzer.begin();
  sound.begin();
  engine.begin();

  Serial.println("Snake II");
}

// ====================================================================================

void loop() {
  engine.update();
  engine.print();
  sound.update();
  display.show();
}

// ====================================================================================
// Fin
// ====================================================================================