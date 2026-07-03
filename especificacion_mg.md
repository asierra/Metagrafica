# MetaGráfica V2 — Especificación del Lenguaje

> **Estado:** Borrador de trabajo. Las secciones marcadas con ⚠️ están abiertas a decisión.

---

## Decisiones de diseño ya tomadas

| Tema | Decisión |
|---|---|
| Variables | Asignación directa `x = 5.5`, sin `SET` |
| Colores | Nombres modernos: `"blue"`, `"#4080FF"` |
| Argumentos | Mixtos: posicionales primero, nombrados después (como Python) |
| Compatibilidad V1 | Traductor Python, no modo dual en el compilador |
| Structs | Con parámetros nativos: `struct Nombre(args) { }` |
| Loops | `for x = 0 to 10 step 1 { }` |
| Nombres | Comandos descriptivos en minúsculas: `polyline`, `circle`, `rectangle`, `bezier`, `text` |
| Caso (case) | **Todo en minúsculas**; `snake_case` para compuestos inevitables (`world_window`, `tick_size`). Se **recomienda Capitalizar** los nombres de structs del usuario (`Flecha`, `Tick`) para distinguirlas de los comandos |
| Paths | Secuencias de pares `x y`; primitivas multi-punto aplican la operación en cada punto |
| Transformaciones | Bloque `transform(translate=, rotate=, scale=, shear=) { }` (local), ver §11 |
| Posición de pluma | `moveto(x,y)` fija el punto actual; `step(...)` define el avance entre elementos, ver §12 |
| Condicionales | `if cond { } else { }` con comparadores y `and`/`or`, ver §6.1 |
| Recursión | Las structs pueden autoinvocarse; paro con `if`, límite con `max_depth`, ver §8.1 |
| Aspect ratio | Espacio isométrico por construcción: escala uniforme (*meet*) + margen; deformación solo con `stretch=true`, ver §3.1 |

---

## 1. Estructura Léxica

- **Identificadores (`ID`):** `[a-zA-Z_][a-zA-Z0-9_]*`
- **Números (`NUMBER`):** `[0-9]+(\.[0-9]+)?` (punto flotante)
- **Cadenas (`STRING`):** texto entre comillas dobles `"..."`
- **Comentarios:** desde `%` hasta fin de línea
- **Delimitadores:** `{ }` para bloques de geometría, `( )` para argumentos, `,` entre parámetros
- **Paths nombrados:** `&nombre` referencia a un path definido previamente

**Convención de nombres.** Todos los comandos y palabras clave del lenguaje van en minúsculas (`polyline`, `place`, `transform`, `for`). Los nombres compuestos inevitables usan `snake_case` (`world_window`, `tick_size`). El compilador distingue mayúsculas y minúsculas, por lo que se recomienda **Capitalizar los nombres de structs definidas por el usuario** (`Flecha`, `Tick`): así un lector distingue de un vistazo lo que es del lenguaje (minúscula) de lo que es del usuario (Mayúscula inicial).

---

## 2. Gramática Central (EBNF)

```ebnf
Program     ::= Statement*
Statement   ::= ConfigStmt
              | Assignment
              | PathDef
              | StructDef
              | ForStmt
              | IfStmt
              | WithStmt
              | TransformStmt
              | CompoundStmt
              | RepeatStmt
              | PenStmt
              | Generator
              | Placement
              | Primitive
              | TextStmt
              | Import

ConfigStmt  ::= "display_size" NUMBER NUMBER
              | "world_window" NUMBER NUMBER NUMBER NUMBER (ID "=" ArgValue)*

Assignment  ::= ID "=" Expression
PathDef     ::= "path" ID "=" PathExpr

StructDef   ::= "struct" ID "(" [ParamList] ")" "{" Statement* "}"
ForStmt     ::= "for" ID "=" Expression "to" Expression ["step" Expression] "{" Statement* "}"
IfStmt      ::= "if" Condition "{" Statement* "}" [ "else" "{" Statement* "}" ]
WithStmt    ::= "with" "(" ArgList ")" "{" Statement* "}"
Import      ::= "include" STRING

TransformStmt ::= "transform" "(" ArgList ")" "{" Statement* "}"
CompoundStmt  ::= "compound" "(" [ArgList] ")" "{" (Primitive | Placement)* "}"
RepeatStmt    ::= "repeat" "(" ID "," ArgList ")"
PenStmt       ::= "moveto" "(" Expression "," Expression ")"
              | "step"   "(" ArgList ")"
Generator     ::= "ticks"   "(" ArgList ")"
              | "numbers" "(" ArgList ")"
              | "axis"    "(" ArgList ")" "{" PointList "}"
              | "grid"    "(" ArgList ")" "{" PointList "}"

Primitive   ::= PrimName "(" [ArgList] ")" "{" PointList "}"
TextStmt    ::= "text" [ "(" ArgList ")" ] "{" STRING "}"

Placement   ::= "place" "(" ID [ "," ArgList ] ")" "{" Locus "}"
              | "fit"   "(" ID ")" "{" PointList "}"
Locus       ::= PointList | "&" ID

PathExpr    ::= "{" PointList "}"                % path literal
              | "bezier"  "{" PointList "}"
              | "spline"  ["(" ArgList ")"] "{" PointList "}"
              | "smooth"  "{" PointList "}"
              | "trail"   "(" ArgList ")"
              | "tile"    "(" "&" ID "," "times" "=" NUMBER ")"
              | "concat"  "(" "&" ID "," "&" ID ")"
              | "reverse" "(" "&" ID ")"
              | "normalize" "(" "&" ID ")"
              | "fitrect" "(" "&" ID ")" "{" PointList "}"

PrimName    ::= "polyline" | "polygon" | "circle" | "rectangle"
              | "arc" | "ellipse" | "dot" | "bezier"

ParamList   ::= Param ("," Param)*
Param       ::= ID ["=" Expression]              % parámetros con default, p. ej. size=0.2
ArgList     ::= Arg ("," Arg)*
Arg         ::= Expression | ID "=" ArgValue     % posicionales primero, nombrados después
ArgValue    ::= Expression | STRING | Tuple | TransformList
Tuple       ::= "(" Expression "," Expression ")"
TransformList ::= TransformCall TransformCall*   % composición: rotate(30) scale(0.95)
TransformCall ::= ("translate" | "rotate" | "scale" | "shear") "(" ExpressionList ")"
PointList   ::= (Expression Expression)*

Condition   ::= Comparison (("and" | "or") Comparison)*
Comparison  ::= Expression RelOp Expression
RelOp       ::= "<" | ">" | "<=" | ">=" | "==" | "!="

Expression  ::= Term (("+" | "-") Term)*
Term        ::= Factor (("*" | "/") Factor)*
Factor      ::= NUMBER | ID | "&" ID | FuncCall | "(" Expression ")" | "-" Factor
FuncCall    ::= ID "(" [ExpressionList] ")"
ExpressionList ::= Expression ("," Expression)*
```

---

## 3. Espacio de trabajo

```text
display_size 12 8        % dimensiones físicas del canvas en cm
world_window 0 24 0 16   % coordenadas de usuario: xmin xmax ymin ymax
```

Las coordenadas en todos los comandos están en unidades de usuario (`world_window`). El compilador aplica la transformación a las unidades de salida.

