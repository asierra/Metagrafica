# Arnés de regresión golden-file

Este directorio contiene un arnés de regresión "golden-file" para el compilador
`mg` (MetaGráfica). La idea es sencilla: por cada archivo de `examples/*.mg` se
genera la salida en cada formato soportado (por ahora `eps` y `svg`), se
normaliza para eliminar cualquier parte volátil, y se compara byte a byte
contra una copia de referencia guardada en `test/golden/`.

Si algún cambio en el compilador altera la salida generada para alguno de los
ejemplos, `test/run.sh check` lo va a reportar como `FAIL`.

## Uso

```bash
make                 # asegúrate de tener bin/mg actualizado
test/run.sh capture  # (re)genera los golden files a partir del build actual
test/run.sh check    # compara la salida actual contra los golden files (modo default)
```

`check` es el modo por defecto: `test/run.sh` sin argumentos es equivalente a
`test/run.sh check`.

El script imprime una línea por cada combinación ejemplo × formato
(`ok    <archivo>` o `FAIL  <archivo>`), un resumen al final, y sale con
código 0 si todo pasó o 1 si hubo al menos un `FAIL` o un error al invocar
`mg`.

Cuando `capture` se corre, los golden files nuevos/actualizados quedan en
`test/golden/` listos para revisarse con `git diff` y comitearse si el cambio
de salida es intencional.

## Qué se normaliza y por qué

- **EPS**: la única línea no determinista es `%%Title:`, que embebe la ruta
  de salida exacta que se le pasó a `mg` (y por lo tanto varía según el
  directorio temporal usado en cada corrida). Se sustituye por una constante
  fija (`%%Title: (normalized)`). El resto del EPS es determinista (no hay
  timestamps).
- **SVG**: no requiere normalización — no embebe ruta de salida ni timestamp.
  La función de normalización lo trata como un caso explícito (`cat`) para
  dejar el hook listo si en el futuro aparece algo volátil.

## Notas de invocación

- Cada corrida de `mg` se hace con el directorio de trabajo en `examples/`,
  porque dos de los ejemplos (`bzsinepaths-examples.mg`, `fig6-1.mg`) usan
  `include` de rutas relativas al CWD (incluyen `bzsinepaths`).
- Se fija `LC_ALL=C` antes de invocar `mg` para que el formato de números
  decimales sea estable entre entornos/locales.
- La salida de `mg` a stderr (mensajes tipo "Opening/Closing/...") se
  descarta; no afecta el archivo de salida.

## Pruebas negativas (`test/errors/`, compuerta `errfail`)

Las otras compuertas miran salida **exitosa**. Los ~150 caminos de error del
compilador (51 `evalError`, 94 `parseError`, 7 `exit`) no tenían ninguna prueba,
y su regresión natural es la peor de todas: **volver al silencio**, que no mueve
un byte de ningún golden. Es la familia de bugs más recurrente del proyecto —
coordenadas sobrantes descartadas, el `bool` de `emitStyleAttr` ignorado, el
punto de control muerto de `fig1`.

Cada `test/errors/*.mg` **debe fallar**, y declara en su propio encabezado lo que
espera (va en git, a diferencia de `test/golden`, y no hay dos listas que
desincronizar):

```mg
% EXPECT: profundidad de recursión excedida
% EXPECT_AT: 4:1          <- opcional: línea:columna a la que debe señalar
```

Se exigen tres cosas, y cada una caza algo distinto:

1. **`exit == 1` exacto**, no «≠ 0» — un segfault también «falla». Ésta es la que
   caza el modo de falla de una recursión sin `max_depth` (139).
2. **El fragmento aparece en stderr** — que el diagnóstico siga existiendo. Se
   compara un fragmento y **no** el mensaje completo a propósito: los mensajes son
   prosa que se va a reescribir, y un golden por bytes castigaría justo las mejoras
   de redacción. El fragmento fija la *afirmación*; la forma queda libre.
3. **No se creó el archivo de salida** — la política de que un documento roto no
   produce salida (la razón de que `evalError` e `include` sean fatales).

Corre en `check` **y** en `capture`, como `gs` y la Capa 3: no depende de bendecir
nada. Añadir un caso es crear el `.mg` con su `% EXPECT:`; no hay lista que tocar.

Verificada como las otras cuatro, reintroduciendo a propósito los bugs que debe
cazar: al descartar otra vez el retorno de `emitStyleAttr`, el golden da
`ok=66 fail=0` y las cuatro compuertas viejas quedan verdes —ciegas— y solo ésta
lo reporta; al quitar la guarda de `max_depth`, la caza por el código 139.
