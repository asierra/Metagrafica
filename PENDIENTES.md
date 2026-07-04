# Correcciones pendientes — auditoría de backend

> **Nota (2026-07-04): este archivo es histórico y puede retirarse al abrir la rama
> de V2.** Ya no queda ningún defecto de código real en el motor V1. Las conclusiones
> que sí importan hacia adelante —qué capa del motor continúa en V2, el estado
> endurecido del backend, el refactor `DeviceBackend` diferido, el puente isométrico y
> la falta de red de regresión— quedaron consolidadas en `especificacion_mg.md` §22.
> Las decisiones de diseño abiertas viven en §19. Lo de abajo es el registro de la
> auditoría (ítems ya resueltos, para trazabilidad).

Estado tras la sesión del 2026-06-30. Los ítems críticos (#1–#4, #6) están resueltos.

**Actualización 2026-07-02:** corregido el bug de `ARCST` en formato apaisado
(`StructureArc::draw_side` compensaba la posición con la rama equivocada; ahora
`stpos.x *= ratio` incondicional). La política de aspect ratio de V2 quedó
definida en `especificacion_mg.md` §3.1 (espacio isométrico); cuando se
implemente, las compensaciones por `getRatio()` de `structure.cpp` desaparecen
del motor. Se añadieron los ítems #16 y #17, detectados al verificar la receta
de la Etapa 1 contra el código. La **Etapa 1 del ítem #8 quedó ejecutada y
verificada** ese mismo día (ver el TL;DR de #8).

**Actualización 2026-07-04:** auditoría completa contra el código. Resueltos desde
entonces: **#7, #9, #10, #14, #15** (lote de quick-wins), **#16** (SVG `deviceTranslate`
ya escala por `dvx/dvy`) y **el bug de PDF de `fig2-3`** (los cuatro bugs de `libharu`
`0x1051/0x1025` se corrigieron; ver commits `245baea`, `ee21b74`). El **#12** quedó
medio resuelto: el refactor de estilos de línea (ver `plan_lineas.md`) reemplazó
`DisplayState::line_width` (int) por `line_width_pt` (float); el `int` restante es
`Attribute::value`, artefacto de la gramática V1 que V2 resuelve con `width=` float.
Siguen abiertos: **#11** (y la mitad de gramática de #12). Ver marcas
`✓ RESUELTO` en cada ítem.

**Actualización 2026-07-04 (bis):** resueltos **#13** (`Matrix` a `double`,
propagado a toda la tubería) y **#17** (`defaultmatrix` → `initmatrix`). El único
pendiente de código real cae; lo que queda abierto (#8 Etapa 2, #11, mitad de #12)
es refactor diferido o materia de diseño de la spec V2, no defectos del motor.

---

## Importante (diseño / fidelidad a la filosofía)

### #7 — Invariante de destrucción implícita entre `structure_map` y `StructureUser` — ✓ RESUELTO (2026-07-04)
**Archivo:** `src/structure.cpp:20-24`, `include/mg.h:160`

> **Resuelto.** El `prlist.clear()` explícito en `~MetaGrafica` ya existía con su
> comentario en `structure.cpp`; se añadió además un comentario cruzado junto a
> `structure_map` en `mg.h` que documenta la invariante. (Nota: la corrección
> propuesta abajo hablaba de "orden de declaración", pero `prlist` y `structure_map`
> están en clases distintas —base vs derivada—; la protección real es el clear()
> explícito, no el orden.)

`~StructureUser` llama `structure->decUses()` sobre un puntero crudo. El
`~MetaGrafica` necesita el `prlist.clear()` explícito para destruir los
`StructureUser` *antes* de que el mapa libere las `Structure`. Funciona, pero el
orden es una invariante implícita que se rompe si alguien reordena los miembros
de `MetaGrafica`.

**Corrección mínima:** documentar la invariante con un comentario en `mg.h` que
explique por qué `prlist` debe declararse antes que `structure_map` (C++ destruye
miembros en orden inverso de declaración).

```cpp
// En MetaGrafica, el orden de declaración importa:
// prlist se destruye ANTES que structure_map porque ~StructureUser
// llama decUses() sobre punteros crudos al mapa.
GraphicsItemList prlist;   // heredado de Structure — debe morir primero
map<string, std::unique_ptr<Structure>> structure_map;
```

---

### #8 — `Display` es una interfaz "gorda" e inconsistente
**Archivo:** `include/Display.h`

> **✓ Etapa 1 ejecutada (2026-07-02), partes 1A y 1B completas.** El estado y
> los métodos duplicados subieron a `Display` (nuevo `src/Display.cpp` para
> `structure()`); las transformaciones quedaron con el patrón `device*` (hooks
> MTLC por backend, contabilidad MTST en la base). Verificado: compilación
> limpia con `make clean && make`, y salida **idéntica byte a byte** en los 9
> ejemplos × 3 formatos (la única diferencia en EPS fue la línea `%%Title`, que
> incluye el nombre del archivo de salida; `fig2-3.pdf` sigue fallando igual por
> el error preexistente de libharu, ver nota). **Etapa 2** (extraer un tipo
> `DeviceBackend` puro) queda **diferida** hasta que haya un cuarto backend o se
> quiera testeabilidad.
>
> La receta de la Etapa 1 se conserva abajo como referencia de lo hecho y como
> plantilla del método de verificación para la Etapa 2.

~40 métodos virtuales puros mezclados con métodos concretos sin criterio claro:
`setLineGray` es concreto, `setLineWidth` es virtual puro, `setLineStyle` tiene
cuerpo virtual. Cada backend nuevo debe implementar las ~40 o el compilador
rechaza la clase.

**Actualizado 2026-07-01**, tras implementar `SVGDisplay` (ver `src/SVGDisplay.cpp`,
sesión de corrección de errores de compilación) — con los tres backends
(`EPSDisplay`, `PDFDisplay`, `SVGDisplay`) ya existentes, se puede señalar
exactamente qué está duplicado y qué es genuinamente específico de cada
formato de salida.

**Evidencia — código duplicado carácter por carácter en los tres backends:**

1. **Miembros de estado de matrices**, privados e idénticos en los tres:
   `include/EPSDisplay.h:110-119`, `include/PDFDisplay.h:84-101`,
   `include/SVGDisplay.h:90-108`:
   ```cpp
   MetaGrafica* mg_context = nullptr;
   float relfontsize;
   Matrix mt;
   Matrix mtst;
   stack<Matrix> mtstack;
   stack<DisplayState> dsstack;
   ```

2. **`pushMatrix(PredefinedMatrix)`, `saveMatrix`, `popMatrix()`,
   `popMatrix(PredefinedMatrix)`, la rama `MTST` de `compose`** — mismo cuerpo
   en `EPSDisplay.cpp:637-711`, `PDFDisplay.cpp:480-540`, `SVGDisplay.cpp:206-244`.
   Ninguno de los tres hace algo distinto con esta aritmética; es pura
   contabilidad de matrices, sin una sola llamada a la API del formato de salida.

3. **`structure(string)`** — `EPSDisplay.cpp:657`, `PDFDisplay.cpp:495`,
   `SVGDisplay.cpp:255`. Los tres resuelven la estructura vía `mg_context`,
   empujan `dspstate`/`mtst`, llaman `strct->draw(*this)` y restauran. La única
   diferencia es que `EPSDisplay` añade un branch para `isDefinedInDevice()`
   (invocar el nombre de la estructura como procedimiento PS ya definido) que
   ni `PDFDisplay` ni `SVGDisplay` implementan — y que en la práctica nunca se
   ejecuta en ningún backend porque `Structure::define_in_device()`
   (`src/structure.cpp:7`) no se invoca desde ningún punto del pipeline hoy.

4. **`setMGContext`** — idéntico en `include/EPSDisplay.h:62`,
   `include/PDFDisplay.h:52`, `include/SVGDisplay.h:60`:
   `{ mg_context = mg; }`.

5. **`setRelFontSize`** — idéntico en `include/EPSDisplay.h:83-88`,
   `include/PDFDisplay.h:68-72`, `include/SVGDisplay.h:77-81` (mismo guard de
   "si no cambió, no hagas nada" + invalidar `fontFace`).

6. **El factor `* 0.2`** para convertir `line_width` a unidades de dispositivo
   está repetido como número mágico en `EPSDisplay.cpp:576`,
   `PDFDisplay.cpp:167`, `SVGDisplay.cpp:44`.

Al escribir `SVGDisplay` varios overrides declarados como "obligatorios" por la
interfaz resultaron ser literalmente el cuerpo por defecto que ya provee
`Display` (p. ej. `setFontSize`, `setLineStyle` antes de que se les diera lógica
propia) — boilerplate forzado por la firma virtual pura, no por una necesidad
real del backend.

**Diagnóstico de fondo.** `Display` mezcla tres responsabilidades sin nombrarlas:

- **A) La máquina de estado semántica de MetaGráfica** — pila de transformaciones
  (`mt`/`mtst`/`mtstack`), estado de dibujo (`dspstate`/`dsstack`), resolución de
  estructuras (`mg_context`), tamaño relativo de fuente. Es lógica *independiente
  del dispositivo* que los tres backends implementan idénticamente.
