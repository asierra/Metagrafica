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

## Pendiente: fase 2 (PDF)

La verificación de la salida PDF queda pendiente para una fase 2. El plan es
comparar por rasterización con `gs` (Ghostscript) en vez de diff byte a byte,
ya que el PDF generado por libharu no es determinista byte a byte (puede
variar en metadatos internos aunque el contenido visual sea idéntico). No
está implementado todavía en este arnés.
