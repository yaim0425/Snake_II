#ifndef BUZZER_H
#define BUZZER_H

#include "Config.h"

#include <Arduino.h>

// ========================================================
// Buzzer — capa de hardware (sonido)
//
// Reproduce UN solo tono a la vez, de forma NO bloqueante:
// tone() enciende el tono y marca su duración; update()
// (llamado desde loop()) lo apaga al agotarse el tiempo.
// Así el juego nunca se congela esperando que suene un
// efecto. Sobre esto se compone Sound (secuencias).
//
// Usa LEDC del núcleo ESP32 (Arduino-ESP32 Core 3.x),
// igual que el GameBuzzer original (respaldo).
// ========================================================

class Buzzer {
public:

  // ========================================================
  // Constructor (pin del buzzer)
  // ========================================================

  Buzzer(uint8_t pin = Config::Pin::BUZZER);

  // ========================================================
  // Inicialización (adjunta el canal LEDC al pin)
  // ========================================================

  bool begin();

  // ========================================================
  // Actualizar: apaga el tono cuando termina su duración.
  // Llamar una vez por loop().
  // ========================================================

  void update();

  // ========================================================
  // Sonido
  // ========================================================

  // Emite un tono (no bloqueante). durationMs > 0 lo apaga
  // automáticamente en update(); 0 = suena hasta stop().
  // frequency 0 = silencio (espera activa de durationMs).
  void tone(uint16_t frequency, uint32_t durationMs = 0);

  // Silencia el buzzer y cancela la duración pendiente.
  void stop();

  // true mientras hay un tono/duración en curso.
  bool busy() const;

  // true si begin() pudo adjuntar el canal LEDC.
  bool attached() const;

private:

  // ========================================================
  // Estado
  // ========================================================

  uint8_t  _pin;
  bool     _attached;
  bool     _busy;
  uint64_t _startMs;
  uint32_t _durationMs;
};

#endif

// ====================================================================================
// Fin
// ====================================================================================