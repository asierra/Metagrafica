#!/usr/bin/env bash
# =====================================================================
#  ver.sh — mirar una figura de MetaGráfica, y comparar dos versiones.
#
#  Las ocho compuertas de test/run.sh cazan CLASES de fallo. Ninguna
#  contesta «¿se ve bien?», y esa pregunta ha destapado, dos veces, más
#  defectos que las compuertas en verde (bitácora 2026-07-20 y 2026-07-28).
#  Este script es la versión ejecutable de esa lección: un comando para
#  rasterizar cualquier salida CORRECTAMENTE, y otro para preguntarle a
#  dos versiones si dibujan lo mismo.
#
#  ─── Por qué Chrome y no rsvg/Inkscape (medido el 2026-07-31) ───
#  SVGDisplay embebe Latin Modern Math como TTF base64 en un @font-face.
#  Chrome y Firefox lo cargan —también en modo <img>, que es el de un
#  README—; `rsvg-convert` e Inkscape lo IGNORAN y caen a una sans. O sea
#  que las dos herramientas de línea de comandos que uno usaría por
#  reflejo MIENTEN sobre la tipografía matemática, y es facilísimo
#  diagnosticar como bug del compilador lo que es sustitución del visor
#  (pasó: una ρ correcta se veía como «ø»). Para JUZGAR un SVG de
#  MetaGráfica hay que usar un navegador. Los otros dos formatos no
#  tienen el problema: gs y pdftoppm leen la fuente embebida.
#
#  ─── Por qué --diff es del MISMO formato, nunca entre backends ───
#  Comparar EPS contra SVG por píxeles NO funciona, y se midió antes de
#  escribir esto: el antialiasing de dos rasterizadores distintos deja
#  6333 píxeles diferentes (con 25% de tolerancia) en una figura idéntica,
#  mientras que un rótulo desplazado 1.4 pt mueve 524. La señal queda
#  MUY por debajo del ruido. Por eso la paridad entre backends se hace
#  contando operadores (Capa 3 de test/run.sh) y no mirando píxeles.
#  Dentro de un mismo formato, en cambio, el piso de ruido es CERO
#  píxeles exactos, así que la comparación es limpia.
#
#  ─── Para qué sirve --diff, si ya está el golden de bytes ───
#  El golden es MÁS sensible: caza cualquier byte. Lo que no puede es
#  contestar la pregunta de un refactor:
#       «los bytes cambiaron A PROPÓSITO, ¿cambió el dibujo?»
#  Ahí el golden solo sabe decir «distinto». Es justo el criterio de
#  aceptación de reescribir lib/pseudo3d.mg sobre `plane3d`
#  (plan_pseudo3d.md, Fase C): la misma figura, otro código.
#
#  Uso:
#     tools/ver.sh figura.svg              # -> figura.svg.png
#     tools/ver.sh figura.mg               # compila y rasteriza los TRES
#     tools/ver.sh -r 300 figura.pdf       # otra resolución (default 150)
#     tools/ver.sh --diff viejo.eps nuevo.eps
#     tools/ver.sh --diff -k a.mg b.mg     # compara los tres formatos
#
#  Solo usa lo que el proyecto ya necesita: gs (EPS), pdftoppm (PDF),
#  ImageMagick (comparación) y un navegador (SVG). Cada uno se reporta
#  como ausente en vez de fallar de forma críptica.
# =====================================================================
set -euo pipefail

DPI=150
OUTDIR="."
KEEP=0
MODE=ver

morir() { printf 'ver.sh: %s\n' "$*" >&2; exit 1; }
hay()   { command -v "$1" >/dev/null 2>&1; }

navegador() {
  local c
  for c in google-chrome chromium chromium-browser google-chrome-stable; do
    hay "$c" && { printf '%s' "$c"; return 0; }
  done
  return 1
}

uso() { sed -n '/^#  Uso:/,/^#$/p' "$0" | sed 's/^# \{0,2\}//; /^$/d'; exit 0; }

while [ $# -gt 0 ]; do
  case "$1" in
    -r|--dpi)  DPI=${2:?falta el valor de -r}; shift 2 ;;
    -o|--out)  OUTDIR=${2:?falta el valor de -o}; shift 2 ;;
    -k|--keep) KEEP=1; shift ;;
    --diff)    MODE=diff; shift ;;
    -h|--help) uso ;;
    --) shift; break ;;
    -*) morir "opción desconocida: $1 (usa -h)" ;;
    *)  break ;;
  esac