### 3.1 Aspect ratio: espacio isométrico por construcción

V2 mapea la `world_window` al `display_size` con un **único factor de escala uniforme**, el mayor que hace caber la ventana completa en el canvas (semántica *meet* de `preserveAspectRatio` en SVG). Una unidad de usuario mide lo mismo en x que en y, **siempre**.

- **Proporciones distintas** → la ventana se centra en el canvas y el sobrante queda como margen. El anclaje se controla con `align=` (`"center"` default; `"left"`, `"right"`, `"bottom"`, `"top"`, combinables como `"bottom-left"`).
- **Deformación solo explícita** — `stretch=true` reproduce el estiramiento por eje de V1, para quien lo quiera deliberadamente:

```text
world_window 0 24 0 16                    % isométrico, centrado (default)
world_window 0 24 0 16 align="bottom"     % isométrico, anclado abajo
world_window 0 24 0 16 stretch=true       % estira por eje, como V1
```

**Consecuencia central del diseño.** Con el espacio isométrico, círculos, arcos, rotaciones, shear, structs y placements funcionan sin compensación alguna: la garantía de no-deformación es del sistema de coordenadas, no de cada primitiva. Desaparecen del motor los ajustes de V1 (radio de círculo normalizado solo con el eje y, structs colocadas a escala `min(dvx,dvy)`, anti-escalas por `ratio` en los placements), que eran compensaciones parciales de la normalización por eje al cuadrado unitario.

*(V1: el parser normalizaba cada eje por separado a [0,1]² y el backend estiraba con `scale(dvx,dvy)`; el manual pedía como "buena práctica" que `WW` y `$D` tuvieran la misma proporción.)*

---

## 4. Primitivas gráficas

Los atributos de estilo se pasan como argumentos nombrados. Los argumentos posicionales son los parámetros geométricos propios de cada primitiva. El bloque `{ }` contiene la lista de puntos.

**Argumentos de estilo comunes (todos opcionales y nombrados):**
- `color="blue"` / `color="#4080FF"` — color de trazo
- `fill="red"` / `fill="#RRGGBB"` — color de relleno (activa relleno sólido)
- `width=2.5` — grosor de línea
- `dash="dashed"` — patrón de línea (ver §4.9)
- `hatch=45` — relleno con trama en lugar de sólido (ver §4.10)

#### Color, gris y contorno

- **Color** se especifica con nombre (`"blue"`, `"red"`, `"cyan"`, …) o hex (`"#RRGGBB"`).
- **Gris** es solo un color: usar `"gray"`, `"lightgray"` o un hex neutro como `"#808080"`. No hay un comando de gris separado. *(V1: `LGRAY`/`FGRAY g` → `color`/`fill` con el gris equivalente.)*
- **Contorno de una forma rellena.** Por defecto una primitiva con `fill=` no dibuja su borde. Si además se da `color=`, se traza el contorno en ese color (convención tipo SVG):

```text
polygon(fill="cyan")                { 0 0  5 0  5 5  0 5 } % solo relleno, sin borde
polygon(fill="cyan", color="black") { 0 0  5 0  5 5  0 5 } % relleno + borde negro
```

*(V1: un valor negativo en `FILL`/`FCOLOR`/`FGRAY` o el prefijo `-` en un color, p. ej. `"-red"`, activaba el contorno. En V2 el contorno es simplemente la presencia de `color=`.)*

### 4.1 polyline — polilínea abierta

```text
polyline(color="black", width=1) {
    0 0   5 3   10 0
}
```

### 4.2 polygon — polígono cerrado

```text
polygon(fill="cyan") {
    0 0   5 0   5 5   0 5
}
```

### 4.3 circle — círculo; múltiples centros = múltiples círculos

```text
circle(r=2)             { 5 5 }           % un círculo
circle(r=0.3)           { 1 1  3 1  5 1 } % tres círculos
circle(r=1, fill="red") { 0 0  10 0 }     % dos círculos rellenos
```

El bloque `{ }` contiene pares `x y` de centros. Por cada par se dibuja un círculo.

### 4.4 rectangle

```text
rectangle(fill="gray") { 1 1  9 7 }  % esquina inferior-izquierda, esquina superior-derecha
```

### 4.5 arc

```text
arc(r=3, from=0, to=180) { 5 5 }   % semicírculo superior, centro (5,5)
arc(rx=4, ry=2, from=0, to=360) { 5 5 }  % elipse
```

### 4.6 dot — punto sólido relleno

```text
dot(r=0.15) { 2 3  4 3  6 3 }   % tres puntos
```

### 4.7 bezier — curva de Bézier cúbica

```text
bezier { p0x p0y  c1x c1y  c2x c2y  p1x p1y }
```

Los puntos son: posición inicial, control 1, control 2, posición final. Segmentos adicionales concatenan la curva.

### 4.8 text — texto en posición

```text
text(x, y) { "Texto normal" }
text(x, y, color="red", size=14) { "Título" }
text(x, y, align="center") { "$\alpha + \beta = \gamma$" }
```

El argumento `align` puede ser `"left"` (default), `"center"`, `"right"`. La forma sin coordenadas, `text { "..." }`, dibuja en la posición de pluma (§12.1). El markup interno del string se documenta en §14.

### 4.9 ellipse — elipse completa

```text
ellipse(rx=4, ry=2) { 5 5 }              % elipse centrada en (5,5)
ellipse(rx=4, ry=2) { 2 2  8 2 }         % múltiples centros = múltiples elipses
```

`ellipse` es una conveniencia para la elipse cerrada completa; equivale a `arc(rx, ry, from=0, to=360)`. Para arcos elípticos parciales se usa `arc` con `from`/`to` (§4.5). *(V1: `EL rx ry`.)*

### 4.10 Patrones de línea (`dash`)

El argumento `dash` acepta un nombre de patrón predefinido:

| `dash` | Patrón |
|---|---|
| `"solid"` (default) | línea continua |
| `"dashed"` | guiones |
| `"dotted"` | puntos |
| `"dashdot"` | guión-punto |
| `"dashdotdot"` | guión-punto-punto |

```text
polyline(dash="dashed") { 0 0  10 0 }
```

*(V1: `LPATRN n` con `n` = 1..5. El alias `LSTYLE` de V1 es un error de mapeo —apunta a `LWIDTH`— y se descarta en V2.)*

> Reservado para el backend SVG (§ROADMAP): forma explícita `dash=[on, off, …]` con longitudes arbitrarias.

### 4.11 Patrones de relleno (`hatch`)

En vez de relleno sólido, una primitiva cerrada puede rellenarse con una **trama** de líneas paralelas. El motor provee tramas en cuatro ángulos:

```text
polygon(hatch=45)              { 0 0  5 0  5 5  0 5 } % trama diagonal a 45°
polygon(hatch=0, hatch_gap=3)  { 0 0  5 0  5 5  0 5 } % trama horizontal, paso 3 pt
```

- `hatch=` — ángulo de la trama en grados: `0`, `45`, `90` o `135`.
- `hatch_gap=` — separación entre líneas en puntos (opcional; default según el motor).
- `color=` traza además el contorno de la región, igual que con `fill` (§4).

