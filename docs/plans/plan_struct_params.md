# plan_struct_params.md — Structs parametrizadas por path + `fit` de una invocación

Estado: **PLANEADO, sin implementar.** Redactado 2026-07-18 al cerrar la sesión de
fig16-9; la discusión de diseño está cerrada y las decisiones tomadas (abajo).
Baseline verificado en ese punto: commit `5ee2d85` + los cambios sueltos de
Alejandro (C++17, `getXBoundsAtY`), `make` limpio y `bash test/run.sh check` →
**ok=51 fail=0 error=0 psfail=0 c3fail=0**.

**Revisión 2026-07-18 (2ª pasada, contra el código):** la primera redacción daba por
funcionando algo que no funciona (apareció el **Hueco 3**, abajo) y dejaba dos
decisiones que se contradicen entre sí (2 y 3 de la lista original, ahora fundidas
en la decisión 2). Todo lo de esta revisión está **verificado ejecutando**, no leído.

## Qué lo compra

`examples/fig16-9.mg` tiene **7 structs `Nivel0..Nivel6` casi idénticas** (56
líneas): solo cambian el path de la onda (`&pw0`..`&pw6`) y el ancho `w` de la
ventana local. Deben colapsar a **una sola de 6 líneas**:

```text
struct Nivel(&onda, w = path_width(&onda)) {
    world_window 0 w -2 2
    line_width 0.1   color "lightgray"   polyline { 0 0  w 0 }
    fill "lightgray"   outlinefill   color "black"   line_width 0.4
    bezier(&onda)
}

fit(Nivel(&pw0), stretch=true) { 1.84615 0.44169  3.46615 1.48169 }
```

El `w` explícito sigue siendo válido (pisa el default) pero ninguna llamada de
fig16-9 lo necesita: el ancho de cada onda **es** su extensión en x (decisión 6).

## Diagnóstico (verificado en código, no supuesto)

**Las structs con parámetros YA EXISTEN** — no es lo que falta. `StructDef` tiene
`params`/`defaults` (`parserv3.cpp:589`), `InvokeStmt` liga posicionales y
nombrados, y `lib/pseudo3d.mg` las usa en producción:
`struct plano(w, h, k=0.4, relleno=gray(0.8), ...)`.

Faltan exactamente **tres** cosas.

### Hueco 1 — `fit` no acepta una struct con argumentos
`parseFit` (`parserv3.cpp:2978`) toma un **nombre pelado**
(`st->structName = lx.next().str`) y después solo admite `stretch=`. Por eso
`fit(Caja(2), stretch=true) { ... }` da **error de sintaxis**. Las structs de
pseudo3d se libran porque se colocan con `at=` en la propia invocación, no con
`fit`; fig16-9 necesita la semántica de `fit` (mapear la ventana local al
rectángulo con stretch).

### Hueco 2 — un path no puede ser argumento
`Value` es `NUMBER | STRING | LIST` (`include/ast.h:18`). Los `PathExpr` viven
**deliberadamente aparte** de esa jerarquía (comentario en `parserv3.cpp:633`).

### Hueco 3 — un parámetro NO se puede usar en el `world_window` del cuerpo
Apareció en la 2ª pasada; la 1ª redacción afirmaba lo contrario. Es **el hueco que
más importa**, porque el `w` de `struct Nivel(&onda, w)` va justo ahí. Verificado:

```text
struct Caja(w) { world_window 0 w 0 1  polyline { 0 0  w 0  w 1  0 1  0 0 } }
fit(Caja, stretch=true) { 1 1  5 3 }
→ Error de evaluación: variable no definida: w      (exit 1)
```

`structBox` (`parserv3.cpp:602`) evalúa `localWindow` con el ámbito que le pasan, y
los **tres** llamadores le pasan el del LLAMADOR, no el local con los parámetros
ligados:

- `parserv3.cpp:774` — `InvokeStmt`: construye `local` en 741-749 y aun así pasa `caller`.
- `parserv3.cpp:994` — `FitStmt`: peor, llama a `structBox` **antes** de construir `local`.
- `parserv3.cpp:1018` — `buildStructure`.

Hoy funciona solo con literales o globales (el ámbito local es hijo del global, así
que las constantes de archivo resuelven igual por la cadena de ámbitos). Un
parámetro, no.

## Decisiones de diseño (cerradas en la discusión del 2026-07-18)

