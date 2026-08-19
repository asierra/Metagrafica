# plan_metricas_texto.md — Exponer las métricas de texto al lenguaje

**Estado:** **PROPUESTA. SIN CLIENTE**, y abierto a sabiendas (2026-08-08). No hay hoy
ninguna figura del corpus que lo pida, y eso es parte de lo que hay que decidir: si el
lenguaje quiere entrar al género de figuras que lo necesita, o si prefiere no entrar.
Ninguna línea de código escrita.

Nació de un diagrama de arquitectura ajeno al corpus (`arquitectura.puml`, servicio de datos
históricos del LANOT) al preguntarse si convenía portarlo a MetaGráfica. La respuesta fue que
no —por razones que no son ésta—, pero al intentarlo apareció el hueco estructural: **una caja
no puede dimensionarse a su rótulo**, porque el `.mg` no tiene con qué preguntar cuánto mide
una cadena. Ver el apéndice.

> ⚠️ **Lo que este plan NO es.** No es «cajas que se ajustan solas». Eso es una primitiva y
> es la respuesta fácil; se argumenta en §5 por qué es la salida equivocada. Este plan es
> sobre exponer un **número** —el ancho de una cadena— y sobre las dos preguntas que ese
> número arrastra: **en qué unidades** y **en qué momento**.

---

## 0. Medir es lo fácil. El plan es sobre CUÁNDO y EN QUÉ UNIDADES

Conviene decirlo de entrada porque desvía la intuición: **el motor ya mide, y ya mide bien.**
`\frac` no podría colocar numerador y denominador por extent medido si no fuera así
(`plan_frac.md`), ni `fit` centrar nada. No hay que escribir un medidor.

Lo que no existe es el **puente** entre ese medidor y el evaluador de expresiones. Y al
tenderlo aparecen dos problemas que no son de tipografía sino de diseño del lenguaje:

1. **Cuándo.** MetaGráfica resuelve el estado tipográfico ambiente **en tiempo de dibujo**
   (la máquina de estados del `Display`), pero una expresión del `.mg` se evalúa **antes**,
   en tiempo de evaluación. La respuesta se necesita en un momento en que la pregunta todavía
   no tiene respuesta única.
2. **En qué unidades.** El ancho de una cadena es una cantidad **física** (pt). Las
   coordenadas de la figura son de **mundo**. La conversión entre las dos no es constante:
   dentro del cuerpo de una struct no existe.

Las dos están **medidas** abajo. La segunda es la que decide la forma de la API.

---

## 1. Estado medido (2026-08-08)

### 1.1 Lo que el motor YA tiene

| Pieza | Dónde | Qué devuelve |
|---|---|---|
| `text_width(TextState, string)` | `src/text.cpp:395`, declarada en `include/text.h:97` | ancho en **em relativos** (× tamaño de fuente del dispositivo → pt) |
| `TextLine::width()` | `src/text.cpp:626` | ancho de una línea con marcación ya resuelta (suma `itemWidth` de cada run, incluye `pre_space` y `Fraction::width()`) |
| `TextLine::vExtent(asc, desc)` | `src/text.cpp:675` | **alto y profundidad**, ya medidos por `\frac` |
| `Fraction::width()` / `vExtent` | `src/text.cpp:688, 711` | ídem para fracciones anidadas |
| `parse_text(utf8, ff, …)` | `src/text_parser.cpp:692` | marcación → `GraphicsItem` (`Text` / `TextLine` / `TextBlock`) |

⚠️ **Y el hallazgo que abarata el plan entero: `parse_text` YA se llama en tiempo de
evaluación.** No es una función de dibujo. `src/parserv3.cpp` la invoca en siete sitios
—`text()` en :2583, rótulos de eje en :3066, marcas en :3011, leyenda en :3301, tabla en
:3483— todos dentro de `exec`. O sea que **el camino cadena → objeto medible ya corre en el
momento en que haría falta**. La medición no hay que inventarla ni adelantarla: hay que
llamarla.

Traducido: el trabajo no es tipográfico. Es de **plomería y de semántica**.

### 1.2 Lo que el lenguaje NO tiene

La tabla completa de funciones builtin es `include/ast.h:273-352`:

