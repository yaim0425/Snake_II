#ifndef GAME_BUZZER_H
#define GAME_BUZZER_H

#include <Arduino.h>

// ============================================================











// ============================================================
//                     GameBuzzer
// ============================================================
//
// Librería de sonidos para juegos
// Compatible con ESP32 / ESP32-S3
//
// Un solo archivo.
//
// Ejemplo:
//
//   #include "GameBuzzer.h"
//   GameBuzzer buzzer(15);
//
//   void setup() {
//      buzzer.begin();
//      buzzer.play(1);
//   }
//
//   void loop() {
//   }
//
// ============================================================











// ============================================================
//                     NOTAS MUSICALES
// ============================================================

#define NOTE_C0 16.35     // C0
#define NOTE_Db0 17.32    // C#0/Db0
#define NOTE_D0 18.35     // D0
#define NOTE_Eb0 19.45    // D#0/Eb0
#define NOTE_E0 20.6      // E0
#define NOTE_F0 21.83     // F0
#define NOTE_Gb0 23.12    // F#0/Gb0
#define NOTE_G0 24.5      // G0
#define NOTE_Ab0 25.96    // G#0/Ab0
#define NOTE_A0 27.5      // A0
#define NOTE_Bb0 29.14    // A#0/Bb0
#define NOTE_B0 30.87     // B0
#define NOTE_C1 32.7      // C1
#define NOTE_Db1 34.65    // C#1/Db1
#define NOTE_D1 36.71     // D1
#define NOTE_Eb1 38.89    // D#1/Eb1
#define NOTE_E1 41.2      // E1
#define NOTE_F1 43.65     // F1
#define NOTE_Gb1 46.25    // F#1/Gb1
#define NOTE_G1 49        // G1
#define NOTE_Ab1 51.91    // G#1/Ab1
#define NOTE_A1 55        // A1
#define NOTE_Bb1 58.27    // A#1/Bb1
#define NOTE_B1 61.74     // B1
#define NOTE_C2 65.41     // C2 (Middle C)
#define NOTE_Db2 69.3     // C#2/Db2
#define NOTE_D2 73.42     // D2
#define NOTE_Eb2 77.78    // D#2/Eb2
#define NOTE_E2 82.41     // E2
#define NOTE_F2 87.31     // F2
#define NOTE_Gb2 92.5     // F#2/Gb2
#define NOTE_G2 98        // G2
#define NOTE_Ab2 103.83   // G#2/Ab2
#define NOTE_A2 110       // A2
#define NOTE_Bb2 116.54   // A#2/Bb2
#define NOTE_B2 123.47    // B2
#define NOTE_C3 130.81    // C3
#define NOTE_Db3 138.59   // C#3/Db3
#define NOTE_D3 146.83    // D3
#define NOTE_Eb3 155.56   // D#3/Eb3
#define NOTE_E3 164.81    // E3
#define NOTE_F3 174.61    // F3
#define NOTE_Gb3 185      // F#3/Gb3
#define NOTE_G3 196       // G3
#define NOTE_Ab3 207.65   // G#3/Ab3
#define NOTE_A3 220       // A3
#define NOTE_Bb3 233.08   // A#3/Bb3
#define NOTE_B3 246.94    // B3
#define NOTE_C4 261.63    // C4
#define NOTE_Db4 277.18   // C#4/Db4
#define NOTE_D4 293.66    // D4
#define NOTE_Eb4 311.13   // D#4/Eb4
#define NOTE_E4 329.63    // E4
#define NOTE_F4 349.23    // F4
#define NOTE_Gb4 369.99   // F#4/Gb4
#define NOTE_G4 392       // G4
#define NOTE_Ab4 415.3    // G#4/Ab4
#define NOTE_A4 440       // A4
#define NOTE_Bb4 466.16   // A#4/Bb4
#define NOTE_B4 493.88    // B4
#define NOTE_C5 523.25    // C5
#define NOTE_Db5 554.37   // C#5/Db5
#define NOTE_D5 587.33    // D5
#define NOTE_Eb5 622.25   // D#5/Eb5
#define NOTE_E5 659.26    // E5
#define NOTE_F5 698.46    // F5
#define NOTE_Gb5 739.99   // F#5/Gb5
#define NOTE_G5 783.99    // G5
#define NOTE_Ab5 830.61   // G#5/Ab5
#define NOTE_A5 880       // A5
#define NOTE_Bb5 932.33   // A#5/Bb5
#define NOTE_B5 987.77    // B5
#define NOTE_C6 1046.5    // C6
#define NOTE_Db6 1108.73  // C#6/Db6
#define NOTE_D6 1174.66   // D6
#define NOTE_Eb6 1244.51  // D#6/Eb6
#define NOTE_E6 1318.51   // E6
#define NOTE_F6 1396.91   // F6
#define NOTE_Gb6 1479.98  // F#6/Gb6
#define NOTE_G6 1567.98   // G6
#define NOTE_Ab6 1661.22  // G#6/Ab6
#define NOTE_A6 1760      // A6
#define NOTE_Bb6 1864.66  // A#6/Bb6
#define NOTE_B6 1975.53   // B6
#define NOTE_C7 2093      // C7
#define NOTE_Db7 2217.46  // C#7/Db7
#define NOTE_D7 2349.32   // D7
#define NOTE_Eb7 2489.02  // D#7/Eb7
#define NOTE_E7 2637.02   // E7
#define NOTE_F7 2793.83   // F7
#define NOTE_Gb7 2959.96  // F#7/Gb7
#define NOTE_G7 3135.96   // G7
#define NOTE_Ab7 3322.44  // G#7/Ab7
#define NOTE_A7 3520      // A7
#define NOTE_Bb7 3729.31  // A#7/Bb7
#define NOTE_B7 3951.07   // B7
#define NOTE_C8 4186.01   // C8
#define NOTE_Db8 4434.92  // C#8/Db8
#define NOTE_D8 4698.64   // D8
#define NOTE_Eb8 4978.03  // D#8/Eb8

