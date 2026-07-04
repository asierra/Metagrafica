# MetaGráfica interactivo — editor web (DRAFT)

> **Estado: borrador exploratorio.** Extraído del antiguo `ROADMAP.md` (Propuesta 7),
> actualizado al estado real del proyecto. No es un plan de implementación cerrado como
> `plan_lineas.md`/`plan_pdf.md`; recoge la visión y las decisiones pendientes para
> retomarlas cuando toque.

## Visión

Un editor en el navegador donde el usuario escribe `.mg` en un panel y ve el gráfico
renderizado al lado, en tiempo real (o con un botón "compilar"). Sin instalación,
accesible desde cualquier dispositivo. Baja radicalmente la barrera de entrada: alguien
puede probar MetaGráfica sin compilar el binario ni tocar la terminal.

## Prerequisitos — ya cumplidos

Cuando se escribió la propuesta original, estos eran bloqueantes; hoy ya están:

- **Backend SVG (era Propuesta 4 del ROADMAP): ✅ hecho.** `SVGDisplay` produce SVG nativo,
  que el navegador renderiza directamente sin conversiones. Con salida EPS el editor no
  sería viable en el navegador; con SVG el resultado se inserta tal cual en el DOM.
- **Errores con número de línea (era Propuesta 1): ✅ hecho.** El compilador ya emite
  `archivo:línea: mensaje`, lo que permite resaltar el error en el editor. *(Pendiente
  menor: la columna.)*

Es decir, el camino está despejado: el trabajo restante es de empaquetado y front-end, no
del motor.

## Arquitectura — dos opciones

### Opción A — Compilador en el servidor
El editor envía el texto `.mg` a un servidor que ejecuta `mg salida.svg` y devuelve el
SVG. Reutiliza el binario existente tal cual; trivial de montar. Contras: requiere
infraestructura (un proceso por petición), latencia de red, y hay que aislar la ejecución
(sandbox) porque se corre código de terceros.

### Opción B — Compilador en WebAssembly (recomendada)
Compilar `mg` a WASM con Emscripten. El compilador corre **completo en el navegador**, sin
servidor: la experiencia es instantánea y no hay infraestructura que mantener ni que
asegurar.

**Ventaja concreta para el editor:** solo se necesita la ruta **SVG**, y el backend SVG
**no depende de libharu** (solo el backend PDF lo usa). Por tanto la build WASM puede
excluir por completo `third_party/libharu/` y compilar únicamente el pipeline
lexer→parser→`SVGDisplay`. Resultado: un módulo WASM pequeño y una superficie de
compilación mucho menor que la del binario nativo.

**Punto a resolver:** el texto matemático. `SVGDisplay` mapea CMMI/Symbol a caracteres
Unicode (griegas, símbolos), así que no embebe fuentes; la fidelidad en el navegador
depende de que los glyphs estén disponibles (fuente del sistema o una webfont
matemática cargada por la página). Hay que decidir si se sirve una webfont para
consistencia.

**Recomendación:** Opción B. Elimina el servidor y aprovecha que el editor solo necesita
SVG (sin libharu). La Opción A sirve como prototipo rápido si se quiere validar la UI
antes de invertir en la build WASM.

## Funcionalidades mínimas del editor

- Editor de texto con resaltado de sintaxis para `.mg`.
- Renderizado SVG al lado (en tiempo real o con botón "compilar").
- Mensajes de error con número de línea, resaltados sobre el editor (el motor ya los da).
- Descarga del SVG (y, si se usa la Opción A o una build WASM con libharu, también EPS/PDF).
- Galería de ejemplos cargables con un click (ver abajo).

## Relación con una galería de ejemplos (era Propuesta 2 del ROADMAP)

La propuesta original incluía una galería web estática (código `.mg` junto a su imagen).
Con SVG nativo, la conversión externa que planteaba (Inkscape/pdf2svg) ya no hace falta:
los `.svg` se generan directamente con `mg`. La galería y el editor comparten objetivo y
se solapan —"ejemplos cargables con un click" es una función del editor—, así que conviene
tratarlas como un solo esfuerzo de documentación/onboarding:

- **Galería estática** (sin editor): HTML que muestra cada `examples/*.mg` con su `.svg`
  generado y comentarios explicando cada sección. Es un subconjunto trivial del editor y
  se puede hacer antes, de forma incremental.
- **Galería dentro del editor:** los mismos ejemplos precargados como plantillas editables.

## Pendientes de decisión

- Opción A vs B (servidor vs WASM) — recomendación: B.
- Estrategia de fuentes matemáticas en el navegador (webfont vs fuentes del sistema).
- Alcance de la galería: ¿estática primero, o directamente integrada en el editor?
- ¿Sobre qué versión del lenguaje se construye? Lo natural es apuntar a **V2** (sintaxis
  moderna, ver `especificacion_mg.md`), pero un prototipo puede arrancar con V1.
- Resaltado de sintaxis: definir la gramática para el editor (CodeMirror/Monaco).

## Origen

Extraído de `ROADMAP.md` (Propuestas 7 y 2) el 2026-07-04, al retirar ese documento por
obsoleto: sus demás propuestas o ya están implementadas (SVG, errores con línea) o quedaron
superadas por `especificacion_mg.md` (variables/loops, nombres, parámetros a structs).