```
sin cos tan sqrt abs atan2 asin acos atan deg rad exp ln mod xyz len str gray
```

Diecisiete, y **ninguna toca el texto**. `str()` fabrica cadenas; nada las mide.

⚠️ **Restricción de plomería, concreta:** `Expr::eval(Scope &)` (`include/ast.h:162`) recibe
**solo el `Scope`**. El documento —`MetaGrafica`, que es quien sabe `display_size`,
`world_window` y `font_size`— se pasa por otro carril: `Stmt::exec(Scope&, MetaGrafica&, …)`.
Así que una función builtin **hoy no puede ver el documento**. Cualquier diseño que necesite
el tamaño de página o la ventana para responder tiene que enhebrar `MetaGrafica` hasta
`eval`, o pasar por una global. Es la primera decisión de implementación y conviene tomarla
a la vista, no de rebote.

### 1.3 El idioma actual: convertir a mano

Hoy, cuando una figura necesita mezclar pt con mundo, el `.mg` hace la conversión él mismo.
Está **documentado como idioma** en `docs/referencia.md` §7 (retiro de un marcador):

```octave
display_size 12 6
world_window 0 10 0 5
% marker_size (pt) -> unidades de mundo
retiro = 5/72*2.54 / (12 / 10)
```

⚠️ **Ese idioma es frágil, y conviene notarlo antes de reproducirlo.** El `12 / 10` es
`ancho_cm / ancho_mundo`, o sea supone que **el eje x es el que amarra la escala**. La escala
real es *meet* isótropa, `s = min(W/wdx, H/wdy)` (`Display::pushWorldMatrix()`, §3.1): en
cuanto el eje que amarra sea el otro, la cuenta queda mal y **nada avisa**. Además repite a
mano tres números que ya están en el documento, así que cambiar `display_size` deja la
constante podrida en silencio — la misma clase de fallo que motivó la compuerta `citafail`.

📌 Esto es un argumento **independiente** para el plan: aunque nadie mida texto nunca, el
lenguaje ya tiene un agujero donde debería haber una conversión pt↔mundo confiable.

---

## 2. El problema del CUÁNDO: el ambiente se resuelve en tiempo de dibujo

`parse_text` acepta `FN_NOFACE` (= *hereda la cara ambiente*) y lo **hornea** en el objeto;
quien lo resuelve es `Text::draw` contra el estado vigente del `Display`. Lo mismo el tamaño:
`TextState::font_size` es **relativo** (1 = el ambiente), y el ambiente en pt es estado del
dispositivo.

Consecuencia medida, leyendo `text_width` (`src/text.cpp:414-430`): ante `FN_NOFACE` el
`switch` cae en `default:` → `serif_metrics_map`. O sea que **hoy `text_width` de un run que
hereda contesta con métricas de Times**, correcto por casualidad cuando el ambiente es Times
y silenciosamente equivocado cuando es negrita, sans o Courier (600/1000 fijo).

Eso no molesta hoy porque los consumidores internos (`fit`, `\frac`, el centrado) miden runs
cuya cara **ya está resuelta** en el mismo objeto. Molestaría en cuanto el usuario escriba:

```octave
font "bold"
w = textwidth("Bucket público NOAA")     % ¿mide en negrita, o en Times?
```

Tres salidas posibles, y hay que elegir una a propósito:

- **(a) Estado en la sombra.** El evaluador lleva su propia copia del estado tipográfico,
  espejo del `Display`. ⚠️ Es una **segunda máquina de estados que hay que mantener
  sincronizada con la primera**, y esa familia de bugs ya mordió a este proyecto: los backends
  reparando por su cuenta una descomposición de arco (`plan_anisotropia.md`) son exactamente
  eso. No hay compuerta que cace una divergencia así — la salida sigue siendo byte-estable.
- **(b) Explícito o nada.** `textwidth(s, font="bold", size=10)`, y **sin argumentos usa el
  documento** (`font_size` de nivel superior, cara por omisión). Nunca lee el ambiente, así
  que nunca miente sobre él: contesta una pregunta bien definida y distinta. El costo es que
  el autor repite la cara si la cambió.
- **(c) Diferido.** `textwidth` no devuelve un número sino un valor perezoso que se resuelve
  en dibujo. Es lo correcto en teoría y es un cambio de modelo del evaluador entero
  (`Value` deja de ser un número). Fuera de escala para lo que se gana.