done

[ $# -ge 1 ] || uso
mkdir -p "$OUTDIR"

RAIZ=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
MG="$RAIZ/bin/mg"
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT

# --- rasterizar UN archivo (eps|svg|pdf) a PNG -----------------------
# $1 = entrada, $2 = png de salida. Cada formato con la herramienta que
# lee su fuente embebida; ver el encabezado sobre por qué SVG va aparte.
raster() {
  local ent="$1" png="$2" ext="${1##*.}"
  case "$ext" in
    eps)
      hay gs || morir "hace falta ghostscript (gs) para rasterizar EPS"
      gs -q -dNOPAUSE -dBATCH -sDEVICE=pngalpha -r"$DPI" -dEPSCrop \
         -o "$png.raw.png" "$ent" 2>/dev/null ;;
    pdf)
      hay pdftoppm || morir "hace falta pdftoppm (poppler-utils) para rasterizar PDF"
      pdftoppm -png -r "$DPI" -singlefile "$ent" "${png%.png}.raw" 2>/dev/null
      mv "${png%.png}.raw.png" "$png.raw.png" ;;
    svg)
      local nav; nav=$(navegador) || morir \
        "hace falta un navegador (google-chrome/chromium) para rasterizar SVG.
 ⚠️ NO uses rsvg-convert ni Inkscape: ignoran el @font-face y la tipografía
    matemática sale sustituida por una sans. Ver el encabezado de este script."
      # Medidas del propio SVG (MetaGráfica las emite en pt) -> píxeles.
      local wpt hpt w h abs
      wpt=$(sed -n 's/.*<svg[^>]*width="\([0-9.]*\)pt".*/\1/p' "$ent" | head -1)
      hpt=$(sed -n 's/.*<svg[^>]*height="\([0-9.]*\)pt".*/\1/p' "$ent" | head -1)
      [ -n "$wpt" ] && [ -n "$hpt" ] || morir "no pude leer width/height de $ent"
      w=$(awk -v v="$wpt" -v d="$DPI" 'BEGIN{printf "%d", v/72*d + 0.5}')
      h=$(awk -v v="$hpt" -v d="$DPI" 'BEGIN{printf "%d", v/72*d + 0.5}')
      abs=$(cd "$(dirname "$ent")" && pwd)/$(basename "$ent")
      # Envoltura HTML con tamaño EXPLÍCITO: apuntar el navegador al .svg
      # directo lo centra y le mete margen, y el tamaño deja de ser el del
      # documento — que es justo lo que --diff necesita que sea exacto.
      printf '<html><body style="margin:0;background:#fff">
<img src="file://%s" width="%s" height="%s"></body></html>' "$abs" "$w" "$h" > "$TMP/w.html"
      "$nav" --headless --disable-gpu --no-sandbox --hide-scrollbars \
             --force-device-scale-factor=1 \
             --user-data-dir="$TMP/perfil" \
             --window-size="$w,$h" --screenshot="$png.raw.png" \
             "$TMP/w.html" >/dev/null 2>&1 ;;
    *) morir "no sé rasterizar .$ext (eps, svg o pdf)" ;;
  esac
  [ -s "$png.raw.png" ] || morir "la rasterización de $ent no produjo imagen"
  # Fondo blanco plano: sin esto, el alfa de gs y el del navegador difieren
  # y --diff contaría como distinto lo que solo es transparencia.
  if hay magick; then
    magick "$png.raw.png" -background white -alpha remove -alpha off "$png"
    rm -f "$png.raw.png"
  else
    mv "$png.raw.png" "$png"
  fi
}