*(V1: `FPATRN n`, donde el índice `n` codifica ángulo y densidad de forma combinada; `n` negativo añadía el contorno.)*

---

## 5. Variables y expresiones

Ámbito léxico. Las variables definidas fuera de un bloque son globales al archivo.

```text
r_base = 5.5
margen = 2 * 0.5
n = 8

circle(r = r_base + 1) { margen margen }
circle(r = r_base / n) { 0 0 }
```

**Funciones matemáticas disponibles:** `sin(x)`, `cos(x)`, `tan(x)`, `sqrt(x)`, `abs(x)`, `atan2(y, x)`, `pi` (constante).

---

## 6. Control de flujo

```text
for i = 0 to 9 {
    circle(0.3) { i * 1.0  5 }
}

for angle = 0 to 360 step 45 {
    dot(0.2) { 5 + 3*cos(angle)  5 + 3*sin(angle) }
}
```

- Las variables del `for` son locales al bloque y restauran el valor previo al salir.
- Loops anidados están soportados.
- El `step` puede ser negativo para contar hacia atrás.

### 6.1 if — condicionales

```text
if r > 2 {
    circle(r=r) { 5 5 }
} else {
    dot(r=0.2)  { 5 5 }
}
```

- Comparadores: `<`, `>`, `<=`, `>=`, `==`, `!=`; combinables con `and` / `or`.
- El bloque tiene ámbito léxico, como `for` y `with`; la rama `else` es opcional.
- Su uso principal es la condición de paro en structs recursivas (§8.1); también permite variantes de una struct según sus parámetros.

---

## 7. Atributos con bloque `with`

Para aplicar atributos de estilo a un grupo de primitivas sin repetirlos en cada una:

```text
with(color="blue", width=2) {
    polyline { 0 0  10 0 }
    polyline { 0 5  10 5 }
    circle(r=1) { 5 2.5 }
}
```

Los atributos en `with` se propagan a todas las primitivas del bloque. Un atributo explícito en una primitiva individual tiene precedencia sobre el `with`.

**Cascada (with anidados).** Un `with` interno **combina** con el externo: hereda los atributos que no redefine y sobreescribe los que repite. La precedencia, de mayor a menor, es:

```text
atributo explícito en la primitiva  >  with interno  >  with externo
```

```text
with(color="blue", width=2) {
    polyline { 0 0  10 0 }                 % azul, width 2
    with(width=4) {
        polyline { 0 1  10 1 }             % azul (heredado), width 4 (redefinido)
        polyline(color="red") { 0 2  10 2 } % rojo (explícito gana), width 4
    }
}
```

---

## 8. Structs

Una struct encapsula un bloque reutilizable. No dibuja nada hasta que se invoca. Recibe parámetros como una función. Hereda el sistema de coordenadas del punto de invocación. Por convención, los nombres de structs se **Capitalizan** para distinguirlos de los comandos (§1).

```text
struct Flecha(lx, ly, size=0.2) {
    polyline { 0 0  lx ly }
    polygon(fill="black") {
        lx-size*cos(atan2(ly,lx)+0.3)  ly-size*sin(atan2(ly,lx)+0.3)
        lx  ly
        lx-size*cos(atan2(ly,lx)-0.3)  ly-size*sin(atan2(ly,lx)-0.3)
    }
}

Flecha(3, 0)              % flecha horizontal
Flecha(0, 2, size=0.3)    % flecha vertical más grande
```

### 8.1 Recursión

Una struct puede invocarse a sí misma, lo que habilita fractales, árboles y patrones autosimilares. Con parámetros propios y ámbito léxico la recursión es segura: cada invocación recibe sus argumentos y no depende de estado global. La condición de paro se expresa con `if` (§6.1):

```text
struct Rama(longitud, grosor) {
    if longitud >= 1 {                    % condición de paro
        polyline(width=grosor) { 0 0  0 longitud }
        transform(translate=(0, longitud), rotate=30) {
            Rama(longitud * 0.7, grosor * 0.7)
        }
        transform(translate=(0, longitud), rotate=-30) {
            Rama(longitud * 0.7, grosor * 0.7)
        }
    }
}

Rama(5, 2)
```

Como red de seguridad, `max_depth n` (§18) limita la profundidad de expansión. Nótese que `exit` (§18) **no** sirve como condición de paro: detiene el parseo del archivo, no la expansión de una struct. *(V1: "una estructura puede contenerse a sí misma, permitiendo sorprendentes efectos" —artículo de 1991—, con `MAXDEEP n` como límite; era frágil porque la pluma y las matrices eran estado global.)*

---

## 9. Paths como objetos de datos

Los paths son objetos nombrados que representan trayectorias geométricas. Se usan en placements (§10) y como splines para dibujo.

```text
% Definición
path sinpi = bezier { 0 0  0.41 1.335  0.59 1.335  1 0 }

% Operaciones de álgebra de paths
path sin2pi  = tile(&sinpi, times=2)         % concatenar consigo mismo
path rev     = reverse(&sinpi)               % invertir dirección
path norm    = normalize(&sinpi)             % normalizar al rango [0,1]
path fitted  = fitrect(&sinpi) { 1 1  9 7 }  % escalar al rectángulo dado
path wave    = concat(&sinpi, &rev)          % unir dos paths

% Dibujar un path nombrado
bezier(&sinpi)
```

> Las dos operaciones "ajustar a rectángulo" tienen nombres distintos según su operando: `fitrect(&path)` produce un path (álgebra, esta sección); `fit(Struct)` (§10.2) coloca una struct. Igual con la repetición: `tile(&path)` concatena un path; `repeat(Struct)` (§17) repite una struct.

### 9.1 spline con control de nodos

`spline` interpola una curva suave a través de los puntos de control. Su comportamiento se ajusta con argumentos nombrados:

```text
path c = spline { 0 0  2 3  5 1  8 4 }              % Catmull-Rom centrípeto (default)
path c = spline(nodes=8) { 0 0  2 3  5 1  8 4 }     % 8 nodos interpolados por segmento
path c = spline(mode="bezier") { 0 0  2 3  5 1  8 4 } % convierte a curva Bézier equivalente
```

- Default: spline cúbico Catmull-Rom centrípeto.
- `nodes=n` (n>1): puntos interpolados por segmento; más nodos = curva más suave, más datos.
- `mode="bezier"`: convierte los puntos de control a una curva Bézier (menos datos, recomendado para la salida). *(V1: `$S 0`.)*
- `mode="conic"`: reservado, aún no soportado. *(V1: `$S 1`.)*

### 9.2 smooth — Bézier suave a través de puntos

```text
path s = smooth { 0 0  2 3  5 1  8 4 }
```

Ajusta segmentos Bézier que pasan **exactamente** por los puntos dados, calculando las tangentes automáticamente. A diferencia de `spline` (donde los puntos son de *control*), aquí los puntos son *nodos* por los que la curva pasa. *(V1: `GNBZPATH name path`.)*

### 9.3 trail — path por avance

```text
path diagonal = trail(start=(0,0), count=8, translate=(1, 0.5))
```

Genera un path repitiendo un punto inicial y aplicando una transformación en cada paso. Se documenta junto con los generadores en §13.4. *(V1: `GNPATH n x y name`.)*

### 9.4 Path compuesto (compound)

