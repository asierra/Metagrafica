# Gráficos de Barras (Polybar)

Para facilitar la creación de histogramas y gráficas de series de datos, MetaGráfica V2 introduce la primitiva polybar, que de hecho se usó en la versión 0 en el primer paper publicado. 

Conceptualmente, polybar toma un path y dibuja un rectángulo vertical (una barra) en cada punto X, elevándose desde una base común hasta la altura Y especificada y con un ancho dado.

## 1. Sintaxis y Atributos

polybar(width=w) { x1 y1   x2 y2   x3 y3 ... }


width (obligatorio): Define el ancho geométrico de la barra en las unidades de la ventana mundial actual (world_window). La barra se dibuja centrada horizontalmente sobre la coordenada X.

(X, Y) en el bloque: Representan el centro superior de cada barra.

## 2. Superposición (Overlapping)

Para lograr el efecto clásico de barras superpuestas (como medir dos variables en el mismo mes, por ejemplo, precipitación teórica vs real), simplemente se declaran dos polybar iterando sobre las mismas coordenadas X, variando el width, el fill (relleno) o el dash (patrón de línea).

  ``` MG 
  % Ejemplo: Gráfica de barras superpuestas
  {
    line_width 1.5   % Todas las barras tendrán un borde de 1.5pt
    
    % Serie 1: Barras más anchas, fondo sólido gris
    polybar(width=0.8, fill="gray") { 
        1 40   2 55   3 30   4 70 
    }
    
    % Serie 2: Barras más angostas superpuestas, fondo rayado o negro
    polybar(width=0.4, fill="black") { 
        1 35   2 60   3 25   4 65 
    }
  }
  ```

## 3. Barras Horizontales (Atributo dir)

Si se requiere que las barras crezcan desde el eje Y hacia la derecha (histograma horizontal), se puede agregar un atributo de dirección dir="horizontal". En este caso, la coordenada X del bloque actúa como la longitud de la barra, y la Y como su posición vertical.

% Barras horizontales (Crecen en X, apiladas en Y)
polybar(width=0.6, dir="horizontal") {
    40 1    55 2    30 3    70 4
}


Impacto en la Implementación (C++ AST)

Implementar polybar es extremadamente elegante porque no requiere modificar los backends (ni SVG, ni EPS, ni PDF).

El nodo PolybarNode en el Árbol de Sintaxis Abstracta (AST) actúa como una "macro de expansión". Al llamar a su método evaluate(), simplemente itera sobre su lista de puntos (X, Y) y, matemáticamente, genera y dibuja internamente un rectángulo para cada uno usando la primitiva rect() que ya existe en el motor base:

---

# ESTADO (act. 2026-07-16)

## Fase 1 — cableado del parser: HECHA

El motor ya tenía la clase `Polybar` (`include/primitives.h`) y su `draw()`
(`src/primitives.cpp:82`, expande cada punto a un `rect()`), y el mapeador log de `plot`
ya contemplaba `GI_POLYBAR` (`src/parserv3.cpp`) — pero **el parser nunca lo despachaba**:
`polybar` no estaba en `isPrim()` y nada llamaba a `setWidth`/`setHorizontal`. Cableado en
`src/parserv3.cpp`, dos cambios:

- `isPrim()`: añadido `polybar`. Hereda gratis el bloque de coords, la validación de pares
  de `checkCoordPairs` y los atributos por-primitiva de `emitPrimStyle` (`fill`/`hatch`/
  `color`/`line_width`/`dash`), ya acotados con push/pop.
- Construcción en `PrimStmt::exec`: `width` posicional o nombrado (`polybar(0.8)` ==
  `polybar(width=0.8)`, como `dot(r)`/`circle(r)`), **obligatorio**; `dir="horizontal"`
  conmuta `setHorizontal`. Ambos errores por `evalError` (fatal).

Cero cambios en los tres backends, como anticipaba el plan. Red golden intacta (`ok=54`).