// ============================================================











// ============================================================
//                   GameBuzzer
// ============================================================

const double notes[] = {
  // Note of the song, 0 is a rest/pulse

  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0,
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
  NOTE_A4, NOTE_G4, NOTE_A4, 0,

  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0,
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
  NOTE_A4, NOTE_G4, NOTE_A4, 0,

  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0,
  NOTE_A4, NOTE_C5, NOTE_D5, NOTE_D5, 0,
  NOTE_D5, NOTE_E5, NOTE_F5, NOTE_F5, 0,
  NOTE_E5, NOTE_D5, NOTE_E5, NOTE_A4, 0,

  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0,
  NOTE_D5, NOTE_E5, NOTE_A4, 0,
  NOTE_A4, NOTE_C5, NOTE_B4, NOTE_B4, 0,
  NOTE_C5, NOTE_A4, NOTE_B4, 0,

  NOTE_A4, NOTE_A4,

  // Repeat of first part

  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
  NOTE_A4, NOTE_G4, NOTE_A4, 0,

  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0,
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, 0,
  NOTE_A4, NOTE_G4, NOTE_A4, 0,

  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, 0,
  NOTE_A4, NOTE_C5, NOTE_D5, NOTE_D5, 0,
  NOTE_D5, NOTE_E5, NOTE_F5, NOTE_F5, 0,
  NOTE_E5, NOTE_D5, NOTE_E5, NOTE_A4, 0,

  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, 0,
  NOTE_D5, NOTE_E5, NOTE_A4, 0,
  NOTE_A4, NOTE_C5, NOTE_B4, NOTE_B4, 0,
  NOTE_C5, NOTE_A4, NOTE_B4, 0,

  // End of Repeat

  NOTE_E5, 0, 0, NOTE_F5, 0, 0,
  NOTE_E5, NOTE_E5, 0, NOTE_G5, 0, NOTE_E5, NOTE_D5, 0, 0,
  NOTE_D5, 0, 0, NOTE_C5, 0, 0,
  NOTE_B4, NOTE_C5, 0, NOTE_B4, 0, NOTE_A4,

  NOTE_E5, 0, 0, NOTE_F5, 0, 0,
  NOTE_E5, NOTE_E5, 0, NOTE_G5, 0, NOTE_E5, NOTE_D5, 0, 0,
  NOTE_D5, 0, 0, NOTE_C5, 0, 0,
  NOTE_B4, NOTE_C5, 0, NOTE_B4, 0, NOTE_A4
};

