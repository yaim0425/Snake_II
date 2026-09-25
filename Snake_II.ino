// ====================================================================================
// SNAKE II — Enlace de dependencias (wiring)
//
// Este archivo define los SERVICIOS GLOBALES (hardware) y crea el Engine:
//   - Display y Buttons: hardware (I2C del OLED y pines de los botones).
//   - Buzzer y Sound: sonido (Buzzer antes que Sound: orden de dependencia).
//     Declarados extern en Globals.h; definidos aquí, en el mismo archivo y
//     en orden de dependencia.
//   - Engine: despachador puro que POSEE las ventanas (Boot, Menu, Credits,
//     Game, Legend) como miembros. En Snake_II.ino ya NO hay ventanas
//     globales: son internas de Engine.
//
// setup() inicia el hardware y luego engine.begin() (entra al primer
// estado, Boot); loop() hace la única lectura de botones del frame
// (buttons.read()) y llama engine.update(), engine.print(), sound.update()
// y display.show().
// ====================================================================================

#include "Config.h"
#include "Globals.h"
#include "Engine.h"

#include <Arduino.h>

// ====================================================================================
// Servicios globales (hardware), compartidos por todas las clases.
// Definidos aquí (no en un .cpp aparte) y en ORDEN DE DEPENDENCIA
// (buzzer antes que sound): al vivir todos en esta misma unidad de
// traducción se garantiza su orden de construcción y que Sound(buzzer)
// ya encuentre al Buzzer construido.
// ====================================================================================

Display display;
Buttons buttons(Config::Pin::BUTTONS);

Buzzer  buzzer;
Sound   sound(buzzer);

// ====================================================================================
// Despachador: posee las ventanas (Boot, Menu, Credits, Game, Legend) y
// decide cuál se ve según su estado. Su constructor no recibe nada: las
// ventanas usan los servicios globales directamente.
// ====================================================================================

Engine engine;

// ====================================================================================
// Inicialización: hardware + primera transición (Boot)
// ====================================================================================

void setup() {
  Serial.begin(115200);

  display.begin();
  buttons.begin();
  buzzer.begin();
  sound.begin();
  engine.begin();

  Serial.println("Snake II");
}

// ====================================================================================
// Bucle principal
// ====================================================================================

void loop() {
  buttons.read();     // una sola lectura de botones por frame (de esta lectura
                      // consumen los eventos todas las ventanas despachadas por Engine)
  engine.update();
  engine.print();
  sound.update();
  display.show();
}

// ====================================================================================
// Fin
// ====================================================================================