- **B) Las primitivas de salida** — "emite una línea / arco / texto en este
  formato". Genuinamente específicas del formato.
- **C) Mutadores de `dspstate`** — `setFillGray`, `setLineColor`, etc. Unos
  concretos, otros virtuales puros, sin criterio.

Verificado contra el código (2026-07-01): la rama `MTLC` de las transformaciones
**nunca** toca `mt`; siempre es nativa del dispositivo (`translate` de PS, `<g>`
de SVG). `mt` solo acumula el *seed* `dvx/dvy` de `start()` y las matrices `MTST`.
Y `save()/restore()` **no** scopa transformaciones sino *estado* (color/dash
emitidos al dibujar una estructura) — dato que descarta la "simplificación
tentadora" de hornearlo todo (ver Preguntas abiertas abajo).

**Corrección recomendada — separar A de B en dos tipos, por etapas.**

La clave que hace esto barato: `primitives.cpp` y `structure.cpp` solo conocen
`Display&`. Si `Display` sigue siendo la fachada concreta y delega en un backend
interno, **el código de `draw()` no se toca**.

*Etapa 1 (bajo riesgo, sin cambiar firmas ni salida) — subir A a `Display` como
`protected` concreto:*

- Miembros: `mt`, `mtst`, `mtstack`, `dsstack`, `mg_context`, `relfontsize`.
- Métodos ya idénticos en los tres: `pushMatrix(Matrix&)`,
  `pushMatrix(PredefinedMatrix)`, `saveMatrix`, `popMatrix()`,
  `popMatrix(PredefinedMatrix)`, la rama `MTST` de `compose` y de
  `translate/scale/rotate/shear`.