# --- compilar un .mg a los tres formatos ------------------------------
compilar() {
  local mg="$1" dest="$2" base f
  [ -x "$MG" ] || morir "no encuentro $MG (corre 'make' primero)"
  base=$(basename "${mg%.mg}")
  # ⚠️ El archivo de errores lleva el PID: en --diff los dos lados se compilan en
  # process substitutions CONCURRENTES, y con un nombre fijo se pisaban dejando
  # medio renglón de basura por pantalla.
  local err="$TMP/mg.$BASHPID.err"
  for f in eps svg pdf; do
    "$MG" "$mg" "$dest/$base.$f" >/dev/null 2>"$err" \
      || { cat "$err" >&2; morir "mg falló compilando $mg a $f"; }
    # «render ->» (stdout) y «Closing» (stderr) son ruido de progreso; cualquier
    # OTRA línea es un AVISO del compilador (símbolo desconocido, lienzo en
    # blanco…) y se muestra: esos avisos son la única pista que tiene el autor.
    grep -v '^Closing ' "$err" >&2 || true
  done
  rm -f "$err"
  printf '%s' "$base"
}

# --- entradas de una petición: un .mg da tres, lo demás da una --------
# Imprime líneas «etiqueta<TAB>ruta».
expandir() {
  local ent="$1" dest="$2" base f
  if [ "${ent##*.}" = mg ]; then
    base=$(compilar "$ent" "$dest")
    for f in eps svg pdf; do printf '%s\t%s\n' "$f" "$dest/$base.$f"; done
  else
    printf '%s\t%s\n' "${ent##*.}" "$ent"
  fi
}

if [ "$MODE" = ver ]; then
  for ent in "$@"; do
    [ -f "$ent" ] || morir "no existe: $ent"
    dest=$TMP/c; mkdir -p "$dest"
    while IFS=$'\t' read -r et ruta; do
      png="$OUTDIR/$(basename "${ent%.*}").$et.png"
      raster "$ruta" "$png"
      printf '  %-4s %-52s %s\n' "$et" "$png" "$(hay magick && magick identify -format '%wx%h' "$png" || echo '')"
      [ "$KEEP" = 1 ] && [ "${ent##*.}" = mg ] && cp "$ruta" "$OUTDIR/"
    done < <(expandir "$ent" "$dest")
  done
  exit 0
fi

# --- modo --diff ------------------------------------------------------
[ $# -eq 2 ] || morir "--diff necesita exactamente DOS archivos"
hay magick   || morir "--diff necesita ImageMagick (magick/compare)"
hay compare  || morir "--diff necesita ImageMagick (compare)"
A="$1"; B="$2"
[ -f "$A" ] || morir "no existe: $A"
[ -f "$B" ] || morir "no existe: $B"
[ "${A##*.}" = "${B##*.}" ] || morir \
  "--diff compara archivos del MISMO formato. Entre backends el antialiasing
 ahoga la señal (6333 px de ruido contra 524 de un rótulo movido); para
 paridad entre backends está la Capa 3 de test/run.sh."

da=$TMP/a; db=$TMP/b; mkdir -p "$da" "$db"
total=0; distintos=0
while IFS=$'\t' read -r et ra <&3 && IFS=$'\t' read -r _ rb <&4; do
  pa="$TMP/a.$et.png"; pb="$TMP/b.$et.png"
  raster "$ra" "$pa"; raster "$rb" "$pb"
  ta=$(magick identify -format '%wx%h' "$pa")
  tb=$(magick identify -format '%wx%h' "$pb")
  if [ "$ta" != "$tb" ]; then
    printf '  %-4s TAMAÑOS DISTINTOS: %s vs %s — cambió el lienzo, no solo el dibujo\n' "$et" "$ta" "$tb"
    distintos=$((distintos+1)); total=$((total+1)); continue
  fi
  n=$(compare -metric AE "$pa" "$pb" null: 2>&1 || true)
  n=${n%% *}
  total=$((total+1))
  if [ "$n" = 0 ]; then
    printf '  %-4s IDÉNTICO (%s, 0 px)\n' "$et" "$ta"
  else
    dif="$OUTDIR/diff.$et.png"
    compare "$pa" "$pb" "$dif" 2>/dev/null || true
    printf '  %-4s %s px distintos de %s  -> %s\n' "$et" "$n" "$ta" "$dif"
    distintos=$((distintos+1))
  fi
done 3< <(expandir "$A" "$da") 4< <(expandir "$B" "$db")

echo "---"
if [ "$distintos" = 0 ]; then
  echo "MISMO DIBUJO en $total/$total (piso de ruido de esta comparación: 0 px)."
  exit 0
fi
echo "DIBUJO DISTINTO en $distintos de $total."
exit 1