const int duration[] = {
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 125,

  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 125,

  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 125, 250, 125,

  125, 125, 250, 125, 125,
  250, 125, 250, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 375,

  250, 125,

  // Rpeat of First Part

  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 125,

  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 125,

  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 250, 125, 125,
  125, 125, 125, 250, 125,

  125, 125, 250, 125, 125,
  250, 125, 250, 125,
  125, 125, 250, 125, 125,
  125, 125, 375, 375,

  // End of Repeat

  250, 125, 375, 250, 125, 375,
  125, 125, 125, 125, 125, 125, 125, 125, 375,
  250, 125, 375, 250, 125, 375,
  125, 125, 125, 125, 125, 500,

  250, 125, 375, 250, 125, 375,
  125, 125, 125, 125, 125, 125, 125, 125, 375,
  250, 125, 375, 250, 125, 375,
  125, 125, 125, 125, 125, 500
};

// ============================================================











// ============================================================
//                 GameBuzzer
// ============================================================

class GameBuzzer {
public:
  // Constructor
  GameBuzzer(uint8_t pin) {
    _pin = pin;
    _started = false;
  }

  // Inicialización
  bool begin() {
    // Arduino-ESP32 Core 3.x
    // ledcAttach(pin, frecuencia, resolución)

    _started = ledcAttach(_pin, 2000, 10);

    stop();

    return _started;
  }

  // Detener sonido
  void stop() {
    if (_started) {
      ledcWriteTone(_pin, 0);
    }
  }

  // Reproducir frecuencia
  void tone(uint16_t frequency, uint16_t duration) {
    if (!_started)
      return;

    if (frequency == 0) {
      ledcWriteTone(_pin, 0);
    } else {
      ledcWriteTone(_pin, frequency);
    }
  }

  // Reproducir un sonido
  // 1 - 50 : efectos
  // 51     : tema final pirata original
  void play(uint8_t sound) {
    switch (sound) {
      case 1:
        sound01();
        break;
      case 2:
        sound02();
        break;
      case 3:
        sound03();
        break;
      case 4:
        sound04();
        break;
      case 5:
        sound05();
        break;
      case 6:
        sound06();
        break;
      case 7:
        sound07();
        break;
      case 8:
        sound08();
        break;
      case 9:
        sound09();
        break;
      case 10:
        sound10();
        break;

      case 11:
        sound11();
        break;
      case 12:
        sound12();
        break;
      case 13:
        sound13();
        break;
      case 14:
        sound14();
        break;
      case 15:
        sound15();
        break;
      case 16:
        sound16();
        break;
      case 17:
        sound17();
        break;
      case 18:
        sound18();
        break;
      case 19:
        sound19();
        break;
      case 20:
        sound20();
        break;

      case 21:
        sound21();
        break;
      case 22:
        sound22();
        break;
      case 23:
        sound23();
        break;
      case 24:
        sound24();
        break;
      case 25:
        sound25();
        break;
      case 26:
        sound26();
        break;
      case 27:
        sound27();
        break;
      case 28:
        sound28();
        break;
      case 29:
        sound29();
        break;
      case 30:
        sound30();
        break;

      case 31:
        sound31();
        break;
      case 32:
        sound32();
        break;
      case 33:
        sound33();
        break;
      case 34:
        sound34();
        break;
      case 35:
        sound35();
        break;
      case 36:
        sound36();
        break;
      case 37:
        sound37();
        break;
      case 38:
        sound38();
        break;
      case 39:
        sound39();
        break;
      case 40:
        sound40();
        break;

      case 41:
        sound41();
        break;
      case 42:
        sound42();
        break;
      case 43:
        sound43();
        break;
      case 44:
        sound44();
        break;
      case 45:
        sound45();
        break;
      case 46:
        sound46();
        break;
      case 47:
        sound47();
        break;
      case 48:
        sound48();
        break;
      case 49:
        sound49();
        break;
      case 50:
        sound50();
        break;

      case 51:
        pirateTheme();
        break;

      default:
        stop();
        break;
    }
  }

