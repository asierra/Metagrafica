# plan_verbose.md — Mensajes de depuración (`--verbose` / `--debug`)

**Estado:** BORRADOR (abierto el 2026-11-25). Nada implementado. Las decisiones abiertas están
al final y son de Alejandro.

**Origen:** el diario lo pide **tres veces** —2024-02-07 «Falta logging», 2024-05-09 «hay que
poner y borrar muchos printf en cada caso», 2024-08-01 «una versión práctica de mensajes de
debug»— y estaba anotado en `PENDIENTES.md` («Del diario»). Ninguna figura lo pide; lo pide
la forma de trabajar.

---

## 1. Lo que hay hoy (medido el 2026-11-25)

- **Interfaz:** `mg` acepta solo `-h` y `-v` (versión), y únicamente como `argv[1]`
  (`src/main.cpp`). No hay análisis de opciones: un flag más el archivo no cabe sin
  reescribir esa parte. ⚠️ **`-v` ya es la versión**, así que no puede ser «verbose».
- **Salida normal:** `stdout` lleva una sola línea, `render -> archivo`; `stderr`, los avisos
  (`Aviso: …`) y los errores (`Error de sintaxis en L:C`, `Error de evaluación:`).
- **Rastros de depuración:** **17 `printf` comentados** en el árbol, y casi todos están en el
  parser del marcado de texto: `text_parser.cpp` (10: pila de estado, cadenas, símbolos,
  llaves), `text.cpp` (1), `EPSDisplay.cpp` (3: cara, tamaño, color), `structure.cpp` (1:
  `gap` de `place`) y `primitives.cpp` (2: save/restore). Son exactamente los «poner y borrar
  printf» del diario: se escriben para un bug, se comentan al cerrarlo y se pudren, porque
  nada los compila.
- **Información que el compilador ya calcula y no muestra:**
  - **La caja de tinta** (`Display::warnIfOffCanvas`): se imprime solo cuando el dibujo
    entero cae fuera del lienzo. El desborde PARCIAL se calla a propósito, porque como aviso
    daría falsos positivos, pero como información pedida es justo lo que se quiere ver.
    `world_window` es el tropiezo nº 1 de la referencia (§2 y §15).
  - **El mapeo del lienzo:** escala uniforme (*meet*), margen de centrado y `stretch`, en
    `Display::pushWorldMatrix()`.
  - **La resolución de `include`:** se prueban hasta tres rutas (local → `MG_LIBDIR` → junto
    al `.exe` en Windows) y no se dice cuál ganó. La local le gana a la de la biblioteca a
    propósito, y por eso mismo una copia vieja junto a la figura puede ganar sin que nadie
    lo note.
- **En el lenguaje no hay `print`:** no hay forma de ver el valor de una variable salvo
  dibujarla con `text(str(x, 3))`.

## 2. Dos públicos, que no piden lo mismo

| | Quien MANTIENE `mg` | Quien ESCRIBE una figura |
|---|---|---|
| Pregunta | ¿qué hizo el parser de marcado con esta cadena?, ¿qué cara quedó puesta? | ¿dónde cayó mi dibujo?, ¿qué archivo incluyó?, ¿cuánto vale `E`? |
| Lo que hoy usa | descomentar un `printf`, recompilar, volver a comentarlo | dibujar valores con `text`, adivinar con la ventana |
| Vocabulario | interno (`dspstate`, `dev_face`, *runs*) | el del `.mg` y la referencia (cm, `world_window`, nombres de struct) |

Mezclarlos en una sola salida haría que la del usuario se llenara de ruido interno. La
propuesta los separa en **categorías** dentro de un mismo mecanismo.

## 3. Propuesta

### 3.1 Interfaz

```
mg --verbose figura.mg salida.svg          # lo que le sirve al AUTOR de la figura
mg --debug=text,font figura.mg salida.svg  # categorías internas, para quien mantiene
```

- `--verbose` = las categorías de autor (§3.2). `--debug=` elige categorías sueltas, de
  cualquiera de los dos grupos; `--debug=all` las enciende todas.
- Los flags van **antes** del archivo, como `-h` y `-v`. Hace falta un análisis de opciones
  mínimo en `main.cpp`, hecho a mano (sin `getopt`, para no tener una dependencia más en el
  `.exe` de MinGW).

### 3.2 Categorías

**Para el autor (`--verbose`):**