1. **Parámetro de tipo path marcado con el sigilo `&`**, no `PATH` dentro de
   `Value`. Se declara `struct Nivel(&onda, w)` y se llama `Nivel(&pw3, 6)`.
   Razones: meter paths en `Value` se derrama a `str()`, `+`, listas y al
   estampado de `text()` sin que ninguna figura lo pida; el `&` documenta el tipo
   en los dos extremos y le da al parser una señal inequívoca (`startsPathExpr`
   ya la reconoce). **Descartado** pasar el nombre como cadena con deref dinámico
   (sin verificación, contra el estilo del lenguaje).
2. **Se liga el par `{PathExpr*, Scope* del llamador}`**, no un puntero pelado.
   *(Funde las decisiones 2 y 3 de la 1ª redacción, que se contradecían: "ligar un
   puntero, evaluación diferida" + "resolver en el ámbito del llamador" no pueden
   cumplirse a la vez, porque `PathRef::evalPath(s)` reenvía **el ámbito vigente**
   —`parserv3.cpp:660`— que dentro del cuerpo es el de la struct, hijo del global.
   Con el puntero pelado, `Nivel(sine(amplitude=a), 3)` con `a` local del llamador
   resolvería `a` en el callee: hoy eso es `evalError` fatal, pero la semántica
   resultante sería "ámbito del callee", no la decidida.)*

   ```cpp
   struct PathBinding { const PathExpr *expr; Scope *scope; };   // scope = el del LLAMADOR
   ```

   `PathRef::evalPath` evalúa `b.expr->evalPath(*b.scope)`. Conserva la laziness (los
   `PathExpr` siguen siendo dueños de su árbol, en `g_paths` o en el AST del `fit`) y
   hace **verdadera** la resolución en el ámbito del llamador. Tiempo de vida
   garantizado: el marco de `exec` del llamador vive durante la ejecución del cuerpo.

   **Descartada** la alternativa —evaluar el `Path` al ligar y guardar el valor—:
   también cumple la semántica, pero obliga a meter `Path`/`point` en `ast.h`, que es
   donde vive `Scope`. Con el par bastan declaraciones adelantadas
   (`struct PathExpr;`). *Esta decisión fija el layering del header.*
3. **Un parámetro path ensombrece** un path global del mismo nombre dentro del
   cuerpo. Es lo deseable, pero que sea deliberado.
4. **Las listas de parámetros y de argumentos son PARALELAS EN ÍNDICE**, no listas
   aparte. Una lista `pathParams` separada perdería el orden posicional
   (`struct Nivel(&onda, w)` dejaría de saber que `&onda` es el posicional 0) y
   entonces `Nivel(&pw0, 3)` no se podría ligar. Del lado de la llamada, el
   `std::vector<ExprPtr> pos` que llena `parseArgList` no admite un path intercalado:

   ```cpp
   std::vector<bool> paramIsPath;              // declaración; misma longitud que params
   struct Arg { ExprPtr e; PathExprPtr p; };   // llamada; uno de los dos, nunca ambos
   ```

   Con esto el chequeo de tipos (paso 6) es un `if` por índice, casi gratis.
5. **`at=`/`rotate=`/`scale=` dentro de `fit(Nivel(...), ...)` son ERROR**, no se
   ignoran. `InvokeStmt` los reserva como modificadores de colocación
   (`parserv3.cpp:756`), pero en un `fit` competirían con la matriz del propio `fit`.
   Silenciarlos sería la clase de trampa muda que el proyecto persigue.
