# MetaGráfica — referencia del lenguaje

> ⚠️ **BORRADOR** — para revisión.
>
> **Convención mientras dure:** Alejandro edita la prosa directamente y deja lo estructural
> como `[[AS: ...]]`. Antes de darlo por terminado hay que barrer que no quede ninguna
> (`grep -n '\[\[AS:' docs/*.md`) y quitar este bloque.

Este documento describe **lo que el compilador hace hoy**, completo y sin historia. Para el
porqué de las decisiones está [`especificacion_mg.md`](../especificacion_mg.md), que además
describe cosas que aún no existen; para las opciones del binario, `man mg`.

Está pensado para leerse **de corrido una vez** y consultarse después. Las tablas del final
son el resumen.

---

## 1. Qué es

Describes *qué es la figura* y `mg` la compila:

```bash
mg figura.mg              # → figura.eps
mg figura.mg salida.svg   # el formato lo elige la extensión: .eps .svg .pdf
```

Un archivo mínimo no necesita preámbulo:

```octave
circle(1) { 0 0 }
polyline { 0 0  2 1 }
```

**Lo que MG no es, a propósito.** No es un lenguaje de programación de propósito general:
tiene variables, expresiones, `for`, `if` y poco más. No analiza datos — `polybar` recibe
intervalos ya contados, no observaciones; para llegar de una hoja de cálculo a un `.mg` está
`tools/hist2mg.py`. No hace 3D. No compone párrafos.

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

> ⚠️ **La causa nº 1 de «no se ve nada» o «se ve a medias»: los datos caen FUERA de la
> ventana.** `world_window` no se ajusta solo a lo que dibujas — es un recorte fijo. Si tu
> curva vive en `y` de 9 a 15 y la ventana es `y` de 0 a 8, no verás casi nada, y **no hay
> ningún error**: el compilador dibujó bien, simplemente fuera de cuadro. Los síntomas
> engañan (una figura vacía, «solo salen 3 puntos», un EPS casi en blanco) y parecen bugs
> del motor. Antes de sospechar del motor, **comprueba que el rango de tus datos quepa en
> `world_window`**. Es el tropiezo más común, y le pasa hasta a quien conoce el lenguaje.

> ⚠️ **El motor es isométrico.** La escala es la misma en x y en y, así que si la proporción
> de `world_window` no coincide con la de `display_size`, la figura queda centrada con
> márgenes (*letterbox*). Para llenar el lienzo, iguala las dos proporciones.

> ⚠️ **Consecuencia poco intuitiva:** como el texto es físico y los paneles son cantidad de
> mundo, **achicar `display_size` no achica el texto** — lo hace relativamente más grande y
> encima los rótulos. Para una figura más pequeña, baja `font_size`.

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
pasa a una primitiva con el sigilo `&`:

```octave
path perfil = { 0 0  1 2  3 1  4 0 }
polyline(&perfil)                          % dibujarlo…
polygon(&perfil)                           % …o rellenarlo, es el mismo trayecto
```

**Un trayecto es la misma lista de puntos, la interprete quien la interprete.** Por eso
`&perfil` sirve igual a `polyline` (lo traza), `polygon` (lo rellena) o `polybar` (lee cada
punto como el tope de una barra) — cada primitiva lo lee a su manera.

> ⚠️ **`&trayecto` va como PRIMER argumento, siempre.** `dot(&p, size=2, color="lime")` ✅,
> pero `dot(2, &p)` (radio primero) ❌ y `dot(size=2) { &p }` (en el bloque) ❌. El radio y
> el estilo van **nombrados** después del trayecto: `dot(&p, size=2)`, `marker(&p,
> shape="x")`, `polybar(&p, width=0.5)`. El error que sale —«se esperaba una expresión, se
> encontró `&`»— no lo dice, así que conviene saberlo de antemano.

**Operaciones sobre trayectos** — producen otro trayecto, así que se anidan:

| | |
|---|---|
| `concat(a, b, …)` | encadena, en ese orden. **Variádica, sin auto-reversión** |
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

