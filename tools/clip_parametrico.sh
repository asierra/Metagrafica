#!/usr/bin/env bash
# =====================================================================
#  clip_parametrico.sh — un GIF de una figura reaccionando a UN parámetro.
#
#  El argumento de §3 de plan_promocion.md («la figura como documento
#  fuente reproducible», paramétrica y no medida a ojo) es difícil de
#  enseñar con una imagen quieta y trivial de enseñar en movimiento: se
#  cambia un número del .mg y la figura entera se reacomoda de forma
#  coherente. Este script produce ese clip.
#
#  ─── Por qué GENERADO y no editado a mano ───
#  La gramática está en BETA y puede renombrar (title→label, 2026-07-16).
#  Un clip editado en un programa de video se pudre con el primer
#  renombre y rehacerlo cuesta una sesión; éste cuesta un comando. Es la
#  misma razón por la que docs/img, la galería y el Modelfile se generan
#  en vez de mantenerse: salida derivada que nadie regenera es salida
#  que miente. ⚠️ Y a diferencia de aquéllos, un GIF NO tiene compuerta
#  que lo vigile —ninguna sabe mirar un video—, así que la única defensa
#  es que rehacerlo sea barato.
#
#  ─── Por qué rasteriza con tools/ver.sh y no con su propio chrome ───
#  Porque la receta correcta ya está medida y explicada AHÍ (Chrome, no
#  rsvg-convert ni Inkscape: ignoran el @font-face de LM Math y la
#  tipografía matemática sale sustituida por una sans). Duplicarla aquí
#  sería crear la segunda copia que se desincroniza.
#
#  ─── El barrido es de IDA Y VUELTA, a propósito ───
#  Un GIF vuelve al primer cuadro de golpe al terminar. Con el barrido
#  solo de ida, ese salto se lee como un parpadeo defectuoso; con ida y
#  vuelta el lazo es continuo y el ojo sigue el reacomodo en los dos
#  sentidos, que es justo lo que hay que enseñar. Las pausas en los dos
#  extremos existen para poder LEER la figura, que es el objeto del clip
#  —sin ellas el extremo es un cuadro de 1/12 de segundo—.
#
#  Uso:
#     tools/clip_parametrico.sh                     # el clip del README
#     tools/clip_parametrico.sh -o /tmp/prueba.gif
#     tools/clip_parametrico.sh --var re2 --de 1.48 --a 1.20 \
#                               --paso 0.02 examples/franck_condon.mg
#
#  Necesita: bin/mg, un navegador (vía ver.sh), ImageMagick y ffmpeg.
# =====================================================================
set -euo pipefail

RAIZ=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)

# Defaults = el clip que pide plan_promocion.md §5: la anarmonicidad del
# estado fundamental de franck_condon, desde el valor publicado. El pozo
# se hace menos profundo, la línea de disociación baja y los niveles se
# apiñan contra ella — todo de mover un número.
FUENTE="$RAIZ/examples/franck_condon.mg"
VAR=xe1
DE=0.028
# El extremo es el de docs/img/franck_condon_anarm.mg, así que el clip cubre
# el par de imágenes publicado de punta a punta.
#
# ⚠️ No siempre pudo: hasta el 2026-08-05 este barrido ABORTABA en
# xe1 = 0.040, porque el `for v = 7 to 12` del ejemplo no consultaba
# vmax = 1/(2·xe) − ½ —su propia NOTA— y ahí vmax da 12.00 exacto, o sea
# s = 1 y ln(0). Lo encontró este script: un barrido visita valores que
# nadie compiló, y un par de imágenes quietas se los salta. Vale la pena
# recordarlo si algún día se le agrega otro parámetro.
A=0.045
# ⚠️ El barrido va por PASO y no por número de cuadros, y el paso decide
# los decimales del rótulo. Así cada cuadro muestra un valor que alguien
# TECLEARÍA (0.028, 0.029, …) en vez de un 0.03687 de interpolación, los
# extremos coinciden exactos con los del README, y el rótulo no cambia
# de ancho — el jitter de un número que crece y encoge distrae de lo
# único que el clip tiene que dejar ver, que es la figura moviéndose.
PASO=0.0005
PAUSA=6            # cuadros de pausa en cada extremo
FPS=10
DPI=110
ROTULO=1
# ⚠️ Escribe en el directorio ACTUAL, y NO en docs/img/, a propósito. El GIF no
# va en git: es un render para una ocasión —el taller, un post— y no un asset del
# repo. La razón no es el peso sino que no puede tener compuerta: dos corridas
# seguidas dan un GIF byte a byte idéntico (medido), pero esa determinación es
# prestada de la paleta de ffmpeg y del rasterizador de Chrome, y nadie fija esas
# versiones — la de docs/img/*.svg es del propio compilador y el workflow la
# verifica en tres sistemas. Una compuerta de bytes sobre el GIF sería verde aquí
# y roja en la máquina del siguiente. Y docs/img/ es justo la carpeta que uno
# añade entera a un commit sin mirar.
SALIDA="franck_condon_parametrico.gif"

