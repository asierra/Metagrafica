#!/usr/bin/env bash
#
#  social_preview.sh — la imagen de vista previa social del repositorio.
#
#  Uso:  bash tools/social_preview.sh
#        → docs/img/social-preview.png  (1280x640)
#
#  Se sube A MANO en GitHub → Settings → Social preview. La API no expone ese
#  campo, así que es el único de los tres ajustes de vitrina (plan_promocion.md
#  §5) que no se puede automatizar; los otros dos —topics y Website— se pusieron
#  con `gh repo edit` el 2026-08-04.
#
#  ─── Por qué existe el archivo, y no solo el PNG ───
#  El PNG es un asset GENERADO y committeado, como lib/polar_map.mg o docs/img:
#  la regla del proyecto es que lo generado lleve al lado el comando que lo
#  rehace. Sin esto, el día que `seccion_eficaz.mg` cambie nadie sabría cómo
#  volver a producir la imagen, ni con qué figura se hizo.
#
#  ⚠️ NO lo vigila ninguna compuerta. `imgfail` itera sobre docs/img/*.svg
#  (test/run.sh), así que un .png ahí es inerte, y eso es a propósito: la
#  vista previa no tiene por qué regenerarse cuando cambia una figura. Si
#  cambia mucho, se corre esto y se sube de nuevo.
#
#  ─── Las decisiones, para no re-litigarlas ───
#  · 1280x640 es lo que pide GitHub (2:1). Se rasteriza a 640x320 con factor de
#    escala 2 para que el texto salga nítido.
#  · La figura es `seccion_eficaz` porque su lienzo es 13x5.98 = 2.17:1, o sea
#    casi exactamente el 2:1 del formato: llena el marco sin blanco muerto. Las
#    demás candidatas son 16:9 (1.78) o cuadradas y dejan bandas a los lados.
#  · Se rasteriza con CHROME y no con rsvg-convert ni Inkscape, que ignoran el
#    @font-face de LM Math y sustituyen la tipografía matemática por una sans.
#    Es la misma razón —y los mismos flags— que tools/ver.sh; ver su encabezado.
#
set -e

RAIZ=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
SVG="$RAIZ/docs/img/seccion_eficaz.svg"
PNG="$RAIZ/docs/img/social-preview.png"

[ -f "$SVG" ] || { echo "falta $SVG (corre: bash test/run.sh images)" >&2; exit 1; }

nav=""
for c in google-chrome chromium chromium-browser google-chrome-stable; do
  command -v "$c" >/dev/null 2>&1 && { nav="$c"; break; }
done
[ -n "$nav" ] || { echo "hace falta google-chrome o chromium (ver encabezado)" >&2; exit 1; }

TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT

cat > "$TMP/p.html" <<EOF
<html><body style="margin:0;background:#fff;width:640px;height:320px;
  position:relative;overflow:hidden;display:flex;align-items:center">
<img src="file://$SVG" style="width:100%;height:auto">
<div style="position:absolute;left:26px;bottom:18px;
            font-family:Georgia,'Times New Roman',serif">
  <span style="font-size:23px;color:#111">MetaGr&aacute;fica</span>
  <span style="font-size:12.5px;color:#888;margin-left:11px;
               font-family:ui-monospace,Menlo,Consolas,monospace">
    .mg &rarr; EPS &middot; SVG &middot; PDF</span>
</div>
</body></html>
EOF

"$nav" --headless --disable-gpu --no-sandbox --hide-scrollbars \
       --force-device-scale-factor=2 --user-data-dir="$TMP/perfil" \
       --window-size=640,320 --screenshot="$PNG" "$TMP/p.html" >/dev/null 2>&1

echo "escrito $PNG"
command -v identify >/dev/null 2>&1 && identify -format '  %wx%h  %b\n' "$PNG"
