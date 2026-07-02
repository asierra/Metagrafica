# Correcciones pendientes — auditoría de backend

Estado tras la sesión del 2026-06-30. Los ítems críticos (#1–#4, #6) están resueltos.

---

## Importante (diseño / fidelidad a la filosofía)

### #7 — Invariante de destrucción implícita entre `structure_map` y `StructureUser`
**Archivo:** `src/structure.cpp:20-24`, `include/mg.h:160`

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

> **TL;DR / prioridad.** Hacer **Etapa 1** (subir a `Display` el estado y los
> métodos de matrices que hoy están duplicados byte a byte en los tres backends).
> Es bajo riesgo: no cambia firmas públicas ni la salida generada. **Etapa 2**
> (extraer un tipo `DeviceBackend` puro) queda **diferida** hasta que haya un
> cuarto backend o se quiera testeabilidad. La Etapa 1 es prerrequisito natural
> de la 2, así que no es trabajo desechable.
>
> **La receta paso a paso para la Etapa 1 está al final de esta sección
> ("Etapa 1 — receta ejecutable"); sigue esos pasos en orden.**

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
- Eliminar la asimetría `x*dvx` vs `mt`: aplicar `dvx/dvy` en un solo lugar de la
  máquina de estado, no a mano en cada rama `MTLC`.
- `structure(string)` pasa a concreto en la base. La optimización de EPS
  (`isDefinedInDevice()`) es hoy **código muerto** (`define_in_device()` no se
  llama desde ningún backend): borrarla por ahora, y reintroducirla en la Etapa 2
  como capacidad del backend, no como `if` en `structure()`.

Tras la Etapa 1, cada backend deja de implementar ~15 métodos de contabilidad;
solo le quedan las primitivas reales (B).

*Etapa 2 (mayor alcance) — extraer un `DeviceBackend` puro:*

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
MTLC hoy, así que su versión base solo hace la rama MTST y nada más). Repite para
los cinco. Si esto se siente arriesgado, **es válido dejar 1B sin hacer** y quedarte
solo con 1A: ya elimina la mayor parte de la duplicación.

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

### #9 — `point` no es const-correcto y tiene copy-ctor manual innecesario
**Archivo:** `include/primitives.h:80-140`

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

### #10 — Copias innecesarias de `Path`
**Archivo:** `include/primitives.h:175-176`

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

## Menor (al tocar esa zona)

### #11 — "Signo como bandera" repartido por el código
`fillpattern < 0`, `fillgray < 0`, `fillcolor < 0`, `shift < 0` y `scale < 0`
se usan para indicar "outline" o "ambos lados". Es implícito. Centralizar en
una función con nombre o en la spec V2 darle sintaxis explícita.

### #12 — `int` para anchos de línea y tamaños
`Attribute::value` e `DisplayState::line_width` son `int`. No hay anchos de línea
fraccionarios — limitación para gráficos de publicación finos. Considerar `float`.

### #13 — `Matrix` usa `float`; `deg2rad` es `double`
Composiciones largas acumulan error; `deg2rad` se trunca de `double` a `float` en
cada rotación. Para "publication quality", `double` en la matriz es casi gratis.

### #14 — `getType()` devuelve `int` en vez de `GraphicsItemType`
`include/primitives.h:160`. Pierde el chequeo del compilador. Cambiar a:
```cpp
GraphicsItemType getType() const { return type; }
```

### #15 — Encabezado incorrecto en `mg.h`
`include/mg.h:4` dice `File: structure.h`. Cambiar a `File: mg.h`.
