# MetaGráfica — referencia del lenguaje

Esta referencia puede leerse **de corrido una vez** o consultarse mientras se construye una nueva figura. Las tablas del final
son el resumen.

---

## 1. Qué es

Describes *qué es la figura* y `mg` la compila:

```bash
mg figura.mg              # → figura.eps
mg figura.mg salida.svg   # el formato lo elige la extensión: .eps .svg .pdf
```

Un archivo mínimo genera salida sin necesitar preámbulo:

```octave
circle(1) { 0 0 }
polyline { 0 0  2 1 }
```

**Lo que MG no es.** No es un lenguaje de programación de propósito general:
tiene variables, expresiones, `for`, `if` y poco más. No analiza datos — `polybar` recibe
intervalos ya contados, no observaciones; para llegar de una hoja de cálculo a un `.mg` está
`tools/hist2mg.py`. No hace 3D —lo de [§13](#13-escenas-pseudo-3d) es una cámara que proyecta,
sin superficies ocultas ni iluminación—. No compone párrafos.

---

## 2. El lienzo y las unidades

```octave
display_size 12 8              % tamaño FÍSICO del dibujo, en centímetros
world_window -1 11 -2 6        % región visible del plano: xmin xmax ymin ymax
font_size 9                    % cuerpo de letra en puntos tipográficos
```

`display_size` es el tamaño del recuadro que ocupa la figura al embeberla en otro documento
—no el de una página—, en centímetros.

**Dos clases de cantidad, y no se mezclan:**

| clase | unidad | qué es | ejemplos |
|---|---|---|---|
| **de mundo** | las de `world_window` | se transforma con el sistema de coordenadas | coordenadas, radio de `circle`, `xstep` de `grid` |
| **física** | puntos tipográficos (pt) | inmune a la ventana y a las transformaciones | `line_width`, `font_size`, radio de `dot`/`marker`, `hatch_gap`, y todo el mobiliario de `plot` (`margin`, `tick_size`, `col_widths`…) |

Por eso `dot` es el marcador correcto dentro de una gráfica deformada: cae donde debe y no
se convierte en elipse.

> ⚠️ **Si no ves nada, o ves la figura a medias, mira `world_window` antes que nada:** es un
> recorte fijo, no se ajusta a tus datos. Es el tropiezo nº 1 ([detalle](#15-errores-comunes)).

**El motor es isométrico:** la escala es la misma en x y en y. Si la proporción de
`world_window` no coincide con la de `display_size`, la figura queda centrada con márgenes
(*letterbox*); para llenar el lienzo, iguala las dos proporciones.

> 🔑 **Y de ahí sale la regla para elegir herramienta.** `world_window` sirve cuando los dos
> ejes **comparten unidad** —un plano, una órbita, un mapa, una figura geométrica—, porque
> ahí la isometría es justo lo que quieres. Si x e y miden cosas distintas, o sus rangos
> difieren por más de dos o tres veces, la misma escala en ambos aplasta la figura contra un
> borde: `world_window 0 1 0 6` en un lienzo de 12 × 8 cm deja el eje x entero en 1.3 cm.
> Para eso está `plot` ([§11](#11-gráficas)), que mapea datos a una caja y estira cada eje
> por separado. **No es un atajo para hacer gráficas: es el sistema de coordenadas correcto
> cuando los ejes no son comparables** — y trae ejes, marcas y rótulos de paso.

> 💡 **`plot` es la excepción, y es deliberada.** Un `plot` mapea sus **unidades de datos** a
> su caja, y ahí x e y se estiran por separado: segundos contra voltios no tienen una
> proporción común que respetar. Lo isométrico gobierna el **plano de la figura**
> —centímetros contra unidades de mundo—; el mapeo de datos de un `plot` es otra cosa
> ([§11](#11-gráficas)).

> ⚠️ **Achicar `display_size` no achica el texto** — lo agranda relativamente ([detalle](#15-errores-comunes)).

---

## 3. Trayectos

Un **punto** es una pareja de coordenadas; un **trayecto** (path) es una lista de puntos. Es
la estructura de datos básica del lenguaje: casi todo lo que se dibuja consume un trayecto,
y esa lista puede escribirse literal, construirse con operaciones, o nombrarse y reusarse.

La forma corriente es el **bloque literal** que lleva cada primitiva:

```octave
polyline { 0 0  1 2  3 1 }
```

Pero un trayecto también es un **valor** con nombre propio, que se declara con `path` y se
pasa a una primitiva anteponiéndole `&`:

```octave
path perfil = { 0 0  1 2  3 1  4 0 }
polyline(&perfil)                          % dibujarlo…
polygon(&perfil)                           % …o rellenarlo, es el mismo trayecto
```

**Un trayecto es la misma lista de puntos, la interprete quien la interprete.** Por eso
`&perfil` sirve igual a `polyline` (lo traza), `polygon` (lo rellena) o `polybar` (lee cada
punto como el tope de una barra) — cada primitiva lo lee a su manera.

Con eso basta para dibujar. Un trayecto además se puede **operar** —encadenar, espejar,
suavizar, muestrear— y eso vive aparte, en §10: no hace falta para empezar.

> ⚠️ **`&trayecto` va como PRIMER argumento, siempre**, y el resto nombrado detrás:
> `dot(&p, size=2)` ✅, `dot(2, &p)` ❌ ([detalle](#15-errores-comunes)).

---

## 4. Primitivas

Cada primitiva lleva sus **coordenadas entre `{ }`** y sus **argumentos entre `( )`**:

```octave
polyline { 0 0  1 2  3 1 }                 % polilínea abierta
polyline(closed=true) { 0 0  1 0  1 1 }    % contorno cerrado, sin repetir el vértice
polygon { 0 0  1 0  1 1 }                  % relleno
rectangle { 0 0  4 3 }                     % dos esquinas opuestas
rectangle(w=4, h=3, at=(2, 1.5))           % …o centro + tamaño (at = centro; solo w = cuadrado)
circle(2) { 5 5  9 5 }                     % un círculo por punto; 2 = radio (mundo)
ellipse(3, 1.5) { 5 5 }                    % radios x, y
arc(2, from=0, to=120) { 0 0 }             % grados, positivo = antihorario
arc(4, 2, from=270, to=450) { 0 0 }        % arco ELÍPTICO (rx, ry); también rx=, ry=
dot(2) { 1 1  2 3 }                        % disco de radio FÍSICO 2 pt
marker(3, shape="cross") { 1 1 }           % símbolo de radio físico
bezier { 0 0  1 2  3 2  4 0 }              % p0 c1 c2 p1 [c1 c2 p2 …]  (3k+1 puntos)
smooth { 0 0  1 2  3 1  4 3 }              % NODOS: las tangentes las deriva el compilador
polybar(width=0.5) { 1 3  2 5  3 4 }       % cada punto es el centro superior de una barra
sine(half_cycles=2, amplitude=1) { 0 0  4 0 }
```

> ⚠️ **El `{` va en la MISMA LÍNEA que la primitiva.** Por dentro, tanto los `( )` como los
> `{ }` pueden ocupar cuantas líneas quieras; lo que no puede haber es un salto de línea
> **antes** del `{` ([detalle](#15-errores-comunes)).

`polyline`, `polygon` y `bezier` admiten **subtrayectos disjuntos** separados por `;`:

```octave
polyline { 0 0  1 1 ;  2 0  3 1 }          % dos trazos, mismo estilo
```

En el bloque, una coordenada puede ser un par de escalares (`x y`) **o un punto `[x,y]`** —una
lista de dos, como devuelve `point_at` (§10) o un literal—, y se mezclan:

```octave
marker(shape="x") { point_at(&curva, 0.5) }   % un punto directo
polyline { 0 0  (p)  5 5 }                     % escalares y un punto p mezclados
```

**Formas de `marker`:** `circle`, `square`, `diamond`, `cross`, `x`, `triangle`, `arrow`,
`circle-dot`. También el nombre de un `struct` propio. `dot(r)` es el atajo del disco y no
lleva `shape=`.

**Flechas y marcadores SOBRE una línea.** Esto es lo que se busca casi siempre, y **no** se
hace estampando un `marker` aparte en la punta: cualquier primitiva de trayecto los lleva como
atributo y los coloca y **orienta sola**.

```octave
polyline(marker_end="arrow") { 1 2  4 2 }                       % punta al final
polyline(marker_start="arrow", marker_end="arrow") { 1 1  4 1 }  % en los dos extremos
polyline(marker="circle", marker_size=2) { 5 1  6 2  7 1 }       % en TODOS los vértices
polyline(marker_end="arrow", marker_end_shift=0.3) { 5 2  7 2 }  % retrasada sobre la línea
```

| atributo | |
|---|---|
| `marker_start` / `marker_mid` / `marker_end` | dónde: primer vértice, intermedios, último |
| `marker` | la forma **en todos** los vértices (o solo la forma, si hay `marker_at`) |
| `marker_size` | radio **físico**, en pt |
| `marker_color` / `marker_fill` | color; por default el marcador **hereda el de su línea** |
| `marker_orient` | `"auto"` (tangente, default), `"reverse"`, `"fixed"`, o un ángulo |
| `marker_start_orient` / `marker_end_orient` | lo mismo, para un solo extremo |
| `marker_start_shift` / `marker_end_shift` | correr el marcador a lo largo de la línea |

La flecha se **orienta a la tangente local** sin que se lo pidas, así que una línea quebrada o
un arco la reciben bien girada — por eso estampar un `marker` suelto en la punta es peor: hay
que repetir la coordenada y calcular el ángulo a mano.

**Dónde se ancla el marcador, que depende de la forma.** Medido:

| forma | se ancla por | rebasa el vértice |
|---|---|---|
| `arrow` | **la punta** | nada: la punta cae *en* el vértice y el cuerpo queda atrás |
| `circle`, `square`, `diamond`, `triangle`, `cross` | **el centro** | `marker_size` pt |

⚠️ **Con una forma simétrica, medio marcador se sale por delante del vértice.** Si la línea
termina justo contra una caja o contra otro dibujo, el marcador la invade `marker_size` pt. Solo
se nota ampliando; a tamaño completo pasa por buena. Una flecha **no** tiene ese problema.

Para retirarlo hay dos caminos. El directo es acortar la línea, y como `marker_size` es una
cantidad **física** y las coordenadas son de **mundo**, hay que convertir:

```octave
display_size 12 6
world_window 0 10 0 5
% marker_size (pt) -> unidades de mundo
retiro = 5/72*2.54 / (12 / 10)
rectangle { 6 1  9 4 }
polyline(marker_end="circle", marker_size=5) { 1 2.5  (6 - retiro) 2.5 }
```

**`marker_start_shift` / `marker_end_shift`** es el otro camino, y su unidad no es obvia: **es
una fracción del segmento del extremo**, interpolando desde el vértice contiguo hacia el vértice
del marcador. **El default es 1** —el marcador en su vértice—, `0` lo lleva al vértice contiguo,
`0.5` al punto medio de ese segmento y un valor mayor que 1 lo extrapola más allá. En un trayecto
de tres o más puntos actúa solo sobre el **último** segmento (o el primero, para `marker_start`),
no sobre el recorrido entero.

**Marcadores sobre un arco o una elipse (`marker_at`).** `arc`, `ellipse` y `circle` aceptan
marcadores en posiciones **paramétricas**, no solo en los extremos, cada uno orientado a la
tangente local:

```octave
ellipse(3, 5, marker="arrow", marker_at=[0, 180]) { 0 0 }   % flechas de sentido
arc(2, from=0, to=180, marker="arrow", marker_at=45) { 0 0 } % un solo ángulo, sin lista
```

Los valores van en **grados**, el mismo parámetro que `from`/`to` de la propia primitiva (no
son `t ∈ [0,1]` como en `point_at`/`sample`, que recorren por longitud de arco). Con `marker_at`
presente, `marker=` es solo la **forma**: los extremos hay que pedirlos aparte con
`marker_start`/`marker_end`. ⚠️ En una elipse los ángulos paramétricos **no** quedan
equiespaciados sobre la curva. Un marcador es de tamaño **físico** y de un solo color; para
estampar una struct completa con sus colores, sobre el mismo arco, va `place(..., at=)`.

**Recortar un arco: dibujar solo el tramo que se ve.** `from`/`to` no se normalizan —el
barrido es `to − from`, puede pasar de 360° y empezar donde sea—, así que un `arc` es la
manera de trazar *parte* de una elipse: por ejemplo la mitad de una órbita que no queda
tapada por el planeta. Y los ángulos del corte se calculan **en el propio `.mg`**, porque el
evaluador trae trigonometría ([§7](#7-expresiones-y-control-de-flujo)): no hay que medir
sobre el dibujo ni partir la curva a mano.

```octave
% Órbita de semiejes a, b concéntrica con un globo de radio R. El tramo oculto es el
% que va por detrás Y cae dentro del disco; los cruces salen de |P(t)|² = R².
s  = sqrt((R*R - a*a)/(b*b - a*a))
tc = deg(asin(s))                               % el semiángulo, en grados
arc(a, b, from=(180+tc), to=(540-tc)) { 0 0 }   % se salta el tramo t ∈ (180−tc, 180+tc)
```

Los extremos del trazo caen sobre el limbo **por construcción**, sin ajustar nada, porque el
círculo del globo y la ecuación comparten centro y radio. Nótese que esto no es un problema
de intersección de contornos —el álgebra de trayectos ([§10](#10-álgebra-de-trayectos)) no
ayudaría—: la oclusión es cuestión de profundidad, y en 2-D hay que decidir **cuál** mitad va
detrás. Figura completa en [`examples/orbita_polar.mg`](../examples/orbita_polar.mg).

**`compound`** une varias primitivas en **un solo** trazo relleno:

```octave
compound(fill="orange") { circle(2) { 0 0 }   circle(1) { 0 0 } }
```

---

## 5. Estilo: dos registros

Es el punto donde más se tropieza, así que conviene tenerlo claro desde el principio.

**Sentencia de estado** — vale desde donde aparece hasta que otra la cambie:

```octave
color "red"
line_width 0.8
polyline { 0 0  1 1 }      % roja
polyline { 1 1  2 0 }      % también roja
```

**Atributo por-primitiva** — vale solo para esa primitiva:

```octave
polyline(color="red", line_width=0.8) { 0 0  1 1 }
polyline { 1 1  2 0 }      % NO es roja
```

| se puede como sentencia | y como atributo de cualquier primitiva |
|---|---|
| `color`, `fill`, `line_width`, `dash`, `hatch`, `hatch_gap`, `gradient`, `gradient_angle`, `outlinefill`, `font`, `font_size`, `align`, `valign` | `color=`, `fill=`, `line_width=`, `dash=`, `hatch=`, `hatch_gap=`, `hatch_angle=`, `gradient=`, `gradient_angle=` |

> ⚠️ **`outlinefill`, `font`, `font_size`, `align` y `valign` no son atributos de estilo
> genéricos:** `polyline(align="center")` es error. Pero **`text()` sí los acepta**, porque
> ahí no son estilo prestado sino argumentos propios suyos —
> `text("m", align="center", font="italic", font_size=9) { 5 1 }` es correcto.
> Son dos cosas distintas con la misma pinta: el **atributo de estilo**, que vale en toda
> primitiva, y el **argumento propio** de una primitiva concreta (como `shape=` en `marker`
> o `closed=` en `polyline`). `outlinefill` no vale en ninguna de las dos formas: el
> contorno sobre relleno se pide dando `color=` junto a `fill=`.

**Relleno y contorno.** `fill=` enciende el relleno; `color=` es el trazo. Juntos = relleno
contorneado. `fill="none"` apaga el relleno.

> ⚠️ **`fill="none"` no vale en `polygon`, y es error.** Un polígono **rellena por
> definición** —es justamente lo que lo distingue de una polilínea cerrada—, así que
> apagarle el relleno se contradice con la primitiva. La forma de pedir un **contorno
> cerrado sin relleno** es `polyline(closed=true)`, que además no repite el primer
> vértice al final. Hasta el 2026-08-06 `polygon(fill="none")` se aceptaba en silencio y
> salía **relleno de negro** (y sin `color=`, además sin contorno).

**Colores:** los **148 nombres CSS** (`"steelblue"`, `"orange"`…), `"#rrggbb"`, o
`gray(0.4)`. Sin excepciones: `green` y `orange` valen lo que valen en CSS.

**Líneas:** `line_width` en pt. `dash` acepta `"solid"`, `"dashed"`, `"dotted"`,
`"longdashed"`, `"shortdashed"`, `"dashdot"`, `"dashdotdot"`.

**Degradados:** `gradient` acepta una **lista de colores** —las paradas, repartidas por igual a
lo largo del eje— y `gradient_angle` el ángulo en grados, 0 = izquierda→derecha:

```octave
rectangle(gradient=["orange", "#D93622", "black"], color="black") { 0 0  10 2 }
rectangle(gradient=[gray(0.8), gray(0.3)], gradient_angle=90) { 0 3  10 5 }
```

> ⚠️ **El ángulo es el de la PÁGINA, no el de la figura.** Un rectángulo girado 30° con
> `gradient_angle=0` sigue degradando de izquierda a derecha en el papel. Es la misma regla que
> `hatch_angle`, y no un descuido: son las dos maneras de rellenar un área con algo que no es un
> color plano, y las dos se orientan respecto de la hoja.

`gradient` y `hatch` son **la misma ranura**, así que un `gradient=` por-primitiva pisa un `hatch`
ambiente y se restaura al salir, como cualquier atributo. Darlos **juntos en la misma primitiva**
es error: ahí no hay orden que interpretar. Con `color=` al lado, el degradado se contornea, igual
que un `fill=`. Ver [`examples/espectro.mg`](../examples/espectro.mg).

**Tramas:** `hatch` acepta un **ángulo libre** en grados o un estilo nombrado
(`"hatch"` 45°, `"hatchback"` 135°, `"crosshatch"` ambas). `hatch_gap` es la separación en
pt; `hatch_angle` es la **orientación base** (desacopla el ángulo del tipo, como `hatch_gap`
desacopla el paso): en `"crosshatch"` gira toda la rejilla, así que `hatch_angle=0` la
**endereza** a 0°+90°. La trama usa el **color de relleno** (`fill=`, no `color=`, que
contornea el borde); no hay `hatch_color`.

---

## 6. Texto y matemáticas

```octave
text("masa del electrón", align="center") { 5 1 }
```

Argumentos: `align` (`"left"`/`"center"`/`"right"`), `valign`
(`"baseline"`/`"top"`/`"middle"`/`"bottom"`), `font_size` (alias `size`), `color`, `font`.

**Markup dentro de la cadena:**

| | |
|---|---|
| `/b` `/i` | negrita, itálica |
| `/r` `/s` `/c` | romana, sans-serif, monoespaciada |
| `/n` | **rompe el renglón** |
| `$…$` | modo matemático |
| `\alpha`, `\nabla`, … | símbolo (dentro o fuera de math) |
| `_x`  `^x`  `_{xy}`  `^{xy}` | sub y superíndice — **solo dentro de `$…$`** |
| `'` | prima (dentro de math) |
| `\frac{a}{b}` | fracción (numerador sobre denominador) — dentro de `$…$` |
| `\hat{n}` `\vec{v}` | acento sobre la base: versor y vector — dentro de `$…$` |
| `\,` `\;` `\!` `\quad` | ajuste fino de espacio math (fino, grueso, fino negativo, 1 em) |
| `\{` `\}` `\$` `\\` `\/` … | **escape**: el carácter tal cual, sin función |

```octave
text("$\Delta T_1$/n(BT 10.3 - 12.3 $\mu/rm$)", align="center") { 5 1 }
```

Las matemáticas se componen con **Latin Modern Math**, que `mg` embebe en la salida: la
figura se ve igual en cualquier máquina, sin fuentes ni TeX instalados.

> ⚠️ **Va `/n` y no `\n`.** La barra invertida consume todo lo alfabético que sigue —así se
> leen `\alpha` y `\nabla`—, así que `"uno\ndos"` buscaría un símbolo llamado `ndos`.

#### El escape: cómo escribir un carácter que es sintaxis

Una barra invertida seguida de un carácter **no alfabético** significa *ese carácter, literal*.
Es la salida para los cinco que el marcado se reserva:

| escribes | sale | por qué hacía falta |
|---|---|---|
| `\{` `\}` | `{` `}` | sin escapar agrupan (`{/bnegrita}`) |
| `\$` | `$` | sin escapar abre y cierra el modo math |
| `\\` | `\` | sin escapar empieza un `\comando` |
| `\/` | `/` | sin escapar puede cambiar la cara tipográfica |

```octave
text("a\{b\}c y el precio \$5") { 1 2 }   % → a{b}c y el precio $5
text("5 m\/s") { 1 1 }                    % → 5 m/s
```

> ⚠️ **La barra `/` se come la letra que sigue si es una de éstas: `b e i g r s c t $ n`.**
> Es la lista de códigos de cara (más `/n`, el salto de renglón), y por eso `text("5 m/s")`
> dibuja **`5 m`**: `/s` se lee «sans-serif» y la `s` desaparece. Toca a casi toda unidad con
> barra —`m/s`, `J/g`, `cal/g`, `1/e`—, mientras que `W/m2` se salva de casualidad, porque `m`
> no está en la lista. Escribe `\/` y no hay ambigüedad.
>
> `mg` **avisa** cuando el cambio de cara queda al final de la cadena, que es donde no puede
> equivocarse. En medio (`v (m/s)`) no avisa: ahí es indistinguible de un cambio de cara
> intencional, así que la defensa es escribir `\/`.

Dentro de `$…$` un `\{` escapado no es un carácter cualquiera: es un **delimitador**, y recibe
el espaciado que le toca por abrir o cerrar, igual que `(` y `[`.

> Todavía **no hay llave extensible** —la que crece con lo que encierra—. `\{` da una llave del
> tamaño de la fuente. Para abarcar varios renglones o un rango del dibujo, hoy se dibuja a
> mano.

En un texto multilínea, `valign` alinea **el bloque entero**, no cada renglón.

### Espaciado matemático y fracciones

Dentro de `$…$` **el espaciado lo pone `mg`, no tú**: los espacios que escribas se ignoran y el
hueco entre símbolos sale de su papel —relación, operador binario, función—, como en TeX. Así
`$F = ma$` y `$F=ma$` se ven idénticos (el `=` recibe su espacio a ambos lados) y `$a + b$` separa
el `+` como binario, mientras que un `-` inicial (`$-x$`) va pegado por ser unario. Para el ajuste
fino que la regla no acierte hay cuatro comandos: `\,` (fino), `\;` (grueso), `\!` (fino negativo)
y `\quad` (un cuadratín).

**Fracciones.** `\frac{numerador}{denominador}` compone la fracción en dos dimensiones —traza la
raya y centra numerador y denominador sobre ella—. Se **anidan** (`\frac{1}{1+\frac{1}{x}}`) y
colocan sus partes según la **altura real** del contenido, de modo que los subíndices del
numerador y los superíndices del denominador libran la raya. Van a tamaño pleno (display) y se
componen inline dentro de una fórmula mayor:

```octave
text("$F = G \frac{m_1 m_2}{r^2}$", size=12) { 1 7 }
```

**Acentos.** `\hat{n}` pone un circunflejo y `\vec{v}` una flecha, centrados sobre su base y a
su ancho — sobre una letra ancha salen anchos, sin tener que pedirlo aparte. La altura se toma
del contenido, así que `\hat{n}` se apoya en la equis y `\hat{A}` sube a la mayúscula.

```octave
text("$\vec{F} = m\vec{a}$   $\hat{n} \cdot \hat{r}$") { 1 5 }
```

> La marca se **dibuja**, no sale de la fuente, y es deliberado: un acento combinante tiene
> avance cero y se coloca con las tablas internas de la fuente, que MG no lee — habría que
> escribir el posicionado igual. Es el mismo criterio por el que la raya de `\frac` es un trazo.
> Por eso hay `\hat` y `\vec` y no toda la familia: `\bar`, `\tilde` y `\dot` entrarán cuando
> una figura los pida.

### Los símbolos que se escriben `\comando`

Son **110**, y esta es la lista completa. Se pueden escribir dentro o fuera de `$…$`.
La lámina [`examples/symbols.mg`](../examples/symbols.mg) los muestra dibujados, uno a uno.

**Letras griegas** (41) — minúsculas, sus variantes, mayúsculas, y la ħ:

```
\alpha \beta \gamma \delta \epsilon \zeta \eta \theta \iota \kappa \lambda \mu
\nu \xi \pi \rho \sigma \tau \upsilon \phi \chi \psi \omega
\varepsilon \vartheta \varpi \varrho \varsigma \varphi
\Gamma \Delta \Theta \Lambda \Xi \Pi \Sigma \Upsilon \Phi \Psi \Omega
\hbar
```

**Operadores y relaciones** (34):

```
\int \prod \sum \partial \nabla \surd \pm \cdot \times \div \oplus \otimes \oslash
\wedge \vee \cap \cup \diamond \bullet \sharp \neg
\leq \geq \neq \approx \cong \equiv \sim \propto \mid \in \ni \colon \angle
```

**Conjuntos, flechas y delimitadores** (23):

```
\subset \supset \subseteq \supseteq \forall \exists \therefore \bot
\leftarrow \rightarrow \leftrightarrow \Leftarrow \Rightarrow \Leftrightarrow
\uparrow \downarrow \Uparrow \Downarrow
\langle \rangle \lceil \rceil \lfloor \rfloor
```

**Otros** (12):

```
\aleph \wp \Re \Im \infty \prime \textdegree
\clubsuit \diamondsuit \heartsuit \spadesuit
```

> ⚠️ Un nombre que no esté en esta lista **no aborta la compilación**: avisa por la salida
> de error (`Warning: symbol name unknown alfa`) y **descarta el símbolo**, así que la
> figura se genera con un hueco donde iba el glifo. Si te falta un símbolo, el aviso está
> en la terminal aunque el `.svg` se haya escrito.

---

## 7. Expresiones y control de flujo

```octave
n = 60
r = 2 * sqrt(n) / 3
etiqueta = "v = " + str(r, 2)          % concatenación con +
xs = [0, 1.5, 3, 4.5]                  % lista
primero = xs[0]
```

**Funciones:** `sin` `cos` `tan` `asin` `acos` `atan` `atan2(y,x)` `sqrt` `abs` `exp` `ln`
`mod(a,b)` `deg(radianes)` `rad(grados)` `len(lista)` `str(x)` `str(x,decimales)` `gray(g)`.
Constantes: `pi`, `true`, `false`.

Las trigonométricas trabajan en **radianes**, y los ángulos de las primitivas (`arc`, `rotate`,
`view3d`…) en **grados**: `deg` y `rad` son el puente, y conviene usarlas en vez de multiplicar
a mano por `180/pi` — llevan la misma constante que el compilador, así que un ángulo escrito
por ti y uno calculado por él no pueden separarse en el último dígito.

Con `asin`/`acos` se resuelven **en el `.mg`** los ángulos de una construcción geométrica —por
ejemplo dónde recortar un arco ([§4](#4-primitivas))— en vez de medirlos sobre el dibujo.

> ⚠️ **`sqrt`, `asin`, `acos` y `ln` ABORTAN fuera de su dominio**, diciendo qué valor
> recibieron. Es deliberado: fuera de rango no dan un resultado raro, dan `NaN`, y un `NaN` que
> entra en una coordenada se propaga sin ruido hasta el archivo de salida. Si tu construcción
> puede salirse por redondeo, o porque el caso límite es legítimo —un arco que queda entero
> oculto—, acota tú el valor antes: es lo que hace `angulo_solido` con su recorte a [−1, 1], y
> esa saturación **es** la geometría, no un parche.

**Operadores:** `+ - * / ^`, comparación `== != < <= > >=`, lógicos `and` `or` `not`.

```octave
for i = 0 to n-1 {
    x = i/n
    polyline { (x) (x*x)   ((x+1/n)) (((x+1/n))*((x+1/n))) }
}

if r > 2 and n < 100 { text("grande") { 0 0 } } else { text("chica") { 0 0 } }
```

`for` acepta `step`: `for t = 0 to 1 step 0.05 { … }`.

> ⚠️ **En un bloque `{ }` los valores se separan por espacios**, así que un `+` o un `-` dentro de
> una coordenada la parte en dos: `{ 12 y-11 }` son **tres** términos (`12`, `y`, `-11`); lo que
> quieres es `{ 12 (y-11) }`. **Encierra en `( )` toda coordenada que sume o reste**; productos,
> cocientes, potencias y un menos inicial van sueltos (`x*2`, `x/n`, `x^2`, `-x`). Mezclar variables
> sueltas con coordenadas entre paréntesis es correcto: `{ x y (x+1) (y+1) }` ([detalle](#15-errores-comunes)).

> ⚠️ **Una llamada a función va PEGADA al paréntesis: `f(x)`.** Con un espacio de por medio —`f (x)`—
> el `(` es un término aparte, no una llamada; por eso en `{ x y (x+1) }` la `y` no se traga el
> paréntesis siguiente. Escribe siempre las llamadas sin espacio: `sqrt(n)`, `point_at(&p, 0.5)`.

> ⚠️ **Un literal de lista no se puede indexar.** `[10,20,30][1]` es error de sintaxis; hay
> que pasar por una variable (`xs = [10,20,30]` y luego `xs[1]`).

### Todas las formas de repetir

Hay **un solo lazo general**, `for`. Lo demás son construcciones que repiten algo concreto,
y lo que las distingue no es *qué* repiten sino **dónde van las copias**:

| construcción | repite | dónde van las copias |
|---|---|---|
| `for v = a to b [step s] { }` | lo que escribas en el cuerpo | donde el cuerpo diga — lo calculas tú |
| bloque de coordenadas `{ p1 p2 … }` | una **primitiva** | un ejemplar por punto |
| `place(Struct) { p1 p2 p3 … }` (§8) | una **struct** | un ejemplar por punto (3 o más), orientado a la tangente |
| `place(Struct, count=N) { p1 p2 }` (§8) | una **struct** | N repartidos por igual entre los dos puntos |
| `repeat(Struct, count=…)` (§8) | una **struct** | progresión, cada copia **relativa a la anterior** |
| `numbers`, `ticks` (§11) | una etiqueta / una marca | progresión `at` + `advance`·k |
| `sample(&p, n)` (§10) | — | no repite nada: **produce** n puntos |

💡 **Un bloque de coordenadas ya es un lazo.** `dot(2) { 0 5  1 5  2 5 }` son tres puntos,
`circle(0.4) { c1 c2 }` dos círculos, `text("×") { p1 p2 }` la misma cadena estampada dos
veces, `polybar(width=0.4) { … }` una barra por punto. **Todas** las primitivas funcionan
así, y no hay que hacer nada para que lo hagan. Con **tres o más** puntos, `place` es
exactamente eso mismo para **structs**, que no caben en un bloque de coordenadas — no son
dos ideas, es una con dos nombres, y el nombre aparte existe porque una struct no es una
primitiva.

> ⚠️ **Con DOS puntos `place` es otra cosa: una línea guía con algo encima, y dibuja la
> línea.** Para sembrar copias, da 3 o más puntos, o usa `count=` ([detalle](#15-errores-comunes)).

💡 **Lo que justifica `repeat` es la acumulación.** Es el único que compone la
transformación: con `transform=rotate(30)` la copia *k* va girada 30°·*k* respecto a la
anterior, y de ahí salen abanicos y espirales. Sin acumular, un `for` que invoca la struct
hace lo mismo — y se lee mejor. Si no estás acumulando, usa `for`.

💡 **`sample` no es iteración**: no dibuja ejemplares, devuelve un trayecto de *n* puntos
repartidos por longitud de arco. Produce **datos**, y quien los dibuja es la primitiva a la
que se los pases.

> ⚠️ **`to` es inclusivo; `count` es una cantidad.** `for i = 0 to 4` da **cinco** vueltas
> (0,1,2,3,4) y `repeat(…, count=4)` da **cuatro** copias. Es el off-by-one de la casa; los
> constructos que cuentan (`repeat`, `numbers`, `ticks`) usan `count`, y sólo `for` usa `to`.

---

## 8. Estructuras

Un `struct` agrupa elementos y se coloca, escala, gira y repite como una unidad. Su cuerpo
vive en una **caja unitaria** que se mapea al sitio donde lo invoques.

```octave
struct Cuadro(lado = 1) {
    circle(lado/2) { 0 0 }
    polyline(closed=true) { -1 -1  1 -1  1 1  -1 1 }
}

Cuadro()                                   % en su sitio natural
Cuadro(at=(3,2), scale=0.5, rotate=30)     % colocado
for i = 0 to 11 { Cuadro(rotate=i*7.5, scale=1+i*0.35) }
```

Argumentos de colocación: `at=(x,y)`, `scale=`, `rotate=` (grados), `transform=`.

**Parámetros de tipo trayecto** — se marcan con `&` en la declaración:

```octave
struct Nivel(&onda, w = path_width(&onda)) {
    world_window 0 w -2 2
    bezier(&onda)
}
Nivel(&pw3)
```

**Colocar y ajustar:**

```octave
place(Cuadro) { 0 0  3 0  3 3 }            % 3+ puntos: una instancia por punto
place(Cuadro, count=5) { 0 0  4 0 }        % 2 puntos: 5 repartidas por igual
place(Cuadro, gap=0.5) { 0 0  4 0 }        % 2 puntos: línea guía CON hueco, 1 instancia
place(Cuadro, r=5, from=0, to=90) { 5 5 }  % locus ARCO: lo dibuja y pone la struct al final
place(Sat, rx=3, ry=6, at=[30, 210]) { 0 0 }  % locus ELÍPTICO; at= son GRADOS, y con at
                                           % el locus NO se dibuja (ya está trazado aparte)
fit(Cuadro) { 1 1  4 3 }                   % ajustado a ese rectángulo
fit(Cuadro, stretch=true) { 1 1  4 3 }     % deformando (si no, MEET centrado)
repeat(Cuadro, count=6, at=(0,0), advance=(1.2,0), rotate=15)
```

> El orden de las esquinas de `fit` **importa y refleja**: `{ .5 0  0 1 }` espeja en x.

> ⚠️ **El locus de `place` se escribe literal**: hoy no acepta un trayecto con nombre, ni en
> el bloque (`place(P) { &datos }`) ni como argumento (`place(P, &datos)`). Es la excepción a
> la regla de §3 —el `&` vale en las primitivas de dibujo, no aquí—, y para sembrar una struct
> sobre puntos calculados hay que escribirlos en las llaves.

**Recursión** — una struct puede invocarse **a sí misma**, y ahí es donde una definición
corta produce una figura que no se podría dibujar a mano. La condición de paro es un `if`:

```octave
struct arbol(theta, phi, n, s) {
    polyline { 0 0  0 0.5 }                          % el tronco
    if n > 0 {                                       % <- la condición de paro
        { translate 0 0.5  scale s  rotate theta   arbol(theta, phi, n-1, s) }
        { translate 0 0.5  scale s  rotate phi     arbol(theta, phi, n-1, s) }
    }
}
arbol(28, -28, 8, 0.6)        % 511 segmentos de una struct de cuatro líneas
```

Un tronco y, en su punta, dos copias menores de sí mismo. Cada rama va en **su propio
bloque** porque `rotate`/`scale` son estado con ámbito (§9): así la segunda no hereda el
giro de la primera. Cambiar los dos ángulos cambia el árbol entero
([`examples/fractal_tree.mg`](../examples/fractal_tree.mg) dibuja dos con la misma struct).

`max_depth n` fija el tope de expansión; **32** por default. Cuenta **anidamiento, no
invocaciones**: mil copias colocadas una al lado de otra no lo tocan, y vale igual para las
cuatro formas de invocar (directa, `place`, `fit`, `repeat`).

> `max_depth` es la **red**, no el freno. El freno es el `if`. Una recursión sin condición
> de paro no dibuja «hasta donde alcance»: aborta con el error de profundidad, nombrando la
> struct.

---

## 9. Transformaciones

Sentencias que afectan a lo que sigue en el bloque, en orden de escritura:

```octave
{ translate 3 2   rotate 30   scale 2 1   shear 0.4 0
  Cuadro() }
```

También como argumento por-primitiva o de colocación: `polyline(transform=rotate(30)) { … }`, `Cuadro(transform=scale(2))`. (Un constructor; varios yuxtapuestos solo en `repeat`.)

> Bajo un `transform`, el texto mueve su **ancla**; los glifos no se deforman (salvo
> `rotate`, que sí los gira).

> ⚠️ **`scale` con dos factores VARIABLES: paréntesis en el segundo si sigue otra sentencia en
> el mismo renglón.** `scale` es la única transformación de aridad variable (uno o dos
> factores), así que un identificador suelto detrás es ambiguo: puede ser su segundo factor o
> puede ser el comienzo de la sentencia siguiente. La regla es que solo se toma como factor si
> **termina la sentencia**; para forzarlo, paréntesis, que ninguna sentencia empieza por `(`:
> `{ scale sx sy }` ✅ (`sy` termina la sentencia, se toma) y
> `{ scale sx (sy)   shear kk 0 }` ✅ (con otra sentencia detrás, paréntesis).
>
> Sin ellos, `sy` arranca una sentencia nueva y se traga lo que siga; el error habla de lo que
> venía después —«variable no definida: shear»— y señala una línea que está bien escrita. Con
> literales (`scale 2 1`) no hay ambigüedad y no hace falta nada.

Un `rotate`, un `scale` con factores distintos en x e y o un `shear` sobre un `circle`, un
`arc` o una `ellipse` dan la elipse **girada** que corresponde, con sus ángulos intactos: se
transforma la figura entera, no sus radios por separado. Vale en los tres formatos de salida.

> ⚠️ **`rotate` gira el plano, no la figura alrededor de su centro.** Si el centro no está en
> el origen, girar lo desplaza. Para girar en el sitio, lleva el origen al centro primero y
> dibuja ahí: `{ translate 0 cy   rotate 15   ellipse(a, b) { 0 0 } }`. Es el tropiezo que
> descentra dos elipses concéntricas hacia lados opuestos.

---

## 10. Álgebra de trayectos

Un trayecto no solo se escribe: se **opera**. Todo lo de esta sección toma trayectos y
devuelve otro trayecto (o una medida suya), así que se anida y se compone. Nada de esto hace
falta para dibujar —§3 basta—, pero es lo que permite construir una curva a partir de otras
en vez de teclear sus puntos.

**Operaciones** — producen otro trayecto, así que se anidan:

| | |
|---|---|
| `concat(a, b, …)` | encadena, en ese orden — cuantos quieras, y **sin darles la vuelta por su cuenta** (para orientar, `reverse`) |
| `reverse(p)` | invierte el orden de los puntos |
| `flip_x(p)` `flip_y(p)` | espeja |
| `transpose(p)` | intercambia x↔y |

```octave
path media    = { 0 0  1 2  2 2  3 0 }
path completa = concat(reverse(flip_x(&media)), &media)   % un perfil simétrico
bezier(&completa)
```

**Generadores** — construyen un trayecto a partir de parámetros, no de puntos escritos:

```octave
path onda = sine(half_cycles=2, amplitude=1) { 0 0  4 0 }   % media entre dos extremos
path suave = smooth { 0 0  1 2  3 1  4 3 }                   % pasa por esos NODOS
```

> ⚠️ **Un bloque `{ }` es siempre literal, y los generadores solo aceptan bloque.**
> `smooth { &nodos }` no vale —dentro de las llaves van coordenadas, no trayectos— y
> `smooth(&nodos)` **tampoco**: `smooth` y `sine` exigen sus puntos escritos ahí mismo. La
> forma de paréntesis con `&` es de las primitivas que CONSUMEN un trayecto —`polyline(&p)`,
> `polygon(&p)`, `bezier(&p)`, como en §3—, no de las que lo generan. Suavizar un trayecto que
> ya tienes en una variable **no se puede hoy**; sus nodos hay que escribirlos literales.

> ⚠️ **Para RELLENAR una curva generada va `bezier(&p, fill=)`, no `polygon(&p, fill=)`.**
> `sine` y `smooth` devuelven puntos de **control** de Bézier, y `polygon` los lee como
> vértices: el relleno sale con esquinas rectas —una cometa en vez del lóbulo— y nada avisa,
> porque las dos son lecturas legítimas de la misma lista de puntos. `bezier` los interpreta
> como lo que son, y el relleno cierra solo por la cuerda que une los extremos. Es lo que hace
> `onda_electromagnetica` para pintar cada media onda.

**Acumular en un lazo** — para una curva cuyo número de piezas depende de una variable, algo
que `concat` no cubre, porque sus piezas hay que escribirlas una por una:

```octave
path w = { 0 0 }
for k = 0 to n {
    path w += sine(half_cycles=1, phase=90, amplitude=amp) { (k) 0  (k+1) 0 }
}
```

> ⚠️ **`+=` SUELDA piezas relativas**: cada una se traslada para continuar donde acabó la
> anterior, así que se escriben relativas, no absolutas ([detalle](#15-errores-comunes)).

**Una curva calculada, punto por punto.** Es el caso más común del lenguaje —la curva sale
de una fórmula, no de coordenadas medidas— y tiene su propia forma, porque una pieza de **un
solo punto se añade tal cual, en coordenadas absolutas**:

```octave
A = 1   tau = 4   T = 1.5   n = 200

path onda = { 0 (A) }                       % semilla: el primer punto, literal
for i = 1 to n {
    t = i * 10 / n
    path onda += { (t) (A*exp(-t/tau)*cos(2*pi*t/T)) }
}
polyline(&onda)
```

Cada vuelta calcula su punto y lo añade; al final el trayecto se dibuja **una vez**, con la
primitiva que quieras (`polyline` para quebrada, `bezier` o `smooth` para curva). Es lo que
hace [`examples/tiro_parabolico.mg`](../examples/tiro_parabolico.mg).

> 🔑 **Las dos reglas de `+=`, que no son la misma:**
> - Pieza de **un punto** → se añade **absoluto**. Es la forma de arriba, la de una curva
>   muestreada: tú calculas cada punto en el sistema de la figura.
> - Pieza de **dos o más** → el primer punto es un **ancla**: se pega al final de lo
>   acumulado y no se duplica, y los demás son **desplazamientos desde él**. El valor del
>   ancla da igual: `+= { 5 5  6 6 }` y `+= { 0 0  1 1 }` producen exactamente lo mismo.
>   Por eso las piezas de varios puntos se escriben empezando en `{ 0 0 … }`.

> ⚠️ **Un `for` NO puede ir dentro de un bloque de coordenadas.** `polyline { for i = … }`
> es error de sintaxis: el bloque `{ }` es una lista de coordenadas, no un cuerpo de
> sentencias. Para generar puntos con un lazo, se acumulan en un `path` como arriba.

> ⚠️ **`path x = …` se evalúa al DIBUJAR; `path x += …` en el acto.** Por eso la semilla de un
> acumulador tiene que ser un literal, sin variables que el lazo vaya a pisar ([detalle](#15-errores-comunes)).

**Reducciones trayecto→número** — leen una medida de un trayecto: `path_width(&p)`,
`path_x_min_at_y(&p, y [, expand])`, `path_x_max_at_y(&p, y [, expand])`. Operan sobre el
polígono de control, así que son exactas en trayectos monótonos y aproximadas en una bézier
genuinamente curva.

**Muestreo** — leen geometría de un trayecto en un parámetro `t ∈ [0,1]`, recorrido por
**longitud de arco** (así `t = 0.5` es el medio *geométrico*, no la mitad de los segmentos):

<!-- mg-noexec: firmas, no código: los corchetes marcan el argumento opcional -->
```octave
sample(&p, n [, curve=b])       % n puntos equiespaciados por arco → un TRAYECTO
point_at(&p, t [, curve=b])     % el punto en t → [x, y]
angle_at(&p, t [, curve=b])     % ángulo (grados) de la tangente en t → número
```

El argumento **`curve`** (nombrado o posicional) fija cómo se interpreta el trayecto: `false`
(default) trata los puntos como **vértices** (interpolación lineal — exacto para una
polilínea; sobre una bézier toca el *polígono de control*, no la curva), `true` los trata
como **controles bézier**
(evalúa la curva — toca la *curva* real). Usos típicos:

```octave
polyline(sample(&curva, 60, curve=true))          % densificar una bézier gruesa
dot(sample(&curva, 8, curve=true), size=2)        % 8 marcadores repartidos por arco
Marca(at=point_at(&curva, 0.5, curve=true))       % colocar una struct en el medio
marker(shape="arrow",
       marker_orient=angle_at(&curva, 0.5, curve=true)) { point_at(&curva, 0.5, curve=true) }
```

El último renglón muestra las dos formas de usar el punto: en `at=` de una **struct**, o
**directo en el bloque `{ }`** de una primitiva (§4: el bloque acepta un punto donde iría un
par de escalares). Un marcador se orienta con `marker_orient=` (grados), no con `rotate=`.

---

## 11. Gráficas

`plot` mapea **unidades de datos** a una caja física y dibuja su contenido dentro:

```octave
plot(x=(0,10), y=(0,100), box=(0,0, 9,4.5), frame=true) {
    polyline { 0 0  1 1  2 4  3 9  4 16  5 25 }
    marker(size=4, shape="cross") { 0.9 10  2.5 15 }

    xaxis(step=2, label="x")
    yaxis(step=25, label="$y = x^2$", grid=true, grid_dash="dashed")

    rule(x=3, color="red", dash="dashed", label="umbral", label_at="legend")

    legend(at="top-left", font_size=8)

    table(at="top-right", col_widths=(20,30), decimals=3, label_col=true) {
        row("Media", 4.21)
        row("SD",    0.87)
    }
}
```

**`plot`**: `x=(de,a)`, `y=(de,a)`, `box=(x0,y0,x1,y1)` (mundo; default = la ventana),
`xscale`/`yscale="log"`, `grid=`, `grid_dash=`, `frame=`.

**Ejes** (`xaxis`/`yaxis` dentro de `plot`, o `axis` suelto con su bloque de dos puntos):

| | |
|---|---|
| malla | `step`, `start`, `ticks` (`"out"`/`"in"`/`"both"`/`"none"`/`"grid"`), `tick_size`, `minor` |
| rótulos de marca | `tick_labels` (true/false), `decimals`, `strip_zero`, `tick_label_gap`, `tick_label_size`, `tick_label_font`, `tick_label_align`, `tick_label_valign` |
| nombre del eje | `label`, `label_at` (`"center"`/`"start"`/`"end"`), `label_gap`, `label_size`, `label_font` |
| rango de datos | `from`, `to` (dentro de un `plot` los hereda de `x=`/`y=`; en un `axis` suelto los pones tú) |
| geometría y estilo | `base=`, `extend=`, `scale="log"`, `field=`, `color`, `line_width`, `dash`, `grid`, `grid_dash` |

> **La nomenclatura, que es la que usa todo el mundo:** `label` es el **nombre del eje** (el
> `xlabel` de matplotlib) y `tick_labels` son **los números de las marcas**. `title` queda
> reservado para el encabezado del plot.

**`rule`** — el valor notable (un umbral, un nivel), distinto de la malla regular:
`x=` o `y=` (uno de los dos), `to=`, `label=`, `label_at` (`"axis"`/`"legend"`), `color`,
`dash`, `line_width`. **Sin `to=` la línea cruza la caja entera**, que es lo que suele
quererse; `to=` la corta en ese valor de datos.

> ⚠️ **«No lineal» no quiere decir «log».** La escala log es para datos *multiplicativos* y
> no existe en valores ≤ 0; si tus puntos solo están mal repartidos, lo que quieres son
> líneas guía, no una escala ([detalle](#15-errores-comunes)).

**Con un eje logarítmico, `plot` mapea posiciones, no formas.** Un eje lineal envuelve el
contenido en una matriz y lo transforma entero; el log no puede —no es una transformación
afín—, así que remapea **coordenada por coordenada**. La consecuencia es una regla, no un
caso raro: lo que no sea una coordenada se queda como lo escribiste, en unidades de la
página. Eso ya valía para `line_width`, la tipografía y el radio de `dot`; vale igual para el
**radio** de `circle`/`arc`/`ellipse` y el `width` de `polybar`.

```octave
plot(x=(1,1000), y=(0,10), xscale="log") {
    circle(0.5) { 10 5 }     % círculo de radio 0.5 EN LA PÁGINA, centrado en el dato (10,5)
    dot(3)      { 10 5 }     % lo correcto para marcar un dato: físico, 3 pt
}
```

> ⚠️ **El mismo `circle(0.5)` mide cosas distintas según el eje.** Con eje lineal la matriz lo
> transforma con todo lo demás (y en un marco anisótropo sale elipse); con eje log conserva su
> tamaño de página. No es un descuido: bajo un logaritmo una circunferencia de datos no es ni
> círculo ni elipse —es un huevo asimétrico—, así que no hay forma «correcta» que dibujar.
> Para marcar datos usa `dot`, que es físico y por eso se comporta igual en las dos rutas.

**`legend`** — `at=` combina `"top"`/`"center"`/`"bottom"` con `"-left"`/`"-right"`; más
`margin`, `sample_width`, `sample_height`, `gap`, `row_gap`, `font_size` (todos en pt). Con
bloque, cada entrada declara su muestra; **sin bloque**, las entradas las ponen los `rule`
con `label_at="legend"`:

```octave
legend(at="top-right") {
    entry("Teoría")      { polyline { 0 0.5  1 0.5 } }
    entry("Experimento") { marker(3, shape="cross") { 0.5 0.5 } }
}
```

**`table`** — `col_widths=(…)` en pt (obligatorio; su tamaño fija el nº de columnas),
`row_height`, `decimals`, `align`, `border`, `fill`, `label_col`, `label_font`, `font_size`,
`margin`.
`at=` acepta una **esquina nombrada** (dentro de un plot) o un **punto `(x,y)`** (fuera).

### Generadores sueltos

`numbers`, `ticks`, `axis` y `grid` funcionan también **fuera** de un `plot`, en
coordenadas del mundo. Los tres primeros no llevan mapeo: colocas tú el primero y dices
cuánto avanza cada paso, con el par `at=` (dónde empieza) y `advance=` (cuánto se mueve por
paso). Es el molde compacto del que `axis` es la versión que se arma sola.

```octave
numbers(from=0, by=0.1, count=10, decimals=1, at=(0.5, 0.2), advance=(1, 0))
ticks(10, mark=(0, 0.15), at=(0.5, 0.35), advance=(1, 0))
grid(xstep=1, ystep=0.5) { 0 0  10 5 }
axis(from=0, to=100, step=25, label="v") { 0 0  10 0 }
```

| | |
|---|---|
| **`numbers`** | `from` (primer **valor**), `by` (cuánto crece), `count`, `decimals`, `prefix`, `suffix` (cadenas que envuelven al número), `at`, `advance` |
| **`ticks`** | `count` (también posicional: `ticks(10, …)`), `mark=(dx,dy)` (el segmento que se traza en cada marca, y su dirección), `at`, `advance` |
| **`grid`** | `xstep`, `ystep` sobre el rectángulo del bloque `{ esq-inf-izq  esq-sup-der }`, inclusivo en ambos extremos |
| **`axis`** | todo lo de la tabla de arriba; el bloque son **dos puntos** (los extremos del eje) y el rango de datos va en `from=`/`to=` |

Las etiquetas de `numbers` heredan el estado de texto vigente (`font`, `font_size`, `align`,
`color`), igual que un `text()`.

> ⚠️ **Bajo escala log**, colocar structs dentro del contenido es error (su matriz no compone
> con un mapeo no afín), igual que `grid()`/`ticks()`/`axis()` pelados: usa `grid=` y
> `xaxis`/`yaxis`.

---

## 12. Bibliotecas

Una **biblioteca es un `.mg` de structs** que se trae con `include`:

```octave
include "pseudo3d.mg"      % relativo al archivo que lo incluye
prisma(2, 1, 1.5)               % ancho, alto, profundidad
```

El `include` debe preceder al uso, y **falla el compilado** si el archivo no resuelve. En
`lib/` vienen `pseudo3d.mg` (piezas puestas en la escena de la cámara de
[§13](#13-escenas-pseudo-3d): `prisma` y `lamina`, de caras planas, y `cono` y `cilindro`, cuya
**silueta se calcula**), dos **iconos** —`satellite.mg` (`struct Satellite`) y `sun.mg`
(`struct Sun`, el Sol: disco de radio 1 y un anillo de rayos, `rays=` cuántos)— y tres **mapas
del mundo** en proyección ortográfica, generados de datos reales (Natural Earth) con
`tools/geo2mg.py`: `polar_map.mg` (`PolarMap`, vista desde el polo norte),
`fulldisk_map.mg` (`FullDiskMap`, ecuatorial) y `mapa_p30_n55.mg` (`Mapa`, lat 30, lon −55).

Los tres son structs normales, normalizados a **radio 1**, así que se colocan como cualquier
otra —`scale` es el radio del globo en unidades de mundo— y aceptan `grid=false` para quitar
la retícula, más `ocean=`/`land=`/`grid_color=`/`grid_width=`:

```octave
include "../lib/mapa_p30_n55.mg"
Mapa(scale=5, at=(0, 0.5), grid=false)      % globo de radio 5 centrado en (0, 0.5)
```

Las piezas de `pseudo3d.mg` se colocan en **coordenadas de la escena**, con `pos=`, y viven bajo
la `view3d` vigente. En `cono` y `cilindro`, `axis` es el vector que va de `pos` al otro extremo:
lleva dirección y longitud juntas, así que no hay parámetro de altura. `prisma` acepta además `ex`/`ey`/`ez`,
las direcciones de sus tres aristas, para ponerlo **inclinado**: se dan como vectores —el mismo
estilo con que `plane3d` toma `u` y `v`— y no como un ángulo, porque un ángulo obliga a elegir
alrededor de qué, y eso depende de la figura.

```octave
include "../lib/pseudo3d.mg"
view3d(azimuth=-25, elevation=22)
prisma(1.2, 0.8, 1.0, pos=[0, 0, 0])
g = rad(14)                                  % la misma caja, inclinada
prisma(1.2, 0.8, 1.0, pos=[0, 1.4, 0], ex=[cos(g), sin(g), 0], ey=[-sin(g), cos(g), 0])
cono(0.9, axis=[0, 3.2, 0], pos=[3.5, 0, 0])
cilindro(0.85, axis=[2.4, 1.3, -0.6], pos=[6.5, 0.6, 0])
```

> Van en **alambre**: se trazan los bordes, no se rellena el cuerpo. Y la silueta de un cono
> pide que el ápice caiga **fuera** de su base proyectada; si no, se está mirando el cono por
> dentro y solo se dibuja la base.

**Dónde busca el `include`:** primero **junto a tu archivo** (ruta relativa), y luego en la
**biblioteca instalada** (`make install` copia `lib/*.mg` a `$PREFIX/share/metagrafica/lib`).
Lo local **pisa** lo instalado. Por eso `include "satellite.mg"` a secas usa la lib del
sistema, y en el repo se escribe la ruta relativa (`include "../lib/satellite.mg"`).

---

## 13. Escenas pseudo-3D

MG dibuja en el plano. Lo que esta sección añade es una **cámara**: la declaras una vez y
después describes la figura con coordenadas del espacio, en vez de calcular a mano dónde cae
cada cosa en el papel.

Lo que se gana no es sintaxis, es que **la cámara pasa a ser un parámetro**: cambias la
elevación y la malla, los rayos y las piezas se mueven juntos, porque todos salen del mismo
número. Es lo contrario de colocar cada pieza hasta que se vea bien.

> ⚠️ Esto es **simulación** de 3-D en 2-D: no hay z-buffer, ni superficies ocultas, ni
> iluminación, ni perspectiva. El orden de pintado es el **orden de escritura** —dibuja primero
> lo lejano—, y qué cara de un cuerpo queda detrás lo decides tú. Para 3-D de verdad, Blender;
> MG no lo va a sustituir.

### Qué usar para qué

Una figura de éstas casi nunca es *una* cosa: suele ser tres o cuatro clases de pieza en la
misma escena. Conviene mirarla por partes y preguntarse, de cada una, a cuál de estas
pertenece:

| lo que quieres dibujar | con qué | ejemplo trabajado |
|---|---|---|
| un círculo, un arco o una curva que **vive en un plano** del espacio | `plane3d`, y dentro dibujas en 2-D | `angulo_solido` |
| una **curva rellena**, una onda, marcadores sobre ella | `plane3d` + las primitivas de siempre | `onda_electromagnetica` |
| un **rayo**, una cota, un rótulo colgado de un punto: algo que no pertenece a ninguna pieza | `xyz()` | los tres |
| la **silueta** de un cono o un cilindro | se calcula en el `.mg`; la receta está en el encabezado de `irradiancia` | `irradiancia` |
| una **caja**, una placa, un **cono** o un **cilindro** | `prisma` `lamina` `cono` `cilindro` de `lib/pseudo3d.mg` ([§12](#12-bibliotecas)) | — |
| un **arco de ángulo**, una flecha, un contorno a mano alzada | nada especial: se dibuja en el papel | `irradiancia` |

**Por dónde empezar.** Si la figura es sobre todo círculos o curvas puestos en planos, lee
`angulo_solido`: es el caso puro y no usa nada más. Si además hay que rellenar o poner
marcadores, `onda_electromagnetica`. Y si aparece un cuerpo redondo, `irradiancia`, que es la
única que calcula una silueta.

> ⚠️ **Dos avisos sobre la última fila.** El arco que marca un ángulo es anotación del **papel**,
> no de la escena, porque el ángulo entre dos direcciones proyectadas **no** es la proyección del
> ángulo del espacio. Si quieres que mida de verdad, dibújalo en el plano que contiene a las dos
> direcciones —es lo que hace `irradiancia` con su φ— y entonces el barrido del arco *es* el
> ángulo.

### Los ejes

**x a la derecha, y arriba, z hacia el observador.** El plano del papel es **xy**; el suelo, si
la figura tiene suelo, es **xz**, y una altura es `y`.

La convención se eligió para que `view3d(azimuth=0, elevation=0)` sea la **identidad**: sin
cámara —o con la cámara al frente— el lenguaje se comporta exactamente como en el resto de esta
referencia, y una figura plana no paga nada por que esto exista.

### `view3d` — la cámara

Sentencia de estado con alcance, como `translate` o `rotate` ([§9](#9-transformaciones)): vale
desde donde aparece hasta el fin del bloque. Hay dos proyecciones y **una sola sentencia**, con
`type=`: una figura declara *una* cámara.

```octave
view3d(azimuth=35, elevation=25)                     % axonométrica — la de default
view3d(type="oblique", angle=45, foreshorten=0.5)    % oblicua (caballera/gabinete)
```

**Axonométrica ortográfica** (`type="axonometric"`, o nada) — acimut θ, que gira la escena sobre
la vertical, y elevación φ, que levanta la cámara sobre el horizonte:

    X = x·cos θ + z·sin θ
    Y = y·cos φ + (x·sin θ − z·cos θ)·sin φ

Los casos que conviene tener a mano para orientarse: θ=φ=0 es la identidad; θ=0, φ=90° da la
**planta** (se ve desde arriba); θ=90°, φ=0 da la vista desde +x.

**Oblicua** (`type="oblique"`) — la cara frontal, la de `z=0`, **conserva su forma real
siempre**, sea cual sea `angle`; solo la profundidad recede, en la dirección `angle` y acortada
por `foreshorten` (1 = caballera, 0.5 = gabinete):

    X = x − z·f·cos(angle)
    Y = y − z·f·sin(angle)

Es la proyección de las figuras de libro donde una cara tiene que poder medirse. Defaults:
`azimuth=0 elevation=0`; `angle=45 foreshorten=0.5`. Los ángulos van en **grados**.

### `plane3d` — dibujar sobre un plano de la escena

Sentencia con alcance, como las transformaciones: dentro del bloque, las coordenadas son
**locales al plano** y el dibujo corriente se proyecta solo.

```octave
plane3d(at=[0,0,0], u=[1,0,0], v=[0,1,0])   % los defaults: el plano del papel
```

`at` es el origen local, y `u` y `v` son los dos vectores del plano, que **llevan la escala**:
el local `(0,0)` cae en `at`, el `(1,0)` en `at+u` y el `(0,1)` en `at+v`.

```octave
view3d(azimuth=35, elevation=25)

% una pantalla vertical, y un anillo dibujado SOBRE ella
{ plane3d(at=[8,0,0], u=[0,0,1], v=[0,1,0])
  rectangle { 0 0  1.4 4.2 }
  circle(0.6) { 0.7 2.1 } }

% los meridianos de una esfera: siete elipses en dos renglones
for k = 0 to 6 {
    lam = k * pi / 7
    { plane3d(u=[cos(lam), 0, sin(lam)], v=[0, 1, 0])
      circle(2) { 0 0 } }
}
```

📌 **Ese `circle` sale como la elipse exacta de su proyección**, no como una aproximación ni
como una elipse que hubo que medir: el motor representa una elipse por su centro y sus
semidiámetros conjugados, que es precisamente la forma cerrada de proyectar un círculo del
espacio. Vale igual en EPS, SVG y PDF.

📌 **Dentro de un `plane3d` funciona el dibujo 2-D de siempre** —`sine`, `smooth`, `bezier`, un
`polygon` relleno, `place` de una struct—, porque lo único que cambió es la matriz vigente. Una
onda sobre un plano del espacio es un `sine` corriente; no hay que muestrear nada a mano. Y el
`from`/`to` de un `arc` sigue siendo el ángulo **del plano**, no el de la página.

### `xyz(x, y, z)` — un punto suelto del espacio

Para lo que no pertenece a ningún plano: los rayos, las líneas de construcción, el rótulo que
cuelga de un punto. Devuelve el punto **ya proyectado**, así que se escribe donde va una
coordenada.

```octave
view3d(azimuth=35, elevation=25)
h = 6   d = 1.2

polyline { xyz(0, h, 0)   xyz(3*d, 0, 5*d) }     % un rayo, de la óptica al suelo
text("CIV") { xyz(3*d, 0, 5*d) }
```

> ⚠️ **`xyz()` va FUERA de todo `plane3d`.** El punto que devuelve ya está en coordenadas del
> documento; dentro de un `plane3d` se transformaría **dos veces** y caería en cualquier parte.
> Dentro de un plano se dibuja en coordenadas locales, que es justo lo que hace innecesario a
> `xyz()` ahí.

### Piezas sólidas

`lib/pseudo3d.mg` ([§12](#12-bibliotecas)) trae dos, construidas sobre lo anterior: `prisma(w,
h, d, pos=[x,y,z])`, una caja de tres caras visibles sombreadas de claro a oscuro, y
`lamina(h, d, pos=…)`, la placa de una sola cara con trama. Ambas se colocan en coordenadas de
la **escena** con `pos=` — `at=` sigue siendo la palabra de colocación de una struct
([§8](#8-estructuras)), y sirve para correr la pieza en la página *después* de proyectarla.

⚠️ El orden en que `prisma` pinta sus caras es el correcto para un observador arriba y a la
derecha (elevación positiva, o `angle` oblicuo entre 0 y 90°). Con la cámara del otro lado hay
que reordenar a mano: sin z-buffer, eso es **modelado**, y ninguna biblioteca puede adivinarlo.

---

## 14. Cómo falla el compilador

MG **aborta** en vez de producir una figura a medias: un error de evaluación, un `include`
que no resuelve o un conteo impar de coordenadas terminan la compilación con código 1. Los
**avisos** no fatales sí siguen, y salen por `stderr`: un color desconocido (cae a negro), un
carácter sin glifo (se descarta) o una figura cuyo dibujo entero quedó fuera del lienzo
([§15](#15-errores-comunes)) — ahí no hay nada que corregir por el compilador, pero sí algo
que mirar.

Eso es deliberado: **un documento inconsistente no debe producir salida**. En una figura
derivada de fórmulas, un error de física suele aparecer como error de compilación — dar mal
la profundidad de un pozo de Morse hace que su punto de retorno deje de existir, y lo que
salta es `ln: argumento no positivo`.

**Compilar por partes.** `exit`, en una línea al **nivel superior** del archivo, detiene ahí
la lectura: lo que sigue se ignora, incluidos sus errores de sintaxis. Sirve para levantar
una figura por etapas sin comentar el resto ni mantener copias.

```octave
polyline { 0 0  1 1 }
exit                       % de aquí para abajo, como si no estuviera

polygon { 2 2   3 2        % a medio escribir: sin cerrar la llave, y sobra una coord
```

> `exit` es de tiempo de **lectura**, no de dibujo: dentro de un `if` no sería condicional
> (la condición se evalúa después), así que anidarlo es error. Y no sirve para parar una
> recursión — para eso, el `if` de §8.

> ⚠️ **`exit` calla los errores de sintaxis de más abajo, pero no los léxicos** (un `@`
> suelto, una letra acentuada fuera de una cadena) ([detalle](#15-errores-comunes)).

---

## 15. Errores comunes

Los tropiezos que se repiten, con el síntoma primero — porque cuando ocurren no se sabe
todavía cuál es la causa. Los tres primeros son, con diferencia, los más frecuentes.

**No se ve nada, o se ve a medias, o «solo salen 3 puntos».** Casi siempre los datos caen
**fuera de `world_window`**, que es un recorte fijo y no se ajusta a lo que dibujas. Si tu
curva vive en `y` de 9 a 15 y la ventana va de 0 a 8, el compilador dibujó bien: fuera de
cuadro. **No hay ningún error**, y por eso el síntoma parece un bug del motor (una figura
vacía, un EPS casi en blanco). Antes de sospechar del motor, comprueba que el rango de tus
datos quepa en la ventana. Le pasa hasta a quien conoce el lenguaje.

> Cuando **todo** cae fuera, MG sí avisa (`la figura sale EN BLANCO`) y te dice en cm dónde
> quedó el dibujo y dónde el lienzo. Si se sale solo en parte, se calla: salirse por un borde
> es a menudo deliberado.

Y el caso hermano, que no es un recorte sino una **proporción**: los datos caben en la
ventana y aun así la figura sale como una tira ilegible. El motor es isométrico ([§2](#2-el-lienzo-y-las-unidades)),
así que una `world_window 0 1 0 6` en un `display_size 12 8` usa la misma escala en los dos
ejes —la que hace caber el eje largo— y el eje x entero acaba midiendo 1.3 cm de los 12. No
hay aviso porque no hay nada mal: dibujar así es lo que pediste. **La ventana del mundo es
para figuras cuyos dos ejes comparten unidad** (un plano, una órbita, un mapa). En cuanto x
e y midan cosas distintas, o sus rangos difieran por más de dos o tres veces, lo que quieres
es un [`plot`](#11-gráficas), que mapea datos a una caja y estira cada eje por su cuenta.

**Un `{ }` de coordenadas se queja de un número impar, o la figura sale deformada.** Dentro
de un bloque los valores se separan por **espacios**, así que un `+` o un `-` dentro de una
coordenada la parte en dos: `{ 12 y-11 }` son **tres** términos (`12`, `y`, `-11`), no dos —
lo que quieres es `{ 12 (y-11) }`. La regla: **encierra en `( )` toda coordenada que sume o
reste**; productos, cocientes y potencias van sueltos (`x*2`, `x/n`, `x^2`), y las variables
sueltas conviven con coordenadas entre paréntesis (`{ x y (x+1) (y+1) }`). El conteo impar es
error de compilación, con línea y columna, así que al menos no falla en silencio. Y una
**llamada a función va pegada** (`f(x)`): con un espacio, `f (x)`, el `(` es un término aparte,
no una llamada.

**`dot(2, &p)` no compila y el error habla de una expresión inesperada.** El trayecto va
**siempre como primer argumento** y el resto nombrado detrás: `dot(&p, size=2)`,
`marker(&p, shape="x")`, `polybar(&p, width=0.5)`. En el bloque `{ }` tampoco vale.

**El texto se ve enorme al reducir la figura.** El texto es una cantidad **física** (pt) y
los paneles son cantidad de **mundo**, así que achicar `display_size` no achica el texto: lo
agranda en relación con el dibujo, y los rótulos se encabalgan. Para una figura más pequeña,
baja `font_size`; el lienzo no es la palanca.

**`polyline(…, outlinefill)` da error, y `polyline(align="center")` también.** Ninguno de
`outlinefill`, `font`, `font_size`, `align` y `valign` es un atributo de estilo genérico: como
atributo entre paréntesis solo valen donde son argumentos **propios** de la primitiva, o sea
en `text()` —`text("m", align="center", font_size=9) { 5 1 }` es correcto—. Fuera de ahí van
como **sentencia de estado**. `outlinefill` no vale como atributo en ninguna primitiva: el
contorno sobre un relleno se pide dando `color=` junto a `fill=`, que ya implica contornear.

**Una curva acumulada con `+=` sale desplazada o encimada.** `+=` **suelda piezas
relativas**: cada una se traslada para que su primer punto continúe donde acabó la anterior.
Por eso las piezas se escriben relativas (`{ 0 0  … }`) y no con coordenadas absolutas — si
las escribes absolutas esperando que se acumulen tal cual, la pieza se desplaza.

**Un acumulador `path` da valores raros dentro de un lazo.** `path x = …` es **diferido**:
guarda la expresión y la evalúa al dibujar, así que si la semilla lleva variables que el lazo
pisa, leerá los valores finales. `path x += …` evalúa en el acto. La semilla, por tanto, tiene
que ser un literal. Y al reasignar dentro del lazo se repite la palabra `path`.

**`place` me dibujó una línea que yo no pedí.** Con **dos** puntos `place` no es «una copia
por punto» sino una **línea guía con algo encima** —la flecha con etiqueta de toda la vida— y
dibuja la línea: `shift` mueve el ejemplar, `both_sides=true` pone dos, `gap=` parte la línea
para dejar hueco a un letrero, y la forma sobre **arco** (`r=`, `from=`, `to=`) también la
dibuja. Para sembrar copias sin línea: da **3 o más** puntos, o usa `count=` sobre los dos.

**La malla logarítmica no pasa por mis puntos.** «No lineal» no quiere decir «log». La escala
log es para datos **multiplicativos** —cada paso multiplica— y no existe en valores ≤ 0. Si
tus puntos simplemente no están regularmente espaciados (una parábola, una `1/r`, algo que
cruza el cero), ninguna malla log va a pasar por ellos. Lo que quieres no es una escala de
eje sino **líneas guía en los puntos**, dibujadas en el mismo lazo que genera los datos.
Malla regular y «líneas donde están los puntos» son cosas distintas — la misma distinción que
hay entre `grid=` y `rule`. Ver `examples/tiro_parabolico.mg`.

**«Este `{` abre un bloque de ámbito y su contenido son sentencias, pero empieza con un
número».** El bloque de coordenadas tiene que empezar en la **misma línea** en la que acabó la
cabeza de la primitiva —el `)` de los argumentos, o el nombre si no lleva—:

<!-- mg-expect-error -->
```octave
rectangle(fill="red") { 0 0  4 3 }         % ✅
rectangle(fill="red")
    { 0 0  4 3 }                           % ❌ error, y señalando al 0
```

Por dentro no hay restricción: los argumentos, una lista y el propio bloque de coordenadas
pueden partirse en varias líneas y llevar comentarios, con tal de que el `{` haya abierto ya.

```octave
rectangle(gradient=["magenta",      % una lista partida…
                    "red"]) { 0 0  4 3 }   % ✅ …pero el { sigue arriba
polyline { 0 0
           4 4 }                           % ✅ el bloque abierto sigue en la línea de abajo
```

**Por qué el compilador lo cuenta así:** un `{ }` suelto **es un constructo válido** —el bloque
de ámbito de [§9](#9-transformaciones), el de `{ translate 3 2  Cuadro() }`—. Así que no ve una
primitiva rota: ve una primitiva sin coordenadas y, a continuación, un bloque de ámbito lleno de
números donde esperaba sentencias. El error nombra esa causa y te da **la línea que hay que
juntar**. Dentro del cuerpo de un `for` o un `if` el mismo `{ 0 0 }` da la otra mitad del
diagnóstico: ahí no hay nada que juntar, y lo que se quiere es acumular los puntos en un `path`
con `+=` ([§10](#10-álgebra-de-trayectos)).

**Puse `exit` y sigue fallando más abajo.** `exit` calla los errores de **sintaxis**
posteriores —llaves sin cerrar, una primitiva mal escrita, coordenadas impares—, que es el
caso normal del código a medio escribir. No calla los **léxicos**: un carácter que el lenguaje
no reconoce (`@`, o una letra acentuada fuera de una cadena) aborta igual, porque el archivo
se convierte en tokens **entero** antes de leerse. Una nota en prosa debajo del `exit` va en
un comentario `%`.

> 💡 **Los comentarios `%` sí admiten acentos** —y eñes, y cualquier cosa: el lexer se salta
> el comentario entero sin mirarlo. La regla de los acentos vale para el **código**, no para
> lo que escribes sobre él.

**Un lazo hace una vuelta de más o de menos.** `to` es **inclusivo** y `count` es una
**cantidad**: `for i = 0 to 4` da cinco vueltas y `repeat(…, count=4)` da cuatro copias.

---

## 16. Referencia rápida

**Primitivas** · `polyline` `polygon` `rectangle` `circle` `ellipse` `arc` `dot` `marker`
`bezier` `smooth` `polybar` `sine` `compound` `text`

**Sentencias de estado** · `color` `fill` `line_width` `dash` `hatch` `hatch_gap`
`outlinefill` `font` `font_size` `align` `valign`

**Configuración** · `display_size` `world_window` `max_depth` `exit`

**Transformaciones** · `translate` `rotate` `scale` `shear`

**Escena pseudo-3D** · `view3d(azimuth=, elevation=)` / `view3d(type="oblique", angle=,
foreshorten=)` · `plane3d(at=, u=, v=)` · función `xyz(x, y, z)`

**Control** · `for … to … [step …] { }` · `if … { } else { }` · `struct` (recursivo) · `include`

**Colocación** · `place` `fit` `repeat` · invocación `Nombre(at=, scale=, rotate=, transform=)`

**Trayectos** · `path x = …` · `path x += …` · `&nombre` · `concat` `reverse` `flip_x`
`flip_y` `transpose` · generadores `sine` `smooth` · reducciones `path_width`
`path_x_min_at_y` `path_x_max_at_y` · muestreo `sample` `point_at` `angle_at` (argumento `curve=`)

**Gráficas** · `plot` `xaxis` `yaxis` `axis` `rule` `legend`/`entry` `table`/`row` `grid`
`numbers` `ticks`

**Funciones** · `sin` `cos` `tan` `asin` `acos` `atan` `atan2` `sqrt` `abs` `exp` `ln` `mod`
`deg` `rad` `len` `str` `gray` `xyz`
· constantes `pi` `true` `false`