Combina varias primitivas (líneas, arcos, curvas) en un **único contorno**, que puede rellenarse o trazarse como una sola región. Sin esto, primitivas heterogéneas (p. ej. una línea y un arco) no comparten path y no se pueden rellenar juntas.

```text
compound(fill="cyan") {
    polyline { 0 0  4 0 }
    arc(r=2, from=0, to=180) { 2 0 }
}
```

Las primitivas dentro del bloque no se cierran individualmente; sus extremos se unen en un solo path, que se rellena/traza según los atributos del `compound`. *(V1: `OPPT` … `CLPT`.)*

**Regla de relleno (`fill_rule`).** Un path compuesto puede encerrar subregiones (una dona, texto hueco). El argumento `fill_rule` decide qué queda dentro: `"non-zero"` (default, cuenta orientaciones de los subpaths) o `"even-odd"` (alterna dentro/fuera en cada cruce). Ambas reglas son nativas de PostScript y SVG, así que el backend solo las declara — el trabajo topológico lo hace el intérprete de salida.

```text
compound(fill="orange", fill_rule="even-odd") {
    circle(r=3)   { 5 5 }
    circle(r=1.5) { 5 5 }    % agujero: una dona
}
```

*(V1: el relleno topológico `FL` de 1991 —flood fill del área cerrada que contiene cada punto— se reemplaza por reglas de relleno vectoriales.)*

---

## 10. Placements — loops implícitos sobre locus geométricos

Esta es la característica distintiva de MetaGráfica. Un *placement* repite una struct a lo largo de un locus geométrico (línea, arco, path), rotando cada instancia para alinearla con la tangente local. Hay un solo comando, `place`, cuyo tipo de locus se infiere de la geometría del bloque; y `fit`, que ajusta una struct a un rectángulo.

### 10.1 place — a lo largo de un locus

`place(Struct, ...)` coloca la struct repetida a lo largo del locus definido en el bloque, rotando cada instancia con la tangente local. **El tipo de locus se infiere:**

- **2 puntos** → línea recta.
- argumentos de arco (`r`, `sweep`, `from`) → **arco** circular.
- **3+ puntos** o una referencia **`&path`** → trayectoria (path).
- override explícito con `along="line" | "arc" | "path"`.

```text
% Línea: marcas uniformes entre dos puntos, eje X local en la dirección de la línea
place(Tick, scale=0.3) { 0 0  10 0 }

% Arco: radio 5, barrido 90°, desde 0°, centro (5,5)
place(Tick, scale=0.3, r=5, sweep=90, from=0) { 5 5 }

% Path: en cada punto de la trayectoria, según la tangente
place(Tick, scale=0.2) { &sinpi }
place(Tick, scale=0.2) { 0.1 0.2  0.3 0.5  0.7 0.8 }
```

**Argumentos nombrados:**
- `scale` — tamaño de la struct (default 1.0).
- `shift` — desplazamiento del inicio como fracción del intervalo (default 0).
- `gap` — espaciado entre instancias (default = escala). *(solo línea)*
- `both_sides` — colocar también en el lado opuesto (default false).
- `r`, `sweep`, `from` — radio, barrido angular total y ángulo inicial en grados. *(solo arco)*

> ⚠️ **Abierto:** sobre un path, `place` instancia en cada punto del path. Falta definir el espaciado uniforme por longitud de arco (extender `gap=` a paths), útil para marcas cada *n* unidades a lo largo de una curva. Ver §19.

**V1:** `LNST` (línea), `ARCST` (arco), `DPST` / `&name path` (path).

### 10.2 fit — ajustar una struct a un rectángulo

No repite: transforma el sistema de coordenadas de la struct para que ocupe el rectángulo dado.

```text
fit(Panel) { 0 0  10 8 }                % escala uniforme, contenido centrado
fit(Panel, stretch=true) { 0 0  10 8 }  % deforma para llenar el rectángulo exacto
```

Por default preserva la proporción de la struct: escala uniforme al mayor tamaño que cabe en el rectángulo, con el contenido centrado (misma semántica *meet* de §3.1). Con `stretch=true` se deforma para ocupar el rectángulo exacto, que a veces es lo deseado.

**V1:** `PWST` / `PVPT` (siempre deformaban al rectángulo).

### Cuadro comparativo V1 → V2

| V1 | V2 | Semántica |
|---|---|---|
| `LNST sc [sh [n [gap]]] p1 p2` | `place(s, scale, shift, gap) { p1 p2 }` | Struct a lo largo de línea |
| `ARCST sc r da ai [sh [n]] p` | `place(s, scale, r, sweep, from) { c }` | Struct a lo largo de arco |
| `DPST name` / `&name path` | `place(s, scale) { &path }` | Struct a lo largo de path |
| `PWST p1 p2` / `PVPT name p1 p2` | `fit(s, stretch=true) { p1 p2 }` | Struct ajustada a rectángulo (V1 deformaba; el default V2 preserva proporción) |
| `RPPT name n` | `path x = tile(&name, times=n)` | Repetir path (álgebra) |
| `CTPT name` + ... `CLPT` | `path x = concat(...)` | Concatenar paths |

> **Resuelto:** el repetidor V1 `RPST n` se mapea a `repeat(...)` (§17), que avanza la posición con `step` (§12) y acumula una transformación sobre la struct. Cuando no hay acumulación, un `for` loop es equivalente.

---

## 11. Transformaciones y sistemas de coordenadas

MetaGráfica transforma cada coordenada a través de una pila de matrices homogéneas 3×3. El motor mantiene **cinco matrices** con roles distintos. La mayoría se manejan de forma implícita; V2 expone explícitamente la transformación local mediante el bloque `transform`.

| Matriz | Rol | Cómo se controla en V2 |
|---|---|---|
| **Local** (MTLC) | Transformación local a un bloque | bloque `transform { }` (§11.1) |
| **Structure** (MTST) | Rotación/escala de una struct colocada | placements (automático, §10) o `transform` alrededor de la llamada (§11.2) |
| **Plume** (MTPP) | Avance de la pluma entre elementos generados | `step(...)` (§12) |
| **Path** (MTPT) | Se aplica al reutilizar un path nombrado | álgebra de paths (§9), normalmente implícito |
| **Repeat** (MTRS) | Transformación acumulada en repeticiones de struct | `repeat(...)` (§17) |

### 11.1 El bloque `transform`

Aplica una transformación afín a todas las primitivas y structs contenidas en el bloque. Todos los argumentos son opcionales y nombrados.

```text
transform(translate=(dx,dy), rotate=deg, scale=(sx,sy), shear=(hx,hy)) {
    ... primitivas y structs ...
}
```

- `translate=(dx,dy)` — desplazamiento en unidades de usuario.
- `rotate=deg` — rotación en grados (positivo = antihorario).
- `scale=(sx,sy)` o `scale=s` — escala por eje, o uniforme con un solo valor.
- `shear=(hx,hy)` — cizalladura.

**Orden de composición:** dentro de un mismo bloque la transformación se aplica en el orden **escala → rotación → traslación** (la geometría se escala, luego se rota y finalmente se traslada a su posición). Los bloques anidados **componen**: el hijo se multiplica con el del padre.