morir() { printf 'clip_parametrico.sh: %s\n' "$*" >&2; exit 1; }
hay()   { command -v "$1" >/dev/null 2>&1; }
uso()   { sed -n '/^#  Uso:/,/^#$/p' "$0" | sed 's/^# \{0,2\}//; /^$/d'; exit 0; }

while [ $# -gt 0 ]; do
  case "$1" in
    --var)     VAR=${2:?}; shift 2 ;;
    --de)      DE=${2:?}; shift 2 ;;
    --a)       A=${2:?}; shift 2 ;;
    --paso)    PASO=${2:?}; shift 2 ;;
    --pausa)   PAUSA=${2:?}; shift 2 ;;
    --fps)     FPS=${2:?}; shift 2 ;;
    -r|--dpi)  DPI=${2:?}; shift 2 ;;
    --sin-rotulo) ROTULO=0; shift ;;
    -o|--out)  SALIDA=${2:?}; shift 2 ;;
    -h|--help) uso ;;
    -*) morir "opción desconocida: $1 (usa -h)" ;;
    *)  FUENTE=$1; shift ;;
  esac
done

[ -f "$FUENTE" ]     || morir "no existe: $FUENTE"
[ -x "$RAIZ/bin/mg" ] || morir "no encuentro bin/mg (corre 'make' primero)"
hay ffmpeg  || morir "hace falta ffmpeg para armar el GIF"
hay magick  || morir "hace falta ImageMagick (magick)"
grep -qE "^[[:space:]]*$VAR[[:space:]]*=" "$FUENTE" \
  || morir "no encuentro una asignación de nivel superior '$VAR =' en $FUENTE"

TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT

# Monoespaciada para el rótulo, la primera que exista. Si no hay ninguna
# se cae al default de ImageMagick con aviso: un clip con el rótulo en
# otra tipografía sigue sirviendo, uno que no se genera no.
FUENTE_ROTULO=
if [ "$ROTULO" = 1 ]; then
  # ⚠️ La lista se guarda ENTERA y se busca dentro, en vez de tubear a
  # `grep -q` una vez por candidata: `-q` cierra la tubería al primer
  # acierto, `magick` muere de SIGPIPE y con `pipefail` la tubería
  # entera cuenta como fallo — o sea que encontrar la fuente se leía
  # como no encontrarla.
  FUENTES=$(magick -list font 2>/dev/null || true)
  for f in DejaVu-Sans-Mono Liberation-Mono Nimbus-Mono-PS Andale-Mono Courier; do
    case $FUENTES in *"Font: $f"$'\n'*) FUENTE_ROTULO=$f; break ;; esac
  done
  [ -n "$FUENTE_ROTULO" ] || \
    printf 'aviso: sin monoespaciada; el rótulo sale en la tipografía default\n' >&2
fi

# El .mg de cada cuadro se escribe en un árbol ESPEJO: examples/ con lib/
# de hermano, y por symlink. Así el `include "../lib/x.mg"` de cualquier
# ejemplo resuelve igual que en el árbol real (es la misma razón por la
# que make install los deja hermanos) sin ensuciar el repo con archivos
# temporales que una interrupción dejaría atrás.
mkdir -p "$TMP/examples"
ln -s "$RAIZ/lib" "$TMP/lib"
BASE=$(basename "${FUENTE%.mg}")

# Decimales del rótulo = los del dato más fino que dio el usuario. Se leen
# de la CADENA y no del número: es la única forma de saber que 0.001 pide
# tres y no seis, y de que 0.028 no salga como «0.0280».
DEC=$(awk -v a="$DE" -v b="$A" -v p="$PASO" 'BEGIN{
  n = 0
  split(a "\n" b "\n" p, v, "\n")
  for (i = 1; i <= 3; i++) { d = index(v[i], "."); if (d) { l = length(v[i]) - d; if (l > n) n = l } }
  print n }')