  // ========================================================
  //                    50 SONIDOS
  // ========================================================

  // 01 - Click
  void sound01() {
    tone(1800, 35);
  }

  // 02 - Click grave
  void sound02() {
    tone(800, 50);
  }

  // 03 - Movimiento
  void sound03() {
    tone(500, 35);
    tone(700, 35);
  }

  // 04 - Movimiento rápido
  void sound04() {
    tone(500, 25);
    tone(800, 25);
    tone(1100, 25);
  }

  // 05 - Confirmación
  void sound05() {
    tone(700, 50);
    tone(1000, 80);
  }

  // 06 - Confirmación positiva
  void sound06() {
    tone(800, 50);
    tone(1200, 100);
  }

  // 07 - Error
  void sound07() {
    tone(600, 100);
    tone(400, 150);
  }

  // 08 - Error grave
  void sound08() {
    tone(500, 120);
    tone(300, 200);
  }

  // 09 - Alerta
  void sound09() {
    tone(1000, 80);
    tone(0, 40);
    tone(1000, 80);
  }

  // 10 - Alerta rápida
  void sound10() {
    tone(1500, 50);
    tone(0, 30);
    tone(1500, 50);
    tone(0, 30);
    tone(1500, 50);
  }

  // 11 - Moneda
  void sound11() {
    tone(988, 60);
    tone(1319, 100);
  }

  // 12 - Moneda grave
  void sound12() {
    tone(700, 60);
    tone(1000, 100);
  }

  // 13 - Salto
  void sound13() {
    tone(500, 50);
    tone(800, 50);
    tone(1200, 80);
  }

  // 14 - Salto corto
  void sound14() {
    tone(700, 40);
    tone(1100, 70);
  }

  // 15 - Vida
  void sound15() {
    tone(500, 60);
    tone(700, 60);
    tone(900, 100);
  }

  // 16 - Power-up
  void sound16() {
    tone(600, 50);
    tone(800, 50);
    tone(1000, 50);
    tone(1300, 100);
  }

  // 17 - Power-up rápido
  void sound17() {
    tone(600, 35);
    tone(800, 35);
    tone(1000, 35);
    tone(1200, 35);
    tone(1500, 100);
  }

  // 18 - Victoria
  void sound18() {
    tone(523, 80);
    tone(659, 80);
    tone(784, 100);
    tone(1047, 180);
  }

  // 19 - Victoria corta
  void sound19() {
    tone(784, 80);
    tone(988, 80);
    tone(1175, 150);
  }

  // 20 - Victoria retro
  void sound20() {
    tone(523, 70);
    tone(659, 70);
    tone(784, 70);
    tone(1047, 70);
    tone(1319, 180);
  }

  // 21 - Derrota
  void sound21() {
    tone(700, 100);
    tone(600, 100);
    tone(500, 100);
    tone(350, 250);
  }

  // 22 - Game Over
  void sound22() {
    tone(800, 100);
    tone(650, 100);
    tone(500, 150);
    tone(300, 300);
  }

  // 23 - Explosión
  void sound23() {
    tone(180, 180);
    tone(120, 150);
    tone(80, 220);
  }

  // 24 - Explosión corta
  void sound24() {
    tone(300, 80);
    tone(150, 150);
    tone(70, 180);
  }

  // 25 - Disparo
  void sound25() {
    tone(1800, 30);
    tone(1200, 30);
    tone(700, 50);
  }

