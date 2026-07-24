# Plan: `\frac` — composición 2-D de fracciones en el modo math

Motivado por `examples/gravitacion_orbita.mg`, cuyas fórmulas fingen la fracción con `/n`.

✅ **Dependencia SATISFECHA (2026-07-23): `plan_text_space` Partes A y B HECHAS.** `\frac`
dimensiona la fracción con `TextLine::width()` —centra numerador/denominador y traza la raya al
ancho del más ancho—; si `Text` no mide lo que dibuja, sale torcido. La **Parte A** (medición
precisa) era el prerrequisito y ya está; la **Parte B** (espaciado automático estilo TeX, antes
la sección "(2)" de este plan) también, y **ambas ALIMENTAN `\frac`**: verificado el mismo día
que, tras A+B, el numerador de `$\frac{G m_1 m_2}{r^2}$` compone **tight** (`Gm₁m₂`, sin los
espacios literales) y `$\frac{a+b}{c}$` trae el `+` binario bien espaciado (`a + b`). La
fundación que `\frac` necesitaba ya no es futuro.

Estado: **SPIKE hecho (2026-07-23), producción DIFERIDA por decisión de
Alejandro.** El código del spike de `\frac` queda **committeado en `main` como base WIP**
(dormante: `ok=66`, ningún ejemplo lo usa; standalone compone bien en EPS/PDF salvo métricas,
SVG con placement/centrado rotos, inline sin implementar); este plan guarda lo aprendido para
retomarlo sin re-descubrirlo. **El espaciado automático (2) ya NO es de este plan: es la Parte
B de `plan_text_space`, hecha.**

Lo destapó `examples/gravitacion_orbita.mg`: sus fórmulas (`F = G m₁m₂/r²`) fingen una fracción
con `/n` (salto de línea) + `align="center"`, y se amontonan — el numerador mete `F =` en la
parte de arriba, así que `r²` queda centrado bajo `= G m₁` y los subíndices chocan con el
denominador. `/n` **apila renglones**, no **compone** una fracción. `\frac` es el arreglo.

## Qué probó el spike (funciona)

`\frac{A}{B}` **standalone** (cuando es todo el `text()`): mide numerador y denominador,
centra el angosto sobre el ancho, traza la raya, y —en EPS/PDF— avanza y ancla bien.

- **EPS y PDF: composición correcta**, centrado e izquierdo (métricas verticales con roce en
  contenido con sub/superíndices — ver el re-test más abajo, que lo agravó a defecto visible).
- **SVG: con bugs de placement y centrado** (el re-test tras A+B mostró que es más que el
  centrado; ver abajo).
- **~118 líneas, 4 archivos, CERO métodos nuevos en los backends.** La clave arquitectónica
  se validó: la composición 2-D vive en `Fraction::draw` usando solo virtuals que ya existen
  (`moveto`/`moveto_nopath`/`rmoveto`/`rlineto`/`stroke`/`text`), igual que `TextBlock`. El
  `currentpoint` nativo de PostScript/PDF hace el trabajo.
- **Aditivo**: `ok=66`, corpus intacto (ningún ejemplo usa `\frac`).

### Dónde está el código del spike

- `include/primitives.h` — `GI_FRACTION` en el enum.
- `include/text.h` — clase `Fraction` (dos hijos `unique_ptr<GraphicsItem>`, `childWidth`).
- `src/text.cpp` — `Fraction::draw` (mide, coloca num/den centrados, traza raya) +
  `Fraction::childWidth` (por `getType()`, sin RTTI).
- `src/text_parser.cpp` — al inicio de `parse_text`, detecta `\frac{A}{B}` (opcionalmente
  entre `$…$`), extrae los dos grupos `{…}` balanceados y **recursa** `parse_text` sobre cada
  uno envuelto en `$…$`. La máquina de estados no es reentrante (globales de archivo) pero se
  reinicia al entrar, así que las llamadas secuenciales funcionan capturando cada resultado.

### Métricas usadas (TeX-simplificado, relativas a `font_size`)