- `setMGContext`, `setRelFontSize` dejan de ser virtuales.
- `structure(string)` pasa a concreto en la base. La optimización de EPS
  (`isDefinedInDevice()`) es hoy **código muerto** (`define_in_device()` no se
  llama desde ningún backend): borrarla por ahora, y reintroducirla en la Etapa 2
  como capacidad del backend, no como `if` en `structure()`.

Tras la Etapa 1, cada backend deja de implementar ~15 métodos de contabilidad;
solo le quedan las primitivas reales (B).

*Etapa 2 (mayor alcance) — extraer un `DeviceBackend` puro.* Aquí (no en la
Etapa 1) va también eliminar la asimetría `x*dvx` vs `mt` — aplicar `dvx/dvy` en
un solo lugar de la máquina de estado en vez de a mano en cada rama `MTLC` —
porque cambia los bytes emitidos y hoy cada backend escala distinto (ver #16),
lo que contradice el requisito "salida idéntica" de la Etapa 1:

```cpp
// Interfaz mínima, SIN estado, todas las coords ya transformadas por la máquina A.
struct DeviceBackend {
  virtual ~DeviceBackend() = default;
  // ciclo de vida
  virtual void begin(float wpt, float hpt) = 0;
  virtual void end() = 0;
  // agrupamiento de ESTADO (gsave/grestore, <g>) — no de transformaciones
  virtual void beginGroup() = 0;
  virtual void endGroup() = 0;
  // transform nativo del MTLC (PS translate / SVG <g transform>)
  virtual void concatDeviceTransform(const Matrix&) = 0;
  // primitivas (reciben estilo explícito, ver Pregunta abierta 2)
  virtual void path(const PathCmds&, const Style&) = 0;
  virtual void arc(...) = 0;
  virtual void dot(...) = 0;
  virtual void text(...) = 0;
  virtual void setFont(FontFace, float sizePt) = 0;  // cada backend resuelve nombres
  // capacidad opcional para reintroducir la optimización de EPS:
  virtual bool defineStructure(const string&) { return false; }
  virtual void invokeStructure(const string&) {}
};
```

`Display` deja de tener virtuales de dibujo: es la máquina A concreta que sostiene
un `std::unique_ptr<DeviceBackend>` y traduce sus ~40 métodos a la docena de
primitivas del backend. El factor `* 0.2` de `line_width` vive una sola vez en A.

**Impacto.** Un backend nuevo implementa **solo** `DeviceBackend` (~12 métodos) y
*estructuralmente no puede* equivocarse con la pila de transformaciones — ni
siquiera tiene acceso a ella. El peor bug de la sesión de `SVGDisplay` fue
exactamente eso: un `pushMatrix(Matrix&)` no-op que dejaba las estructuras
repetidas sin posición; con este diseño ese error es imposible de escribir.

**Preguntas abiertas (NO son mejoras gratis — anotar antes de decidir):**

1. *¿Hornear también `MTLC` en `mt`, eliminando el transform nativo del backend?*
   Tentador, pero riesgoso: en PS/PDF el ancho de línea vive en user-space y
   escala con la CTM; si se hornean las coords pero `line_width` sigue siendo
   escalar, un `scale` no uniforme rompería la fidelidad del grosor. Además
   cambiaría el estilo del `.eps` generado (coords premultiplicadas en vez de
   `translate`/`scale`). Mantener el transform nativo del backend es más seguro.
2. *¿Backend sin estado (estilo explícito por primitiva) vs. con estado?* SVG ya
   funciona sin estado (cada `<path>` lleva sus atributos). EPS/PDF son con
   estado (emiten `setrgbcolor` y confían en `gsave`). Un backend sin estado es
   más limpio y testeable, pero volvería el `.eps` más verboso. Decidir por
   backend, no globalmente.

---

## Etapa 1 — receta ejecutable

Objetivo: mover a `Display` el estado y los métodos de matrices duplicados, **sin
cambiar el comportamiento observable**. Al terminar, los tres `.eps/.pdf/.svg`
generados deben ser **idénticos byte a byte** a los de antes del cambio.

La Etapa 1 tiene dos partes. **1A es puramente mecánica (hazla siempre). 1B
requiere un poco de criterio (opcional; si dudas, déjala para después).**

### Paso 0 — Capturar salida de referencia ANTES de tocar nada

```bash
make
mkdir -p /tmp/mg_ref
for f in examples/*.mg; do
  b=$(basename "$f" .mg)
  ./bin/mg "$f" "/tmp/mg_ref/$b.eps" 2>/dev/null
  ./bin/mg "$f" "/tmp/mg_ref/$b.svg" 2>/dev/null
  ./bin/mg "$f" "/tmp/mg_ref/$b.pdf" 2>/dev/null
done
```

(Algún ejemplo puede fallar por un bug preexistente de `INCLUDE`; ignóralo, solo
importa que la MISMA lista falle igual después.)

### Paso 1A — Mover estado y métodos idénticos a `Display` (mecánico)

En `include/Display.h`, en la sección `protected:` (donde ya vive `dspstate`),
**añadir** estos miembros:

```cpp
  MetaGrafica* mg_context = nullptr;
  float relfontsize = 1.0f;
  Matrix mt;
  Matrix mtst;
  stack<Matrix> mtstack;
  stack<DisplayState> dsstack;
```

(Requiere `#include <stack>` y `using std::stack;` en `Display.h` si no están.)

Y **añadir** estos métodos concretos a `Display` (dejan de ser virtuales puros;
borra sus declaraciones `= 0` previas):

```cpp
  void setMGContext(MetaGrafica* mg) { mg_context = mg; }

  void setRelFontSize(float rfz) {
    if (rfz == relfontsize) return;
    relfontsize = rfz;
    dspstate.fontFace = FN_NOFACE;
  }

  void pushMatrix(Matrix& m)              { mtstack.push(mt); mt *= m; }
  void pushMatrix(PredefinedMatrix pdmt)  { if (pdmt == MTST) { mtstack.push(mtst); mt *= mtst; } }
  void saveMatrix(PredefinedMatrix pdmt)  { if (pdmt == MTST) mtstack.push(mtst); }
  void popMatrix()                        { mt = mtstack.top(); mtstack.pop(); }
  void popMatrix(PredefinedMatrix pdmt)   { if (pdmt == MTST) mtst = mtstack.top(); mtstack.pop(); }
```

Luego, en **cada** backend (`EPSDisplay`, `PDFDisplay`, `SVGDisplay`), **borrar**:
- Los seis miembros duplicados (`mg_context`, `relfontsize`, `mt`, `mtst`,
  `mtstack`, `dsstack`) de su header.
- Las declaraciones y definiciones de `setMGContext`, `setRelFontSize`,
  `pushMatrix(Matrix&)`, `pushMatrix(PredefinedMatrix)`, `saveMatrix`,
  `popMatrix()`, `popMatrix(PredefinedMatrix)` (headers y `.cpp`).

Ubicaciones exactas hoy: `EPSDisplay.cpp:675-711`, `PDFDisplay.cpp:513-540`,
`SVGDisplay.cpp:206-244` (los cuerpos borrados deben coincidir palabra por
palabra con los de arriba; si alguno difiere, PÁRATE y revisa por qué antes de
borrar).

**Nota sobre `structure(string)`:** los tres cuerpos son equivalentes salvo que
`EPSDisplay` tiene un branch extra `if (strct->isDefinedInDevice())`. Ese branch
es código muerto: `isDefinedInDevice()` solo se vuelve `true` dentro de
`Structure::define_in_device()` (`src/structure.cpp:7`), que **no se llama desde
ningún lado**. Por tanto siempre se toma la rama normal (dibujo recursivo). Puedes
mover `structure()` a `Display` como método concreto usando el cuerpo de
`PDFDisplay`/`SVGDisplay` (sin el branch) y borrar los tres overrides — el
comportamiento no cambia. (Deja `define_in_device` y `isDefinedInDevice` donde
están; limpiarlos es aparte.)

**Ojo — `structure()` no puede ser inline en `Display.h`:** llama
`mg_context->getStructure(name)` y `Display.h` solo tiene la *forward
declaration* de `MetaGrafica` (el include completo crearía un ciclo con `mg.h`).
Hay que crear `src/Display.cpp` con ese único método, añadirlo a `SRCS` en el
`Makefile` (y su línea de dependencias). Los demás métodos sí pueden ser inline.

**Ojo — constructores:** `EPSDisplay` y `SVGDisplay` asignan `relfontsize = 1.0`
en su constructor; al borrar el miembro hay que borrar también esas asignaciones
(el de PDF usa inicializador en línea y se va con el miembro). En `Display`, el
miembro se declara con inicializador: `float relfontsize = 1.0f;`.

### Paso 1B — Unificar la rama MTST de las transformaciones (requiere criterio)

`translate/scale/rotate/shear/compose` tienen DOS ramas: `MTLC` (específica del
backend — emite al archivo/API) y `MTST` (idéntica en los tres — solo muta
`mtst`). Solo la rama MTST se comparte; **NO muevas la rama MTLC.**

Patrón por cada método (ejemplo con `translate`): en `Display`, hacerlo concreto,
manejar MTST ahí, y delegar MTLC a un nuevo hook virtual puro:

```cpp
// en Display (concreto):
void translate(float x, float y, PredefinedMatrix pdmt=MTLC) {
  if (pdmt == MTST) { mtst.translate(x, y); return; }
  deviceTranslate(x, y);            // hook
}
protected:
  virtual void deviceTranslate(float x, float y) = 0;   // solo la parte MTLC
```

Cada backend renombra el cuerpo de su rama MTLC actual a `deviceTranslate` (y
análogos `deviceScale`, `deviceRotate`, `deviceShear`; `compose` no tiene efecto
MTLC hoy, así que su versión base solo hace la rama MTST y nada más). Son **seis**
métodos, no cinco: `init_matrix` también tiene rama MTST idéntica
(`mtst.initialize()`) y rama MTLC específica — solo EPS emite algo
(`defaultmatrix`, ver #17), así que el hook `deviceInitMatrix()` lleva cuerpo
default vacío y solo EPS lo sobreescribe. Para preservar la semántica exacta,
la base debe replicar el patrón `if (MTLC) hook; else if (MTST) mtst.op;` — otros
valores de `PredefinedMatrix` no hacen nada, igual que hoy. Si esto se siente
arriesgado, **es válido dejar 1B sin hacer** y quedarte solo con 1A: ya elimina
la mayor parte de la duplicación.

### Paso 2 — Verificar (obligatorio)

```bash
make            # debe compilar limpio, sin warnings nuevos
for f in examples/*.mg; do
  b=$(basename "$f" .mg)
  ./bin/mg "$f" "/tmp/mg_new_$b.eps" 2>/dev/null
  diff -q "/tmp/mg_ref/$b.eps" "/tmp/mg_new_$b.eps"   # sin salida = idéntico
  ./bin/mg "$f" "/tmp/mg_new_$b.svg" 2>/dev/null
  diff -q "/tmp/mg_ref/$b.svg" "/tmp/mg_new_$b.svg"
done
```

Cualquier `diff` con salida = introdujiste un cambio de comportamiento: revísalo,
no lo aceptes. (Los `.pdf` pueden variar por timestamps internos; si difieren,
compara visualmente en vez de por bytes.)

**Criterio de éxito:** compila sin warnings; los tres backends implementan ~15
métodos menos cada uno; `diff` limpio en todos los `.eps` y `.svg`.

---

**Etapa 2:** diferida. Emprenderla solo si se prevé un cuarto backend o se quiere
testeabilidad (mock/null backend). Ya no aplica el "no urgente hasta que haya un
tercer backend": `SVGDisplay` ya existe y confirma el diagnóstico con evidencia
concreta.

---

### #9 — `point` no es const-correcto y tiene copy-ctor manual innecesario — ✓ RESUELTO (2026-07-04)
**Archivo:** `include/primitives.h:80-140`

> **Resuelto.** Copy-ctor a `= default`; `distancesq`, `distance`, `length`,
> `operator*`, `operator==`, `operator!=` marcados `const`.

**a)** El copy-ctor manual `point(const point&p)` es idéntico al que generaría el
compilador, pero al declararlo hace `point` no-trivial y suprime el move. Borrarlo
(o `= default`) restaura trivialidad y ayuda con el ítem anterior.

**b)** `operator==`, `operator!=`, `length()`, `distance()`, `distancesq()`,
`operator*` no son `const` → no se pueden usar sobre `const point`. Marcarlos
`const`.