**Recomendación: (b).** Es la única que no puede desincronizarse, y la que se puede subir a
(a) más tarde sin romper nada escrito.

---

## 3. El problema de las UNIDADES, y la medición que lo decide

### 3.1 Medido: el texto NO escala con la colocación de una struct

Prueba (2026-08-08), la misma struct puesta dos veces con escalas distintas:

```octave
display_size 10 10
font_size 12
world_window 0 10 0 10
struct caja() { text("Hola") { .5 .5 } }
caja(at=(1,1), scale=1)
caja(at=(5,1), scale=3)
```

SVG resultante: **`font-size="12.000000"` en los dos**. El texto es una cantidad física y se
comporta como tal — igual que `line_width`, `marker_size` o la `depth` de `brace`.

### 3.2 Por lo tanto, dentro de una struct no hay respuesta en unidades de mundo

El cuerpo de una struct mantiene su ventana local normalizada al cuadrado unitario (contrato
de colocación V1, §3.1). Cuánto vale ahí un «1» en la página depende de **cómo se coloque la
struct**, y eso no se sabe cuando el cuerpo se ejecuta — puede colocarse muchas veces y con
escalas distintas, como en la prueba de arriba.

⚠️ **Conclusión dura: `textwidth` no puede devolver unidades de mundo.** No es que sea
incómodo; es que dentro de un cuerpo de struct **la cantidad no está definida**. Y el cuerpo
de una struct es justo donde se querría usar —un icono con rótulo—, así que no es un rincón
raro: es el caso central.

**`textwidth` devuelve PUNTOS.** Y entonces el problema se vuelve el conocido: mezclar una
cantidad física con coordenadas de mundo.

### 3.3 Es exactamente la lección de `brace`, y conviene no re-litigarla

`plan_llaves.md` §0 llegó a la misma pared por otro lado: el vano de la llave va en mundo y su
profundidad en pt, y por eso `Display::brace` es una **virtual** en vez de componerse con
`moveto`/`lineto` —que transformarían sus argumentos por el marco y estirarían los ganchos con
la ventana—. Medido allí: bajo `scale 4 1` la llave sale idéntica en vez de estirarse 4×.

Aquí pasa lo mismo un piso más arriba. Las dos salidas coherentes son:

- **una conversión explícita en el lenguaje** (`pt2world(x)`), que reemplaza el idioma a mano
  de §1.3 y **usa la escala meet de verdad** en vez de suponer que amarra x; o
- **primitivas que acepten tamaños en pt**, como ya hace `marker_size` y como estrenó `brace`
  con `depth`.

No son excluyentes, y la primera es barata y útil sola. ⚠️ Pero `pt2world` **hereda el
problema de §3.2**: dentro de una struct tampoco está definida. Tiene que ser un error
honesto ahí, no un número plausible. Es la pregunta abierta 3.

---

## 4. Superficie mínima propuesta

Cuatro funciones, ninguna primitiva nueva:

| Función | Devuelve | Notas |
|---|---|---|
| `textwidth(s)` | pt | marcación **completa** (`$…$`, `\frac`, super/subíndices) vía `parse_text` + `TextLine::width()` × `font_size` |
| `textascent(s)` | pt | de `vExtent`, ya existe |
| `textdescent(s)` | pt | ídem, positivo hacia abajo |
| `pt2world(x)` | unidades de mundo | escala meet real; error dentro de struct (pregunta 3) |

Con eso, el caso que motivó el plan se escribe sin machinery nueva:

```octave
pad = 6                                    % pt
w   = pt2world(textwidth(msg) + 2*pad)
rectangle(w=w, h=alto) { … }
text(msg) { … }
```

**Lo que NO propone este plan, y por qué.** Una primitiva `box("rótulo")` que se dimensione
sola es la respuesta obvia y es la equivocada: hornea una decisión de composición —cuánto
respiro, dónde el rótulo, qué pasa con dos renglones, qué borde— en el motor, donde no se
puede ajustar. MetaGráfica es descriptivo; la caja bonita es asunto de una `struct` en `lib/`,
y para escribirla lo único que falta es el número. **Da el número.**

---

## 5. Preguntas abiertas