`axis = 0.32·fs` (alto de la raya), `numDrop = 0.55·fs` (base del numerador sobre la raya),
`denRaise = 0.30·fs` (base del denominador bajo la raya), raya de `0.045·fs` de grueso.

### Re-test tras Partes A y B (2026-07-23) — lo que cambió y lo que quedó al descubierto

Con A (medición precisa) y B (espaciado automático) hechas, se volvió a rasterizar el spike
standalone (`\frac{G m_1 m_2}{r^2}`, `\frac{a+b}{c}` centrado, `\frac{1}{2}`):

- ✅ **La composición INTERNA mejoró sola:** el numerador ya no arrastra los espacios del
  fuente (`Gm₁m₂`) y los operadores se espacian por clase (`a + b`). Es la prueba directa de
  que `Fraction::childWidth` → `TextLine::width()` hereda A+B (incluido el `pre_space` de B, que
  `width()` ya suma). No hubo que tocar `\frac` para esto: cayó por la fundación.
- ⚠️ **Las MÉTRICAS verticales son ahora el defecto visible** (punto 3, ascendido de "fino" a
  "hay que hacerlo"): con contenido real, el subíndice del numerador (`m₁`) y el superíndice del
  denominador (`r²`) **chocan con la raya**. `numDrop`/`denRaise` son constantes fijas que no
  miran la altura/profundidad real del contenido. En EPS/PDF se nota; hay que subir los huecos o
  medir extent vertical.
- 🔴 **SVG peor que "centrado con bug":** el denominador queda **demasiado alto** (pegado al
  numerador, con la raya por debajo de ambos), el centrado no coloca el denominador bajo el
  numerador, y en `align="left"` la raya casi no se ve. El modelo de posicionamiento de texto
  del SVG (pluma SIMULADA `cur_x` + `text-anchor="start"` siempre) no coopera con la composición
  manual por `rmoveto` como sí lo hace el `currentpoint` nativo de PS/PDF. Es la misma raíz que
  el "bug de centrado" del punto 2, pero afecta también la colocación VERTICAL — más grande de lo
  que decía el spike. **Al retomar, tratar el SVG como su propio subproblema.**

## Qué NO está (el costo real)

1. **INLINE — el grueso.** `F = \frac{...}{...}` **no se apila**: el spike solo detecta `\frac`
   cuando es *todo* el `text()`. Para que un `\frac` viva **dentro** de una fórmula mayor hay
   que:
   - **Generalizar `TextLine`** de `std::vector<std::unique_ptr<Text>>` a un contenedor de
     `GraphicsItem` genéricos, para que un `Fraction` sea un hijo inline entre trozos de texto.
     ⚠️ **Conservar el `pre_space` por run que introdujo la Parte B:** hoy `TextLine::width()`
     lo suma y `draw()` lo aplica sobre `Text`; el contenedor genérico tiene que seguir
     haciéndolo (o el espaciado math se pierde al generalizar).
   - Enganchar la detección de `\frac` **dentro del bucle char-a-char** del modo math
     (`text_parser.cpp`), no solo al inicio — **en la rama `case '\\'`, junto a los símbolos**.
   - **Integrar con la máquina de espaciado de la Parte B:** un `Fraction` inline es un ÁTOMO y
     necesita su clase para el glue con los vecinos (en TeX una fracción es **Inner**, que se
     comporta ≈ Ord). Al empujar el `Fraction` al `TextLine`, llamar a `mathAtomSpace(MC_ORD)`
     (o añadir `MC_INNER` a `mathGlue` si se quiere fidelidad) y guardarle su `pre_space`, igual
     que hacen `add_symbol`/`add_word`. Sin esto, `x+\frac{a}{b}` no pondría el med del `+`.
   - Que `TextLine::width()` y `TextLine::draw` **cuenten la fracción** como un elemento de
     ancho `W` (avanzar la pluma por `W` tras dibujarla, dejándola en la línea base a
     `anchor.x + W`).
   - Que `Fraction::draw` **avance la pluma** (hoy no lo hace: es standalone).

   Esta generalización de `TextLine` es la que ripplea (width, draw, medición) y se lleva los
   días. **Es la pieza que decide el costo.** La Parte B ya dejó parte del camino: la lógica de
   "sellar un run para que no se fusione" (`mathSeal`) y el `pre_space` en `TextState` son el
   molde de cómo un item no-`Text` entra a la línea.

