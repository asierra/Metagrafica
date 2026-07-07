#!/usr/bin/env bash
#
# Golden-file regression harness for MetaGrafica (mg compiler).
#
# Modes:
#   ./run.sh capture   - (re)generate the golden files from the current build
#   ./run.sh check     - (default) compare current output against the golden files
#
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MG="$ROOT/bin/mg"
EXDIR="$ROOT/examples/v1"
GOLD="$ROOT/test/golden"

FORMATS="eps svg"
EXAMPLES="bzsinepaths-examples bzsinepaths fig2-1 fig2-3 fig6-1 fill_styles line_patterns primitives rpstest"

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
        *)
            echo "normalize: unknown format '$fmt'" >&2
            return 1
            ;;
    esac
}

ok_count=0
fail_count=0
error_count=0

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
    total=$((ok_count + fail_count + error_count))
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
