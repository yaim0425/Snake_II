// Librerias
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "GameBuzzer.h"

// ====================================================================================
// PANTALLAS
// ====================================================================================

// Tamaño de la pantalla
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED
#define OLED_SDA 8
#define OLED_SCL 9
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Iniciar el controlador
Adafruit_SSD1306 Screen(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET);

// ====================================================================================
// ENTRADAS
// ====================================================================================

// Buzzer
#define PIN_BUZZER 11
GameBuzzer buzzer(PIN_BUZZER);

// Botones
const int8_t COUNT_BUTTOM = 8;
const int8_t BUTTOM[] = {
  41, 01, 02, 40,
  37, 39, 38, 36
};
enum Buttom {
  ACTION_UP = 0,
  ACTION_RIGHT,
  ACTION_DOWN,
  ACTION_LEFT,

  MOVE_UP,
  MOVE_RIGHT,
  MOVE_DOWN,
  MOVE_LEFT
};

// Indicador de activación
bool buttom[] = {
  0, 0, 0, 0,
  0, 0, 0, 0
};

// Tiempo entre lectura de botones
uint32_t buttomLast = 0;
int16_t buttomDelay = 150;

// Leer el estado de los botoes
void readButtom() {
  for (int8_t i = 0; i < COUNT_BUTTOM; i++)
    buttom[i] = 0;

  if (millis() - buttomLast < buttomDelay) return;
  buttomLast = millis();

  for (int8_t i = 0; i < COUNT_BUTTOM; i++)
    buttom[i] = digitalRead(BUTTOM[i]) == HIGH;
}

// ====================================================================================
// INICIALIZAR PROGRAMA
// ====================================================================================

void setup() {
  // Conexion serial
  Serial.begin(115200);

  // I2C del OLED
  Wire.begin(OLED_SDA, OLED_SCL);

  // OLED
  if (!Screen.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("Error OLED");
    while (true) {}
  }

  // Pulsadores
  for (int8_t i = 0; i < COUNT_BUTTOM; i++)
    pinMode(BUTTOM[i], INPUT_PULLDOWN);

  // Iniciar tonos
  buzzer.begin();
}

// ====================================================================================










// ====================================================================================
// CODIGO DE PRUEBA
// ====================================================================================

int R = 0;
int X = 0;
int Y = 0;

void Cuadricula(int N) {
  // Tamaño real de cada cuadro
  int T = N * 2;

  // Distancia entre el inicio de un cuadro y el siguiente
  int paso = T * 2;

  // Margen inicial
  int margenX = T / 2;
  int margenY = T / 2 + 16;


  Y = 0;
  for (int y = margenY; y < SCREEN_HEIGHT; y += paso) {
    Y++;
    X = 0;
    for (int x = margenX; x < SCREEN_WIDTH; x += paso) {
      Screen.fillRect(x, y, T, T, SSD1306_WHITE);
      X++;
    }
  }

  R = X * Y;
}

void Sonido() {

  Screen.setTextSize(2);  // 12x16

  Screen.setCursor(0, 24);
  Screen.println("Tono: " + String(R) + "   ");

  if (!buttom[MOVE_UP])
    if (!buttom[MOVE_RIGHT])
      if (!buttom[MOVE_DOWN])
        if (!buttom[MOVE_LEFT])
          return;

  if (buttom[MOVE_UP]) {
    R++;
    if (R > buzzer.count()) R = 0;
  }

  if (buttom[MOVE_DOWN]) {
    R--;
    if (R < 0) R = buzzer.count();
  }

  buzzer.play(R);
}

void Parpadeo() {
  Screen.setTextSize(1);  // 6x8

  Screen.setCursor(6 * R, 16);
  Screen.println("X");

  // if (millis() - moveLast < moveDelay) return;
  // moveLast = millis();

  R++;
  if (R > 20) R = 0;
}

// ====================================================================================










// ====================================================================================
// LOGICA DEL JUEGO
// ====================================================================================

// Tonos
#define TONE_SELECT 36
#define TONE_MOVE_MENU 12
#define TONE_MOVE_SNAKE 4
#define TONE_ERROR 29
#define TONE_COMER 25
#define TONE_PIRATA 51
int8_t toneSound = 0;