**Acumular en un lazo** — para una curva cuyo número de piezas depende de una variable, algo
que `concat` (de aridad fija en el fuente) no cubre:

```octave
path w = { 0 0 }
for k = 0 to n {
    path w += sine(half_cycles=1, phase=90, amplitude=amp) { (k) 0  (k+1) 0 }
}
```

> ⚠️ **`+=` SUELDA piezas relativas, no añade puntos absolutos.** Cada pieza se traslada para
> que su primer punto continúe desde el final de la anterior. Por eso las piezas del ejemplo
> empiezan en `{ 0 0 }` (o `{ (k) 0 … }` ya conectado): se escriben *relativas* y `+=` las
> encadena. Si escribes puntos **absolutos** sueltos esperando que se acumulen tal cual, el
> resultado no es el que crees — la pieza se desplaza. (Un punto suelto sí se añade sin
> trasladar, pero para acumular puntos absolutos punto a punto es más claro escribir cada
> `{ x y }` como su propia pieza y dejar que la traslación sea nula porque ya coinciden.)

> ⚠️ **`path x = …` es DIFERIDO** (guarda la expresión y la evalúa al dibujar); `path x += …`
> **evalúa en el acto**. Por eso la semilla de un acumulador no puede llevar variables que el
> lazo vaya a pisar — usa un literal, como el `{ 0 0 }` de arriba. Y al reasignar dentro del
> lazo se repite la palabra `path`.

**Reducciones trayecto→número** — leen una medida de un trayecto: `path_width(&p)`,
`path_x_min_at_y(&p, y [, expand])`, `path_x_max_at_y(&p, y [, expand])`. Operan sobre el
polígono de control, así que son exactas en trayectos monótonos y aproximadas en una bézier
genuinamente curva.

**Muestreo** — leen geometría de un trayecto en un parámetro `t ∈ [0,1]`, recorrido por
**longitud de arco** (así `t = 0.5` es el medio *geométrico*, no la mitad de los segmentos):

```octave
sample(&p, n [, curve=b])       % n puntos equiespaciados por arco → un TRAYECTO
point_at(&p, t [, curve=b])     % el punto en t → [x, y]
angle_at(&p, t [, curve=b])     % ángulo (grados) de la tangente en t → número
```

El flag **`curve`** (nombrado o posicional) fija cómo se interpreta el trayecto: `false`
(default) trata los puntos como **vértices** (interpolación lineal — exacto para una
polilínea, sobre una bézier toca la *envolvente*), `true` los trata como **controles bézier**
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

## 4. Primitivas

Cada primitiva lleva sus **coordenadas entre `{ }`** y sus **argumentos entre `( )`**:

```octave
polyline { 0 0  1 2  3 1 }                 % polilínea abierta
polyline(closed=true) { 0 0  1 0  1 1 }    % contorno cerrado, sin repetir el vértice
polygon { 0 0  1 0  1 1 }                  % relleno
rectangle { 0 0  4 3 }                     % dos esquinas opuestas
circle(2) { 5 5  9 5 }                     % un círculo por punto; 2 = radio (mundo)
ellipse(3, 1.5) { 5 5 }                    % radios x, y
arc(2, from=0, to=120) { 0 0 }             % grados, positivo = antihorario
dot(2) { 1 1  2 3 }                        % disco de radio FÍSICO 2 pt
marker(3, shape="cross") { 1 1 }           % símbolo de radio físico
bezier { 0 0  1 2  3 2  4 0 }              % p0 c1 c2 p1 [c1 c2 p2 …]  (3k+1 puntos)
smooth { 0 0  1 2  3 1  4 3 }              % NODOS: las tangentes las deriva el compilador
polybar(width=0.5) { 1 3  2 5  3 4 }       % cada punto es el centro superior de una barra
sine(half_cycles=2, amplitude=1) { 0 0  4 0 }
```

`polyline`, `polygon` y `bezier` admiten **subtrayectos disjuntos** separados por `;`:

```octave
polyline { 0 0  1 1 ;  2 0  3 1 }          % dos trazos, mismo estilo
```