```cpp
// En vez de:
point (const point&p) { x = p.x; y = p.y; }
// Usar:
point(const point&) = default;

// Y marcar como const:
float length() const { return sqrt(x*x + y*y); }
float distance(point a) const { ... }
float distancesq(point a) const { ... }
bool operator==(const point& p2) const { ... }
bool operator!=(const point& p2) const { ... }
point operator*(const double& f) const { ... }
```

---

### #10 — Copias innecesarias de `Path` — ✓ RESUELTO (2026-07-04)
**Archivo:** `include/primitives.h:175-176`

> **Resuelto.** `setPath(Path p) { path = std::move(p); }` y
> `const Path& getPath() const { return path; }`. (No había ningún llamador de
> `getPath`, así que el cambio de firma es seguro.)

```cpp
void setPath(Path p) { path = p; }   // copia el argumento
Path getPath() { return path; }      // devuelve copia del vector
```

**Corrección:**

```cpp
void setPath(Path p) { path = std::move(p); }
const Path& getPath() const { return path; }
```

---

### #16 — `SVGDisplay::translate` (MTLC) no escala por `dvx/dvy` — ✓ RESUELTO
**Archivo:** `src/SVGDisplay.cpp` (`deviceTranslate`)

> **Resuelto.** `SVGDisplay::deviceTranslate` ya emite
> `<g transform="translate(x*dvx, y*dvy)">`, igual que EPS/PDF.

