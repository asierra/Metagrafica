# MetaGráfica — Plan de Evolución

## Visión general

MetaGráfica es un lenguaje gráfico vectorial con 40 años de uso real en publicaciones científicas. El objetivo de esta evolución es ampliar su base de usuarios sin perder su fortaleza principal: la precisión geométrica y la calidad de publicación. Las propuestas están ordenadas por impacto en nuevos usuarios versus esfuerzo de implementación.

---

## Propuesta 1 — Mensajes de error con número de línea

**Prioridad: Alta. Impacto inmediato en usabilidad.**

### Problema actual

El parser no reporta el número de línea donde ocurre un error. Un archivo `.mg` con 200 líneas que falla silenciosamente, o con un mensaje como `Error: A number was expected 258`, obliga al usuario a adivinar dónde está el problema.

### Propuesta

Agregar seguimiento de línea y columna al lexer (`MGLexer`) y propagarlo al `Parser`. Todos los mensajes de error existentes incluirían la ubicación.

**Ejemplo del cambio:**
```
Antes:  Error: Structure no definida <flecha>
Después: archivo.mg:47: Error: estructura 'flecha' no definida
```

### Implementación

1. `MGLexer` ya extiende `yyFlexLexer` — Flex mantiene `yylineno` automáticamente si se activa con `%option yylineno` en `mgpp.l`.
2. Exponer `yylineno` como método público en `MGLexer`.
3. En `Parser`, acceder a `lexer->yylineno` en cada `fprintf(stderr, ...)`.
4. Opcionalmente, agregar el nombre del archivo (ya disponible en `Parser::filename`).

### Esfuerzo estimado
- `mgpp.l`: 1 línea (`%option yylineno`)
- `MGLexer.h`: exponer `getLine()`
- `Parser.cpp`: actualizar ~15 mensajes de error

**Muy bajo riesgo, muy alto impacto.**

---

## Propuesta 2 — Galería de ejemplos con documentación visual

**Prioridad: Alta. Reduce la curva de aprendizaje.**

### Problema actual

La documentación existe en la man page (`mg.1.md`) pero es puramente textual. Un usuario nuevo no puede ver qué produce cada comando sin ejecutarlo. Los ejemplos en `examples/` no están explicados.

### Propuesta

Crear una galería web estática: cada ejemplo muestra el código `.mg` al lado de la imagen generada. No requiere servidor — HTML+SVG estático funciona.

### Estructura propuesta

```
docs/
  index.html          ← galería principal
  examples/
    primitivos.mg     ← código fuente comentado
    primitivos.svg    ← imagen generada
    estructuras.mg
    estructuras.svg
    texto.mg
    texto.svg
    ...
```

### Implementación

**Fase A — Conversión EPS→SVG:**
Los archivos `.eps` actuales se pueden convertir a SVG con `Inkscape --export-type=svg` o `pdf2svg` (vía PDF intermedio). Esto no requiere cambios al compilador.

**Fase B — SVGDisplay (ver Propuesta 4):**
Con un backend SVG nativo, la galería se genera directamente sin conversión.

**Fase C — Ejemplos comentados:**
Reescribir los `.mg` existentes con comentarios que expliquen cada sección, y agregar nuevos ejemplos cubriendo:
- Primitivos básicos (líneas, círculos, rectángulos, curvas Bézier)
- Atributos (grosor, color, relleno, patrones)
- Texto matemático (LaTeX-style: subíndices, superíndices, símbolos griegos)
- Estructuras simples (definición y uso)
- Estructuras sobre líneas y arcos
- Splines

### Esfuerzo estimado
- Fase A: 1-2 días (scripting de conversión + HTML)
- Fase B: depende de Propuesta 4
- Fase C: trabajo continuo, se puede hacer incrementalmente

---

## Propuesta 3 — Loops y variables

**Prioridad: Media-Alta. Elimina el mayor techo del lenguaje.**

### Problema actual

Sin loops ni variables, figuras que requieren elementos repetidos (una malla, una serie de marcadores en un eje, una secuencia de flechas) obligan a calcular y escribir cada coordenada a mano. Esto es el ítem #1 del TODO original.

### Filosofía

MetaGráfica es un minilenguaje nativo y autocontenido. Las variables y loops se implementan dentro del compilador — un archivo `.mg` no depende de nada externo.

### Diseño propuesto

#### Variables numéricas

```
SET r 2.5
SET n 8
CR [r] 0 0        % círculo de radio r en el origen
```

- `SET nombre valor` define una variable float
- `[nombre]` sustituye el valor donde se espera un número
- `[expr]` permite aritmética básica: `[r*2]`, `[n+1]`, `[r/2]`

#### Loops con contador

