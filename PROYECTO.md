# Snake II — ESP32-S3

Proyecto: Snake II (estilo Nokia) para placa ESP32-S3 con Arduino IDE.
Programación orientada a objetos: cada clase en su archivo `.h` y `.cpp`.

---

## 0. Reglas de trabajo con la IA

- **La IA no debe hacer nada sin una orden explícita del usuario.**
- No crear, modificar ni eliminar archivos por iniciativa propia.
- No proponer ni iniciar fases de desarrollo por su cuenta; solo actuar cuando
  el usuario lo indique.
- **La IA debe hacer un commit al modificar archivos** (cuando el usuario lo ordene
  o como parte del flujo de trabajo ya autorizado).
- **La IA debe actualizar este documento (`PROYECTO.md`) antes de hacer el commit**:
  toda modificación de código debe quedar reflejada (CHANGELOG y, si corresponde,
  secciones/API), y ese cambio a `PROYECTO.md` debe incluirse en el mismo commit.

### Frase para iniciar una nueva sesión

Para no empezar desde cero, el usuario debe decir exactamente:

> **"Lee PROYECTO.md y continúa desde donde quedamos"**

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
| `Menu.h` / `Menu.cpp` | Clase `Menu` (menú inicial con carousel lateral y rombos de posición). Completa. |
| `Snake_II.ino` | Actualmente: arranca y ejecuta el menú inicial (`Menu`). |
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
| `void drawTextInverted(texto, x, y, size=1)` | Imprime texto en negro (sobre cualquier fondo) en posición exacta. |
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

## 8. Clase `Menu` — API

Ubicación: `Menu.h` / `Menu.cpp`.

### Constructor

```cpp
Menu(Display& display, Buttons& buttons, uint8_t topScore = 0, const char* version = "v0.1");
```

### Métodos

| Método | Descripción |
|--------|-------------|
| `void begin()` | Restablece el estado de la animación y del parpadeo. |
| `void setOptions(textos, conteo)` | Fija la lista y la cantidad de opciones (1..`MAX_OPTIONS`=8). El menú (textos y rombos) se adapta al conteo. |
| `void update()` | Lee botones, navega con `MOVE_RIGHT`/`MOVE_LEFT` y anima el deslizamiento lateral; log en Serial al cambiar de opción. |
| `void print()` | Dibuja título, cuadro fijo con la opción deslizante, rombos de posición y pie (Top + versión). |
| `int8_t selected()` | Índice de la opción seleccionada. |
| `void setTopScore(uint8_t)` | Actualiza el puntaje máximo mostrado. |

### Opciones y enum

```cpp
enum Option : uint8_t {
  OPC_NUEVO = 0, OPC_CONTINUAR, OPC_DIFICULTAD, OPC_SONIDO, OPC_CREDITOS
};
```

El enum documenta los índices de las 5 opciones por defecto. La cantidad real es
variable (`setOptions`), con `MAX_OPTIONS = 8`.

### Diseño del menú (carousel)

- **Título:** "Snake II", `TEXT_12x16`, centrado en `REGION_HEADER`. Se dibuja al
  inicio, luego se limpia la banda (0..16) con negro y se redibuja.
- **Pie:** "Top: X pts" (`LEFT_DOWN`) y versión (`RIGHT_DOWN`) en `TEXT_6x8`,
  **bajado 1 px** (fila 57, `PIE_TOP = 57`). Se limpian con negro las filas
  `54..63` antes de dibujar. Una **línea horizontal de 1 px** de grosor,
  `drawFastHLine`, en la fila `54` (`PIE_LINE_ROW`), a **2 px sobre el pie**
  (filas libres `55..56` entre la línea y el texto). Las 2 filas sobre el pie
  quedan siempre limpias.
- **Cuadro de selección:** **fijo** y de **ancho completo** (128 px),
  `BOX_TOP = 25`, `BOX_HEIGHT = 18` (banda 25..42). Con el rombo activo de punta
  en la `45`: quedan **2 filas libres** (`44..43`) sobre el rombo y el cuadro
  arranca en la **fila 3** desde la punta (`42`) hacia arriba. **No se mueve**;
  el tamaño del texto (tamaño 2) tampoco cambia.