// Partes de la serpiente
const bool PART[][4][4] = {
  { { 0, 1, 1, 0 }, { 0, 1, 1, 0 }, { 0, 1, 0, 0 }, { 0, 1, 0, 0 } },  // TAIL_TO_UP
  { { 0, 0, 0, 0 }, { 0, 0, 1, 1 }, { 1, 1, 1, 1 }, { 0, 0, 0, 0 } },  // TAIL_TO_RIGHT
  { { 0, 1, 0, 0 }, { 0, 1, 0, 0 }, { 0, 1, 1, 0 }, { 0, 1, 1, 0 } },  // TAIL_TO_DOWN
  { { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 1, 1, 1, 1 }, { 0, 0, 0, 0 } },  // TAIL_TO_LEFT
  { { 0, 1, 1, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 1, 1, 0 } },  // BODY_TO_UP
  { { 0, 0, 0, 0 }, { 1, 1, 0, 1 }, { 1, 0, 1, 1 }, { 0, 0, 0, 0 } },  // BODY_TO_RIGHT
  { { 0, 1, 1, 0 }, { 0, 0, 1, 0 }, { 0, 1, 0, 0 }, { 0, 1, 1, 0 } },  // BODY_TO_DOWN
  { { 0, 0, 0, 0 }, { 1, 0, 1, 1 }, { 1, 1, 0, 1 }, { 0, 0, 0, 0 } },  // BODY_TO_LEFT
  { { 0, 1, 1, 0 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 }, { 0, 0, 0, 0 } },  // CORNER_RIGTH_UP
  { { 0, 0, 0, 0 }, { 0, 0, 1, 1 }, { 0, 1, 0, 1 }, { 0, 1, 1, 0 } },  // CORNER_RIGHT_DOWN
  { { 0, 1, 1, 0 }, { 1, 0, 1, 0 }, { 1, 1, 0, 0 }, { 0, 0, 0, 0 } },  // CORNER_LEFT_UP
  { { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 1, 0, 1, 0 }, { 0, 1, 1, 0 } },  // CORNER_LEFT_DOWN
  { { 0, 0, 0, 0 }, { 0, 1, 1, 0 }, { 0, 1, 1, 0 }, { 0, 1, 0, 1 } },  // HEAD_UP_CLOSE
  { { 1, 0, 0, 0 }, { 0, 1, 1, 0 }, { 1, 1, 1, 0 }, { 0, 0, 0, 0 } },  // HEAD_RIGHT_CLOSE
  { { 0, 1, 0, 1 }, { 0, 1, 1, 0 }, { 0, 1, 1, 0 }, { 0, 0, 0, 0 } },  // HEAD_DOWN_CLOSE
  { { 0, 0, 0, 1 }, { 0, 1, 1, 0 }, { 0, 1, 1, 1 }, { 0, 0, 0, 0 } },  // HEAD_LEFT_CLOSE
  { { 0, 0, 0, 0 }, { 1, 0, 0, 1 }, { 0, 1, 1, 0 }, { 0, 1, 0, 1 } },  // HEAD_UP_OPEN
  { { 1, 0, 1, 0 }, { 0, 1, 0, 0 }, { 1, 1, 0, 0 }, { 0, 0, 1, 0 } },  // HEAD_RIGHT_OPEN
  { { 0, 1, 0, 1 }, { 0, 1, 1, 0 }, { 1, 0, 0, 1 }, { 0, 0, 0, 0 } },  // HEAD_DOWN_OPEN
  { { 0, 1, 0, 1 }, { 0, 0, 1, 0 }, { 0, 0, 1, 1 }, { 0, 1, 0, 0 } },  // HEAD_LEFT_OPEN
  { { 0, 1, 1, 0 }, { 1, 1, 0, 1 }, { 1, 0, 1, 1 }, { 0, 1, 1, 0 } },  // BELLY_TO_RIGHT || BELLY_TO_UP
  { { 0, 1, 1, 0 }, { 1, 0, 1, 1 }, { 1, 1, 0, 1 }, { 0, 1, 1, 0 } },  // BELLY_TO_LEFT || BELLY_TO_DOWN
  { { 0, 1, 1, 1 }, { 0, 1, 0, 1 }, { 0, 0, 1, 1 }, { 0, 0, 0, 0 } },  // BELLY_RIGTH_UP
  { { 0, 0, 0, 0 }, { 0, 0, 1, 1 }, { 0, 1, 0, 1 }, { 0, 1, 1, 1 } },  // BELLY_RIGHT_DOWN
  { { 1, 1, 1, 0 }, { 1, 0, 1, 0 }, { 1, 1, 0, 0 }, { 0, 0, 0, 0 } },  // BELLY_LEFT_UP
  { { 0, 0, 0, 0 }, { 1, 1, 0, 0 }, { 1, 0, 1, 0 }, { 1, 1, 1, 0 } },  // BELLY_LEFT_DOWN

  { { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 } },  //
  { { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 },
    { 0, 0, 0, 0 } }  //
};

