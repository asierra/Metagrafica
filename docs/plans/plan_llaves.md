# plan_llaves.md — Llaves, delimitadores extensibles y el escape que falta

**Estado:** propuesta para discutir. Nada implementado. Abierto el 2026-08-06 a raíz de
la reconstrucción de la Fig. 1.7 de Lillesand (`reflectancia_vegetacion.mg`, curso ENCiT),
cuya llave del margen derecho hubo que sustituir por una cota de doble flecha porque MG no
tiene con qué dibujarla.

---

## 0. Son DOS objetos, y confundirlos es la trampa del diseño

La palabra «llave» nombra aquí dos cosas que se parecen en el papel y no se parecen en nada
en su origen:

1. **Delimitador de fórmula.** `${x : x > 0}$`. Su tamaño **se deriva** de lo que encierra:
   nadie lo teclea, sale de medir el contenido. Vive en la maquinaria de texto, junto a
   `\frac` y `\hat`. Es lo que LaTeX resuelve con `\left`/`\right`.

2. **Mobiliario de página.** La llave de Lillesand que abarca de 24.5 % a 44 % de
   reflectancia y dice «nótese el rango de valores espectrales». Su tamaño **se deriva
   también**, pero de la GEOMETRÍA DEL DIBUJO, no de un texto: abarca un rango de datos, y
   el rótulo va al lado, no dentro. Vive en la maquinaria gráfica, junto a `polyline` y los
   marcadores.

⚠️ **LaTeX solo tiene el primero.** Copiarle la sintaxis entera nos daría la mitad que menos
falta: en las 31 figuras del corpus no hay **una sola** fórmula que necesite un delimitador
extensible, y el caso 2 apareció a la primera figura de curso que se intentó. Si el plan
sale de aquí con una sola decisión firme, que sea ésta: **el caso 2 no puede ser marcación
dentro de `text()`**, porque su altura está en unidades de mundo y meterla en una cadena
obligaría al autor a convertir mundo→pt a mano —exactamente el rodeo que ya tuvo que hacer
`reflectancia_vegetacion.mg` con `pv_top`/`pv_bot` para colocar su cota—.

Lo que sí comparten es el **dibujo**. Ahí conviene que haya una sola fuente de geometría,
como con `markers.h`.

---

## 1. Estado medido (2026-08-06)

### 1.1 Qué se traga la marcación hoy

Medido compilando y leyendo los `<tspan>` del SVG:

