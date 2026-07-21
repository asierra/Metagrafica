#!/usr/bin/env bash
#
# Regression harness for mg1to2.py (traductor V1 -> V3, ver plan_mg1to2.md).
#
# Modes:
#   ./run_translator.sh capture   - (re)generate the golden files from the
#                                    current mg1to2.py + bin/mg
#   ./run_translator.sh check     - (default) compare current output against
#                                    the golden files
#
# Pipeline por ejemplo: examples/v1/X.mg --(mg1to2.py)--> X.mg (V3)
# --(bin/mg)--> X.svg. La red golden es la salida del propio traductor+
# compilador V3 (AUTO-regresion, mismo principio que test/run.sh: "salida
# del propio renderer V3, no el oraculo V1" -- ver CLAUDE.md). La fidelidad
# pixel contra el oraculo V1 (examples/v1/reference/*.svg) para fill_styles/
# line_patterns/fig2-1/fig2-6/fig6-10/primitives/rpstest ya se verifico A
# MANO durante el desarrollo (inkscape + imagemagick compare, ver la sesion
# que escribio mg1to2.py) -- este harness solo detecta que el traductor deje
# de producir lo que ya se sabe correcto, no re-verifica fidelidad cada vez.
#
# Todos los .mg traducidos se escriben a un mismo directorio temporal para
# que `include "arrow.mg"` (INPUT arrow en V1) resuelva entre ellos, igual
# que el INPUT de V1 resuelve relativo al archivo que lo usa.
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MG="$ROOT/bin/mg"
TRANSLATOR="$ROOT/tools/mg1to2.py"
V1DIR="$ROOT/examples/v1"
GOLD="$ROOT/test/golden_translator"

FORMATS="svg"
# Los 15 del corpus V1<->V3 (plan_mg1to2.md §1), MENOS sines/texto: esos dos
# no tienen fuente V1 (nunca existio examples/v1/sines.mg ni texto.mg -- se
# escribieron directo en V3 para ejercitar la gramatica, no son traduccion de.
# (markers-demo se removio 2026-07-17: era prototipo de la era V3 del diseno de
# marcadores, en sintaxis V1 pero SIN figura de libro detras -- no es traduccion.
# Con el se fue examples/v1/markers.mg, su libreria de structs Mk*, ya sin uso.)
# nada). Incluirlos aqui solo produce un ERROR "file not found" perpetuo, no
# una senal sobre el traductor. `bzsinepaths` no es uno de los 16 (es una
# biblioteca, ver bzsinepaths.mg) pero SI se incluye: fig4-10/fig6-1 la
# `INPUT`-ean, y sin su propia entrada en el workdir compartido el `include
# "bzsinepaths.mg"` que emite mg1to2.py no resuelve al compilar con bin/mg
# (mg1to2.py YA resuelve el &nombre/PWPT en Python contra el .mg de V1, asi
# que esto no afecta la fidelidad -- solo evita el include colgado).
# No todos estan cubiertos todavia por mg1to2.py (ver plan_mg1to2.md, orden
# de implementacion §7); los que fallan en la traduccion se reportan ERROR,
# no rompen el harness.
EXAMPLES="arrow bzsinepaths curvas3 fig2-1 fig2-3 fig2-6 fig4-1 fig4-10 fig6-1 fig6-10 fill_styles line_patterns primitives rpstest"

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

if [ ! -f "$TRANSLATOR" ]; then
    echo "error: translator not found: $TRANSLATOR" >&2
    exit 2
fi

mkdir -p "$GOLD"

ok_count=0
fail_count=0
error_count=0

# --- Paso 1: traducir TODOS los ejemplos a un directorio compartido ---
workdir="$(mktemp -d)"
trap 'rm -rf "$workdir"' EXIT

translate_ok=""
for example in $EXAMPLES; do
    if python3 "$TRANSLATOR" "$V1DIR/$example.mg" "$workdir/$example.mg" \
        >"$workdir/$example.translate.log" 2>&1; then
        translate_ok="$translate_ok $example"
    else
        echo "ERROR $example.mg (fallo mg1to2.py; ver $workdir/$example.translate.log)"
        error_count=$((error_count + 1))
    fi
done

# --- Paso 2: compilar + comparar contra golden cada traduccion exitosa ---
for example in $translate_ok; do
    for fmt in $FORMATS; do
        base="$example.$fmt"
        outfile="$workdir/$base"

        ( cd "$workdir" && "$MG" "$example.mg" "$outfile" ) >/dev/null 2>/dev/null
        status=$?

        if [ $status -ne 0 ] || [ ! -s "$outfile" ]; then
            echo "ERROR $base (mg exit=$status, output-exists=$( [ -f "$outfile" ] && echo yes || echo no ))"
            error_count=$((error_count + 1))
            continue
        fi

        if [ "$MODE" = "capture" ]; then
            cp "$outfile" "$GOLD/$base"
            echo "captured $base"
        else
            if [ ! -f "$GOLD/$base" ]; then
                echo "FAIL  $base (no golden file present; run 'capture' first)"
                fail_count=$((fail_count + 1))
            elif diff -q "$GOLD/$base" "$outfile" >/dev/null 2>&1; then
                echo "ok    $base"
                ok_count=$((ok_count + 1))
            else
                echo "FAIL  $base"
                fail_count=$((fail_count + 1))
            fi
        fi
    done
done

echo "---"
if [ "$MODE" = "capture" ]; then
    echo "capture done. errors: $error_count"
    if [ "$error_count" -ne 0 ]; then
        exit 1
    fi
    exit 0
else
    echo "check summary: ok=$ok_count fail=$fail_count error=$error_count"
    if [ "$fail_count" -ne 0 ] || [ "$error_count" -ne 0 ]; then
        exit 1
    fi
    exit 0
fi