```text
transform(translate=(5,5), rotate=30) {
    rectangle(fill="cyan") { 0 0  3 2 }   % rectángulo rotado 30° alrededor de (5,5)
}
```

**Alcance (scope):** la transformación es léxica al bloque y se restaura al salir (equivale a un `gsave`/`grestore`).

**Equivalencia V1 → V2:**

| V1 | V2 |
|---|---|
| `TLLC dx dy` | `transform(translate=(dx,dy)) { }` |
| `RTLC a` | `transform(rotate=a) { }` |
| `SCLC sx sy` | `transform(scale=(sx,sy)) { }` |
| `SHLC hx hy` | `transform(shear=(hx,hy)) { }` |
| `IDLC` | (cierre del bloque `transform`) |

### 11.2 Transformaciones sobre structs (MTST)

Los placements (§10) ya fijan la matriz de estructura automáticamente para alinear cada instancia con la tangente local. Para transformar manualmente una struct al invocarla, basta envolver la llamada en un bloque `transform`, que compone sobre la matriz de estructura:

```text
transform(rotate=45, scale=1.5) {
    Flecha(3, 0)        % flecha rotada y escalada
}
```

No se necesita sintaxis adicional: MTST se deriva de la composición léxica de los `transform` activos en el punto de invocación.

---

## 12. Posición de pluma y dibujo relativo

Además de las coordenadas absolutas, MetaGráfica mantiene una **posición de pluma** (el *punto actual*). Sirve para (a) secuencias de texto, (b) los generadores (§13) y (c) el dibujo incremental.

- **`moveto(x, y)`** — fija la posición de pluma en coordenadas de usuario. *(V1: `XYPP x y`.)*
- **`step(translate=(dx,dy), rotate=deg, scale=(sx,sy))`** — define la **transformación de avance**: cómo se mueve la pluma de un elemento al siguiente en los generadores y en el texto secuencial. *(V1: la matriz `MTPP`, p. ej. `TLPP dx dy`, `RTPP a`.)*

La transformación de `step` se **acumula**: en cada avance se aplica al punto actual. Como puede incluir rotación y escala, permite disposiciones radiales o en espiral, no solo lineales.

> El comando de pluma `step(...)` (una llamada) no debe confundirse con la palabra clave `step` del `for` loop (§6, infija) ni con el argumento `step=` de `axis` (§13.5, intervalo de valor). El contexto los distingue.

### 12.1 Texto secuencial

`text { "..." }` (sin coordenadas) dibuja en la posición de pluma y la avanza un `step`. Es la forma relativa de `text(x, y)`, que es absoluto. *(V1: `DT`.)*

```text
moveto(0, 0)
step(translate=(0, -0.6))
text { "Primera línea" }
text { "Segunda línea" }     % colocada un step más abajo
text { "Tercera línea" }
```

### 12.2 Relación con los generadores

Los generadores (§13) toman `moveto` como punto de partida y `step` como avance entre elementos. Definir la pluma y el avance una vez evita repetir coordenadas en cada marca, número o repetición.

> **Principio de diseño.** `moveto`/`step` son el mecanismo de *bajo nivel* heredado del motor (la matriz de pluma). Para las tareas comunes —ejes y retículas— se recomiendan las construcciones declarativas `axis` y `grid` (§13.5–§13.6), que esconden por completo la pluma. La meta de V2 es que el modelo de pluma sea un detalle de implementación, no la API que el usuario maneja a diario.

---

## 13. Generadores y ejes

Los generadores producen series de elementos repetidos a partir de la posición de pluma (§12) y la transformación de avance `step`. Son la base para construir ejes, retículas y escalas sin escribir cada coordenada a mano.

**Dos niveles.** `ticks`, `numbers` y `trail` (§13.1–§13.4) son los generadores *de bajo nivel*: exponen la pluma y el `step` directamente, y dan control total. Por encima de ellos, `axis` y `grid` (§13.5–§13.6) son construcciones *declarativas* que orquestan la pluma internamente: un humano describe el rango y la decoración, no la mecánica de avance. **Para ejes y retículas comunes, usa `axis`/`grid`; recurre a los generadores de bajo nivel solo para casos no estándar.**

### 13.1 ticks — marcas de eje

```text
moveto(0, 0)
step(translate=(2, 0))
ticks(6, mark=(0, 0.3))
```

Genera `count` marcas; cada marca es un segmento dado por el vector `mark=(dx,dy)`, dibujado en la posición de pluma, que avanza con `step` en cada repetición. *(V1: `TICKS n dx dy`, con la pluma y `MTPP`.)*

- `count` — número de marcas (posicional).
- `mark=(dx,dy)` — vector que define longitud y dirección de cada marca.

### 13.2 numbers — etiquetas numéricas

```text
moveto(0, -0.5)
step(translate=(2, 0))
numbers(from=0, by=2, count=6, decimals=0)   % 0  2  4  6  8  10
```

Genera `count` etiquetas numéricas, empezando en `from` e incrementando `by` en cada paso, con `decimals` decimales, en el estilo de texto actual (§14), posicionadas en la pluma y avanzando con `step`. *(V1: `GNNUM i0 inc n decimals`.)*

> Nota: `by` es el incremento del **valor numérico**; `step` es el avance de la **posición**. Son independientes.

### 13.3 Eje completo, a bajo nivel (ejemplo)

Combinando una polilínea con `ticks` y `numbers` se obtiene un eje X de 0 a 10 con marcas y etiquetas cada 2 unidades. Este es el desglose explícito; en la práctica se prefiere `axis` (§13.5), que produce exactamente esto:

```text
polyline { 0 0  10 0 }                       % línea del eje

moveto(0, 0)
step(translate=(2, 0))
ticks(6, mark=(0, -0.3))                     % marcas hacia abajo

moveto(0, -0.7)
step(translate=(2, 0))
numbers(from=0, by=2, count=6, decimals=0)   % etiquetas
```

### 13.4 trail — generar un path por avance

```text
path diagonal = trail(start=(0,0), count=8, translate=(1, 0.5))
```

Genera un path nombrado con `count` puntos, empezando en `start` y aplicando la transformación dada (`translate`, `rotate`, `scale`) en cada paso. *(V1: `GNPATH n x y name`, con la matriz `MTPT`.)* Útil para crear trayectorias regulares reutilizables en placements (§10).

> Nota: `GNBZPATH` (ajustar segmentos Bézier a través de puntos arbitrarios) corresponde a `smooth` en el álgebra de paths (§9.2).

### 13.5 axis — eje declarativo

`axis` describe un eje completo (línea + marcas + etiquetas + título opcional) en una sola sentencia. El bloque `{ }` da los dos extremos físicos del eje; los argumentos describen el rango numérico y la decoración. Internamente es un `place` a lo largo de una línea (§10.1) que coloca marcas y etiquetas calculadas desde el rango — la pluma y el `step` se orquestan solos.

```text
% Eje X: de (0,0) a (10,0), valores de 0 a 10, marca y etiqueta cada 2
axis(from=0, to=10, step=2, ticks="out", labels=true, title="x / cm") { 0 0  10 0 }

% Eje Y: el mismo constructo, distinto par de extremos
axis(from=0, to=8, step=1, ticks="out", labels=true) { 0 0  0 8 }
```

**Argumentos:**

