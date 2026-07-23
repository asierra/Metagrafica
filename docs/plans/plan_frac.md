# Plan: tipografía matemática — `\frac` + espaciado automático estilo TeX

**Paquete** de dos mejoras al modo math que motiva la MISMA figura
(`examples/gravitacion_orbita.mg`) y que conviene hacer juntas: **(1)** `\frac`
(composición 2-D de fracciones) y **(2)** el espaciado automático (ignorar los espacios
del fuente e insertarlos por clase de átomo, como TeX). Las dos van en este plan porque son
el mismo esfuerzo —"que `$…$` componga como TeX"— y la figura que pide una pide la otra.

Estado: **`\frac` con SPIKE hecho (2026-07-23), producción DIFERIDA por decisión de
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

## (2) Espaciado automático estilo TeX

**Diagnosticado el 2026-07-23** al notar que `$F = G m_1 m_2$` sale con espacios de más.
Hoy un espacio dentro de `$…$` cae al `default` del bucle de `parse_text` (`text_parser.cpp`)
y se **imprime literal**, igual que en prosa.

**El arreglo NO es "quitar los espacios".** Se probó (una rama `case ' '` que los descarta si
`math_mode`) y es demasiado bruto: rompe **12 goldens** del corpus y cramps las fórmulas que
usan los espacios a propósito —`$\phi = sin(n \pi x),  n = 1..4$` → `φ=sin(nπx),n=1..4`,
ilegible: el `=` pegado, `sin` contra el paréntesis, la coma contra la `n`. Revertido.

La razón es que **TeX hace DOS cosas**: ignora los espacios del fuente **e inserta** espaciado
por **clase de átomo**:
- espacio **medio** alrededor de RELACIONES (`=`, `<`, `>`, `≈`…),
- espacio **medio** alrededor de OPERADORES binarios (`+`, `−`, `×`… — dependiente de contexto),
- espacio tras NOMBRES DE FUNCIÓN (`sin`, `cos`, `log`, `tan`…),
- espacio **fino** tras PUNTUACIÓN (`,` `;`),
- nada entre átomos ORDINARIOS (`G m_1 m_2` → adyacentes, solo corrección itálica).

**Propuesta:**
1. Un **clasificador de átomos** en el bucle math: cada trozo emitido lleva su clase
   (ordinario / relación / operador / función / puntuación / apertura / cierre). Ya casi está
   la información —los símbolos entran por `\name` con su código, los operadores y relaciones
   son caracteres conocidos, `math_functions[]` ya lista `sin`/`cos`/…—.
2. Al descargar cada trozo, **insertar el espacio** que corresponde según la clase del trozo
   anterior y el actual (tabla de pares, como la de TeX). El espacio del FUENTE se **ignora**
   (se consume sin imprimir), como pedía la intuición original.
3. **Comandos explícitos** para forzar/anular: `\,` (fino), `\;` (medio), `\quad`, y quizá `\!`
   (negativo), para los casos que la tabla no acierta.
4. **Churn de goldens ESPERADO y para bien**: las 12 fórmulas del corpus cambian a su forma
   correcta. Re-bendecir con `capture`/`images` tras revisar que cada una mejora.

**Costo**: moderado —una tabla de espaciado por pares de clases + el clasificador—, del orden
de un par de días. Menor que `\frac`, y comparte la infraestructura de trozos del text_parser.

**Orden sugerido dentro del paquete**: el espaciado (2) es independiente de `\frac` y más
barato; puede ir primero y dejar el corpus math ya bien espaciado, y luego `\frac` (1) monta
encima. O juntos, si se re-bendice una sola vez.

## Decisión de futuro pendiente (no del spike)

El comentario reservado de `text_parser.cpp` (`frac/int/prod/sum`) advierte que esto
"probablemente se descarte si a futuro se embeben fragmentos rendereados por TeX". `\frac`
standalone/inline es el caso **ligero** (sin motor de layout completo); `int`/`prod`/`sum` con
límites, radicales y matrices es donde embeber TeX gana. `\frac` no compromete esa decisión:
es la composición 2-D mínima y la más pedida, y el resto puede esperar a que una figura lo
exija.
