# Snake II — ESP32-S3

Proyecto: Snake II (estilo Nokia) para placa ESP32-S3 con Arduino IDE.
Programación orientada a objetos: cada clase en su archivo `.h` y `.cpp`.

---

## 1. Estado actual del proyecto

En desarrollo por partes. La clase `Display` está completa. `Snake_II.ino` contiene
actualmente un menú de prueba. El código del juego original está respaldado (no
restaurado) en `Snake_II_juego_backup.txt`.

Fases pendientes: integración de la clase `Display` en el juego, botones/pulsadores,
buzzer, menú, lógica de la serpiente.

---

## 2. Hardware y pines

| Elemento  | Pin |
|-----------|-----|
| ACTION_UP | 41  |
| ACTION_RIGHT | 1  |
| ACTION_DOWN | 2  |
| ACTION_LEFT | 40 |
| MOVE_UP   | 37  |
| MOVE_RIGHT | 39 |
| MOVE_DOWN | 38  |
| MOVE_LEFT | 36  |
| Buzzer    | 11  |
| OLED SDA  | 8   |
| OLED SCL  | 9   |

- **Pantalla:** OLED SSD1306 monocromo (amarillo) 128x64, I2C, dirección `0x3C`.
- **Librerías:** Adafruit_SSD1306, Adafruit_GFX.
- **Botones:** configurados como `INPUT_PULLDOWN` en el juego original.

---

## 3. Estructura de archivos

Directorio: `D:\Documents\ESP32S3\Snake_II`

| Archivo | Contenido |
|---------|-----------|
| `Display.h` / `Display.cpp` | Clase `Display` (control del OLED). Completa. |
| `Buttons.h` / `Buttons.cpp` | Clase `Buttons` (lectura con debounce, `pressed`/`released`). Completa. |
| `Snake_II.ino` | Actualmente: menú de prueba (PRUEBA) con selección automática por rebote. |
| `Snake_II_juego_backup.txt` | Respaldo del código del juego (Snake_II.ino original). |
| `GameBuzzer.h` | Clase del buzzer del juego original (sin cambios). |
| `PROYECTO.md` | Este documento. |

Nota: Arduino solo compila el `.ino` del sketch. El respaldo quedó como `.txt`
para que no interfiera en la compilación.

---

## 4. Pantalla: geometría

- Tamaño: **128x64 píxeles**.
- Celda: **8x8 px** → rejilla de **16 columnas × 8 filas**.
- Los métodos usan coordenadas de celda o de píxel según el método.
- `cellSize` configurable por constructor (default 8).

### Fuente / tamaño de texto (Adafruit_GFX)

| Enumerador  | textSize | Dimensiones |
|-------------|----------|-------------|
| `TEXT_6x8`  | 1        | 6×8 px      |
| `TEXT_12x16` | 2       | 12×16 px    |
| `TEXT_18x24` | 3       | 18×24 px    |

---

## 5. Clase `Display` — API

Ubicación: `Display.h` / `Display.cpp`.

### 5.1 Constructor

```cpp
Display(uint8_t sda = 8, uint8_t scl = 9, uint8_t address = 0x3C,
        uint8_t width = 128, uint8_t height = 64, uint8_t cellSize = 8);
```

### 5.2 Enums y struct

```cpp
enum TextSize { TEXT_6x8 = 1, TEXT_12x16 = 2, TEXT_18x24 = 3 };

enum TextAlign {
  LEFT_UP, CENTER_UP, RIGHT_UP,
  CENTER_LEFT, CENTER, CENTER_RIGHT,
  LEFT_DOWN, CENTER_DOWN, RIGHT_DOWN
};

struct TextPos { int16_t x; int16_t y; };  // esquina sup-izquierda del texto
```

### 5.3 Regiones

La pantalla se divide en dos regiones para el texto. Las alineaciones (`TextAlign`)
se aplican dentro de la región elegida:

| Región | Zona |
|--------|------|
| `REGION_HEADER` | (0,0)  - (128,15)  |
| `REGION_BODY`   | (0,16) - (128,64)  |
| `REGION_FULL`   | (0,0)  - (128,64)  |

Cualquier texto puede alinearse en cualquiera de las 9 posiciones de cada región
y con cualquier tamaño (`TEXT_6x8`, `TEXT_12x16`, `TEXT_18x24`).