```
SET paso 0.5
FOR i 0 9
  DOT 0.1 [i*paso] 0
ENDFOR
```

- `FOR var inicio fin` itera `var` de `inicio` a `fin` con paso 1
- `FOR var inicio fin paso` con paso explícito (puede ser float)
- Las variables del loop son locales al bloque
- El loop se unrollea en tiempo de parseo — compatible con la arquitectura actual sin cambios al sistema de draw

#### Expresiones aritméticas

Operadores propuestos: `+`, `-`, `*`, `/`, `^` (potencia), funciones `sin(x)`, `cos(x)`, `sqrt(x)`, `abs(x)`.

```
FOR i 0 359 10
  CR 0.05  [5*cos(i)]  [5*sin(i)]
ENDFOR
```

### Implementación

1. **`mgpp.l`:** agregar tokens `SET`, `FOR`, `ENDFOR`, y reconocer `[expresión]` como un token numérico evaluable
2. **`Parser.h`:** agregar tabla de variables `map<string, float> var_map`
3. **`Parser.cpp`:**
   - `parseFloat()` detecta `[expr]` y evalúa la expresión con sustitución de variables
   - Implementar `parseFOR()` que itera y llama `parsePrimitives()` en cada paso
   - Implementar un evaluador de expresiones simple (shunting-yard o recursivo descendente)
4. **Semántica de scope:** las variables definidas con `SET` son globales al archivo; las variables de `FOR` son locales al bloque y restauran el valor previo al salir

### Consideraciones de diseño abiertas

- **Sintaxis de expresiones:** `[i*0.5]` vs `(i*0.5)` vs `$i*0.5` — los corchetes son consistentes con el estilo existente del lenguaje
- **Loops anidados:** soportar `FOR` dentro de `FOR` desde el inicio
- **Variables de string:** útiles para nombres de estructuras parametrizadas, pero aumentan complejidad; dejar para después

### Esfuerzo estimado
- Variables básicas sin expresiones (`SET`, `[nombre]`): 2-3 días
- Evaluador de expresiones aritméticas: 3-4 días adicionales
- Loops `FOR` completos: 2-3 días adicionales
- **Total: ~1.5 semanas para el sistema completo**

---

## Propuesta 4 — Backend SVG

**Prioridad: Media. Amplía enormemente el alcance del lenguaje.**

### Por qué SVG

- Es el estándar vectorial de la web — cualquier navegador lo renderiza
- Inkscape, Illustrator y Affinity lo importan nativamente
- Se convierte a PDF moderno sin pérdida
- Resuelve el problema de Unicode: SVG soporta texto UTF-8 directamente
- La galería de documentación (Propuesta 2) se beneficia directamente

### Diseño

La arquitectura de `Display` hace que esto sea limpio: `SVGDisplay` es una nueva clase que hereda de `Display` e implementa todos sus métodos virtuales emitiendo XML SVG en lugar de PostScript.

```cpp
class SVGDisplay : public Display {
public:
  SVGDisplay(string filename);
  void start() override;
  void end() override;
  void moveto(float, float) override;
  void lineto(float, float) override;
  // ... etc
private:
  FILE* file;
  string current_path;  // acumula el path SVG actual
};
```

### Diferencias principales EPS vs SVG

| Concepto EPS | Equivalente SVG |
|---|---|
| `moveto/lineto/stroke` | `<path d="M x y L x y"/>` |
| `gsave/grestore` | `<g>...</g>` con atributos |
| `setlinewidth` | `stroke-width` en atributo |
| Definición de estructura | `<defs><g id="nombre">...</g></defs>` |
| Uso de estructura | `<use href="#nombre"/>` con transform |
| Fuente TeX CMMI | Web font o SVG embebido |

### Integración

Agregar opción `-svg` al binario:
```bash
mg archivo.mg          # genera archivo.eps (comportamiento actual)
mg -svg archivo.mg     # genera archivo.svg
```

En `main.cpp`:
```cpp
if (use_svg)
  SVGDisplay g(outname);
else
  EPSDisplay g(outname);
mg->draw(g);
```

### Esfuerzo estimado
- Implementación básica (primitivos, líneas, rectángulos, texto simple): 1-2 semanas
- Soporte completo (patrones de relleno, fuente CMMI, texto matemático): 3-4 semanas adicionales

---

## Propuesta 5 — Modernización de nombres de comandos

**Prioridad: Baja-Media. Impacta la adopción a largo plazo.**

### Problema actual

Los nombres de comandos (`OPST`, `MKST`, `YLNST`, `YDPST`, `PWPT`) vienen de abreviaturas de los años 80 que no son intuitivas para un nuevo usuario. `PL` (polyline), `CR` (circle), `BR` (box/rectangle) son razonables, pero las operaciones de estructuras son crípticas.