CUADROS=$(awk -v a="$DE" -v b="$A" -v p="$PASO" \
          'BEGIN{ d = b - a; if (d < 0) d = -d; if (p < 0) p = -p
                  if (p == 0) exit 1; printf "%d", d/p + 1.5 }') \
  || morir "el paso no puede ser cero"
[ "$CUADROS" -ge 2 ] || morir "el paso ($PASO) es más grande que el intervalo $DE→$A"

printf 'barriendo %s de %s a %s de %s en %s (%d cuadros)\n' \
  "$VAR" "$DE" "$A" "$PASO" "$BASE" "$CUADROS"

n=0
while [ "$n" -lt "$CUADROS" ]; do
  val=$(awk -v a="$DE" -v b="$A" -v i="$n" -v k="$((CUADROS-1))" -v d="$DEC" \
        'BEGIN{ printf "%.*f", d, a + (b-a)*i/k }')
  mg="$TMP/examples/f.mg"
  # Se sustituye SOLO la asignación de nivel superior, y solo la primera:
  # el mismo nombre puede reaparecer dentro de una struct o de un lazo,
  # donde reescribirlo cambiaría la figura por otra razón.
  awk -v v="$VAR" -v x="$val" '
    !hecho && $0 ~ "^[[:space:]]*" v "[[:space:]]*=" { sub(/=.*/, "= " x); hecho=1 }
    { print }' "$FUENTE" > "$mg"

  # Un cuadro que no compila ABORTA el clip, y muestra el error del
  # compilador. Saltarlo en silencio sería lo peor posible: un barrido
  # visita valores que un ejemplo nunca probó —así se encontró que
  # franck_condon aborta en xe1 = 0.040— y ese hallazgo vale más que el
  # cuadro que falta.
  "$RAIZ/bin/mg" "$mg" "$TMP/f.svg" >/dev/null 2>"$TMP/err" \
    || { printf '\n'; cat "$TMP/err" >&2; morir "mg falló con $VAR = $val"; }
  "$RAIZ/tools/ver.sh" -r "$DPI" -o "$TMP" "$TMP/f.svg" >/dev/null
  png=$(printf '%s/cuadro%03d.png' "$TMP" "$n")
  if [ "$ROTULO" = 1 ]; then
    # El rótulo es el argumento entero: el espectador tiene que ver que lo
    # único que cambia es un número del fuente. Va arriba a la izquierda,
    # sobre la franja que la figura deja libre (la caja del plot empieza
    # en y=1.0 de 11), y en MONOESPACIADA a propósito: así se lee como la
    # línea del .mg que es y no como un rótulo de la figura, que va en la
    # serif de la tipografía matemática.
    magick "$TMP/f.svg.png" -gravity northwest -fill '#1a1a1a' \
           ${FUENTE_ROTULO:+-font "$FUENTE_ROTULO"} \
           -pointsize $((DPI/7)) -annotate +14+12 "$VAR = $val" "$png"
  else
    mv "$TMP/f.svg.png" "$png"
  fi
  printf '.'
  n=$((n+1))
done
printf '\n'

# --- la secuencia: pausa, ida, pausa, vuelta -------------------------
SEQ="$TMP/seq"; mkdir -p "$SEQ"
i=0
poner() { ln -s "$(printf '%s/cuadro%03d.png' "$TMP" "$1")" \
             "$(printf '%s/s%04d.png' "$SEQ" "$i")"; i=$((i+1)); }

k=0; while [ "$k" -lt "$PAUSA" ]; do poner 0; k=$((k+1)); done
k=0; while [ "$k" -lt "$CUADROS" ]; do poner "$k"; k=$((k+1)); done
k=0; while [ "$k" -lt "$PAUSA" ]; do poner $((CUADROS-1)); k=$((k+1)); done
k=$((CUADROS-2)); while [ "$k" -gt 0 ]; do poner "$k"; k=$((k-1)); done

# --- GIF con paleta propia -------------------------------------------
# palettegen/paletteuse en vez de la paleta web: la figura es casi toda
# blanco, negro y un naranja, así que 64 colores sobran y el archivo
# queda una fracción de lo que da la cuantización por default.
mkdir -p "$(dirname "$SALIDA")"
ffmpeg -y -loglevel error -framerate "$FPS" -i "$SEQ/s%04d.png" \
  -filter_complex "[0:v]split[a][b];[a]palettegen=max_colors=64[p];[b][p]paletteuse=dither=bayer:bayer_scale=3" \
  "$SALIDA"

printf '%s  (%s, %s cuadros, %s fps)\n' \
  "$SALIDA" "$(du -h "$SALIDA" | cut -f1)" "$i" "$FPS"