- **Animación lateral (carousel):** al navegar, la opción **anterior no
  desaparece**: la **entrante** (nueva seleccionada) se desliza hasta centrarse
  en el cuadro y la **previa** (`_prev`), dibujada centrada, **se borra
  progresivamente desde la dirección contraria** a la entrada (barrido/wipe:
  `MOVE_RIGHT` → la previa se borra de izquierda a derecha; `MOVE_LEFT` → de
  derecha a izquierda). Sentido: derecha (`MOVE_RIGHT`) → la entrante entra por la derecha
  (siguiente opción); izquierda (`MOVE_LEFT`) → entra por la izquierda (anterior).
  Movimiento `2 px / 15 ms`, salto total `SLIDE_DIST = 48 px`. Si llega otro pulso
  a mitad de la animación, la transición se reinicia desde el lado correspondiente.
- **Recorte del texto deslizante:** las opciones se dibujan en un `GFXcanvas8`
  (128×18, `_chipBox`) que recorta los caracteres parciales en ambos bordes
  (`drawChar` de la librería *no* recorta en X); el canvas se vuelca a la banda
  del cuadro con un blit (chip blanco `255`, texto negro `1`, resto transparente).
  Colores del canvas: `CHIP_WHITE = 255`, `CHIP_TEXT = 1`.
- **Rombos de posición:** banda `45..53`, pegada a la línea separadora en la `54`
  (antes de la fila del pie, `DIA_TOP = 45`). Solo el **seleccionado** es un **rombo simétrico
  completo** de 9 filas (dibujado con dos `fillTriangle`, como el alimento del
  juego): punta superior en la `45`, hombros en la `49` y **punta inferior en la
  `53`** (visible, justo sobre la línea separadora `54`). Los **no seleccionados** son
  solo la **punta** (triángulo superior), **bajada hasta la línea separadora**:
  base en la `53` y vértice en la `50`. Reparto uniforme en el ancho
  (`cx = (i+1)·128/(n+1)`). Si se **mantiene** seleccionado sin navegar
`BLINK_HOLD = 500 ms`, el rombo **parpadea** mostrándose el **75%** de cada
  período y oculto el **25%** (período `BLINK_PERIOD = 1000 ms`, fase oculta = primer
  25%). Antes de dibujarlos se limpia con negro la banda
  `45..53` (`fillRect(0, DIA_TOP, ancho, DIA_SIZE+1)`).
- Primera y última opción no conectadas (navegación con límites).

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
- **[2026-09-17] `Snake_II.ino`: menú inicial**: título "Snake II" (`TEXT_12x16`)
  centrado en Header; en el Body, "Top: 0 pts" en `LEFT_DOWN` y "v0.1" en
  `RIGHT_DOWN` (tamano 1). El Top cambiará después (puntaje máximo guardado).
- **[2026-09-17] Clase `Menu`**: se crean `Menu.h`/`Menu.cpp`. Menú inicial con 5
opciones (`Nuevo, Continuar, Dificultad, Sonido, Creditos`) tamaño 2 en una pila
   deslizante hacia un cuadro de selección fijo (centrado en la banda 16..55). Animación
   1 px/15 ms. Navegación con `MOVE_UP/DOWN` y los 4 botones de Acción (`ACTION_UP`/
   `ACTION_LEFT` suben, `ACTION_DOWN`/`ACTION_RIGHT` bajan; sin conexión
   primera-última). Pie = Top + versión (la fila del pie se limpia antes de escribir).
   `Snake_II.ino` ahora arranca el menú con `Buttons`.
- **[2026-09-17] `Menu`: los 4 botones de Acción navegan**: se agregan `ACTION_LEFT`/`ACTION_RIGHT`
   a la navegación (left sube, right baja) y un log en Serial al cambiar de opción,
   para verificar que los pulsos llegan.
- **[2026-09-17] Ajustes del menú (experimental)**: (1) visibilidad simétrica de las
  opciones superior e inferior (8 px c/u) — las bandas de Header y pie se limpian y
  redibujan para el recorte; (2) el cuadro de selección ahora es **fijo** y de
  **ancho completo** (128 px); (3) solo el texto se desliza; la opción seleccionada se
  repinta en negro (`drawTextInverted`, nuevo método en `Display`) mientras cruza el
  cuadro.
- **[2026-09-17] `Menu`: navegación en subrutina y solo `MOVE_UP`/`MOVE_DOWN`**: se
  separa la navegación en `Menu::navigate()` (como `animate()`); solo `MOVE_UP`/
  `MOVE_DOWN` navegan (los 4 botones de Acción ya no mueven el cursor).