// Indice de las partes
enum Part {
  TAIL_TO_UP = 0,
  TAIL_TO_RIGHT,
  TAIL_TO_DOWN,
  TAIL_TO_LEFT,
  BODY_TO_UP,
  BODY_TO_RIGHT,
  BODY_TO_DOWN,
  BODY_TO_LEFT,
  CORNER_RIGTH_UP,
  CORNER_RIGHT_DOWN,
  CORNER_LEFT_UP,
  CORNER_LEFT_DOWN,
  HEAD_UP_CLOSE,
  HEAD_RIGHT_CLOSE,
  HEAD_DOWN_CLOSE,
  HEAD_LEFT_CLOSE,
  HEAD_UP_OPEN,
  HEAD_RIGHT_OPEN,
  HEAD_DOWN_OPEN,
  HEAD_LEFT_OPEN,
  BELLY_TO_RIGHT,
  BELLY_TO_LEFT,
  BELLY_RIGTH_UP,
  BELLY_RIGHT_DOWN,
  BELLY_LEFT_UP,
  BELLY_LEFT_DOWN
};

// Estructura para las posiciones
struct Point {
  int8_t x;
  int8_t y;
  Part part;
  Part dir;
};

// Posición de la comida
Point food;

// Partes de la serpiente
Point snake[200];

// Tamaño de la serpiente
uint8_t snakeLength;

// Nivel de dificultad
enum Difficulty {
  EASY = 0,
  NORMAL,
  DIFFICULT
};
int8_t difficulty = DIFFICULT;

// Posiciones a evaluar
Part nextDirection;

// Estado del juego
enum {
  GAME_MENU = 0,
  GAME_NOW,
  GAME_PAUSE,
  GAME_OVER
} gameStatus = GAME_MENU;

// Tiempo de movimiento
uint32_t moveLast = 0;
int16_t moveDelay = 0;

// Puntuación
uint8_t score = 0;
uint16_t heightScore = 0;

// Tiempo del regalo
uint32_t giftLast = 0;
int8_t giftSeg = 0;

// ====================================================================================

// Menú inicial
void menuPrint() {

  // Titulo
  Screen.setTextSize(2);  // 12x16

  Screen.setCursor(13, 0);
  Screen.println("Snake II");  // X = 96 pixeles

  // Cuerpo
  Screen.setTextSize(2);  // 12x16
  Screen.setTextColor(SSD1306_BLACK, SSD1306_WHITE);

  if (difficulty == EASY) {
    Screen.fillRoundRect(34 - 4, 26 - 4, 60 + 6, 16 + 4, 0, SSD1306_WHITE);
    Screen.setCursor(34, 26);
    Screen.println("Facil");  // X = 60 pixeles
  }

  if (difficulty == NORMAL) {
    Screen.fillRoundRect(28 - 4, 26 - 4, 72 + 6, 16 + 4, 0, SSD1306_WHITE);
    Screen.setCursor(28, 26);
    Screen.println("Normal");  // X = 72 pixeles
  }

  if (difficulty == DIFFICULT) {
    Screen.fillRoundRect(22 - 4, 26 - 4, 84 + 6, 16 + 4, 0, SSD1306_WHITE);
    Screen.setCursor(22, 26);
    Screen.println("Dificil");  // X = 84 pixeles
  }

  Screen.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

  // Pie de pagia
  Screen.setTextSize(1);  // 6x8

  Screen.setCursor(0, 57);
  Screen.println("YAIM425");  // X = 42 pixeles

  Screen.setCursor(104, 57);
  Screen.println("v1.0");  // X = 24 pixeles

  // Mostrar puntaje maximo
  String HS = "TOP:" + String(heightScore);

  int16_t x1, y1;
  uint16_t w, h;
  Screen.getTextBounds(HS, 0, 0, &x1, &y1, &w, &h);
  int8_t x = 42 + (62 - w) / 2;

  Screen.setCursor(x, 56);
  Screen.fillRoundRect(x - 2, 57 - 2, w + 2, h + 2, 0, SSD1306_WHITE);

  Screen.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  Screen.println(HS);
  Screen.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
}