### 5.4 Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | `Wire.begin(sda, scl)`, crea el OLED y lo limpia. |
| `void clear()` | Limpia el buffer de la pantalla. |
| `void show()` | Envía el buffer al OLED. |
| `void drawPixel(x, y, black=false)` | Dibuja 1 píxel (blanco o negro). |
| `TextPos getTextPos(texto, align, size=1, region=FULL)` | Devuelve x,y (esquina sup-izq) según alineación y región. |
| `void drawText(texto, x, y, size=1)` | Imprime texto en posición píxel exacta. |
| `void drawTextAligned(texto, align, size=1, region=FULL)` | Posición según alineación/región + imprime. |
| `void drawHighlight(texto, x, y, size=1)` | Texto resaltado (cuadro blanco + texto invertido) en posición exacta. |
| `void drawHighlightAligned(texto, align, size=1, region=FULL)` | Igual que `drawHighlight`, posicionado por alineación/región. |
| `uint8_t getTextWidth(texto, size=1)` | Ancho del texto = `strlen × 6 × size` (clamp 255). |
| `uint8_t getTextHeight(size=1)` | Alto del texto = `8 × size` (clamp 255). |
| `uint8_t getWidth()` / `getHeight()` | Ancho/alto en píxeles (128/64). |
| `uint8_t getCellSize()` | Píxeles por celda (8). |
| `uint8_t getColumns()` / `getRows()` | Columnas (16) / filas (8) de la rejilla. |
| `Adafruit_SSD1306& screen()` | Acceso directo al objeto OLED. |

### 5.5 Renderizado de texto y resaltado

- **Texto normal (`drawText`/`drawTextAligned`):** blanco sobre fondo negro, sin cuadro.
- **Resaltado (`drawHighlight`/`drawHighlightAligned`):** cuadro blanco (`fillRoundRect`,
  radio 0) con el texto invertido (negro) centrado dentro del cuadro.
- El cuadro **rebasa al texto**: `+2*size` px en X (1/2/3 px por lado) y `+1` px
  arriba y `+1` px abajo (total `+2` px en Y), centrando el texto en el cuadro.

```
size=1 -> texto 6x8      -> cuadro  (6+2)  x (8+2)  = 8x10
size=2 -> texto 12x16    -> cuadro  (12+4) x (16+2) = 16x18
size=3 -> texto 18x24    -> cuadro  (18+6) x (24+2) = 24x26
```

- No existen "botones no seleccionados": se usa texto normal o resaltado.

---

## 6. Decisiones de diseño

- **`char*` en lugar de `String`** en la clase `Display`: evita fragmentación del
  heap en bucles continuos. Las funciones de impresión aceptan `char*` (NULL-terminated).
- **Retornos `uint8_t`** para anchos/altos de texto: suficiente para la pantalla.
  Con clamp a 255 para evitar desbordes.
- **El ancho de texto usa `strlen` (bytes):** con caracteres UTF-8 (á, é, ñ) el ancho
  sería incorrecto. Actualmente se usan solo ASCII.
- **Posición de texto devuelta:** siempre la *esquina superior-izquierda* del área del texto.
- **Clase `Display` encapsula** el OLED; el juego que estaba en `Snake_II.ino` usaba
  `Adafruit_SSD1306` directamente (orden: `Screen`). Al integrar, migrar al objeto `Display`.

---

## 7. Clase `Buttons` — API

Ubicación: `Buttons.h` / `Buttons.cpp`. Basada en el diseño de `GameInput` (referencia
`D:\Documents\ESP32S3\Snake_2\GameInput.{h,cpp}`).

### Pines (orden del enum)

```cpp
const int8_t BUTTON_PINS[Buttons::MAX_BUTTONS] = {
  37, 39, 38, 36,   // MOVE_UP, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT
  41, 01, 02, 40    // ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};
```

### Enum

```cpp
enum Button : uint8_t {
  MOVE_UP = 0, MOVE_RIGHT, MOVE_DOWN, MOVE_LEFT,
  ACTION_UP, ACTION_RIGHT, ACTION_DOWN, ACTION_LEFT
};
```

### Métodos

| Método | Descripción |
|--------|-------------|
| `Buttons(const int8_t* pins, uint32_t buttonDelay = 30)` | Constructor, recibe los pines y el tiempo de debounce en ms. |
| `void begin()` | Configura `INPUT_PULLDOWN` y lee el estado inicial. |
| `void read()` | Leer físicamente, aplicar debounce y generar eventos. Llamar una vez por `loop()`. |
| `bool state(index)` / `pressed(index)` / `released(index)` | Acceso por índice (0-7) útil para ciclos genéricos. |
| `moveUp() / moveRight() / moveDown() / moveLeft()` | Estado actual (mantenido). |
| `actionUp() / actionRight() / actionDown() / actionLeft()` | Estado actual (mantenido). |
| `...,Pressed()` | Evento de pulso (true solo en el ciclo en que se presiona). |
| `...,Released()` | Evento de liberación (true solo en el ciclo en que se suelta). |

### Diseño del debounce

- Lee en bruto (`_rawButtons`), detecta el cambio físico y toma nota del instante.
- Solo acepta el nuevo estado tras `_buttonDelay` ms de estabilidad.
- Genera `_pressed`/`_released` de un solo ciclo.

---

## 7. CHANGELOG

Formato: `[fecha] descripción`. Se agrega una entrada por cada cambio al código.

- **[2026-09-16] Creación del proyecto y `Display.h`/`.cpp`**: clase base con ancho,
  alto, celda (8px), columnas (16), filas (8), `begin`, `clear`, `show`.
- **[2026-09-16] Correcciones en `Display`**: `_screen = nullptr` (era `==`),
  dirección I2C correcta en el constructor de `Adafruit_SSD1306`, null-check en
  `drawPixel`, `display()` tras el `clearDisplay()` del `begin`.