### Propuesta

Agregar alias en inglés descriptivos, manteniendo los originales para compatibilidad:

| Comando actual | Alias propuesto |
|---|---|
| `OPST nombre` | `DEFST nombre` o `STRUCT nombre` |
| `MKST nombre` | `USEST nombre` |
| `CLST` | `ENDST` |
| `YLNST` | `LINEST` |
| `YARCST` | `ARCST` (ya existe, mantener) |
| `PWST` | `RECTST` |
| `DT` | `TEXT` |

### Implementación

Solo requiere agregar entradas en `keyword_map` en `MGLexer::init_tables()`. Los alias pueden coexistir indefinidamente con los nombres originales.

### Esfuerzo estimado
- Muy bajo: 1-2 horas en `mgpp.l`

---

## Propuesta 6 — Parámetros a estructuras (OPST)

**Prioridad: Media. Ítem #2 del TODO original.**

### Problema actual

Una estructura es siempre idéntica: no se puede definir una `flecha` y luego invocarla con distintos tamaños o colores. Cada variante requiere una definición separada.

### Propuesta

Permitir parámetros al invocar una estructura:

```
OPST flecha
  PL 0 0  [largo] 0
  ...
CLST

MKST flecha
LNST [escala] [p1x] [p1y] [p2x] [p2y]
```

Donde `[largo]` y `[escala]` serían parámetros sustituidos en tiempo de invocación.

### Nota

Esta propuesta está acoplada a la Propuesta 3 (variables). Implementar variables primero hace que los parámetros sean un caso especial de variables con scope local.

---

## Propuesta 7 — Editor interactivo web (HTML5)

**Prioridad: Futura. Proyecto complementario de gran impacto.**

### Visión

Un editor en el navegador donde el usuario escribe `.mg` en un panel izquierdo y ve el gráfico renderizado en tiempo real en el panel derecho. Sin instalación, accesible desde cualquier dispositivo.

### Dependencia crítica

Esta propuesta requiere la Propuesta 4 (SVG) como prerequisito. Con salida EPS no es viable en el navegador; con SVG el resultado se renderiza directamente sin conversiones.

### Arquitectura posible

**Opción A — Compilador en el servidor:**  
El editor envía el texto `.mg` a un servidor que ejecuta `mg -svg` y devuelve el SVG. Simple, reutiliza el compilador existente tal cual.

**Opción B — Compilador en WebAssembly:**  
Compilar `mg` a WASM con Emscripten. El compilador corre completamente en el navegador, sin servidor. Más complejo pero la experiencia es instantánea y no requiere infraestructura.

### Funcionalidades mínimas del editor

- Editor de texto con resaltado de sintaxis para `.mg`
- Renderizado SVG al lado (en tiempo real o con botón "compilar")
- Mensajes de error con número de línea (requiere Propuesta 1)
- Descarga del SVG o EPS generado
- Galería de ejemplos cargables con un click

### Nota

Es efectivamente un proyecto separado en cuanto a implementación, pero comparte objetivos con la galería de documentación (Propuesta 2) y depende directamente del backend SVG (Propuesta 4). Conviene diseñar esas propuestas teniendo en mente este editor.

---

## Resumen y orden recomendado

| # | Propuesta | Esfuerzo | Impacto en nuevos usuarios | Dependencias |
|---|---|---|---|---|
| 1 | Números de línea en errores | Muy bajo | Alto | — |
| 5 | Alias de comandos | Muy bajo | Medio | — |
| 2A | Galería con conversión EPS→SVG | Bajo | Alto | — |
| 3 | Variables y loops nativos | Medio-Alto | Muy alto | — |
| 4 | Backend SVG | Alto | Muy alto | — |
| 2B | Galería con SVG nativo | Bajo | Alto | Propuesta 4 |
| 6 | Parámetros a estructuras | Medio | Alto | Propuesta 3 |
| 7 | Editor interactivo HTML5 | Muy alto | Muy alto | Propuesta 4 |

### Secuencia sugerida

**Etapa 1 — Victorias rápidas** (días):
Propuesta 1 + Propuesta 5 — mejoran la experiencia inmediatamente con mínimo riesgo.

**Etapa 2 — Documentación** (semanas):
Propuesta 2A — galería con los ejemplos existentes convertidos a SVG externamente.

**Etapa 3 — Evolución del lenguaje** (meses):
Propuesta 3 (variables y loops nativos), luego Propuesta 4 (SVG).

**Etapa 4 — Completar el lenguaje** (futuro):
Propuesta 6 (parámetros a estructuras) una vez que la Propuesta 3 esté implementada.

**Etapa 5 — Editor web** (proyecto paralelo/futuro):
Propuesta 7, una vez que el backend SVG (Propuesta 4) esté estable.