En el bloque, una coordenada puede ser un par de escalares (`x y`) **o un punto `[x,y]`** —una
lista de dos, como devuelve `point_at` (§3) o un literal—, y se mezclan:

```octave
marker(shape="x") { point_at(&curva, 0.5) }   % un punto directo
polyline { 0 0  (p)  5 5 }                     % escalares y un punto p mezclados
```

**Formas de `marker`:** `circle`, `square`, `diamond`, `cross`, `x`, `triangle`, `arrow`,
`circle-dot`. También el nombre de un `struct` propio. `dot(r)` es el atajo del disco y no
lleva `shape=`.

**`compound`** une varias primitivas en **un solo** trazo relleno (§9.4):

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

| se puede como sentencia | y como atributo |
|---|---|
| `color`, `fill`, `line_width`, `dash`, `hatch`, `hatch_gap`, `outlinefill`, `font`, `font_size`, `align`, `valign` | `color=`, `fill=`, `line_width=`, `dash=`, `hatch=`, `hatch_gap=` |

> ⚠️ **`outlinefill`, `font`, `align` y `valign` NO son atributos por-primitiva.**
> `polyline(..., outlinefill)` da error. Como atributo, el contorno sobre relleno se pide
> **dando `color=` junto a `fill=`**: eso ya implica contornear.

**Relleno y contorno.** `fill=` enciende el relleno; `color=` es el trazo. Juntos = relleno
contorneado. `fill="none"` apaga el relleno.

**Colores:** los **148 nombres CSS** (`"steelblue"`, `"orange"`…), `"#rrggbb"`, o
`gray(0.4)`. Sin excepciones: `green` y `orange` valen lo que valen en CSS.

**Líneas:** `line_width` en pt. `dash` acepta `"solid"`, `"dashed"`, `"dotted"`,
`"longdashed"`, `"shortdashed"`, `"dashdot"`, `"dashdotdot"`.

**Tramas** (§4.11): `hatch` acepta un **ángulo libre** en grados o un estilo nombrado
(`"hatch"` 45°, `"hatchback"` 135°, `"crosshatch"` ambas). `hatch_gap` es la separación en
pt. La trama usa el **color de relleno**; no hay `hatch_color`.

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

```octave
text("$\Delta T_1$/n(BT 10.3 - 12.3 $\mu/rm$)", align="center") { 5 1 }
```

Las matemáticas se componen con **Latin Modern Math**, que `mg` embebe en la salida: la
figura se ve igual en cualquier máquina, sin fuentes ni TeX instalados.

> ⚠️ **Va `/n` y no `\n`.** La barra invertida consume todo lo alfabético que sigue —así se
> leen `\alpha` y `\nabla`—, así que `"uno\ndos"` buscaría un símbolo llamado `ndos`.

En un texto multilínea, `valign` alinea **el bloque entero**, no cada renglón.

---

## 7. Expresiones y control de flujo

```octave
n = 60
r = 2 * sqrt(n) / 3
etiqueta = "v = " + str(r, 2)          % concatenación con +
xs = [0, 1.5, 3, 4.5]                  % lista
primero = xs[0]
```

**Funciones:** `sin` `cos` `tan` `atan2(y,x)` `sqrt` `abs` `exp` `ln` `mod(a,b)` `len(lista)`
`str(x)` `str(x,decimales)` `gray(g)`. Los ángulos van en **radianes** (`cos(a*pi/180)`).
Constantes: `pi`, `true`, `false`.

**Operadores:** `+ - * / ^`, comparación `== != < <= > >=`, lógicos `and` `or` `not`.

```octave
for i = 0 to n-1 {
    x = i/n
    polyline { (x) (x*x)   ((x+1/n)) (((x+1/n))*((x+1/n))) }
}

if r > 2 and n < 100 { text("grande") { 0 0 } } else { text("chica") { 0 0 } }
```

`for` acepta `step`: `for t = 0 to 1 step 0.05 { … }`.