2. **SVG roto en composición 2-D** (más que "centrado", ver el re-test de arriba). Además del
   `dx` de centrado de más (`align="center"`, medido 39.06 px = W/2 sobre fs=20), el re-test del
   2026-07-23 mostró que en SVG el **denominador queda mal colocado en vertical** y la **raya no
   siempre se ve**. EPS/PDF componen bien con el mismo código → es el modelo de posicionamiento
   del SVG (pluma **simulada** `cur_x` + `text-anchor="start"` fijo) el que no coopera con la
   composición manual por `rmoveto` como sí lo hace el `currentpoint` nativo. Hipótesis del
   centrado a verificar: `TextLine::draw` aplica su `dx` de alineación (`width·fs/(3-align)`)
   porque ve `align≠0` pese al `setTextAlign(0)` de `Fraction::draw` — revisar cómo el SVG
   propaga `text_align` por `pushDrawState`/`text()`. La "cero cambios en backends" vale para el
   *mecanismo* (no hacen falta virtuals nuevos), pero el **posicionamiento de texto del SVG es su
   propio subproblema** y hay que resolverlo con su propia batería de casos.

3. **Métricas verticales** (ascendido de "fino": el re-test lo volvió el defecto más visible en
   EPS/PDF). El subíndice del numerador y el superíndice del denominador **chocan con la raya**
   porque `numDrop`/`denRaise` son constantes que no miran la altura/profundidad real del
   contenido. Opciones: subir los huecos (barato, aproximado) o medir el extent vertical del
   hijo (correcto, más trabajo — hoy solo se mide el ancho).

## Costo estimado (medido, no adivinado)

| Pieza | Costo |
|---|---|
| `\frac` standalone (compuesto + raya + EPS/PDF) | **hecho en el spike** (~½ día) |
| Fix SVG (placement vertical + centrado + raya) | ~1 día (el re-test lo agrandó: es más que el `dx`) |
| **Inline + generalizar `TextLine`** | **2–4 días** (el grueso; toca width/draw/medición) |
| Métricas verticales (roce con la raya) | ~pocas horas (subir huecos) — o +1 día si se mide extent |
| Espaciado interno de num/den | **0: lo dio `plan_text_space` A+B** |
| **Total producción** | **~1 semana**, dominado por la generalización de `TextLine` |

Sigue siendo la más cara de las tres necesidades que destapó la órbita (vs. `rectangle
w/h/at` ≈ 1 h, `lib/` instalable ≈ ½ día). Lo que el spike compró: saber que el costo **no**
está en los backends sino en volver `TextLine` un contenedor de items, y que el mecanismo de
composición ya está probado end-to-end.

## Propuesta para producción (cuando se retome)

1. **Sintaxis**: `\frac{A}{B}` dentro de `$…$`. A y B son fórmulas math completas (recursivo →
   fracciones anidadas salen gratis del contenedor genérico). Alternativa a considerar: la
   convención `/frac{}{}` de MG (como `/n`, `/i`), pero `\frac` es lo que un usuario de LaTeX
   ya escribe y no colisiona con nada.
2. **Paso 1 — generalizar `TextLine`** a `vector<unique_ptr<GraphicsItem>>`. Verificar cero
   churn en el corpus (los rótulos existentes solo tienen `Text`). Es refactor puro; hacerlo
   aislado y con el golden como red.
3. **Paso 2 — `\frac` inline** en el bucle math: al ver `\frac`, extraer los dos grupos,
   construir un `Fraction` (con hijos ya generalizados) y **empujarlo como un item más** del
   `TextLine` en curso.
4. **Paso 3 — avance de pluma**: `Fraction` reporta su ancho `W`; `TextLine::draw` avanza `W`
   tras dibujarlo; `TextLine::width()` lo suma. Esto arregla de paso el centrado (el `dx` de
   alineación pasa a contar el ancho real de la línea, fracciones incluidas).