1. **¿(b) o (a) para el ambiente?** §2 recomienda (b), explícito. Falta decidir si el default
   sin argumentos es el `font_size` del documento —lo natural— y qué pasa si `textwidth` se
   llama **antes** de `font_size` en el archivo. Probablemente error, no un 1 silencioso.
2. **¿Cómo se enhebra `MetaGrafica` hasta `Expr::eval`?** (§1.2). Cambiar la firma virtual
   toca todos los `Expr`; una global es menos invasiva y este archivo ya tiene `g_baseDir`,
   `g_flags` y `g_maxDepth`, así que hay precedente. Decidir a la vista.
3. **`pt2world` dentro de una struct: ¿error o se permite?** Recomendado **error con
   mensaje que nombre la razón** — es la clase de cantidad que sale plausible y equivocada,
   y la política del proyecto ante eso ya está sentada dos veces (`polygon(fill="none")` de
   manchón mudo a error, 2026-08-06; el aviso de cambio de cara al final de cadena).
4. **¿Y `stretch`?** Con `world_window … stretch=true` (§13.7) la escala **no es isótropa**:
   hay dos, una por eje. `pt2world(x)` deja de tener una respuesta. ¿`pt2worldx`/`pt2worldy`,
   o error bajo stretch? Es un tic reconocible de `plan_anisotropia.md` —fórmula isótropa
   aplicada al caso anisótropo— y hay que resolverlo **antes** de escribir código, no después.
5. **¿Qué mide `textwidth` de un texto multilínea (`/n`)?** ¿El renglón más ancho? Es lo
   único útil, pero conviene que la referencia lo diga en vez de que se descubra.

---

## 6. Qué haría falta para que valga la pena

Este plan no tiene cliente y **no debe implementarse hasta tenerlo**. La política del repo ya
tiene esa forma para las bibliotecas —una `lib/*.mg` sin ejemplo que la incluya se pudre en
silencio— y aplica igual a una función del lenguaje: una builtin sin figura que la use no la
compila ninguna de las diez compuertas.

Clientes plausibles, en orden de cuán cerca están de existir:

- **Leyendas y tablas a la medida.** `legend` y `table` ya calculan anchos **por dentro**
  (`col_widths`, `sample_width`, `margin` en `parserv3.cpp:3225, 3351`). Hoy el autor los da a
  mano en pt. Es el cliente más cercano: la maquinaria interna ya mide, y exponer la medida
  cerraría el círculo.
- **Recuadros y anotaciones** sobre una figura científica —una nota con marco dentro del
  panel—, que es lo que pediría un ejemplo del corpus antes que un diagrama UML.
- **Diagramas de cajas y flechas.** El género que abrió el plan. ⚠️ Y el más honesto de
  descartar: le falta mucho más que medir texto (ruteo de aristas, colocación de etiquetas,
  evitar cruces), y ninguna de esas piezas está en la trayectoria de MetaGráfica ni debería
  estarlo. **Medir texto no acerca a MG a ser PlantUML, y no es su objetivo.**

📌 Si el criterio de entrada es «una figura del corpus lo necesita», el candidato natural es
el primero, no el tercero.

---

## Apéndice — el caso que lo destapó

Un diagrama de arquitectura de seis nodos escrito en PlantUML, que se consideró portar a
MetaGráfica por control estético. El esqueleto que se intentó (`lib/uml.mg`, sin trackear):

```
struct uml_Box(message, w=1, h=1) {
        rectangle(w,h) { 0 0 }
        text(message) { (w/2) (h/2) }
}
```

La firma cuenta el problema entera: **`w` y `h` son parámetros porque no pueden deducirse de
`message`**. Cada caja se dimensiona a mano, y como los extremos de las flechas tienen que
caer en los bordes, cada rótulo que cambie obliga a re-afinar su vecindad. En un diagrama que
seguirá cambiando mientras cambie el sistema que describe, eso se paga en cada edición — y es
justo lo que una herramienta con auto-trazado no cobra.

📌 **La medida no fue el motivo de quedarse con PlantUML** (pesaron más el auto-trazado y que
la figura hermana es un diagrama de actividad). Pero fue lo que dejó a la vista que el hueco
es del **lenguaje** y no de esa figura.