6. **`w` sale del path por default: `w = path_width(&onda)`.** *(Revisado en la 2ª
   pasada; la 1ª difería el accesor y dejaba `w` explícito.)* Pasar `&pw3` con un `w`
   equivocado **achata la onda en silencio**, y `w` es redundante: es la extensión en
   x del propio path (cada pieza mide 1 unidad, cada plana 0.5).

   Lo que abarató la decisión: **los defaults ya pueden referirse a parámetros
   anteriores** — feature existente, no documentada, verificada ejecutando
   (`struct Caja(a, b=a*2)` + `Caja(2)` → 2×4 unidades). `InvokeStmt` liga en orden de
   declaración y evalúa cada default en `local` (`parserv3.cpp:747`). Así que sobre el
   accesor, el default **no cuesta nada**: la maquinaria ya está.

   Ventaja sobre derivar la ventana *implícitamente* del bbox (descartado por mágico):
   el llamador escribe `fit(Nivel(&pw0), stretch=true)` y el ancho sale solo, pero si
   una figura futura quiere una ventana distinta del bbox (una onda con margen) pasa
   `w` explícito y lo pisa. También descartado **validar** (errorar si el `w` dado
   discrepa del bbox): mal definido en cuanto una struct tenga más de un param path.

   **`path_width` es más barato de lo que parecía**: no toca `CallExpr` ni `ast.h`.
   `PathExpr` vive en `parserv3.cpp` y `Expr` es base virtual pura → un
   `struct PathWidthExpr : Expr { PathExprPtr arg; }` se define ahí mismo; solo hay que
   enrutar el nombre en la posición de expresión primaria. El bbox ya está escrito
   inline en `FitStmt::exec:977-982` (rama `fit(&path)`) → se factoriza a `path_bbox()`
   y sirve a los dos.

   **La objeción de la decisión 1 no aplica**: rechazamos `PATH` dentro de `Value`
   porque se derrama hacia afuera (`str()`, `+`, listas, estampado de `text()`).
   `path_width` es una **reducción** path→número; lo que sale es un `double` corriente.
   La fuga es direccional y ésta va al revés.

   ⚠️ **Hereda la advertencia de `getXBoundsAtY`** (ver apartado final): opera sobre los
   PUNTOS del path, que en un path bezier son los **controles**, no la curva. Para las
   ondas de fig16-9 es **exacto** —las piezas son monótonas en x (`plana` 0→.1667→
   .3333→.5; los medios ciclos 0→1), así que los extremos en x son el punto inicial y
   el final, que sí están sobre la curva—; conviene comprobarlo al implementar.