  // 26 - Láser
  void sound26() {
    tone(1800, 30);
    tone(1500, 30);
    tone(1200, 30);
    tone(900, 30);
    tone(600, 50);
  }

  // 27 - Espada
  void sound27() {
    tone(2200, 25);
    tone(1800, 25);
    tone(1400, 25);
    tone(900, 50);
  }

  // 28 - Golpe
  void sound28() {
    tone(250, 100);
    tone(150, 120);
  }

  // 29 - Daño
  void sound29() {
    tone(400, 70);
    tone(250, 100);
    tone(150, 150);
  }

  // 30 - Corazón / vida
  void sound30() {
    tone(500, 60);
    tone(700, 100);
  }

  // 31 - Contador 3
  void sound31() {
    tone(400, 120);
  }

  // 32 - Contador 2
  void sound32() {
    tone(600, 120);
  }

  // 33 - Contador 1
  void sound33() {
    tone(800, 120);
  }

  // 34 - GO!
  void sound34() {
    tone(800, 60);
    tone(1100, 60);
    tone(1500, 150);
  }

  // 35 - Pickup
  void sound35() {
    tone(900, 40);
    tone(1200, 40);
    tone(1500, 80);
  }

  // 36 - Item raro
  void sound36() {
    tone(500, 50);
    tone(750, 50);
    tone(1000, 50);
    tone(1400, 100);
    tone(1800, 120);
  }

  // 37 - Teletransporte
  void sound37() {
    tone(400, 30);
    tone(600, 30);
    tone(900, 30);
    tone(1300, 30);
    tone(1800, 80);
  }

  // 38 - Sirena
  void sound38() {
    tone(600, 100);
    tone(1000, 100);
    tone(600, 100);
    tone(1000, 100);
  }

  // 39 - Bip
  void sound39() {
    tone(1000, 70);
  }

  // 40 - Bip doble
  void sound40() {
    tone(1000, 50);
    tone(0, 40);
    tone(1000, 50);
  }

  // 41 - Menú
  void sound41() {
    tone(700, 40);
    tone(900, 70);
  }

  // 42 - Selección
  void sound42() {
    tone(900, 50);
    tone(1200, 80);
  }

  // 43 - Nivel completado
  void sound43() {
    tone(523, 60);
    tone(659, 60);
    tone(784, 60);
    tone(1047, 120);
    tone(1319, 180);
  }

  // 44 - Nivel nuevo
  void sound44() {
    tone(400, 50);
    tone(500, 50);
    tone(650, 50);
    tone(800, 50);
    tone(1000, 120);
  }

  // 45 - Récord
  void sound45() {
    tone(700, 50);
    tone(900, 50);
    tone(1100, 50);
    tone(1400, 50);
    tone(1800, 180);
  }

  // 46 - Estrella
  void sound46() {
    tone(1200, 40);
    tone(1500, 40);
    tone(1800, 80);
  }

  // 47 - Magia
  void sound47() {
    tone(600, 40);
    tone(800, 40);
    tone(1100, 40);
    tone(1400, 40);
    tone(1800, 100);
  }

  // 48 - Fantasma
  void sound48() {
    tone(900, 60);
    tone(700, 60);
    tone(500, 60);
    tone(350, 120);
  }

  // 49 - Alarma
  void sound49() {
    tone(1400, 80);
    tone(900, 80);
    tone(1400, 80);
    tone(900, 80);
  }

  // 50 - Sonido especial
  void sound50() {
    tone(500, 50);
    tone(700, 50);
    tone(900, 50);
    tone(1200, 50);
    tone(1500, 50);
    tone(1800, 150);
  }

  // 51 - Caribbean Pirate Theme
  void pirateTheme() {
    for (int i = 0; i < 203; i++)
      tone(notes[i], duration[i]);

    stop();
  }

  // Obtener cantidad de sonidos

  uint8_t count() {
    return 51;
  }

private:
  uint8_t _pin;
  bool _started;
};

// ============================================================











// ============================================================

#endif