void menuButtom() {
  if (buttom[ACTION_UP] || buttom[ACTION_RIGHT] || buttom[ACTION_LEFT] || buttom[ACTION_DOWN]) {
    toneSound = TONE_SELECT;
    gameReset();
  }

  if (buttom[MOVE_UP] || buttom[MOVE_RIGHT])
    if (difficulty < 2) {
      toneSound = TONE_MOVE_MENU;
      difficulty++;
    }

  if (buttom[MOVE_DOWN] || buttom[MOVE_LEFT])
    if (difficulty > 0) {
      toneSound = TONE_MOVE_MENU;
      difficulty--;
    }
}

void menuSpeed() {
  if (difficulty == EASY)
    moveDelay = 1000;

  if (difficulty == NORMAL)
    moveDelay = 300;

  if (difficulty == DIFFICULT)
    moveDelay = 100;
}

// ====================================================================================

// Game
void gameReset() {
  score = 0;
  nextDirection = BODY_TO_RIGHT;
  snakeLength = 4;
  for (int8_t i = 0; i < snakeLength; i++)
    snake[i] = { i + 1, 2, BODY_TO_RIGHT, BODY_TO_RIGHT };
  gameStatus = GAME_PAUSE;
}

void gameScoreShow() {
  Screen.setTextSize(2);
  Screen.setCursor(0, 0);
  Screen.println(score);
}

void gameGiftTime() {

  // Limitar
  if (giftLast == 0) return;

  // Mostrar el tiempo
  Screen.setTextSize(2);
  if (giftSeg >= 9) {
    Screen.setCursor(104, 0);
    Screen.println("  ");
    Screen.setCursor(104, 0);
  } else {
    Screen.setCursor(116, 0);
    Screen.println(" ");
    Screen.setCursor(116, 0);
  }
  Screen.println(giftSeg);

  // Tiempo de espera
  if (millis() - giftLast < 1000) return;
  giftLast = millis();

  // Descontar
  giftSeg--;

  // ELIMINAR O CREAR EL PREMIO
}

void gameSnakePrintPart(int8_t part, int8_t x, int8_t y) {

  // Ajustar posición
  x = 0 + x * 8;
  y = 16 + y * 8;

  // Pintar cada pixel
  for (int8_t i = 0; i < 4; i++)
    for (int8_t j = 0; j < 4; j++)
      if (PART[part][i][j]) {

        // Ajustar en horizontal
        int8_t a = 2 * j + x;
        if (a > 127) a -= 128;
        if (a < 0) a += 128;

        // Ajustar en vertical
        int8_t b = 2 * i + y;
        if (b > 63) b -= 48;
        if (b < 16) b += 48;

        // Mostrar el pixel
        Screen.fillRect(a, b, 2, 2, SSD1306_WHITE);
      }
}

void gameSnakePrint() {
  for (int8_t n = 1; n < snakeLength - 1; n++)
    gameSnakePrintPart(snake[n].part, snake[n].x, snake[n].y);

  Point head = snake[snakeLength - 1];
  if (head.dir == BODY_TO_UP)
    gameSnakePrintPart(HEAD_UP_CLOSE, head.x, head.y);
  else if (head.dir == BODY_TO_RIGHT)
    gameSnakePrintPart(HEAD_RIGHT_CLOSE, head.x, head.y);
  else if (head.dir == BODY_TO_DOWN)
    gameSnakePrintPart(HEAD_DOWN_CLOSE, head.x, head.y);
  else if (head.dir == BODY_TO_LEFT)
    gameSnakePrintPart(HEAD_LEFT_CLOSE, head.x, head.y);

  Point tail = snake[0];
  if (tail.dir == BODY_TO_UP)
    gameSnakePrintPart(TAIL_TO_UP, tail.x, tail.y);
  else if (tail.dir == BODY_TO_RIGHT)
    gameSnakePrintPart(TAIL_TO_RIGHT, tail.x, tail.y);
  else if (tail.dir == BODY_TO_DOWN)
    gameSnakePrintPart(TAIL_TO_DOWN, tail.x, tail.y);
  else if (tail.dir == BODY_TO_LEFT)
    gameSnakePrintPart(TAIL_TO_LEFT, tail.x, tail.y);
}