> ⚠️ **La trampa de los bloques de coordenadas.** Dentro de `{ }` los valores se separan por
> espacios, así que `+` y `-` y los paréntesis interactúan: `{ 12 y-11 }` son **tres**
> términos (`12`, `y`, `-11`), no dos. La regla práctica: **o parentizas todas las
> coordenadas o ninguna** — `{ (12) (y-11) }`. Un conteo impar es error de compilación con
> línea y columna, así que no falla en silencio.

> ⚠️ **Un identificador seguido de `(` es una llamada.** En coordenadas, `dx (h+dy)` se lee
> como `dx(h+dy)`. Parentiza: `(dx) (h+dy)`.

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
| `numbers`, `ticks` (§10) | una etiqueta / una marca | progresión `at` + `advance`·k |
| `sample(&p, n)` (§3) | — | no repite nada: **produce** n puntos |

💡 **Un bloque de coordenadas ya es un lazo.** `dot(2) { 0 5  1 5  2 5 }` son tres puntos,
`circle(0.4) { c1 c2 }` dos círculos, `text("×") { p1 p2 }` la misma cadena estampada dos
veces, `polybar(width=0.4) { … }` una barra por punto. **Todas** las primitivas funcionan
así, y no hay que hacer nada para que lo hagan. Con **tres o más** puntos, `place` es
exactamente eso mismo para **structs**, que no caben en un bloque de coordenadas — no son
dos ideas, es una con dos nombres, y el nombre aparte existe porque una struct no es una
primitiva.

> ⚠️ **Con DOS puntos, `place` es otra cosa: una línea guía con algo encima**, y **dibuja la
> línea**. Es la flecha con etiqueta de toda la vida. Sin `count` pone **un** ejemplar (dónde
> lo dice `shift`, 1 = el segundo punto; `both_sides=true` pone dos), y `gap=` **parte la
> línea** para dejarle hueco a un letrero. `place(S, r=…, from=…, to=…) { cx cy }` es la
> versión sobre un **arco**, que también lo dibuja. En cambio `count=N` sobre los dos puntos
> reparte N ejemplares por igual y **no** dibuja la línea. Cuatro comportamientos bajo un
> nombre: si lo que quieres es sembrar copias, da los puntos (3 o más) o usa `count`.

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

**Parámetros de tipo path** — el sigilo `&` en la declaración:

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
fit(Cuadro) { 1 1  4 3 }                   % ajustado a ese rectángulo
fit(Cuadro, stretch=true) { 1 1  4 3 }     % deformando (si no, MEET centrado)
repeat(Cuadro, count=6, at=(0,0), advance=(1.2,0), rotate=15)
```

> El orden de las esquinas de `fit` **importa y refleja**: `{ .5 0  0 1 }` espeja en x.

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

Sentencias que afectan a lo que sigue en el bloque (§11.1), en orden de escritura:

```octave
{ translate 3 2   rotate 30   scale 2 1   shear 0.4 0
  Cuadro() }