- **[2026-09-17] `Menu`: define `optionText()`**: se agregaba la definición que
  faltaba de `Menu::optionText()` (error de enlazado "undefined reference").
- **[2026-09-17] `Menu` rediseñado como carousel lateral**: se elimina la pila
  vertical (opciones apiladas) y el deslizamiento vertical. Ahora solo la opción
  seleccionada se muestra en el cuadro fijo; al navegar, la saliente se desliza hacia
  un lado y la entrante entra por el opuesto (bajar→sale izq/entra der; subir→al
  revés). Se usa un `GFXcanvas8` (128×18) para recortar el texto en los bordes.
  Las opciones pasan a ser de cantidad variable (`setOptions`, máx. 8). Se agregan
  **rombos de posición** en la banda 46..53 (uno por opción, repartidos en el ancho):
  el seleccionado se eleva 1 px y, al mantenerlo `500 ms`, parpadea cada `250 ms`.
  Se mantienen título, cuadro fijo con texto tamaño 2, pie y las 2 filas libres
  (54..55) sobre el pie.
- **[2026-09-17] Eliminado el diagrama de conexiones**: se retira
  `Conexiones_Snake_II_ESP32S3.png` (imagen y entradas del CHANGELOG).
- **[2026-09-17] Frase para nueva sesión**: se agrega a la sección 0 la frase exacta
  "Lee PROYECTO.md y continúa desde donde quedamos" para retomar el proyecto sin
  empezar desde cero.
- **[2026-09-17] Documentación antes del commit**: se agrega a las reglas (sección 0)
  que la IA debe actualizar `PROYECTO.md` (CHANGELOG y secciones/API) antes de cada
  commit, incluyendo ese cambio en el mismo commit.
- **[2026-09-17] Compromiso de la IA con commits**: se agrega a las reglas (sección 0)
  que la IA debe hacer un commit al modificar archivos.
- **[2026-09-17] Reglas de trabajo con la IA**: se agrega la sección "0. Reglas de
  trabajo con la IA": la IA no debe hacer nada sin una orden explícita del usuario.
- **[2026-09-17] `Menu`: 2 píxeles libres sobre el pie**: se limpian con negro las
  filas `54..63` (2 px `54..55` sobre el pie `56..64`) justo antes de escribir el pie,
  así los 2 píxeles sobre el pie quedan siempre limpios.
- **[2026-09-17] `Menu`: rombo completo solo para el seleccionado**: los rombos no
  seleccionados ahora son solo la punta (triángulo superior) en la fila `47`; el
  rombo completo (8×8) solo se dibuja para el seleccionado en `DIA_TOP = 46`
  (banda `46..53`), con 2 filas libres `44..45` sobre él. La banda de rombos se
  limpia con `fillRect(0, DIA_TOP-1, ancho, DIA_SIZE+1)` (45..53). Se elimina el
  estado `_prev` (ya no se dibuja ninguna opción saliente).
- **[2026-09-17] `Menu`: animación "desaparece y entra"**: al navegar, la opción
  anterior ya no se dibuja (se elimina `_prev` y el dibujo de la saliente); solo la
  nueva seleccionada se desliza desde un lado hasta centrarse (`_slideX` de
  `±SLIDE_DIST` a 0, `startSlide(dir)` sin parámetro `prev`). La transición se
  reinicia desde el lado correspondiente si llega otro pulso a mitad de la animación.
- **[2026-09-18] `Menu`: cuadro de selección subido al límite del Body (fila 16)**:
  `BOX_TOP` pasa a 16 (banda 16..33, pegado al límite superior del cuerpo) y el texto
  a `TEXT_SEL_TOP = 17`.
- **[2026-09-18] `Menu`: restaurada la animación del deslizamiento lateral**: se
  revierte el cambio anterior; la opción entrante vuelve a deslizarse hasta
  centrarse en el cuadro (2 px / 15 ms, `SLIDE_DIST = 48 px`). Se restablecen
  `startSlide`/`animate`, el estado `_dir`/`_slideX`/`_animLast`, las constantes
  de animación y el `GFXcanvas8 _chipBox` para recortar el texto al deslizar.