**Las barras contiguas de un polybar NO se tapan entre sí**, y no por suerte: `rect()` hace
fill dentro de `gsave` y stroke afuera, **por barra**, así que la barra k+1 cubre el borde de
la barra k con su relleno pero acto seguido lo retraza con el suyo. La condición es que haya
`outlinefill`; sin él un `fill` sólido sí fusionaría las barras en una silueta.

## Pendientes conocidos

- **`width` en eje log**: `width` es miembro aparte, no va en el path, así que el mapeador
  log de `plot` no lo alcanza → en un eje x logarítmico el ancho de barra queda mal (ya
  anotado en `plan_plot.md`). Sin resolver; ningún ejemplo lo pide todavía.
- `base=` (base común distinta de 0) no existe; `Polybar::draw` siempre crece desde 0.

## Fase 2 — `examples/fig_polybar.mg` (Fig. 5 del primer artículo): HECHA

Decidido con el autor (2026-07-16): **tres pasadas** y datos a **3 decimales**.

**Cerrado el 2026-07-16.** El ejemplo está escrito, verificado contra el escaneo y en el
corpus golden (18→19 ejemplos, `ok=57 fail=0 error=0 psfail=0 c3fail=0`). Es el primero que
ejercita `polybar` (§4.12) y `fill`-SIN-`outlinefill`. Cero cambios al motor: todo salió con
lo que ya había. Verificaciones que corrieron:

- **Checksum de la leyenda, releído del `.mg` ya escrito**: `Σ(a_522−a_261)·0.5 = 0.3695`
  (−1.2% vs 0.374) — el número predicho, y las pasadas 1 y 3 son coord-por-coord idénticas.
- **Trama vs. escaneo**: espaciamiento horizontal de las rayas a 300 dpi = 8.25 px (EPS/PDF),
  8.33 (SVG), **8.07 en el original** → `hatch_gap=1.4` es fiel (2–3%). En una comparación a
  baja resolución la trama del SVG *parece* más densa: es antialiasing del rasterizador, no
  geometría (por eso se midió a 300 dpi en vez de creerle a la vista).
- **Paridad de los tres backends**: bandas rayadas en los mismos valores → la pasada blanca
  opaca borra igual en EPS/SVG/PDF. La Capa 3 no lo habría cazado (mira texto y líneas de
  área nula, no rellenos blancos).

**Medidas del escaneo que fijaron el layout** (calibración de la sección anterior):

| qué | píxeles (crop) | resultado en el `.mg` |
|---|---|---|
| caja de datos | 125..1594 × 118..1063 | `box=(0,0, 12.46,8.0)` cm, `x=(11.5,22.5)`, `y=(0,1)` |
| eje y en λ=11.5, marcas **adentro** ~7 px | 126..132 | `ticks="in", tick_size=1.5` |
| eje x **sin marcas** | nada en λ=22 | `ticks="none"` |
| hueco eje→etiqueta ~8–10 px | | `label_gap=2` |
| alto de dígito 27 px ≈ 6.5 pt | | `font_size 9.5` |
| grosor de línea ~2 px | | `line_width 0.4` |

⚠️ **La tinta bajo λ=12..20 NO son marcas de eje**: son cantos de barra (en λ=22, donde no
hay barra, no hay tinta). Confundirlas habría metido `ticks="out"` espurio.

**Detalles de rótulo** (§14): `$a_B$` y `$\lambda(\mu/rm)$`. El `/r` sirve doble: termina el
comando `\mu` (cualquier no-alfabético lo cierra) **y** pone la `m` en romano como el
original — `\mu m` habría dejado el espacio visible. Posiciones tomadas del escaneo, en
coords de mundo fuera del `plot`: `title=` los centraría y el libro los pone al extremo
(mismo pendiente `title_at=` que fig6-4/fig4-5, Fase 3 de `plan_plot.md`).

**Única redundancia que quedó:** la serie de 522 va escrita dos veces (pasadas 1 y 3), porque
un bloque de coords no acepta lazos y no se quiso degradar el ejemplo a `for`+`polybar` de un
punto (que no ejercitaría la expansión multi-punto, justo lo que la red golden debe cubrir).
Va comentado en el archivo; el checksum es la red que caza una edición a medias.