| Arg | Default | Significado |
|---|---|---|
| `from`, `to` | — | valores numéricos en el extremo inicial y final |
| `step` | — | intervalo de valor entre marcas/etiquetas |
| `ticks` | `"out"` | dirección de las marcas: `"out"`, `"in"`, `"both"`, `"none"` (relativas a la línea) |
| `tick_size` | pequeño | longitud de las marcas en unidades de usuario |
| `labels` | `true` | mostrar etiquetas numéricas |
| `decimals` | `0` | decimales en las etiquetas |
| `label_gap` | auto | distancia de las etiquetas a la línea |
| `title` | — | rótulo del eje (texto, admite markup matemático §14) |

La orientación de marcas y etiquetas se deriva de la dirección de la línea (la lógica de tangente del motor de placements), así que un eje horizontal, vertical o inclinado se escriben igual, cambiando solo los extremos del bloque. Aquí `step` es el **intervalo de valor** (como en el `for` loop, §6), no el `step` de avance de pluma (§12).

*(V1: combinación manual de `TICKS` + `GNNUM` + `LNST`/`PL`; ver el desglose de bajo nivel en §13.3.)*

### 13.6 grid — retícula

`grid` dibuja una retícula sobre el rectángulo dado por el bloque (esquina inferior-izquierda, esquina superior-derecha), con líneas verticales y horizontales a intervalos regulares.

```text
grid(xstep=2, ystep=1, color="lightgray") { 0 0  10 8 }
grid(xstep=2, ystep=2, dash="dotted")      { 0 0  10 8 } % retícula punteada
```

**Argumentos:** `xstep` / `ystep` (intervalos en unidades de usuario), más los atributos de estilo comunes (`color`, `width`, `dash`). Útil como fondo de una gráfica antes de trazar datos y ejes.

---

## 14. Texto matemático

El texto es procesado por un módulo independiente (`text_parser`) que se hereda **sin cambios** de V1. El compilador V2 extrae el string del bloque `{ "..." }` y lo pasa a `parse_text()`, que produce una `TextLine` con los chunks ya descompuestos por fuente, tamaño y nivel de script.

### 14.1 Markup del string

El contenido del string soporta el siguiente markup, procesado en parse time:

#### Modo matemático

```text
$...$    activa la fuente CMMI (Computer Modern Math Italic) para el bloque
```

```text
text(5, 3) { "$\alpha + \beta = \gamma$" }
text(2, 7) { "Radio $r = 2\pi / \lambda$" }
```

#### Símbolos griegos y matemáticos

```text
\nombre    inserta el símbolo correspondiente; selecciona la fuente automáticamente
```

Letras griegas minúsculas: `\alpha \beta \gamma \delta \epsilon \zeta \eta \theta \iota \kappa \lambda \mu \nu \xi \pi \rho \sigma \tau \upsilon \phi \chi \psi \omega`

Variantes: `\varepsilon \vartheta \varpi \varrho \varsigma \varphi`

Letras griegas mayúsculas: `\Gamma \Delta \Theta \Lambda \Xi \Pi \Sigma \Upsilon \Phi \Psi \Omega`

Operadores y símbolos: `\infty \partial \nabla \int \sum \prod \pm \times \div \cdot \leq \geq \neq \approx \equiv \sim \in \subset \supset \rightarrow \leftarrow \Rightarrow \Leftarrow \forall \exists`

Funciones: `\sin \cos \tan \cot \sec \csc` (se renderizan en fuente romana recta)

#### Sub y superíndices

```text
^x       superíndice de un carácter
^{...}   superíndice de un grupo
_x       subíndice de un carácter
_{...}   subíndice de un grupo
```

```text
text(3, 5) { "$x^2 + y^2 = r^2$" }
text(3, 3) { "$E_n = -13.6 / n^2$ eV" }
text(3, 1) { "$\psi_{nlm}(r,\theta,\phi)$" }
```

Los scripts reducen el tamaño de fuente al 70% y son anidables: `$x^{2^n}$`.

#### Cambios de fuente en línea

El prefijo `/` seguido de un código cambia la fuente para el resto del string (o hasta el siguiente cambio). Se aplican dentro o fuera de modo matemático.

| Código | Fuente |
|---|---|
| `/r` | Times Roman |
| `/i` o `/e` | Times Italic |
| `/b` | negrita (combina con la fuente actual) |
| `/s` | Sans-serif (Helvetica) |
| `/g` | Symbol (letras griegas en posición estándar) |
| `/c` o `/t` | Courier |
| `/$` | CMMI (math italic) |

```text
text(2, 5) { "/bTítulo en negrita" }
text(2, 3) { "Normal, /itálica, /rnormal otra vez" }
```

#### Agrupación

`{...}` delimita el alcance de sub/superíndices:

```text
text(3, 5) { "$e^{i\pi} + 1 = 0$" }
text(3, 3) { "$\sum_{n=0}^{\infty} a_n x^n$" }
```

### 14.2 Arquitectura: por qué no cambia en V2

`parse_text()` es una función pura: toma un string UTF-8 y una fuente base, devuelve un `GraphicsItem` (`Text` o `TextLine`). No depende del parser del `.mg` ni del Display. En V2:

- El parser V2 extrae el string de `text(...) { "..." }`
- Lo pasa a `parse_text()` con la fuente base del contexto
- Obtiene un `GraphicsItem` listo para dibujar con cualquier Display

**Lo único que cambia** entre V1 y V2 es cómo se especifica la fuente base y el tamaño desde el archivo `.mg` (antes: `TSTYLE`, `TXSI`; ahora: argumentos de `text`).

### 14.3 Argumentos de `text`

```text
text(x, y)                              % posición, fuente y tamaño del contexto
text(x, y, size=12)                     % tamaño en puntos
text(x, y, font="italic")               % fuente base: "roman", "italic", "bold",
                                        %   "italic-bold", "sanserif", "courier"
text(x, y, align="center", color="red") % alineación y color
```

La fuente base determina el punto de partida del markup interno; los `/b`, `/i`, etc. dentro del string son relativos a ella.

> ⚠️ **Pendiente:** alineación vertical (baseline / centro / caja). Por ahora se asume baseline.

---

## 15. Importar archivos

```text
include "bzsinepaths"    % carga bzsinepaths.mg
include "estilos"
```

**V1:** `INPUT nombre`

Las structs y paths definidos en el archivo incluido quedan disponibles en el archivo que lo importa.

**Alcance (scope).** Las structs y paths son **globales**: una struct definida en un archivo incluido es visible en el archivo padre y en cualquier `include` posterior. Reglas:

- El `include` debe **preceder** al uso de las structs o paths que define.
- **No se permite redefinir** una struct con un nombre ya existente (es un error).
- No hay namespaces por archivo: los nombres comparten un espacio global, así que conviene prefijar los nombres de bibliotecas reutilizables (p. ej. `EjesMarca`, `FlechaSimple`).

---

## 16. Coordenadas locales y ventanas anidadas

Además de la `world_window` global (§3), una ventana de coordenadas puede **redefinirse dentro de un bloque o una struct**, estableciendo un sistema de coordenadas local que se apila y se restaura al salir.

```text
world_window 0 24 0 16            % ventana global

struct Detalle() {
    world_window 0 100 0 100      % coordenadas locales dentro de la struct
    circle(r=10) { 50 50 }        % usa el sistema local (0..100)
}
```