```

También como argumento por-primitiva o de colocación: `polyline(transform=rotate(30)) { … }`, `Cuadro(transform=scale(2))`. (Un constructor; varios yuxtapuestos solo en `repeat`.)

> Bajo un `transform`, el texto mueve su **ancla**; los glifos no se deforman (salvo
> `rotate`, que sí los gira).

---

## 10. Gráficas

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
| geometría y estilo | `base=`, `extend=`, `scale="log"`, `color`, `line_width`, `dash`, `grid`, `grid_dash` |

> **La nomenclatura, que es la que usa todo el mundo:** `label` es el **nombre del eje** (el
> `xlabel` de matplotlib) y `tick_labels` son **los números de las marcas**. `title` queda
> reservado para el encabezado del plot.

**`rule`** — el valor notable (un umbral, un nivel), distinto de la malla regular:
`x=` o `y=` (uno de los dos), `to=`, `label=`, `label_at` (`"axis"`/`"legend"`), `color`,
`dash`, `line_width`.

> ⚠️ **«No lineal» no quiere decir «log».** Una escala **log** es para datos
> **multiplicativos** —cada paso multiplica, como potencias de diez—, y *no existe* en
> valores ≤ 0. Si tus puntos no están regularmente espaciados pero tampoco crecen
> multiplicando (una parábola, una `1/r`, cualquier cosa que pase por cero o se vuelva
> negativa), **una malla log no pasaría por ellos**. Lo que quieres entonces no es una
> escala de eje sino **líneas guía en los puntos**: dibújalas tú, en el mismo lazo que genera
> los datos (`polyline` de cada punto a los ejes). Malla regular y «líneas donde están los
> puntos» son cosas distintas — es la misma distinción entre `grid=` y `rule`. Ver
> `examples/tiro_parabolico.mg`.

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
`row_height`, `decimals`, `align`, `border`, `fill`, `label_col`, `label_font`, `font_size`.
`at=` acepta una **esquina nombrada** (dentro de un plot) o un **punto `(x,y)`** (fuera).

**Generadores sueltos** (§13): `numbers`, `ticks`, `axis`, `grid` funcionan también fuera de
un `plot`, con su propio bloque de coordenadas.

> ⚠️ **Bajo escala log**, colocar structs dentro del contenido es error (su matriz no compone
> con un mapeo no afín), igual que `grid()`/`ticks()`/`axis()` pelados: usa `grid=` y
> `xaxis`/`yaxis`.

---

## 11. Bibliotecas

Una **biblioteca es un `.mg` de structs** que se trae con `include` (§15):

```octave
include "pseudo3d.mg"      % relativo al archivo que lo incluye
prisma(2, 1, 1.5)               % ancho, alto, profundidad
```

El `include` debe preceder al uso, y **falla el compilado** si el archivo no resuelve. En
`lib/` viene `pseudo3d.mg` (volumen simulado por proyección oblicua, sin z-buffer: el orden
de pintado es el de escritura).

---

## 12. Errores

MG **aborta** en vez de producir una figura a medias: un error de evaluación, un `include`
que no resuelve o un conteo impar de coordenadas terminan la compilación con código 1. Los
avisos no fatales (un color desconocido, que cae a negro) sí siguen.

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

> ⚠️ **Lo que `exit` sí calla y lo que no.** Calla los errores de **sintaxis** de más abajo
> —llaves sin cerrar, una primitiva mal escrita, coordenadas impares—, que es el caso normal
> del código a medio escribir. **No** calla los **léxicos**: un carácter que el lenguaje no
> reconoce (`@`, o una letra acentuada fuera de una cadena) sigue abortando, porque el
> archivo se convierte en tokens **entero** antes de leerse. Si dejaste una nota en prosa
> debajo del `exit`, ponla en un comentario `%`.

---

## 13. Referencia rápida

**Primitivas** · `polyline` `polygon` `rectangle` `circle` `ellipse` `arc` `dot` `marker`
`bezier` `smooth` `polybar` `sine` `compound` `text`

**Sentencias de estado** · `color` `fill` `line_width` `dash` `hatch` `hatch_gap`
`outlinefill` `font` `font_size` `align` `valign`

**Configuración** · `display_size` `world_window` `max_depth` `exit`

**Transformaciones** · `translate` `rotate` `scale` `shear`

**Control** · `for … to … [step …] { }` · `if … { } else { }` · `struct` (recursivo) · `include`

**Colocación** · `place` `fit` `repeat` · invocación `Nombre(at=, scale=, rotate=, transform=)`

**Trayectos** · `path x = …` · `path x += …` · `&nombre` · `concat` `reverse` `flip_x`
`flip_y` `transpose` · generadores `sine` `smooth` · reducciones `path_width`
`path_x_min_at_y` `path_x_max_at_y` · muestreo `sample` `point_at` `angle_at` (flag `curve=`)

**Gráficas** · `plot` `xaxis` `yaxis` `axis` `rule` `legend`/`entry` `table`/`row` `grid`
`numbers` `ticks`

**Funciones** · `sin` `cos` `tan` `atan2` `sqrt` `abs` `exp` `ln` `mod` `len` `str` `gray`
· constantes `pi` `true` `false`