### Procedencia

`docs/first_article.pdf`, **página 13, Fig. 5**. Leyenda:

> Computed spectra for CO₂, with 0.5µm resolution, using Smith's (1969) approach. The lower
> histogram corresponds to 261 cm of CO₂, and the upper one to 522 cm. The hatched area, which
> indicates the difference between both cases, is equal to 0.374µm.

Texto (p. 12): `U_B = 261` y `522 cm`, `P_B = 660.15 mb`, `T_B = 263.43 K`. `a_B` =
absortividad de banda del CO₂ contra λ. Fig. 6 es lo mismo con resolución 1 µm (área 0.303 µm).

### Por qué SÍ se vale digitalizar aquí (a diferencia de fig4-5)

No hay fuente V0: es de **1988**, V0 imprimía **directo a impresora láser**, no había EPS.
El PDF es un **escaneo** (`Creator: EPSON TWAIN 5`, `Producer: Acrobat Paper Capture`,
2008): cada página es un JPEG gris de 2550×3507 a 300 dpi; la única fuente incrustada es
`HiddenHorzOCR` (capa OCR invisible). **Cero vectores que rescatar.**

Y no hay fórmula que recuperar: son la salida de un modelo de banda de Smith (1969). Lo
confirma la forma de los datos — en el ala (12.5–13.0) `a_522/a_261 ≈ 2` (régimen lineal en
la cantidad de gas), pero cerca del núcleo (16.0–16.5) la razón cae a 1.04 (saturado). Eso es
un modelo de banda estadístico, no un `1−exp(−τ)` con gaussiana (que sí tendría forma cerrada
y daría `τ_522 = τ_261²` al duplicar; no ajusta). Contraste con fig4-5, donde los 69 puntos
"digitalizados" resultaron ser `V=x²`, `1/r`, `1/(2r²)−1/r` y las coords se pudieron tirar.

**Los píxeles son la fuente primaria que queda.** La leyenda regala el **verificador**:
`Σ(a_522 − a_261)·0.5 == 0.374 µm`.

### Método de digitalización (inversión, para no repetirla)

Fuente: el **escaneo a 300 dpi**, no `examples/polybar.png` (esa es una captura de pantalla
del mismo escaneo, con **el cursor del ratón encima** sobre el hueco de λ≈19).

```
pdfimages -f 13 -l 13 -png docs/first_article.pdf p13     # → p13-000.png (2550×3507)
crop (400,1000)-(2150,2120)                               # → la figura, 1750×1120
binarizar a umbral 128 (estable entre 0.03 y 0.08 de fracción; a 0.12 se rompe)
```

**Calibración** (mínimos cuadrados sobre centroides de etiqueta; residuos <2 px en x sobre
267 px/2µm, <0.7 px en y sobre 946):

```
x_px = 133.7771·λ − 1413.778          (etiquetas 12,14,16,18,20,22 en x=191.1…1528.0)
a_B  = (1062.5 − y_px) / 946.2857     (escala de las etiquetas; CERO = la LÍNEA del eje)
```

⚠️ **Bug de V0 fósil en el escaneo — no usar las etiquetas del eje y como cero.** Las 6 están
sistemáticamente **~13.5 px arriba** de su valor: `0.0` cae en y=1050 pero la línea del eje
está en y=1062.5; `1.0` cae en y=104 pero el eje llega hasta y=118. El centrado vertical de
texto de V0 estaba corrido. La **escala** (946 px/unidad) sí es buena porque sale de
*espaciamientos*, que un desfase constante no toca; solo el **offset** hay que tomarlo de la
línea del eje. Usar las etiquetas como cero mete un sesgo de −0.0134 en las 22 barras.

**Regalo de esa misma calibración:** el extremo superior de la línea del eje y cae en el
**mismo píxel** que el tope de las barras de 14.0–16.0 → esa meseta está saturada en
`a_B = 1.000` exacto (ambas concentraciones). Por eso se redondea a 1.0 y no a 0.999.

