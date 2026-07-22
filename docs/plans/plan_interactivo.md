# MetaGráfica interactivo — editor web (DRAFT)

> **Estado: borrador exploratorio, CONDICIONADO A LA CONDICIÓN 4** (uso real por gente que
> no es el autor, `PENDIENTES.md`). Extraído del antiguo `ROADMAP.md` (Propuesta 7). No es
> un plan de implementación cerrado como `plan_lineas.md`/`plan_pdf.md`; recoge la visión y
> las decisiones pendientes para retomarlas cuando toque.
>
> Act. 2026-07-22, al retirar el `TODO` de 2024 —donde este era el último punto vivo— y al
> repasar `ideas.txt`. Los otros cuatro puntos del `TODO` están cerrados: loops, parámetros
> a `OPST` (structs parametrizadas §8, hoy incluso por path), **recursividad** (§8.1, con
> `max_depth` §18 desde el 2026-07-22 y `examples/fractal_tree.mg` que la ejercita) y, del
> cuarto —«a new cleaner parser with better feedback to final users»—, la mitad del parser
> quedó **decidida en contra por los hechos** (el descenso recursivo aguantó §13 entero) y
> la de la retroalimentación avanzó mucho pero **sigue viva**: faltan la columna en los
> errores, el mensaje de `&path`, el de indexar un literal de lista y los argumentos
> nombrados que las primitivas se tragan en silencio.

## Cuándo se decide, y por qué no ahora

**Ni descartado ni abierto indefinidamente: esperando usuarios.** La propuesta de valor del
editor —bajar la barrera de entrada— es de las que **no se pueden evaluar sin gente de
fuera**, así que decidirla hoy, en un sentido o en el otro, sería especular. Es la regla del
proyecto («no se construye sin una figura que lo pida») aplicada al empaquetado: aquí hace
falta un **usuario** que lo pida.

Lo que sí sabemos hoy, y conviene tener a mano cuando el tema reaparezca:

- 🔎 **La barrera medida no es instalar, es no saber el lenguaje.** El autor tropezó cuatro
  veces armando `tiro_parabolico` y el agente otras cuatro escribiendo `figure_02` y
  `local/karl.mg` (ver condiciones 4 y 5 en `PENDIENTES.md`). **Ninguno** de esos ocho
  tropiezos fue «no pude compilar el binario». Un editor web ahorra un `make`; no quita
  ninguno de los ocho. La **referencia de usuario** (condición 5) sí.
- ⚠️ **El resaltado de sintaxis es una SEGUNDA implementación de la superficie del
  lenguaje** (gramática de CodeMirror/Monaco) que hay que mantener en paralelo con el
  parser. Construirla **antes** de congelar (condición 1), con los nombres todavía en
  movimiento, compra mantenimiento duplicado; después del congelamiento es un costo único.
  El orden correcto es el inverso al que este documento asumía.
- 💡 **El argumento de la IA lo rebasaron los hechos.** Aquí figuraba como funcionalidad del
  editor («conectar un chat a una herramienta IA con el contexto de mg»), y eso ya ocurrió
  en una terminal, con la spec y el `CLAUDE.md` en contexto (`figure_02`, `karl.mg`,
  `fractal_tree`). Lo que lo hizo posible fue **documentación, no interfaz** — y la
  condición 5 ya lleva ese requisito escrito. La referencia entrega el beneficio que el
  editor promete, y además el otro.
- **WASM es barato de construir pero no de tener:** un segundo blanco de compilación, con
  superficie de despliegue, que se rompe en silencio cuando cambie el motor y sin ninguna de
  las cuatro compuertas mirándolo.

**La galería NO está condicionada** (ver su sección abajo): es la mitad barata, no necesita
WASM ni servidor ni gramática de editor, y ataca el cuello de botella real.

## Visión

Un editor en el navegador donde el usuario escribe `.mg` en un panel y ve el gráfico renderizado al lado, en tiempo real (o con un botón "compilar"). Además cuenta con herramientas interactivas para crear primitivas y acomodarlas en el canvas. También se puede conectar un chat a una herramienta IA que tenga el contexto de mg y pueda entender instrucciones para crear y corregir gráficos.

Sin instalación,accesible desde cualquier dispositivo. Baja radicalmente la barrera de entrada: alguien puede probar MetaGráfica sin compilar el binario ni tocar la terminal y usar un asistente IA para crear gráficos complejos.

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
- ~~¿Sobre qué versión del lenguaje se construye?~~ **RESUELTO por los hechos:** V3, que es
  el único compilador que hay (`bin/mg` en `main`). Este punto decía «lo natural es apuntar
  a V2… un prototipo puede arrancar con V1», y las dos desaparecieron — V1 quedó congelada
  en `v1-legacy` y V2 se subsumió en V3. Que el punto siguiera aquí un año es el dato de que
  **nada ha empujado este plan**.
- Resaltado de sintaxis: definir la gramática para el editor (CodeMirror/Monaco).

## Origen

Extraído de `ROADMAP.md` (Propuestas 7 y 2) el 2026-07-04, al retirar ese documento por
obsoleto: sus demás propuestas o ya están implementadas (SVG, errores con línea) o quedaron
superadas por `especificacion_mg.md` (variables/loops, nombres, parámetros a structs).
