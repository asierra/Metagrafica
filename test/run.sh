#!/usr/bin/env bash
#
# Golden-file regression harness for MetaGrafica (mg compiler).
#
# Modes:
#   ./run.sh capture   - (re)generate the golden files from the current build
#   ./run.sh check     - (default) compare current output against the golden files
#
# Dos compuertas, cada una caza una clase distinta (plan_plot.md, "Lecciones"):
#   - Golden por bytes (eps/svg/pdf): caza REGRESIONES de salida. No caza un bug
#     preexistente: se bendice como correcto.
#   - Ghostscript sobre el EPS (psfail): caza los bugs de PRÓLOGO, que el golden
#     no puede ver porque producen un EPS byte-estable que revienta al interpretarse
#     (/undefined in ellipse, /cshow sin su prólogo). Se omite si no hay gs.
#
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MG="$ROOT/bin/mg"
EXDIR="$ROOT/examples"
GOLD="$ROOT/test/golden"

# PDF entra a la red golden (antes solo eps/svg, "PDF se verifica por vista"):
# la salida de libharu es byte-determinista y no depende del path ni de la fecha
# (verificado 2026-07-14: sin CreationDate ni /ID), así que se compara igual que
# EPS/SVG. Cierra el hueco donde vivió el bug de rótulos en blanco en PDF.
FORMATS="eps svg pdf"
# Corpus V3 (post-cutover, §22.6): bin/mg ES el compilador V3. La red golden es
# la salida del propio renderer V3 (regresión), no el oráculo V1 de migración.
# El corpus vive en examples/ (raíz). fig6-4v3-clean ejercita eje log + fit(stretch)
# + math con superíndices + extend + ticks-in: la combinación donde se escondieron
# los bugs de esta línea (plan_plot.md).
EXAMPLES="arrow curvas3 fig2-1 fig2-3 fig2-6 fig4-1 fig4-10 fig6-1 fig6-10 fig6-4v3-clean fill_styles line_patterns markers-demo primitives rpstest sines texto"

export LC_ALL=C

MODE="${1:-check}"

if [ "$MODE" != "capture" ] && [ "$MODE" != "check" ]; then
    echo "Usage: $0 [capture|check]" >&2
    exit 2
fi

if [ ! -x "$MG" ]; then
    echo "error: binary not found or not executable: $MG (run 'make' first)" >&2
    exit 2
fi

# Compuerta de validez PostScript. Un golden por bytes NO puede cazar los bugs de
# prólogo: el `/undefined in ellipse` producía un EPS byte-ESTABLE que reventaba
# en un intérprete real (lo mismo con /cshow si falta la bandera using_textalign).
# Solo un intérprete los detecta. Es OPCIONAL: sin gs el harness sigue usable y
# solo avisa una vez — nunca falla por ausencia de la herramienta.
GS_BIN="$(command -v gs 2>/dev/null || true)"
if [ -z "$GS_BIN" ]; then
    echo "aviso: 'gs' no encontrado; se omite la validación PostScript de los EPS" >&2
fi

mkdir -p "$GOLD"

# normalize <fmt> <archivo>
#
# Writes the normalized contents of <archivo> to stdout, so that the result
# is stable across machines/paths/timestamps and can be diffed or stored as
# a golden file.
normalize() {
    fmt="$1"
    file="$2"
    case "$fmt" in
        eps)
            # The only volatile line in EPS output is %%Title:, which embeds
            # the output path we asked mg to write to. Everything else is
            # deterministic. Replace it with a fixed constant.
            sed 's/^%%Title:.*/%%Title: (normalized)/' "$file"
            ;;
        svg)
            # SVG output is already fully deterministic (no embedded path or
            # timestamp). Identity transform, kept explicit as a hook for
            # any future normalization needs.
            cat "$file"
            ;;
        pdf)
            # Also deterministic: libharu embeds neither CreationDate nor /ID,
            # and the output does not depend on the destination path (unlike
            # EPS, which puts it in %%Title). Binary, but diff -q handles it.
            cat "$file"
            ;;
        *)
            echo "normalize: unknown format '$fmt'" >&2
            return 1
            ;;
    esac
}

ok_count=0
fail_count=0
error_count=0
psfail_count=0

for example in $EXAMPLES; do
    for fmt in $FORMATS; do
        base="$example.$fmt"
        tmpdir="$(mktemp -d)"
        outfile="$tmpdir/$base"

        ( cd "$EXDIR" && "$MG" "$example.mg" "$outfile" ) >/dev/null 2>/dev/null
        status=$?

        if [ $status -ne 0 ] || [ ! -s "$outfile" ]; then
            echo "ERROR $base (mg exit=$status, output-exists=$( [ -f "$outfile" ] && echo yes || echo no ))"
            error_count=$((error_count + 1))
            rm -rf "$tmpdir"
            continue
        fi

        # Validez PostScript del EPS: independiente del golden y de $MODE (en
        # capture evita bendecir un EPS que no interpreta). Ver GS_BIN arriba.
        if [ "$fmt" = "eps" ] && [ -n "$GS_BIN" ]; then
            if ! "$GS_BIN" -q -dNOPAUSE -dBATCH -sDEVICE=nullpage "$outfile" >/dev/null 2>&1; then
                echo "PSFAIL $base (Ghostscript rechaza el EPS: prólogo/undefined)"
                psfail_count=$((psfail_count + 1))
            fi
        fi

        normfile="$tmpdir/$base.norm"
        normalize "$fmt" "$outfile" > "$normfile"

        if [ "$MODE" = "capture" ]; then
            cp "$normfile" "$GOLD/$base"
            echo "captured $base"
        else
            if [ ! -f "$GOLD/$base" ]; then
                echo "FAIL  $base (no golden file present; run 'capture' first)"
                fail_count=$((fail_count + 1))
            elif diff -q "$GOLD/$base" "$normfile" >/dev/null 2>&1; then
                echo "ok    $base"
                ok_count=$((ok_count + 1))
            else
                echo "FAIL  $base"
                fail_count=$((fail_count + 1))
            fi
        fi

        rm -rf "$tmpdir"
    done
done

echo "---"
if [ "$MODE" = "capture" ]; then
    echo "capture done. errors: $error_count psfail: $psfail_count"
    if [ "$error_count" -ne 0 ] || [ "$psfail_count" -ne 0 ]; then
        exit 1
    fi
    exit 0
else
    echo "check summary: ok=$ok_count fail=$fail_count error=$error_count psfail=$psfail_count"
    if [ "$fail_count" -ne 0 ] || [ "$error_count" -ne 0 ] || [ "$psfail_count" -ne 0 ]; then
        exit 1
    fi
    exit 0
fi