**Lectura de cada bin** (bins de 0.5 µm; se toma el interior, ±6 px, para evitar los bordes
verticales). La regla NO es "buscar dos líneas sólidas" — ese fue el primer intento y falló
(daba diferencia 0 en todos los bins y perdía tres enteros):

- `a_522` = **extremo superior de la tinta** (línea sólida de 1 px, `frac=1.0`).
- `a_261` = **extremo inferior de la banda rayada**, donde la trama se corta. **No hay línea
  trazada ahí** (`frac` nunca pasa de ~0.2). Un bin sin banda (tinta ≤4 px) → `a_261 = a_522`.
- Sin corrección de ±1 px por grosor (`pad=0`): con `pad=1` el área baja a 0.355 (−5.1%).

### Cómo está construida la figura (importa para el port)

**No** es una barra blanca opaca encima de una rayada. Si lo fuera, la blanca habría dejado su
propio contorno abajo y habría **borrado los costados verticales** del rayado — y los costados
sí están (la columna x=327 está entintada en todo el alto de la barra, por debajo de `a_261`).
Son **tres pasadas**:

1. `polybar` de 522 con **trama, sin contorno** → rayado de 0 a `a_522`
2. `polybar` de 261 en **blanco opaco, sin contorno** → borra el rayado bajo `a_261`, costados incluidos
3. `polybar` de 522 **solo contorno** → repone costados de altura completa + tope en `a_522`

Las tres se expresan con lo que ya hay: `fill` **sin** `outlinefill` rellena sin trazar, y un
`polybar` pelado (sin `fill`) traza sin rellenar (`EPSDisplay::rect`: `if (isFilled())` fill;
`if (!fill || outlinefill)` stroke). Será el **primer ejemplo del corpus que ejercita
`fill`-sin-`outlinefill`**.

### DATOS (3 decimales; x = centro del bin, bins de 0.5 µm)

| centro λ (µm) | a_261 | a_522 |
|---:|---:|---:|
| 12.25 | 0.016 | 0.031 |
| 12.75 | 0.067 | 0.132 |
| 13.25 | 0.454 | 0.599 |
| 13.75 | 0.930 | 0.976 |
| 14.25 | 1.000 | 1.000 |
| 14.75 | 1.000 | 1.000 |
| 15.25 | 1.000 | 1.000 |
| 15.75 | 1.000 | 1.000 |
| 16.25 | 0.946 | 0.983 |
| 16.75 | 0.805 | 0.908 |
| 17.25 | 0.347 | 0.526 |
| 17.75 | 0.078 | 0.165 |
| 18.25 | 0.030 | 0.062 |
| 18.75 | 0.015 | 0.033 |
| 19.25 | 0.008 | 0.020 |
| 19.75 | 0.007 | 0.007 |

Fuera de 12.0–20.0 no hay tinta (la spec del eje llega a 22 solo por el rótulo `λ(µm)`).

**VERIFICACIÓN: `Σ(a_522 − a_261)·0.5 = 0.3695 µm` vs `0.374 µm` de la leyenda = −1.2%.**
(Sin redondear: 0.3696.) El 1.2% restante cabe en el grosor de línea (~2 px ≈ 0.002 por
frontera, ~13 bins con banda). Es tan bueno como sale de la cadena láser 1988 → papel →
escáner 2008 → umbral. **Si un cambio futuro mueve este número, algo se rompió.**

### Spec §4.12: SINCRONIZADA (2026-07-16)

`width` obligatorio ya estaba; se añadieron los dos párrafos que faltaban: **barras contiguas
no se tapan** (y por qué: fill en `gsave`, stroke afuera, por barra → requiere contorno) y la
**limitación de `width` en eje log**. Más el puntero a `examples/fig_polybar.mg`.

### Pendiente que queda de polybar

Solo los dos "Pendientes conocidos" de arriba (`width` en eje log, `base=`). Ninguno lo pide
un ejemplo todavía; ambos esperan un caso real que los justifique.