5. **Paso 4 — SVG**: con el avance de pluma correcto y el `Fraction` como item de línea, revisar
   que el centrado SVG cuadre; si persiste, resolver el `text_align` residual.
6. **Figura que lo pida**: `gravitacion_orbita` es la candidata natural — al entrar `\frac`,
   sus fórmulas dejan de fingir con `/n` y la figura queda lista para `examples/` (regla del
   proyecto: no se construye sin una figura que lo pida; ya la hay).

## El espaciado automático es la Parte B de `plan_text_space` — HECHA (2026-07-23)

Lo que aquí era la sección "(2)" —ignorar los espacios del fuente e insertarlos por clase de
átomo, como TeX— se ejecutó como la **Parte B** de `plan_text_space.md`, junto con la
**medición precisa** (Parte A). Ambas están **hechas y committeadas**. El spike del 2026-07-23
mostró que la medición y el espaciado comparten la misma causa (runs no homogéneos) y merecían
su propio plan; ahí quedaron. Para `\frac` esto significa dos cosas: (a) el numerador/denominador
ya componen con el espaciado correcto **sin trabajo extra** (verificado), y (b) cuando `\frac`
vaya **inline**, tiene que **usar** esa máquina (clasificarse como átomo Inner≈Ord — ver el
punto 1 de "Qué NO está"), no reimplementarla.

## Decisión de futuro pendiente (no del spike)

El comentario reservado de `text_parser.cpp` (`frac/int/prod/sum`) advierte que esto
"probablemente se descarte si a futuro se embeben fragmentos rendereados por TeX". `\frac`
standalone/inline es el caso **ligero** (sin motor de layout completo); `int`/`prod`/`sum` con
límites, radicales y matrices es donde embeber TeX gana. `\frac` no compromete esa decisión:
es la composición 2-D mínima y la más pedida, y el resto puede esperar a que una figura lo
exija.

## Contexto de tipografía math (salvado de `plan_text_struct.md`, jul-2019, ya borrado)

Aquel plan planteó la decisión antes de que hubiera figura; hoy `gravitacion_orbita` la exige,
pero su análisis sigue vigente y aclara la frontera del trabajo:

- **MG coloca glifos de LM Math por CODEPOINT pero NO lee la tabla MATH de OpenType.** De ahí
  **dos gradas**, y hay que elegir la 1:
  1. **Formas fijas + métricas HORNEADAS** (eje matemático ≈ media altura, barra de grosor
     fijo, script ≈ 0.7): cubre `\frac`, `√` sobre radicando chico, `∫` con límites inline. Es
     factible con lo que hay (glifos + anchos por `fmmap` + una regla) y es **justo lo que hace
     el spike** (métricas TeX-simplificadas de arriba). ← el camino elegido.
  2. **Tabla MATH** (operadores display, radicales altos, delimitadores elásticos que estiran):
     trabajo grande, casi ninguna figura lo pide. Diferido indefinido (coincide con la decisión
     de arriba sobre embeber TeX).
- **"Formas gratis, layout no":** `∫ ∑ ∏` ya están mapeados (`\int`→0x222B…) y salen hoy a
  tamaño fijo inline; `√ ∮` serían triviales de agregar al mapa. Lo caro nunca son las formas,
  es el **layout** (dónde centrar, grosor de barra, ensamblados extensibles).
- **El contenedor es UNO, no dos.** La pila de renglones que `\frac` necesita **ya existe**:
  es `TextBlock` (multilínea §14.1, implementado 2026-07-21). `\frac` = ese apilado
  (numerador/denominador) **+** la raya **+** la colocación inline sobre el eje; la raya y el
  "medir y avanzar la pluma" son del **cliente `\frac`**, no del contenedor. `√`/`∫`-con-límites
  seguirían el mismo patrón. ⚠️ La **`struct` de usuario NO encaja para lo inline**: su
  semántica es colocación por matriz en caja unitaria, que pelea con el eje matemático y el
  avance de pluma a media línea (por eso `GI_TEXTSTRUCT` quedó reservado y no se usó).