La ventana local rige hasta el final del bloque; al salir se recupera la ventana previa. Esto permite que una struct reutilizable trabaje en su propio rango de coordenadas cómodo, independiente de dónde se la coloque. *(V1: `WW` admite anidamiento; el parser apila y restaura la ventana.)*

La ventana anidada se mapea **isométricamente** a su región, igual que la global (§3.1): declarar una ventana local nunca deforma el contenido de la struct. `stretch=true` y `align=` aplican igual que en la ventana global.

> Relación con §11: la `world_window` remapea las coordenadas de usuario al cuadrado unitario interno; las transformaciones `transform` (§11) operan **después** de ese remapeo.

---

## 17. Repetición de estructuras (repeat)

`repeat` coloca la struct indicada `count` veces, avanzando la posición con `step` (§12) y, opcionalmente, **acumulando** una transformación sobre la struct en cada repetición. Esto es lo que permite espirales, patrones de crecimiento y disposiciones cuasi-fractales que un `for` loop expresa de forma menos compacta.

```text
moveto(0, 0)
step(translate=(1.5, 0))
repeat(Hoja, count=8, transform=scale(0.85))
% 8 instancias de Hoja, cada una 0.85× de la anterior, avanzando 1.5 en x
```

- `count` — número de repeticiones (nombrado).
- la posición avanza con el `step` activo (matriz de pluma, §12).
- `transform=` — transformación acumulada sobre la struct: la instancia *k* recibe `transform` elevada a la *k* (composición sucesiva). Equivale a la matriz `MTRS` de V1. Acepta una composición de llamadas — p. ej. `transform=rotate(30) scale(0.95)` — que se multiplica como una sola matriz antes de acumularse:

```text
% 12 pétalos, cada uno rotado 30° y al 95% del tamaño del anterior
repeat(Petalo, count=12, transform=rotate(30) scale(0.95))
```

*(V1: `RPST n`, con `MTPP` para la posición y `MTRS` para transformar la struct.)*

> `repeat(Struct)` (repite una struct, esta sección) no debe confundirse con `tile(&path)` (concatena un path, §9). Operan sobre cosas distintas y por eso tienen nombres distintos.

> Cuándo usar `repeat` vs. `for`: sin acumulación, un `for` loop que coloca la struct es equivalente. La forma que acumula transformación (cada instancia relativa a la anterior) es la que justifica el constructo dedicado.

---

## 18. Controles del compilador y misceláneos

- **Espacio y tamaño base** — `display_size`, `world_window` (§3) y `spline(mode=...)` (§9.1). *(V1: `$D`, `WW`, `$S`.)*
- **Terminar el parseo** — `exit` detiene el procesamiento del archivo en ese punto; el resto se ignora. Útil para depurar. Es un control de parse-time: **no** sirve como condición de paro de una recursión (para eso, `if`, §8.1). *(V1: `EXIT`.)*
- **Salida implícita** — el nombre del archivo de salida se deriva del de entrada (`figura.mg` → `figura.eps`). El backend se selecciona por bandera de línea de comandos (p. ej. `-svg`, ver ROADMAP). *(V1: igual.)*
- **Caracteres acentuados** — para usar caracteres no-ASCII (á, ñ, …) con las fuentes PostScript estándar, V2 activa automáticamente la recodificación del vector de codificación cuando el texto los contiene. *(V1: bandera interna `reencode`.)*
- **Profundidad de recursión** — `max_depth n` fija el límite de expansión para structs recursivas (§8.1). Con la recursión ya especificada deja de ser un campo reservado: el motor debe aplicarlo, con un default razonable (p. ej. 32). *(V1: `MAXDEEP n`.)*

---

## 19. Lo que falta definir (pendientes)

| Tema | Estado | Notas |
|---|---|---|
| **Transformaciones locales** | ✓ Resuelto | Definidas en §11: bloque `transform(translate=, rotate=, scale=, shear=) { }` (matriz MTLC) y composición sobre structs (MTST) |
| **Atributos heredados vs. explícitos** | ✓ Resuelto | Semántica de cascada definida en §7: primitiva > `with` interno > `with` externo |
| **`both_sides` en placements** | ⚠️ Abierto | El parámetro V1 `sc < 0` activa ambos lados de forma implícita. En V2 es `both_sides=true`, pero el significado geométrico exacto (perpendicular vs. especular) necesita documentarse |
| **Texto: fuentes y estilos** | ✓ Resuelto | El markup interno del string (`/b`, `/i`, `$...$`, `\alpha`, `^{}`, `_{}`) lo maneja `parse_text()` sin cambios. Los argumentos de `text` cubren fuente base, tamaño y color (ver §14) |
| **Texto: alineación vertical** | ⚠️ Abierto | ¿Baseline? ¿Centro vertical? ¿Caja del texto? Por ahora se asume baseline |
| **Alcance (scope) de structs** | ✓ Resuelto | Definido en §15: nombres globales, no redefinibles, `include` antes del uso |
| **Manejo de errores** | ◑ Parcial | El compilador ya reporta `archivo:línea` (ROADMAP Propuesta 1). Falta la columna |
| **Ejes y marcas (TICKS)** | ✓ Resuelto | Definido en §13: `ticks`, `numbers`, `trail` y el declarativo `axis` |
| **Patrones de relleno (FPATRN)** | ✓ Resuelto | Definido en §4.11: `hatch=` (ángulo) y `hatch_gap=` (paso) |
| **Color en escala de gris (LGRAY/FGRAY) y outline-fill** | ✓ Resuelto | Definido en §4: gris = color; contorno de relleno = presencia de `color=` |
| **`ellipse` como primitiva** | ✓ Resuelto | Definido en §4.9: `ellipse(rx, ry)` (conveniencia de `arc(rx, ry, from=0, to=360)`) |
| **Línea de puntos y flecha** | ✓ Resuelto | Definido en §4.10: `dash=` con nombres; arreglo explícito `dash=[…]` reservado para SVG |
| **Output implícito** | ✓ Resuelto | Definido en §18: derivado del nombre de entrada; backend por bandera de CLI |
| **Repetición de struct (RPST)** | ✓ Resuelto | Definido en §17: `repeat(...)` |
| **Condicionales (`if`) y recursión de structs** | ✓ Resuelto | Definido en §6.1 (if, comparadores, `and`/`or`) y §8.1 (recursión con paro por `if`, límite `max_depth` §18) |
| **Regla de relleno en `compound`** | ✓ Resuelto | Definido en §9.4: `fill_rule="non-zero"` (default) / `"even-odd"` |
| **Aspect ratio / no-deformación** | ✓ Resuelto | Definido en §3.1: escala uniforme (*meet*) + margen con `align=`; `stretch=true` para deformar; `fit` preserva proporción por default (§10.2); ventanas anidadas isométricas (§16) |
| **Texto bajo `transform`** | ⚠️ Abierto | Deseado: que el texto rote/escale con los `transform` activos (EPS lo permite). Falta definir si se transforma solo el punto de anclaje o también los glifos |
| **Espaciado uniforme en `place` sobre path** | ⚠️ Abierto | Extender `gap=` a paths: instancias cada *n* unidades de longitud de arco, no solo en los puntos del path (§10.1) |
| **Splines cuadráticos (cónicas)** | ⚠️ Abierto | `spline(mode="conic")` reservado en §9.1; V1 lo contemplaba (`$S 1`). Decidir si se soporta nativo (QuadTo en SVG) o por conversión a cúbico |
| **Resolución geométrica (estilo MetaPost)** | ◇ Prospectivo | Describir relaciones en vez de calcular coordenadas: intersección de dos líneas, punto a una fracción de un path, punto medio. Convertiría "dibujar" en "describir"; es el siguiente salto de expresividad tras `axis`/`grid` |