| categoría | qué dice |
|---|---|
| `canvas` | `display_size`, `world_window`, escala (cm por unidad de mundo), margen de centrado, `stretch` |
| `ink` | la caja de tinta en cm **y en unidades de mundo**, siempre, y por qué lado se sale si se sale |
| `include` | cada `include`: qué rutas probó y cuál ganó |
| `struct` | por struct: cuántas veces se expandió y la profundidad máxima que alcanzó frente a `max_depth` |

**Para quien mantiene (solo con `--debug=`):**

| categoría | qué dice | de dónde sale |
|---|---|---|
| `text` | la segmentación del marcado: trozos, caras, runs `$…$` | los 11 `printf` de `text_parser.cpp`/`text.cpp` |
| `font` | cada cambio de cara y tamaño en el dispositivo (`dev_face`/`dev_size`) | los de `EPSDisplay.cpp`, extendido a PDF |
| `place` | el locus y el `gap` de cada `place` | el de `structure.cpp` |

Los `printf` comentados **se convierten** en llamadas a la categoría que les toca y dejan de
estar comentados: así los compila cada `make` y no se pudren.

### 3.3 Invariantes (no negociables)

1. **El modo verbose no cambia un byte de la salida.** Todo va a `stderr`; el EPS, el SVG y
   el PDF salen idénticos con y sin `--verbose`. ⚠️ Es la regresión natural de esta
   característica (un mensaje que se cuela al flujo de salida, o un cálculo de diagnóstico
   con efectos), y **se vigila con una compuerta**: compilar el corpus con `--debug=all` y
   comparar contra el golden. Es barata: reusa los goldens que ya existen.
2. **Sin el flag, nada cambia**: ni la salida ni `stderr`. Los avisos siguen saliendo
   siempre; la depuración nunca.
3. **Mensajes en español**, con el vocabulario del público al que van, y con un prefijo por
   categoría (`[canvas] …`) para que se puedan filtrar con `grep`.
4. **Formato con `printf`**, como el resto de los backends (Code style), detrás de una
   función `dbg(categoría, fmt, …)` que no evalúa nada si la categoría está apagada.

## 4. Pasos

1. Análisis de opciones en `main.cpp` (`--verbose`, `--debug=`, `-h`, `-v`) y la función
   `dbg`. Actualizar `man/mg.1.md` y la ayuda de `-h`, que hoy solo remite al man.
2. La compuerta de §3.3.1, **antes** de la primera categoría: así ningún mensaje entra sin
   estar vigilado.
3. Categorías de autor: `canvas` e `ink` primero (son el tropiezo nº 1), luego `include` y
   `struct`.
4. Convertir los 17 `printf` comentados en `text`, `font` y `place`.
5. Documentar `--verbose` en la referencia (§14, «Cómo falla el compilador») y en §15
   («No se ve nada…»: «compila con `--verbose` y mira `[ink]`»), y re-sellar la traducción.

## 5. Decisiones abiertas (Alejandro)

1. **Nombres de los flags.** ¿`--verbose` y `--debug=`, o uno solo (`-d` / `--debug`) con
   categorías? Recomendación: los dos, porque separan a los dos públicos y `--verbose`
   es lo que un usuario prueba primero.
2. **¿Entra una sentencia `print` al lenguaje?** Resolvería «¿cuánto vale `E`?» sin dibujar,
   pero es **gramática nueva**, y la condición 1 del 1.0 es congelarla. Recomendación: no
   en este plan; anotarla como candidata aparte y decidirla junto con el congelamiento.
3. **¿Se quita la línea `render -> archivo` de `stdout`?** Es la única salida no pedida en
   una corrida normal, y estorba al usar `mg` en un script. Recomendación: moverla a
   `--verbose`. Medido: el único que la lee es `tools/ver.sh`, y es para FILTRARLA como «ruido de
   progreso»; ni `test/` ni `release.yml` dependen de ella. Aun así cambia algo que el
   usuario ve en cada corrida.
4. **¿`ink` en unidades de mundo además de cm?** El usuario piensa en las unidades de su
   `world_window`, y el aviso actual habla en cm. Recomendación: las dos.
5. **¿Pruebas del CONTENIDO de los mensajes?** La compuerta de §3.3.1 solo garantiza que no
   cambian la salida, no que digan algo útil. Un `% EXPECT_VERBOSE:` en `test/errors/`
   vigilaría unas cuantas líneas clave, pero ata el texto de cada mensaje. Recomendación:
   solo para `ink` e `include`, que son las que se citarían en la documentación.
