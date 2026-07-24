# Plan: `\frac` — composición 2-D de fracciones en el modo math

Motivado por `examples/gravitacion_orbita.mg`, cuyas fórmulas fingen la fracción con `/n`.

⚠️ **Depende de `plan_text_space.md` (medición precisa de `Text`).** `\frac` dimensiona la
fracción con `TextLine::width()` —centra numerador/denominador y traza la raya al ancho del más
ancho—; si `Text` no mide lo que dibuja, sale torcido. La **Parte A** de `plan_text_space`
(medición precisa, lista para ejecutar) es su prerrequisito. El **espaciado automático**
(antes "(2)" de este plan) también se mudó allí: son el mismo esfuerzo de fundación.

Estado: **SPIKE hecho (2026-07-23), producción DIFERIDA por decisión de
Alejandro.** El código del spike de `\frac` queda **committeado en `main` como base WIP**
(dormante: `ok=66`, ningún ejemplo lo usa; standalone anda en EPS/PDF, con un bug de centrado
en SVG e inline sin implementar); este plan guarda lo aprendido para retomarlo sin
re-descubrirlo. El espaciado automático (2) está **diagnosticado, no empezado**.

Lo destapó `local/gravitacion_orbita.mg`: sus fórmulas (`F = G m₁m₂/r²`) fingen una fracción
con `/n` (salto de línea) + `align="center"`, y se amontonan — el numerador mete `F =` en la
parte de arriba, así que `r²` queda centrado bajo `= G m₁` y los subíndices chocan con el
denominador. `/n` **apila renglones**, no **compone** una fracción. `\frac` es el arreglo.

## Qué probó el spike (funciona)

`\frac{A}{B}` **standalone** (cuando es todo el `text()`): mide numerador y denominador,
centra el angosto sobre el ancho, traza la raya, y —en EPS/PDF— avanza y ancla bien.

- **EPS y PDF: correcto**, centrado e izquierdo. La fracción `G m₁m₂/r²` sale bien compuesta.
- **SVG: izquierdo bien, centrado con bug** (ver abajo).
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

## Qué NO está (el costo real)

1. **INLINE — el grueso.** `F = \frac{...}{...}` **no se apila**: el spike solo detecta `\frac`
   cuando es *todo* el `text()`. Para que un `\frac` viva **dentro** de una fórmula mayor hay
   que:
   - **Generalizar `TextLine`** de `std::vector<std::unique_ptr<Text>>` a un contenedor de
     `GraphicsItem` genéricos, para que un `Fraction` sea un hijo inline entre trozos de texto.
   - Enganchar la detección de `\frac` **dentro del bucle char-a-char** del modo math
     (`text_parser.cpp`), no solo al inicio.
   - Que `TextLine::width()` y `TextLine::draw` **cuenten la fracción** como un elemento de
     ancho `W` (avanzar la pluma por `W` tras dibujarla, dejándola en la línea base a
     `anchor.x + W`).
   - Que `Fraction::draw` **avance la pluma** (hoy no lo hace: es standalone).

   Esta generalización de `TextLine` es la que ripplea (width, draw, medición) y se lleva los
   días. **Es la pieza que decide el costo.**

2. **Bug de centrado en SVG** (acotado). En `align="center"` el numerador recibe un `dx` de
   más (medido: 39.06 px = W/2 sobre un fs=20). EPS/PDF centran bien con el mismo código, así
   que es específico del SVG: su pluma **simulada** (`cur_x`) + `text-anchor` no cooperan con
   el centrado manual como sí lo hace el `currentpoint` nativo. Hipótesis a verificar al
   retomar: `TextLine::draw` aplica su `dx` de alineación (`width·fs/(3-align)`) porque ve
   `align≠0` pese al `setTextAlign(0)` de `Fraction::draw` — revisar cómo el SVG propaga
   `text_align` a través de `pushDrawState`/`text()`. Nota: la "cero cambios en backends" vale
   para el *mecanismo* (no hacen falta virtuals nuevos), pero el modelo de posicionamiento de
   texto del SVG **sí necesita atención** para el centrado.

3. **Métricas finas.** El subíndice `m₁` roza la raya; el hueco del denominador en `a/b` es
   chico. Ajuste de las constantes de arriba.

## Costo estimado (medido, no adivinado)

| Pieza | Costo |
|---|---|
| `\frac` standalone (compuesto + raya + EPS/PDF) | **hecho en el spike** (~½ día) |
| Fix centrado SVG | ~pocas horas |
| **Inline + generalizar `TextLine`** | **2–4 días** (el grueso; toca width/draw/medición) |
| Métricas | ~pocas horas |
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

## El espaciado automático se mudó a `plan_text_space.md`

Lo que aquí era la sección "(2)" —ignorar los espacios del fuente e insertarlos por clase de
átomo, como TeX— es ahora la **Parte B** de `plan_text_space.md`, junto con la **medición
precisa** (Parte A), que es la fundación que `\frac` necesita (ver la nota de dependencia
arriba). El spike del 2026-07-23 mostró que la medición y el espaciado comparten la misma
causa (runs no homogéneos) y merecen su propio plan.

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