void gameSnakeMove() {

  // Validar movimiento
  if (millis() - moveLast < moveDelay) return;
  moveLast = millis();

  // Mover la piezas
  for (int8_t i = 0; i < snakeLength - 1; i++)
    snake[i] = snake[i + 1];

  //Cargar partes
  Point oldHead = snake[snakeLength - 2];
  Point newHead;

  // Crear la nueva cabeza
  if (oldHead.dir == BODY_TO_UP) {
  } else if (oldHead.dir == BODY_TO_RIGHT) {
    if (nextDirection == BODY_TO_RIGHT) {
      oldHead.part = BODY_TO_RIGHT;
      oldHead.dir = BODY_TO_RIGHT;

      newHead = snake[snakeLength - 1];
      newHead.part = BODY_TO_RIGHT;
      newHead.x++;

    } else if (nextDirection == BODY_TO_DOWN) {
      oldHead.part = CORNER_LEFT_DOWN;
      oldHead.dir = BODY_TO_DOWN;

      newHead = snake[snakeLength - 1];
      newHead.part = BODY_TO_DOWN;
      newHead.y++;

    } else if (nextDirection == BODY_TO_UP) {
      oldHead.part = CORNER_LEFT_UP;
      oldHead.dir = BODY_TO_UP;

      newHead = snake[snakeLength - 1];
      newHead.part = BODY_TO_UP;
      newHead.y--;
    }
  } else if (oldHead.dir == BODY_TO_DOWN) {
  } else if (oldHead.dir == BODY_TO_LEFT) {
  }

  // Ajustar en horizontal
  if (newHead.x > 15) newHead.x = 0;
  if (newHead.x < 0) newHead.x = 15;

  // Ajustar en vertical
  if (newHead.y > 5) newHead.y = 0;
  if (newHead.y < 0) newHead.y = 5;

  // Nuevos valoress
  snake[snakeLength - 1] = newHead;
}

void gameSnakeButtom() {
  if (buttom[ACTION_UP] || buttom[ACTION_DOWN]) {
    gameStatus = GAME_MENU;
    toneSound = TONE_SELECT;
  }

  if (buttom[ACTION_RIGHT] || buttom[ACTION_LEFT]) {
    if (gameStatus == GAME_PAUSE)
      gameStatus = GAME_NOW;
    else
      gameStatus = GAME_PAUSE;
    toneSound = TONE_SELECT;
  }

  if (buttom[MOVE_UP])
    nextDirection = BODY_TO_UP;
  else if (buttom[MOVE_RIGHT])
    nextDirection = BODY_TO_RIGHT;
  else if (buttom[MOVE_DOWN])
    nextDirection = BODY_TO_DOWN;
  else if (buttom[MOVE_LEFT])
    nextDirection = BODY_TO_LEFT;
}

void gameSnakePause() {
  // Cuadro de fondo
  Screen.fillRoundRect(34 - 4, 26 - 4, 60 + 6, 16 + 4, 0, SSD1306_WHITE);

  // Tamaño del texto
  Screen.setTextSize(2);  // 12x16

  // Invertir colores
  Screen.setTextColor(SSD1306_BLACK, SSD1306_WHITE);

  // Mostrar texto
  Screen.setCursor(34, 26);
  Screen.println("PAUSA");  // X = 60 pixeles

  // Invertir colores
  Screen.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
}

// ====================================================================================

// Ciclo infinito
void loop() {
  readButtom();

  Screen.setTextColor(SSD1306_WHITE);
  Screen.clearDisplay();

  if (gameStatus == GAME_MENU) {
    menuPrint();
    menuButtom();
    menuSpeed();
  } else {
    gameScoreShow();
    gameGiftTime();
    gameSnakePrint();
    if (gameStatus == GAME_NOW)
      gameSnakeMove();
    if (gameStatus == GAME_PAUSE)
      gameSnakePause();
    gameSnakeButtom();
  }

  Screen.display();
  buzzer.play(toneSound);
  toneSound = 0;
}

// ====================================================================================
// Fin del juego
// ====================================================================================