- **[2026-09-18] `Menu`: eliminada la animación del deslizamiento lateral**: al
  navegar, la opción seleccionada cambia al instante en el cuadro (ya no se
  desliza). Se eliminan `startSlide`/`animate`, el estado `_dir`/`_slideX`/
  `_animLast`, las constantes `SLIDE_DIST`/`ANIM_TICK`/`ANIM_STEP` y el
  `GFXcanvas8 _chipBox` (usado para recortar el texto al deslizar); la opción se
  dibuja centrada con `drawTextInverted`. La documentación del menú pasa a
  "Diseño del menú" (sin carousel).
- **[2026-09-18] `Menu`: navegación con `MOVE_RIGHT`/`MOVE_LEFT`**: los botones de
  navegación del menú pasan de `MOVE_UP`/`MOVE_DOWN` a `MOVE_RIGHT` (siguiente,
  entra por la derecha) y `MOVE_LEFT` (anterior, entra por la izquierda).
- **[2026-09-18] `Menu`: rombo seleccionado corregido (simétrico, como el
  alimento)**: el rombo completo pasó de un octógono asimétrico de 8 filas a un
  **rombo simétrico de 9 filas** (45..53): punta superior en `DIA_TOP = 45`,
  hombros en `DIA_TOP + DIA_SIZE/2` (49) y punta inferior en `DIA_TOP + DIA_SIZE`
  (53), alineada con la base de las puntas no seleccionadas. La limpieza de la
  banda ahora es `fillRect(0, DIA_TOP, ancho, DIA_SIZE+1)` (45..53).
- **[2026-09-18] `Menu`: puntas de opciones no seleccionadas bajadas al pie**: los
  triángulos superiores (puntas) de las opciones no seleccionadas se bajan hasta
  las 2 filas libres del pie: base en la fila `53` (pegada a las `54..55`) y
  vértice en la `50` (`DIA_TOP + DIA_SIZE/2`). Se elimina la constante `DIA_RISE`.
- **[2026-09-18] `Menu`: punta inferior del rombo seleccionado visible**: el rombo
  completo se dibujaba en 9 filas (46..54) y su vértice inferior (fila 54) era
  borrado por la limpieza del pie. Ahora ocupa 8 filas (46..53): triángulos con
  hombros en `DIA_TOP+3` y vértice inferior en `DIA_TOP+7`, de modo que la punta
  inferior queda en la fila `53` (visible). La punta de las opciones no
  seleccionadas tiene ahora la base en `DIA_TOP+DIA_RISE+3` (fila 50), con más
  aire hasta el pie.
- **[2026-09-18] `Menu`: cuadro de selección referenciado al rombo activo**: con el
  rombo activo de punta en la `45`, quedan **2 filas libres** (`44..43`) sobre el
  rombo y el cuadro arranca en la **fila 3** desde la punta (`42`) hacia arriba:
  `BOX_TOP` pasa a 25 (banda 25..42) y `TEXT_SEL_TOP` a 26.
- **[2026-09-18] `Menu`: pie bajado y línea separadora**: el texto del pie baja
  1 px (fila `57`, `PIE_TOP`) y se dibuja una **línea horizontal de 1 px** en la
  fila `54` (`PIE_LINE_ROW`) a **2 px sobre el pie** (2 filas libres `55..56`
  entre la línea y el texto). El pie se escribe con `drawText` en la posición
  calculada por `getTextPos` (en vez de `drawTextAligned`).
- **[2026-09-18] `Menu`: parpadeo del rombo activo 75/25**: el rombo seleccionado
  ya no alterna 50/50; ahora es **visible el 75%** de cada período y **oculto el
  25%** (`BLINK_PERIOD = 1000 ms`, `BLINK_OFF_PCT = 25`, fase oculta = primer 25%
  del período, en vez de `BLINK_TOGGLE` par/impar).
- **[2026-09-18] `Menu`: transición con barrido (wipe)**: la opción anterior ya no
  desaparece al instante: se guarda en `_prev` y, durante la transición, se dibuja
  centrada y se **borra desde la dirección contraria** a la entrada de la nueva
  (la entrante se desliza desde su lado mientras la previa se barre en sentido
  opuesto con `fillRect` transparente sobre el canvas).

---

## 8. Cómo compilar/probar

- IDE: Arduino IDE, placa `ESP32-S3 (Dev Module)` (verificar puerto).
- Librerías: Adafruit GFX + Adafruit_SSD1306.
- La demo actual (`Snake_II.ino`) no requiere botones ni buzzer (usa solo `Display`).
- Con SDA=8 y SCL=9, dirección 0x3C.