- **[2026-09-16] Directorio de trabajo**: se cambió de `Snake_4` a `Snake_II`.
- **[2026-09-16] Texto**: constantes `TEXT_6x8/TEXT_12x16/TEXT_18x24`
  (font 6x8 base que escala x2 y x3).
- **[2026-09-16] Alineación de texto**: enums `TextAlign`/`TextSize`, struct `TextPos`,
  `getTextPos`, `drawTextAligned`, `getTextWidth`, `getTextHeight`.
- **[2026-09-16] Retornos a `uint8_t`**: `getTextWidth`/`getTextHeight` (antes `uint16_t`),
  con clamp a 255.
- **[2026-09-16] `drawButton`**: botón estilo menú con el texto centrado y opción
  `selected` (invertido). Sin parámetro de radio (fijo 0).
- **[2026-09-16] Demo en `Snake_II.ino`**: reemplazó al juego; se respaldó el juego en
  `Snake_II_juego_backup.txt`. Demo en bucle de una propiedad por pantalla.
- **[2026-09-16] Demo reorganizada**: pantalla "INFO" dividida en pasos individuales
  (ancho, alto, celda, celdas, texto) para que no se sobrecargue la información.
- **[2026-09-17] Menú de prueba en `Snake_II.ino`**: reemplaza a la demo. Título
  "PRUEBA" centrado en (0,0)-(128,15); 5 opciones (Nueva, Continuar, Dificultad,
  Sonido, Creditos) centradas en la banda (0,16)-(128,64), texto 6x8, una selección
  a la vez, cambio automático cada 2000 ms, la primera y la última no conectadas
  (rebote). Opción seleccionada: caja blanca con texto invertido (`fillRoundRect`).
- **[2026-09-17] Regiones en `Display`**: se separan `REGION_HEADER` (0,0)-(128,15)
  y `REGION_BODY` (0,16)-(128,64); las 9 alineaciones y tamaños se aplican dentro
  de cualquiera de las regiones (`getTextPos`/`drawTextAligned` aceptan `Region`,
  default `REGION_FULL`). Se agregan `drawText` y `drawHighlight` (posición exacta)
  y `drawHighlightAligned`.
- **[2026-09-17] Eliminado `drawButton`**: no hay botón no seleccionado; solo texto
  normal o resaltado. El resaltado rellena un cuadro `fillRoundRect` con texto
  invertido centrado; el cuadro rebasa al texto `+2*size` px en X y `+1` px en Y
  (size1->+2x1, size2->+4x1, size3->+6x1). Menú actualizado al nuevo API.
- **[2026-09-17] `Snake_II.ino` actualizada a demo de regiones**: recorre en bucle las
  9 alineaciones en `REGION_HEADER` (H1-H9) y `REGION_BODY` (B1-B9), el resaltado en
  los 3 tamaños (6x8/12x16/18x24), texto normal vs resaltado, y termina cada ciclo
  en el menú PRUEBA (selección automática por rebote).
- **[2026-09-17] Rebose vertical simétrico en `drawHighlight`**: el cuadro del
  resaltado ahora rebasa `+1` px arriba y `+1` px abajo (total `+2` en Y); el texto
  queda centrado verticalmente en el cuadro.
- **[2026-09-17] `Snake_II.ino` restaurada al menú PRUEBA**: volvió el menú con las
  5 opciones (selección automática por rebote cada 2000 ms). El título "PRUEBA" ahora
  se imprime con `TEXT_12x16` (tamaño 2) centrado en `REGION_HEADER`. Las opciones
  siguen en `REGION_BODY` con `TEXT_6x8`, la seleccionada con `drawHighlight`.
- **[2026-09-17] Clase `Buttons`**: se crean `Buttons.h`/`Buttons.cpp` (basadas en el
  ejemplo `GameInput`). 8 botones con enum (`MOVE_*`=0-3, `ACTION_*`=4-7), debounce de
  30 ms por defecto, eventos `pressed`/`released` de un ciclo, y getters de estado.
  Pines: MOVE 37/39/38/36, ACTION 41/1/2/40.
- **[2026-09-17] `Buttons`: acceso por índice**: se agregan `state(index)`,
  `pressed(index)` y `released(index)` para recorrer los 8 botones en ciclos genéricos.
- **[2026-09-17] `Snake_II.ino`: prueba de Buttons**: título "PRUEBA" (tamano 2,
  centrado en Header, de ahora en adelante el título de todas las pruebas). En el
  Body, una tabla de 2 columnas con cada botón: pin, nombre corto y estado
  (H=mantiene, P=recién presionado, R=liberado). Los eventos también se imprimen
  por Serial.

---

## 8. Cómo compilar/probar

- IDE: Arduino IDE, placa `ESP32-S3 (Dev Module)` (verificar puerto).
- Librerías: Adafruit GFX + Adafruit_SSD1306.
- La demo actual (`Snake_II.ino`) no requiere botones ni buzzer (usa solo `Display`).
- Con SDA=8 y SCL=9, dirección 0x3C.