EPS emite `x*dvx, y*dvy` (puntos) y PDF hace lo mismo vía `HPDF_Page_Concat`,
pero SVG emitía `<g transform="translate(x, y)">` con los valores crudos en
espacio unitario — y el espacio SVG tras el `<g scale(1,-1)>` raíz está en
puntos. Un `TLLC` en SVG trasladaba una fracción de punto en vez de la fracción
del canvas.

---

## Menor (al tocar esa zona)

### #11 — "Signo como bandera" repartido por el código
`fillpattern < 0`, `fillgray < 0`, `fillcolor < 0`, `shift < 0` y `scale < 0`
se usan para indicar "outline" o "ambos lados". Es implícito. Centralizar en
una función con nombre o en la spec V2 darle sintaxis explícita.

### #12 — `int` para anchos de línea y tamaños — ◑ MEDIO RESUELTO (2026-07-04)
El refactor de estilos de línea (`plan_lineas.md`) reemplazó
`DisplayState::line_width` (int) por `line_width_pt` (float): el almacenamiento
interno ya es fraccionario. Queda `Attribute::value` como `int`, pero es un
artefacto de la **gramática V1** (el valor parseado de `LWIDTH n`); V2 lo resuelve
en la fuente con `width=` float (`especificacion_mg.md` §4.10). No hay nada más que
hacer en el motor V1.