---

## 20. Estrategia de transición V1 → V2

El compilador V2 **no** mantendrá compatibilidad con la sintaxis V1. La migración es mediante un traductor Python (`mg1to2.py`) que convierte archivos V1 al nuevo formato.

### Cobertura esperada del traductor (>90% automático):

```python
# Reglas de traducción mecánica
OPST name → struct Name() {
CLST       → }
MKST name  → Name()          # uso sin parámetros
PL { ... } → polyline { ... }
CR r : ... } → circle(r=r) { ... }
BR p1 p2   → rectangle { p1 p2 }
LWIDTH w   → (acumular: width=w en siguiente primitiva)
LCOLOR c   → (acumular: color="c" en siguiente primitiva)
FGRAY g    → (acumular: fill=gray_to_hex(g) en siguiente primitiva)
INPUT f    → include "f"
LNST ...   → place(...) { p1 p2 }
ARCST ...  → place(..., r, sweep, from) { c }
```

> Nota: el traductor capitaliza los nombres de structs (`OPST flecha` → `struct Flecha`) para seguir la convención de §1.

### Casos que requieren revisión manual:
- Uso de `MKST` + matrices MTST (los parámetros se transforman en parámetros de struct)
- `RPST n` con `SCST`/`TLST` (requiere modelado como `for` loop)
- Texto con markup complejo (`TSTYLE`, `TXSI`, subíndices/superíndices)
- Tamaños físicos bajo la política isométrica (§3.1): en V1 el radio de `CR` se medía en unidades del eje y, las structs colocadas escalaban a `min(dvx,dvy)` y `DOT` usaba puntos tipográficos. Si el archivo V1 tenía `WW` y `$D` con proporciones distintas, el traductor debe convertir estos valores para conservar el tamaño físico del original.

---

## 21. Cobertura V1 → V2 (auditoría de keywords)

Auditoría completa de cada palabra clave del léxico V1 (`keyword_map` en `MGLexer::init_tables()`) frente a la especificación V2. Estados: ✓ cubierto · ◑ parcial · ✗ se elimina.

### Primitivas y atributos

| V1 | V2 | Estado | Sección |
|---|---|---|---|
| `PL` | `polyline` | ✓ | §4.1 |
| `PG` | `polygon` | ✓ | §4.2 |
| `CR` | `circle` | ✓ | §4.3 |
| `BR` | `rectangle` | ✓ | §4.4 |
| `EL` | `ellipse(rx,ry)` | ✓ | §4.9 |
| `DOT` | `dot` | ✓ | §4.6 |
| `BZ` | `bezier` | ✓ | §4.7 |
| `SP` | `spline` | ✓ | §9.1 |
| `LWIDTH` | `width=` | ✓ | §4 |
| `LPATRN` | `dash=` | ✓ | §4.10 |
| `LSTYLE` | — | ✗ | bug V1 (mapea a `width`), se descarta |
| `LCOLOR` | `color=` | ✓ | §4 |
| `FCOLOR` | `fill=` | ✓ | §4 |
| `FILL` / `NOFILL` | `fill=` (presencia/ausencia) | ✓ | §4 |
| `LGRAY` / `FGRAY` | gris en `color=`/`fill=` | ✓ | §4 |
| `FPATRN` | `hatch=` / `hatch_gap=` | ✓ | §4.11 |
| `TSTYLE` | `font=` | ✓ | §14.3 |
| `THEIGHT` / `TSIZE` | `size=` | ✓ | §4.8 |
| `TALIGN` | `align=` | ✓ | §4.8 |

### Estructuras y placements

| V1 | V2 | Estado | Sección |
|---|---|---|---|
| `OPST` … `CLST` | `struct Nombre(...) { }` | ✓ | §8 |
| `MKST` | `Nombre(...)` (invocación) | ✓ | §8 |
| `LNST` | `place(...) { p1 p2 }` (línea) | ✓ | §10.1 |
| `ARCST` | `place(..., r, sweep, from) { c }` (arco) | ✓ | §10.1 |
| `DPST` / `&name path` | `place(...) { &path }` (path) | ✓ | §10.1 |
| `PWST` | `fit(...)` | ✓ | §10.2 |
| `RPST` | `repeat(...)` | ✓ | §17 |

### Paths

| V1 | V2 | Estado | Sección |
|---|---|---|---|
| `&name path` | `path name = ...` | ✓ | §9 |
| `RPPT` | `tile()` | ✓ | §9 |
| `CTPT` … `CLPT` | `concat()` | ✓ | §9 |
| `INVPT` | `reverse()` | ✓ | §9 |
| `NORMPT` | `normalize()` | ✓ | §9 |
| `PWPT` | `fitrect()` | ✓ | §9 |
| `GNPATH` | `trail()` | ✓ | §9.3 / §13.4 |
| `GNBZPATH` | `smooth()` | ✓ | §9.2 |
| `OPPT` … `CLPT` | `compound { }` | ✓ | §9.4 |

### Transformaciones, pluma y generadores

| V1 | V2 | Estado | Sección |
|---|---|---|---|
| `TLLC`/`RTLC`/`SCLC`/`SHLC`/`IDLC` | `transform(...) { }` | ✓ | §11 |
| `··ST` (MTST) | `transform` sobre la llamada | ✓ | §11.2 |
| `··PP` (MTPP) | `step(...)` | ✓ | §12 |
| `··PT` (MTPT) | álgebra de paths | ✓ | §9 |
| `··RS` (MTRS) | `repeat(transform=...)` | ✓ | §17 |
| `XYPP` | `moveto` | ✓ | §12 |
| `DT` | `text { }` | ✓ | §12.1 |
| `XYDT` | `text(x,y)` | ✓ | §4.8 |
| `TICKS` | `ticks` / `axis` | ✓ | §13.1 / §13.5 |
| `GNNUM` | `numbers` | ✓ | §13.2 |

### Controles y obsoletos

| V1 | V2 | Estado | Sección |
|---|---|---|---|
| `$D` | `display_size` | ✓ | §3 |
| `$P` | tamaño base de texto | ✓ | §3 / §14 |
| `$S` | `spline(mode=, nodes=)` | ✓ | §9.1 |
| `WW` | `world_window` (global y anidado) | ✓ | §3 / §16 |
| `INPUT` | `include` | ✓ | §15 |
| `EXIT` | `exit` | ✓ | §18 |
| `MAXDEEP` | `max_depth` | ✓ | §18 / §8.1 |
| `INTXT` / `XYTXT` | — | ✗ | obsoletos, se eliminan |
| `MKMR` / `MR` | — | ✗ | internos/obsoletos, revisar al traducir |