7. **Nomenclatura de la familia: reservar ahora, construir solo `path_width`.**
   `path_width` y `getXBoundsAtY` son **la misma operación** (reducir un path a
   escalares recorriendo sus puntos), así que el accesor no es un one-off: la familia
   ya tiene un segundo miembro escrito en C++ y con uso nombrado en la misma figura
   (ver apartado final). Misma técnica que §13.0 ("un nombre por parte, reservados
   aunque la parte no exista"), y por el mismo motivo: si `path_width` aterriza como
   nombre suelto y la familia llega después, o queda incoherente o hay renombre — y el
   de §13 costó dos fases para no colisionar.

   ```text
   path_width(&p)              % CONSTRUIDO ahora  — extensión en x
   path_height(&p)             % reservado
   path_x_bounds(&p, at_y=E)   % reservado — el getXBoundsAtY expuesto
   ```

   **Nomenclatura PROVISIONAL**: se refina cuando todo funcione (acordado con Alejandro
   el 2026-07-18). Lo que se fija ahora es que hay familia, no los nombres finales.

   `path_width` debe ser **escalar**, no azúcar sobre los bounds: el default de la
   decisión 6 necesita un número, y `IndexExpr` indexa una **variable nombrada**, no una
   expresión (`ast.h:169`) → `path_x_bounds(&m, at_y=E)[0]` no parsearía ahí. Cuando se
   construya el tercero, el uso natural será `b = path_x_bounds(&m, at_y=E)` y luego
   `b[0]`/`b[1]` (que además evalúa la intersección una sola vez). Pendiente para
   entonces: el `[[nodiscard]] bool` no traduce a MG — hay que decidir si "no hay
   intersección" es `evalError` fatal o lista vacía.
8. **Solo `fit` recibe la invocación ahora.** `place` (`parserv3.cpp:3003`) y
   `repeat` tienen el mismo hueco 1 y por consistencia deberían aceptarla igual,
   pero **ninguna figura lo pide**: se deja la sintaxis *diseñada* en la spec §10
   y **sin construir**, regla de siempre (cada feature la compra una figura).

## Pasos de implementación

0. **Hueco 3 primero, y SOLO (commit aparte).** `structBox` pasa a recibir el ámbito
   LOCAL con los parámetros ligados: reordenar `FitStmt::exec` (ligar params →
   *después* `structBox(local, def)`) y cambiar `caller`→`local` en `InvokeStmt`
   (`:774`) y `buildStructure` (`:1018`). Va solo porque **es el único paso que
   puede mover bytes** (ver Verificación); si el golden se mueve aquí, se ve limpio.
1. **`StructDef` gana parámetros de path.** `std::vector<bool> paramIsPath`
   **paralela en índice** a `params` (decisión 4), llenada en `parseStructDef`
   cuando el parámetro viene con `&`. El lexer ya entrega el token: `startsPathExpr`
   reconoce `T_AMP` (`parserv3.cpp:2505`), solo hay que consumirlo.
2. **`Scope` gana ligaduras de path.** Un `std::map<std::string, PathBinding>`
   (decisión 2) consultado por **`PathRef::evalPath` ANTES de `g_paths`**
   (`parserv3.cpp:660`, **el único sitio de lookup** — verificado: `g_paths` solo se
   consulta ahí). En `ast.h` basta `struct PathExpr;` adelantada. Ojo con el
   alcance: `Scope local(caller.root())` es hijo del global, así que la búsqueda
   debe subir por la cadena de ámbitos como las variables (`Scope::find`).
3. **`InvokeStmt` liga los argumentos path** al construir el ámbito local, guardando
   `{expr, &caller}` en cada ligadura.
4. **`FitStmt`/`parseFit` aceptan una invocación**: si tras el identificador viene
   `(` (desambiguación por `peek(1) == T_LPAREN`; **no** toca la rama
   `startsPathExpr`, que dispara con `T_AMP` o los nombres de operación), parsear la
   lista de argumentos mixta (`Arg` de la decisión 4: `parseExpression` o, si
   `startsPathExpr`, `parsePathExpr`) y guardarla en el `FitStmt`; en `exec`,
   ligarlos al ámbito de la struct igual que hace `InvokeStmt`. Cuidado de no romper
   la rama `fit(&path) { rect }` (§9) ni el `stretch=`. Los `at=`/`rotate=`/`scale=`
   de la invocación interior son error (decisión 5).
5. **`path_width(&p)`** (decisiones 6 y 7): factorizar el bbox de
   `FitStmt::exec:977-982` a un `path_bbox()`, y un `struct PathWidthExpr : Expr` con
   un `PathExprPtr` **en `parserv3.cpp`** (no en `ast.h`, no toca `CallExpr`). Va
   DESPUÉS del paso 2: el default `w = path_width(&onda)` se evalúa en `local` y
   necesita las ligaduras de path ya puestas. Fijar además que **todos los params path
   se ligan antes de evaluar cualquier default** — hoy saldría bien por el orden de
   declaración (`&onda` es el índice 0), pero depender de eso es frágil.

   ⚠️ **El orden del archivo pelea con este paso — NO reordenar `parserv3.cpp`.** El
   punto de enganche es `parseAtom`, `case T_IDENTIFIER`, dentro de la rama
   `if (lx.accept(T_LPAREN))`, ANTES de construir el `CallExpr` (~línea 158). Pero
   `parseAtom` está en la **línea 128** y lo que necesita vive mucho después:
   `struct PathExpr` en la **638** y `parsePathExpr` en la **2896**. Mover cualquiera de
   esos hacia arriba produciría un diff enorme y gratuito. La salida es una función
   puente, declarada arriba y definida abajo:

   ```cpp
   // ~línea 126, junto a las otras adelantadas (parseExpression, parseUnary):
   static ExprPtr parsePathWidthCall(Lexer &);   // path_width(&p): puente Expr↔PathExpr

   // en parseAtom, case T_IDENTIFIER, tras aceptar '(':
   if (name == "path_width") return parsePathWidthCall(lx);   // antes del CallExpr

   // definición DESPUÉS de parsePathExpr (>2896), donde ambos son visibles.
   // struct PathWidthExpr : Expr va después de la 638 (necesita PathExpr completo).
   ```

   Si al implementar aparece la tentación de mover bloques grandes o de meter `PathExpr`
   en `ast.h`, es señal de que se tomó el camino equivocado: **parar y consultar**.
6. **Reescribir `examples/fig16-9.mg`**: 7 structs → 1; los 13 `fit` pasan a
   `fit(NivelN(&pwK), stretch=true) { … }` con los MISMOS rectángulos.
7. **Errores claros**: pasar un path donde se espera número (y al revés);
   aridad incorrecta; `&nombre` no definido.

## Verificación

El refactor es **PURO** (misma geometría, distinta expresión) → la guardia es
exigir los **51 goldens BYTE-IDÉNTICOS**, incluido fig16-9. Es la misma técnica
que validó el renombre de §13 (ver CLAUDE.md, sesión 2026-07-16). Si un solo byte
se mueve, el refactor no era puro y hay que entender por qué **antes** de
re-bendecir. `make` limpio (sin warnings nuevos) y `gs` OK.

**El paso 0 es el único que puede mover bytes** — cambia el ámbito en que se evalúa
una expresión existente, no solo la forma de escribirla. Debería ser puro también, y
está **verificado por qué**: ningún `world_window` del árbol lleva identificadores
(todos son literales), así que no hay nada cuya resolución pueda cambiar. Comprobación
que lo respalda (cero coincidencias = todos literales):

```bash
grep -rn "world_window" examples/*.mg lib/*.mg examples/simulate3d/*.mg \
  | grep -vE "world_window +[-0-9. ]+$"
```

Y aun cuando llevaran uno global, el ámbito local es hijo del global y `Scope::find`
sube la cadena → mismo valor. Por eso va en commit aparte: es la única parte donde
"golden intacto" es evidencia de algo no trivial.

**`path_width` (paso 5) no puede mover bytes**: es una expresión NUEVA, no la
reinterpretación de una existente. Si el default `w = path_width(&onda)` diera un ancho
distinto del `w` que fig16-9 pasa a mano, el golden lo caza de inmediato — o sea que el
propio golden verifica de paso que el accesor es exacto sobre las ondas (decisión 6).

Huella esperada: ~25 líneas (hueco 1) + ~35 (hueco 2) + ~15 (hueco 3) + ~15
(`path_width`), **cero cambios de backend, cero elementos gráficos nuevos** — todo
trabajo de parser, más un campo y una declaración adelantada en `ast.h`.

## Documentación al cerrar

Spec §8 (parámetros de path en structs; **y que el `world_window` del cuerpo puede
usar los parámetros** — hueco 3, hoy ni funciona ni está documentado; **y que un
default puede referirse a un parámetro anterior** — feature existente, verificada, sin
documentar) y §10.2 (`fit` de una invocación; dejar anotada la forma
diseñada-no-construida de `place`/`repeat`). `path_width` y los nombres reservados de
la familia (decisión 7) donde vivan los accesores — **marcados como provisionales**.
Nota de sesión en CLAUDE.md.

---

## Aparte: `getXBoundsAtY` (cambio suelto de Alejandro, 2026-07-18)

Nueva en `src/splines.cpp`: dado un path y un `y_level`, devuelve por referencia
el `xmin`/`xmax` de las intersecciones con esa horizontal (`[[nodiscard] ]bool`,
`std::minmax_element`, structured bindings → motivó el salto a **C++17** en el
`Makefile`). Compila limpio y el corpus sigue verde.

**Para qué es** (Alejandro, 2026-07-18): en fig16-9 las esquinas de los 13 `fit` están
calculadas **a mano**. Deberían derivarse del **ancho de la intersección con la curva de
Morse a la energía del nivel**, más un shift — que es lo que físicamente determina dónde
empieza y acaba cada función de onda (los puntos de retorno clásicos). Eso es un **paso
POSTERIOR** a este plan: aquí los rectángulos se conservan idénticos (es la guardia de
pureza). Pero explica por qué la función existe y por qué la familia de accesores de la
decisión 7 es real y no especulativa.

Tres apuntes para cuando se use:

- **Falta su declaración en `include/splines.h`** (hoy no la tiene): mientras no
  esté, ninguna otra unidad de traducción puede llamarla, y el `[[nodiscard]]`
  de la definición no protege a los llamadores.
- **Es exacta para paths POLILÍNEA** (interpola linealmente entre puntos
  consecutivos). Sobre un path de **puntos de control bezier** —como la curva de
  Morse o las ondas de fig16-9— opera sobre el *polígono de control*, no sobre la
  curva. Para "puntos de retorno clásicos a energía E" sobre un potencial dibujado
  con `bezier`, eso es una aproximación; conviene decidir si el llamador
  teseliza la bezier primero o si la función gana una variante que lo haga.
- ⚠️ **Esa advertencia es LOAD-BEARING para el uso previsto**, no académica, y el caso
  se parte en dos: sobre **las ondas** el error es cero (son monótonas en x → los
  extremos son los puntos inicial y final, que sí están sobre la curva — por eso
  `path_width` de la decisión 6 es exacto ahí); sobre **la curva de Morse** NO, porque
  la horizontal corta en una zona genuinamente curva donde el polígono de control se
  separa de la curva, y el sesgo es mayor en la rama izquierda (curvatura fuerte). O
  sea: justo el caso que motivó escribir la función es el que necesita la decisión de
  teselado. Medir el sesgo antes de decidir.
- **`path_width` (decisión 6) es la misma operación** —reducir un path a escalares
  recorriendo sus puntos— y por eso hereda la advertencia entera. Ver decisión 7 para la
  nomenclatura de familia reservada (`path_x_bounds(&p, at_y=E)` sería ésta expuesta al
  lenguaje) y para lo que falta decidir al exponerla: el `[[nodiscard]] bool` no traduce
  a MG (¿`evalError` fatal, o lista vacía?).