### #13 — `Matrix` usa `float`; `deg2rad` es `double` — ✓ RESUELTO (2026-07-04)
Composiciones largas acumulan error; `deg2rad` se trunca de `double` a `float` en
cada rotación. Para "publication quality", `double` en la matriz es casi gratis.

> **Resuelto.** `Matrix` pasó a `double` y el cambio se propagó mecánicamente a
> toda la tubería: `Display.h`, los tres backends (EPS/PDF/SVG), `point` y demás
> primitivas, `Parser`, texto y splines. No queda ningún `float` en el núcleo
> (solo en `lexmg.cpp` generado y en libharu). Verificado compilando limpio y
> regenerando todos los ejemplos.

### #14 — `getType()` devuelve `int` en vez de `GraphicsItemType` — ✓ RESUELTO (2026-07-04)
`include/primitives.h`: ahora `GraphicsItemType getType() const { return type; }`.
(Sin llamadores que romper.)

### #15 — Encabezado incorrecto en `mg.h` — ✓ RESUELTO (2026-07-04)
`include/mg.h` ahora dice `File: mg.h`.

### #17 — `defaultmatrix` a secas es PostScript inválido — ✓ RESUELTO (2026-07-04)
`EPSDisplay::init_matrix` (rama MTLC, disparada por `IDLC`) emite
`defaultmatrix` sin operandos; el operador requiere una matriz en la pila y no
modifica la CTM (`matrix defaultmatrix setmatrix` sería lo correcto). Latente:
ningún ejemplo usa `IDLC` hoy, pero cualquier `.mg` que lo use produce un EPS
que revienta en el intérprete.

> **Resuelto.** `EPSDisplay::deviceInitMatrix` ahora emite `initmatrix` (operador
> sin operandos que resetea la CTM a la default del dispositivo), que es lo que
> semánticamente se buscaba. Verificado con un `.mg` de prueba que dispara `IDLC`:
> el EPS resultante pasa por Ghostscript sin errores, mientras que la versión con
> `defaultmatrix` produce `stackunderflow in --defaultmatrix--`.