| lo que se escribe | lo que sale | por qué |
|---|---|---|
| `text("a{b}c")` | `abc` | `{`/`}` son **agrupadores** (`tspush`/`tspop`) |
| `text("{")` | *ni un `<text>`* | ídem, y no queda nada que dibujar |
| `text("a\{b")` | `ab` | `\` escanea lo **alfabético** que sigue; `{` no es alfabético, así que no se lee ningún nombre y el bucle se come los dos caracteres |
| `text("$\lbrace$")` | *nada* + `Warning: symbol name unknown lbrace` | no existe en `map_symbol` |
| `text("$\left{x\}$")` | `𝑥` + `Warning: symbol name unknown left` | ídem |
| `text("5 m/s")` | `5 m` | `/s` es un **cambio de cara** (`font_style_codes = "beigrsct$"` + `/n`) |
| `text("W/m2")` | `W/m2` | `m` no está en la tabla de caras, así que cae al literal |

⚠️ **La última fila no es una curiosidad, es un bug con nombre y apellido.** `m/s`, `J/g`,
`cal/g`, `1/r`, `1/e`, `W/cm` — todas pierden lo que va tras la barra, **en silencio**, y
`W/m2` sobrevive por casualidad. El repertorio de MG **no tiene escape**, y ésa es la misma
carencia que impide escribir una llave. Conviene resolverlas juntas.

Caracteres hoy **inalcanzables** en una cadena: `{`, `}`, `$`, `\`, y `/` seguido de
`b e i g r s c t $ n`.

### 1.2 El subset de Latin Modern Math

Medido parseando el `cmap` de `include/font_lmmath_ttf.h`:

- **30 228 bytes, 237 codepoints.** *(El comentario de cabecera dice «186 codepoints»: está
  rancio, quedó de P3. Corregirlo cuando se toque el archivo.)*
- Tablas presentes: `GDEF OS/2 cmap gasp glyf head hhea hmtx loca maxp name post`.
  **NO hay `MATH`, ni `GSUB`, ni `GPOS`.**
- Presentes: `(` `)` `[` `]` `√` `⟨` `⟩` `⌈` `⌊`…
- **Ausentes: `{` (U+007B) y `}` (U+007D).** Entraron los «agrupadores» en P3, pero los que
  entraron fueron los paréntesis y los corchetes; las llaves no, precisamente porque en la
  marcación son sintaxis.
- **Ausentes: TODAS las piezas extensibles** — U+23A7/23A8/23A9/23AA/23AB (llave) y U+239B…
  (paréntesis).

⚠️ **Lo que falta no es sólo el dibujo, es la MECÁNICA.** Un delimitador extensible de
OpenType no se «estira»: se **ensambla** con las piezas y los solapes que declara la tabla
`MATH` (`MathVariants`/`GlyphAssembly`). Sin esa tabla —y sin motor de shaping que la lea—
el ensamblado hay que **escribirlo igual**. La fuente aportaría el *arte*, no el
*comportamiento*.

### 1.3 Lo que ya existe para construir encima

- `Fraction` y `Accent` (`include/text.h`) **componen en espacio de dispositivo** midiendo a
  sus hijos: `vExtent(ascent, descent)`, `childWidth()`. Es exactamente el mecanismo que
  necesita un delimitador que crece con su contenido — ya está escrito y probado.
- `Display::fracRule(dy, len, lw)` y `Display::penSegment(dx1,dy1,dx2,dy2,lw)`: trazos
  **relativos a la pluma que no la mueven**, implementados en los tres backends.
  ⚠️ **No hay equivalente curvo.** Una llave lo pide.
- `markers.h`: geometría exacta en caja unitaria, **una sola fuente de verdad** consultada
  por los tres backends, escalada por un tamaño **en pt**. Es el molde a copiar para la
  llave dibujada.
- `marker_size` en pt sobre coordenadas de mundo: el precedente de «objeto gráfico cuyo
  tamaño es una cantidad física», que es justo la observación de la que sale este plan.

### 1.4 La doctrina de la casa ya está escrita

No hay que inventarla; está en `include/text.h`, sobre `Accent`:

> «La marca no sale de la fuente A PROPÓSITO: un acento combinante tiene avance cero y se
> posiciona con las tablas GPOS/MATH, que MG no parsea, así que el código de posicionado hay
> que escribirlo de todos modos; tomándola de la geometría se ahorra además tocar el subset
> de LM Math y la ruta PUA del PDF. **Es el mismo criterio con el que la raya de `\frac` es
> un trazo y no un glifo.**»

La llave extensible cae **exactamente** en ese criterio, y con más fuerza: la raya de `\frac`
sí existe en las fuentes y aun así se dibuja.

---

## 2. El escape que falta

Decidido por ti: `{` debe poder verse escribiendo `\{`. Propongo generalizarlo en la misma
pasada, porque el `m/s` demuestra que el agujero no es de las llaves sino del lenguaje de
marcación.

**Propuesta E1 — regla de escape general.** `\` seguido de un carácter **no alfabético**
significa *ese carácter, literal, sin función*. Cubre `\{ \} \$ \\ \/ \_ \^` de un golpe, y
es una rama de tres líneas en `case '\\'` de `text_parser.cpp`, justo donde ya viven los
overrides de espaciado `\, \; \!` (que son el mismo patrón: mirar el siguiente carácter
antes del escaneo alfabético).

- ⚠️ **Cuidado con `\,` `\;` `\!`**, que ya están tomados **en modo math**. La regla debe
  ceder ante ellos: primero los overrides, después el escape. Fuera de math no hay conflicto.
- **En modo math, `\{` no es un carácter cualquiera: es un delimitador**, o sea clase
  `MC_OPEN` (y `\}` `MC_CLOSE`) para la máquina de espaciado de `plan_text_space` Parte B.
  Si se les da clase `MC_ORD` el espaciado saldrá mal a los lados, y saldrá mal *poco*, que
  es peor.
- ⚠️ **Requiere meter U+007B/U+007D al subset** (§1.2): hoy no están. Es el único trabajo de
  fuente que este plan considera **inevitable**, y es barato porque no necesita ni `MATH` ni
  piezas.
- Fuera de math el `{` sale de la fuente de texto, que sí lo tiene: ahí no hay nada que
  subsetear.

**Propuesta E2 — `/` deja de comerse lo desconocido.** Independiente de E1 y probablemente
más urgente: `/` seguido de algo que no es código de cara ya cae al literal, pero `/s` en
`m/s` **sí** es código. Dos salidas posibles, a discutir:
 - (a) `\/` (por E1) para forzar la barra, y documentar la lista de caras reservadas;
 - (b) además, **avisar** cuando un cambio de cara no va seguido de nada visible — que es la
   firma de un `m/s` mal leído. Es una prueba `EXPECT_WARN` de las de `errfail`.

⚠️ **Nada de esto lo caza una compuerta hoy**, y es de la clase más cara: la salida es
byte-estable y *plausible*. Cada caso de arriba merece su `test/errors/*.mg`.

---

## 3. La llave alta: tres rutas

### Ruta A — la fuente (piezas extensibles de LM Math)

**Cómo.** Re-subsetear LM Math añadiendo U+23A7…23AB (+ los paréntesis y corchetes
extensibles si se quiere la familia completa), darles ranura de byte en `map_symbol` para
EPS, nombre de glifo en el `/Encoding` del Type42, y **escribir a mano el ensamblado**
(cuántos extensores, cuánto solape, dónde parte el trozo medio).

**A favor**
- El trazo es el de Latin Modern: idéntico en peso y estilo a los paréntesis que ya salen de
  la fuente. Es el único argumento fuerte, y no es menor si la llave va a convivir con un
  `(` de fórmula.

**En contra**
- ⚠️ **El subset no es reproducible hoy.** `plan_lmmath.md` dice «re-subsetear es 1 comando»,
  pero ese comando **no está en el árbol**: no hay script en `tools/`, ni regla en el
  Makefile, ni la fuente original vendorizada. La ruta A empieza por reconstruir un paso de
  build que hoy no existe — y eso es trabajo que hay que hacer **igual** para E1, así que
  conviene contabilizarlo una sola vez.
- Sin tabla `MATH` el ensamblado se escribe igual (§1.2): la fuente ahorra el dibujo, no la
  lógica.
- El solape entre piezas es un parámetro que la fuente declara en la tabla que no tenemos;
  a ojo, las costuras se ven a ciertos tamaños y no a otros — el peor modo de fallo posible,
  porque pasa las compuertas y aparece en una figura concreta.
- Tres rutas distintas que mantener (byte EPS / PUA PDF / codepoint SVG) para cinco glifos
  que solo sirven a este propósito.
- El grosor del trazo lo fija la fuente: **no puede seguir a `line_width`**. Para mobiliario
  de página eso es una limitación real.

### Ruta B — dibujarla en el motor

**Cómo.** Geometría cerrada en un header al estilo `markers.h` (cuatro arcos y un vástago,
parametrizada por longitud, profundidad y posición de la punta), consumida por los tres
backends. Dos consumidores: el delimitador de fórmula (mide su contenido con `vExtent`, como
`Fraction`) y la primitiva de página.

**A favor**
- ⚠️ **Es la única ruta correcta bajo escalado anisótropo**, y ése es el requisito real: una
  llave alta y delgada NO es una llave escalada, es una llave con el vástago más largo y los
  ganchos **del mismo tamaño**. Estirar un glifo (ruta A sin ensamblado) o estirar una struct
  (ruta C con `stretch=true`) deforma los ganchos. Es literalmente la familia de bugs de
  `plan_anisotropia.md`: fórmula isótropa aplicada al caso anisótropo.
- Exacta a cualquier tamaño, sin costuras, sin piezas.
- El grosor sigue a `line_width` y el color a `color`, como cualquier otra primitiva.
- Es la doctrina ya escrita de la casa (§1.4) y reusa maquinaria probada (`fracRule`,
  `penSegment`, `Fraction::vExtent`).
- Una sola geometría sirve a los dos objetos del §0.

**En contra**
- Hace falta una virtual nueva en `Display` —una curva relativa a la pluma, `penCurve`— con
  sus tres implementaciones, para el caso 1 (dentro de la fórmula). El caso 2 no la necesita:
  dibuja en coordenadas de mundo con `curveto`, que ya existe.
- Hay que **diseñar** el trazo. Una llave fea se nota. Mitigación: tomar las medidas del
  glifo de Latin Modern como referencia (proporción gancho/vástago, cómo adelgaza el trazo)
  aunque no se use el glifo.
- Convive con paréntesis de fuente en la misma fórmula: si se dibuja la llave y el `(` sale
  de LM Math, pueden no casar. **Mitigación coherente:** que **todos** los delimitadores
  extensibles se dibujen. Los de tamaño fijo siguen saliendo de la fuente y nadie los compara
  con nada.

### Ruta C — una struct en `lib/`

**Cómo.** `lib/llave.mg` con una struct `Llave(alto)`, cero trabajo de motor.

**A favor**
- Se puede tener **hoy**, sin compilar nada. Para desbloquear la figura de Lillesand esta
  semana, es la respuesta.
- Editable por el usuario, y ejemplifica que la biblioteca sirve para esto.
- Encaja con `place`/`fit`/`repeat` y con `include` local→lib.

**En contra**
- ⚠️ **El cuerpo de una struct se normaliza a la caja unitaria** (contrato de colocación V1),
  así que hacerla alta y delgada pasa por `fit(..., stretch=true)` —deformación anisótropa— o
  por `place(scale=)` —uniforme, que no sirve—. Los ganchos se distorsionan. Es el mismo
  defecto de la ruta A sin ensamblado, y por la misma razón.
- Se puede esquivar **parametrizando la struct** (que dibuje sus ganchos con tamaño absoluto
  y sólo alargue el vástago) — y en cuanto se hace eso, se está escribiendo la ruta B en
  MetaGráfica en vez de en C++, sin acceso a pt y sin poder usarse dentro de una fórmula.
- No resuelve el caso 1 en absoluto.

### Comparación

| | A · fuente | B · motor | C · struct `lib/` |
|---|---|---|---|
| Caso 1 (fórmula) | sí | sí | no |
| Caso 2 (página) | no (tamaño en pt, no en mundo) | sí | sí |
| Alta y delgada sin deformar | sólo con ensamblado a mano | **sí, por construcción** | no sin reescribir la ruta B dentro |
| Sigue a `line_width`/`color` | no | sí | sí |
| Coste inicial | subset reproducible + 5 glifos × 3 rutas + ensamblado | geometría + 1 virtual × 3 backends | horas |
| Riesgo | costuras dependientes del tamaño | diseño del trazo | deformación |

### Recomendación

**B, con C como puente inmediato y A descartada.**

- **C ahora**, para que la figura del curso tenga su llave: `lib/llave.mg` con los ganchos de
  tamaño fijo y el vástago parametrizado. Sirve además de banco de pruebas del trazo: la
  geometría que quede bien ahí es la que se transcribe al header de la ruta B.
- **B después**, en dos entregas: primero la **primitiva de página** (caso 2, sin `penCurve`,
  que es lo que hace falta de verdad y lo que ninguna otra ruta da bien), y sólo si aparece
  una figura que lo pida, el **delimitador de fórmula** (caso 1, con `penCurve`).
- **A descartada** para lo extensible, con una excepción acotada: **U+007B/007D al subset**
  para que `\{` funcione en modo math (§2). Eso sí es fuente, y es inevitable.

⚠️ **No empezar por el caso 1.** Es el que LaTeX hace famoso y el que ninguna figura del
corpus necesita.

---

## 4. Sintaxis propuesta

Tres necesidades distintas, tres formas distintas. **No hay obligación de parecerse a LaTeX**,
y en dos de las tres conviene no parecerse.

### 4.1 Literal — `\{` y `\}`

Por la regla general E1 (§2). Sin semántica: es el carácter. En math, clases `MC_OPEN`/
`MC_CLOSE`.

### 4.2 Delimitador de fórmula — par emparejado

```
$\leftbrace x^2 + y^2 \rightbrace$
```

Se escanea de `\leftbrace` a su `\rightbrace`, se mide lo de en medio con `vExtent` y **los
dos** se dibujan a esa altura. Es «como se esperaría en LaTeX» sin heredar su parte
incómoda: **no** hay `\left.`/`\right.` (delimitador nulo), **no** hay `\big`/`\Big`/`\bigg`,
y **no** hace falta que la pareja sea del mismo tipo.

*Alternativa a considerar (más idiomática de MG, menos familiar):* un comando que toma un
grupo, como `\frac{}{}` y `\hat{}` — `$\braced{x^2 + y^2}$`. Ventaja: reusa `extractGroup`,
que ya existe y ya está probado, y no hay estado de emparejamiento que llevar. Desventaja:
no permite delimitadores asimétricos ni abrir sin cerrar. **Pregunta abierta 5.1.**

### 4.3 Suelta en la página — primitiva, no marcación

```
brace(depth=6) { 14.9 pv_bot   14.9 pv_top }
```

Dos puntos, como una `polyline`: la llave abarca de uno a otro, en **coordenadas de mundo**,
y el lado hacia el que abre sale de la orientación del segmento (girar los puntos la gira).
`depth=` es la profundidad del gancho **en pt**, como `marker_size`, porque es una cantidad
tipográfica y no un dato. Hereda `color`, `line_width`, `dash` como cualquier primitiva.

- `tip=` (0..1, default 0.5) mueve la punta a lo largo del vano — sirve para que apunte al
  rótulo cuando éste no está centrado, que es el caso de Lillesand.
- ⚠️ **Deliberadamente NO acepta un texto.** El rótulo es un `text()` aparte, como en la
  figura actual. Meterle el rótulo obligaría a decidir alineación, separación y giro dentro
  de la primitiva, y eso es composición de página: es del autor.

*Por qué una primitiva y no un marcador:* un marcador se ancla a un punto y tiene un tamaño;
esto abarca un vano y tiene dos extremos. Es una `polyline`, no un `dot`.

### 4.4 Lo que NO propongo

- **`\left`/`\right` genéricos con cualquier delimitador.** Es maquinaria de TeX (clases,
  emparejamiento, `\middle`) para un repertorio que aquí no existe.
- **Tamaños discretos (`\big`, `\Big`).** Son un parche de TeX para cuando el automático
  falla; si el automático mide bien, sobran.
- **Llaves horizontales (`\overbrace`/`\underbrace`).** Son el caso 1 girado y con rótulo
  encima; si la geometría de B queda bien, salen casi gratis — **pero como entrega aparte.**

---

## 5. Preguntas abiertas

1. **§4.2: ¿par emparejado (`\leftbrace … \rightbrace`) o comando con grupo (`\braced{…}`)?**
   La primera es familiar; la segunda reusa código que ya funciona. Mi voto: la segunda, y
   los nombres `\leftbrace`/`\rightbrace` reservados para el literal de tamaño fijo (§4.1),
   sinónimos de `\{`/`\}`.
2. **¿`brace` o `llave`?** Los comandos del lenguaje están en inglés (`polyline`, `rectangle`)
   y los comentarios en español. `brace` es consistente. Pero la biblioteca tiene
   `lib/satellite.mg`, `lib/people.mg`… también en inglés. Voto: `brace`.
3. **¿El caso 2 entra al corpus?** Necesitaría un ejemplo, y `reflectancia_vegetacion.mg` es
   figura de curso, que por la regla vigente **no entra por default**. O se le hace un
   ejemplo propio, o la primitiva nace sin cliente en `examples/` — que es justo lo que
   `seccion_eficaz.mg` vino a cerrar para `lib/`.
4. **¿Se dibujan también `(`/`[`/`⟨` extensibles?** Por coherencia visual (§3, ruta B «en
   contra») la respuesta debería ser sí, pero sólo cuando aparezca la primera fórmula que
   los pida.
5. **E2(b): ¿avisar del cambio de cara que no cambia nada?** Cierra el `m/s`, pero hay que
   medir cuántos falsos positivos da sobre el corpus antes de decidir.

---

## 6. Orden de trabajo sugerido

| # | Entrega | Depende de | Notas |
|---|---|---|---|
| 1 | **E1: escape `\<no-alfabético>`** + pruebas negativas | — | cierra `\{`, `\$`, `\\`, y el `m/s` |
| 2 | **U+007B/007D al subset** + paso de re-subset reproducible en `tools/` | 1 | lo pide E1 en modo math; el paso de build hace falta igual |
| 3 | **`lib/llave.mg`** (ruta C) | — | desbloquea la figura del curso; banco de pruebas del trazo |
| 4 | **Geometría de la llave** en un header estilo `markers.h` | 3 | una fuente de verdad, tres backends |
| 5 | **Primitiva `brace(...)`** (caso 2) | 4 | + ejemplo, + entrada en la galería |
| 6 | *(sólo si hace falta)* `penCurve` + delimitador de fórmula (caso 1) | 4 | la parte que LaTeX hace famosa y nadie ha pedido |

Los pasos 1 y 3 son independientes entre sí y ninguno toca geometría: se pueden hacer en
cualquier orden, o a la vez.
