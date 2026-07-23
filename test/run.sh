#!/usr/bin/env bash
#
# Golden-file regression harness for MetaGrafica (mg compiler).
#
# Modes:
#   ./run.sh capture   - (re)generate the golden files from the current build
#   ./run.sh check     - (default) compare current output against the golden files
#   ./run.sh images    - regenerate docs/img/*.svg + docs/galeria.html (salida
#                        PUBLICADA y versionada)
#
# Seis compuertas, cada una caza una clase distinta (plan_plot.md, "Lecciones"):
#   - Golden por bytes (eps/svg/pdf): caza REGRESIONES de salida. No caza un bug
#     preexistente: se bendice como correcto.
#   - Ghostscript sobre el EPS (psfail): caza los bugs de PRÓLOGO, que el golden
#     no puede ver porque producen un EPS byte-estable que revienta al interpretarse
#     (/undefined in ellipse, /cshow sin su prólogo). Se omite si no hay gs.
#   - Paridad entre backends (c3fail, "Capa 3"): caza bugs PREEXISTENTES que el
#     golden bendice porque un backend omite algo silenciosamente. Dos invariantes
#     robustos (cero falsos positivos en el corpus, sin herramientas externas):
#       (a) TEXTO: nº de operaciones de texto EPS(show) == SVG(<tspan>) == PDF(Tj) —
#           caza "rótulos en blanco en PDF/EPS" (bug de FN_NOFACE/current_font).
#           La unidad de SVG son los <tspan>, no los <text>: desde P2 (2026-07-20)
#           un run math se parte en segmentos homogéneos (letras a LM Math, ' = ' al
#           serif) y SVG los emite como tspans dentro de UN <text> — contar <text>
#           daría 1 donde EPS emite 3. Además se cuentan OCURRENCIAS, no líneas:
#           los tspans de un <text> van todos en el mismo renglón.
#       (b) LÍNEAS RELLENAS: un path SVG de un solo segmento (M..L..) con fill=color
#           y stroke=none es una línea de área nula = invisible → caza "ejes sin
#           trazo en PDF/SVG" (fuga de fill del contenido, Lección 6).
#   - docs/img al día (imgfail): caza que la salida PUBLICADA se quede RANCIA. Los
#     .svg de docs/img están EN GIT (GitHub los muestra en la portada del README) y
#     se regeneran a mano; nada los vigilaba, y entre 2026-07-17 y 2026-07-21 la
#     portada estuvo mostrando la tipografía matemática ANTERIOR a la migración a
#     LM Math — o sea, anunciando una mejora que ella misma no exhibía.
#
#     ⚠️ `capture` NO los regenera, a propósito. test/golden es borrador local sin
#     trackear (bendecir es barato y no sale del disco); docs/img es salida
#     PUBLICADA: bendecirla cambia la cara del proyecto y tiene que ser un commit
#     consciente, no un efecto colateral de "acepta lo que el compilador haga
#     ahora". Por eso regenerar es un modo aparte y explícito (`images`).
#   - Pruebas NEGATIVAS (errfail): las otras cuatro miran salida EXITOSA, así que
#     los ~150 caminos de error del compilador no tenían NINGUNA prueba. Su
#     regresión natural es volver al SILENCIO, que no mueve un byte de ningún
#     golden. Ver el bloque de la compuerta al final del archivo.
#   - La GALERÍA al día (galfail): docs/galeria.html es publicada y DERIVADA, y
#     lo que la vuelve rancia es editar un COMENTARIO — lleva incrustados el
#     encabezado y el código de cada ejemplo. Ninguna de las otras cinco puede
#     verlo: un cambio de comentario no mueve un byte de ningún .svg ni de ningún
#     golden. Se regenera con `images`, no con `capture`, por la misma razón que
#     docs/img: bendecir la cara pública es un commit consciente.
#
set -u

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
MG="$ROOT/bin/mg"
EXDIR="$ROOT/examples"
GOLD="$ROOT/test/golden"
# Renders publicados (EN GIT, a diferencia de $GOLD). La compuerta itera sobre lo que
# HAY en este directorio, no sobre $EXAMPLES: la presencia del .svg ES la declaración,
# así que añadir una imagen a un documento no obliga a tocar ninguna lista, y ningún
# .svg de aquí puede quedar sin vigilar por no estar su nombre en el corpus.
#
# El fuente de cada X.svg se busca en dos sitios, en este orden:
#   1. examples/X.mg    — el ejemplo del corpus (quickstart, fig2-5…)
#   2. docs/img/X.mg    — un .mg que vive JUNTO a su render
# El segundo caso son las VARIANTES: figuras que existen para un documento, no para el
# corpus (p. ej. el mismo ejemplo con un parámetro cambiado, para mostrar un antes y
# después). Ponerlas en examples/ ensuciaría el corpus, pero sin fuente versionado el
# render no sería ni reproducible ni verificable — que es justo lo que esta compuerta
# existe para impedir.
IMGDIR="$ROOT/docs/img"

# Fixtures de la compuerta 5 (pruebas negativas): cada .mg DEBE fallar, y declara
# en su encabezado el fragmento de mensaje que debe salir. Ver el bloque al final.
ERRDIR="$ROOT/test/errors"

# PDF entra a la red golden (antes solo eps/svg, "PDF se verifica por vista"):
# la salida de libharu es byte-determinista y no depende del path ni de la fecha
# (verificado 2026-07-14: sin CreationDate ni /ID), así que se compara igual que
# EPS/SVG. Cierra el hueco donde vivió el bug de rótulos en blanco en PDF.
FORMATS="eps svg pdf"
# Corpus V3 (post-cutover, §22.6): bin/mg ES el compilador V3. La red golden es
# la salida del propio renderer V3 (regresión), no el oráculo V1 de migración.
# El corpus vive en examples/ (raíz). fig6-4 ejercita eje log + fit(stretch)
# + math con superíndices + extend + ticks-in: la combinación donde se escondieron
# los bugs de esta línea (plan_plot.md). fig_polybar es el único que ejercita
# `polybar` (§4.12) y `fill`-SIN-`outlinefill` (relleno que no traza).
# quickstart es el ejemplo del README: está aquí para que la portada del proyecto no
# pueda romperse en silencio (es lo que le pasó al del man, hoy en sintaxis V1 muerta).
# fig1 es el único que ejercita `legend` (§13.9, forma explícita) y el marcador
# compuesto `circle-dot` (⊙, §4.6).
# symbols es el catálogo de los 69 símbolos de map_symbol que salen por el font
# Symbol: es la REFERENCIA de la migración P1 (Symbol -> LM Math), que el golden por
# bytes no puede validar porque todos cambian de fuente por diseño.
# path_sample es el unico que ejercita la familia de muestreo §9 (sample/point_at/
# angle_at) con el flag curve=: los puntos curve=true caen sobre la CURVA, los default
# sobre la ENVOLVENTE; point_at+angle_at colocan una flecha-struct orientada a la tangente.
# tiro_parabolico ejercita la combinación que nada más cubre: cañón como STRUCT
# colocado con at= (§8), `path +=` construyendo la curva punto a punto (§9) con las
# proyecciones a cada eje del mismo lazo, y MALLA AJUSTADA A DATOS (ni grid regular ni
# log — el paso en y es irregular, la trayectoria llega a y<0). Único sin texto.
# turning_points ejercita `smooth` (§9.2, único del corpus), `path +=` usado como
# graficador de funciones, y exp/ln/potencia fraccionaria; es el más pesado en
# cómputo (~4200 iteraciones de cuadratura) y el 2º ejemplo enteramente paramétrico.
EXAMPLES="curvas3 fig1 fig2-1 fig2-5 fig4-1 fig4-4 fig6-4 fig_polybar fill_styles fractal_tree franck_condon line_patterns markers-demo path_sample primitives quickstart rpstest sines symbols texto tiro_parabolico turning_points"

export LC_ALL=C

MODE="${1:-check}"

if [ "$MODE" != "capture" ] && [ "$MODE" != "check" ] && [ "$MODE" != "images" ]; then
    echo "Usage: $0 [capture|check|images]" >&2
    exit 2
fi

if [ ! -x "$MG" ]; then
    echo "error: binary not found or not executable: $MG (run 'make' first)" >&2
    exit 2
fi

# img_source <nombre> -> imprime el .mg fuente de docs/img/<nombre>.svg, o nada.
img_source() {
    if   [ -f "$EXDIR/$1.mg" ];   then echo "$EXDIR/$1.mg"
    elif [ -f "$IMGDIR/$1.mg" ];  then echo "$IMGDIR/$1.mg"
    fi
}

# Modo `images`: regenera la salida publicada y termina. No toca el golden ni
# ejecuta compuertas — es la acción que se toma DESPUÉS de haber verificado con
# `check` que el cambio de salida es el que se quería.
if [ "$MODE" = "images" ]; then
    img_n=0
    for svg in "$IMGDIR"/*.svg; do
        [ -f "$svg" ] || continue
        name="$(basename "$svg" .svg)"
        src="$(img_source "$name")"
        if [ -z "$src" ]; then
            echo "ERROR docs/img/$name.svg no tiene fuente ($name.mg en examples/ ni aquí)" >&2
            exit 1
        fi
        ( cd "$(dirname "$src")" && "$MG" "$(basename "$src")" "$svg" ) >/dev/null 2>&1 || {
            echo "ERROR $name.svg (mg falló al regenerar)" >&2
            exit 1
        }
        echo "regenerado docs/img/$name.svg"
        img_n=$((img_n + 1))
    done
    # La galería es salida publicada igual que los renders, y depende de las dos
    # cosas que este modo bendice (los .svg y el texto de los .mg), así que se
    # regenera aquí y no en `capture`.
    if command -v python3 >/dev/null 2>&1; then
        python3 "$ROOT/tools/galeria.py" || exit 1
    else
        echo "WARN: python3 no encontrado; docs/galeria.html NO se regeneró" >&2
    fi
    echo "---"
    echo "images done: $img_n regenerado(s). Revisa el diff ANTES de commitear: es la cara pública."
    exit 0
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
c3fail_count=0
imgfail_count=0
errfail_count=0
galfail_count=0
err_ok=0

for example in $EXAMPLES; do
    # Capa 3 (paridad entre backends): acumuladores por ejemplo. Se llenan al vuelo
    # dentro del loop de formatos (antes de borrar el tmpdir) y se comparan al salir.
    c3_text_eps=""; c3_text_svg=""; c3_text_pdf=""; c3_filled_lines=0
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

        # Capa 3: extrae las métricas de esta salida ANTES de borrar el tmpdir.
        # Sin herramientas externas: el PDF de libharu no está comprimido, así que
        # los operadores (Tj de texto) son grepables directo, igual que EPS/SVG.
        case "$fmt" in
            eps) c3_text_eps=$(grep -cE '\)[[:space:]]*(show|cshow|rshow|ashow)$' "$outfile") ;;
            svg) c3_text_svg=$(grep -ao '<tspan' "$outfile" | wc -l)
                 c3_filled_lines=$(grep -oE '<path d="M [-0-9.e ]+ L [-0-9.e ]+ " fill="#[0-9a-fA-F]{6}"[^>]*>' "$outfile" | grep -c 'stroke="none"') ;;
            pdf) c3_text_pdf=$(grep -acE '(Tj|TJ)$' "$outfile") ;;
        esac

        rm -rf "$tmpdir"
    done

    # Capa 3: paridad entre backends (independiente del golden y de $MODE, como la
    # compuerta gs). Solo si los tres formatos se renderizaron (si alguno falló, ya
    # lo contó error_count). Ver el bloque de comentario del encabezado.
    if [ -n "$c3_text_eps" ] && [ -n "$c3_text_svg" ] && [ -n "$c3_text_pdf" ]; then
        if [ "$c3_text_eps" != "$c3_text_svg" ] || [ "$c3_text_eps" != "$c3_text_pdf" ]; then
            echo "C3FAIL $example (texto EPS/SVG/PDF = $c3_text_eps/$c3_text_svg/$c3_text_pdf: un backend omite texto)"
            c3fail_count=$((c3fail_count + 1))
        fi
        if [ "$c3_filled_lines" != "0" ]; then
            echo "C3FAIL $example (SVG: $c3_filled_lines línea(s) rellena(s) sin trazo → stroke perdido)"
            c3fail_count=$((c3fail_count + 1))
        fi
    fi
done

# --- Compuerta 4: docs/img al día -------------------------------------------
# El render publicado tiene que coincidir con lo que el compilador produce HOY.
# Independiente del golden y de $MODE (como gs y la Capa 3): en `capture` avisa de
# que falta regenerar, pero NO lo hace — ver el bloque del encabezado. La comparación
# es directa porque la salida SVG ya es determinista y no embebe la ruta.
#
# Se itera sobre docs/img/*.svg, no sobre $EXAMPLES: así también entran las variantes
# (con su .mg al lado) y ningún render publicado puede quedarse sin vigilar.
for svg in "$IMGDIR"/*.svg; do
    [ -f "$svg" ] || continue
    name="$(basename "$svg" .svg)"
    src="$(img_source "$name")"
    if [ -z "$src" ]; then
        echo "IMGFAIL docs/img/$name.svg (huérfano: no hay $name.mg en examples/ ni junto al render)"
        imgfail_count=$((imgfail_count + 1))
        continue
    fi
    imgtmp="$(mktemp -d)"
    if ( cd "$(dirname "$src")" && "$MG" "$(basename "$src")" "$imgtmp/$name.svg" ) >/dev/null 2>&1 \
       && [ -s "$imgtmp/$name.svg" ]; then
        if ! diff -q "$svg" "$imgtmp/$name.svg" >/dev/null 2>&1; then
            echo "IMGFAIL docs/img/$name.svg (rancio: no es lo que compila hoy; './run.sh images' lo regenera)"
            imgfail_count=$((imgfail_count + 1))
        fi
    else
        echo "IMGFAIL docs/img/$name.svg (mg falló al compilar su fuente $src)"
        imgfail_count=$((imgfail_count + 1))
    fi
    rm -rf "$imgtmp"
done

# --- Compuerta 6: la GALERÍA al día (galfail) --------------------------------
# docs/galeria.html es salida publicada y DERIVADA (la sirve GitHub Pages), y lo
# que la vuelve rancia no es tocar el motor sino editar un COMENTARIO: la página
# lleva incrustados el encabezado y el código fuente completo de cada ejemplo. O
# sea que ninguna de las otras cinco compuertas puede verlo — imgfail vigila los
# renders, y un cambio de comentario no mueve un solo byte de ningún .svg.
#
# Se compara regenerando en memoria (galeria.py --check), no por mtime. Se omite
# con aviso si no hay python3, igual que gs.
if command -v python3 >/dev/null 2>&1; then
    if ! galout="$(python3 "$ROOT/tools/galeria.py" --check 2>&1)"; then
        echo "GALFAIL docs/galeria.html ($galout)"
        galfail_count=$((galfail_count + 1))
    fi
else
    echo "WARN: python3 no encontrado; se omite la compuerta de la galería"
fi

# --- Compuerta 5: pruebas NEGATIVAS (errfail) --------------------------------
# Las otras cuatro compuertas miran salida EXITOSA, así que los ~150 caminos de
# error del compilador (evalError/parseError/exit) no tenían una sola prueba. Y su
# regresión natural es la peor: volver al SILENCIO — la familia de bugs más
# recurrente del proyecto (coordenadas sobrantes descartadas, el bool de
# emitStyleAttr ignorado, el punto muerto de fig1...). Un diagnóstico que deja de
# dispararse no mueve un byte de ningún golden.
#
# Cada test/errors/*.mg declara EN SÍ MISMO lo que espera (va en git, a diferencia
# de test/golden, y no hay dos listas que desincronizar):
#     % EXPECT: <fragmento que debe aparecer en stderr>
#     % EXPECT_AT: <línea>:<columna>     (opcional)
#
# Se exigen TRES cosas, y cada una caza algo distinto:
#   (a) exit == 1 EXACTO, no "!= 0": un segfault también "falla". Ésta es la
#       aserción que caza el modo de falla de max_depth antes de su guarda (139).
#   (b) el fragmento aparece: que el diagnóstico siga existiendo.
#   (c) NO se creó el archivo de salida: la política de que un documento roto no
#       produce salida (la razón de que evalError e include sean fatales).
#
# Se compara un FRAGMENTO y no el mensaje completo a propósito: los mensajes son
# prosa que se va a reescribir, y un golden por bytes castigaría justo las mejoras
# de redacción. El fragmento fija la AFIRMACIÓN (qué constructo, qué está mal) y
# deja libre la forma.
#
# Corre en check y en capture (como gs y la Capa 3): no depende de bendecir nada.
for case in "$ERRDIR"/*.mg; do
    [ -f "$case" ] || continue
    name="$(basename "$case" .mg)"
    want="$(sed -n 's/^% EXPECT: //p' "$case" | head -1)"
    want_at="$(sed -n 's/^% EXPECT_AT: //p' "$case" | head -1)"
    if [ -z "$want" ]; then
        echo "ERRFAIL $name (el fixture no declara '% EXPECT: ...')"
        errfail_count=$((errfail_count + 1))
        continue
    fi
    errtmp="$(mktemp -d)"
    ( cd "$ERRDIR" && "$MG" "$name.mg" "$errtmp/out.svg" ) >/dev/null 2>"$errtmp/stderr"
    code=$?
    if [ "$code" -eq 0 ]; then
        echo "ERRFAIL $name (COMPILÓ: se esperaba que fallara con «$want»)"
        errfail_count=$((errfail_count + 1))
    elif [ "$code" -ne 1 ]; then
        # 139 = SIGSEGV, 134 = abort: falla sucia. Es exactamente lo que hacía una
        # recursión sin max_depth, y lo que un "!= 0" habría dado por bueno.
        echo "ERRFAIL $name (salió con $code, no 1: no abortó limpio — ¿señal?)"
        errfail_count=$((errfail_count + 1))
    elif ! grep -qF "$want" "$errtmp/stderr"; then
        echo "ERRFAIL $name (falló, pero sin decir «$want»)"
        echo "        dijo: $(head -1 "$errtmp/stderr")"
        errfail_count=$((errfail_count + 1))
    elif [ -n "$want_at" ] && ! grep -qF " $want_at:" "$errtmp/stderr"; then
        echo "ERRFAIL $name (mensaje correcto pero no señala $want_at)"
        echo "        dijo: $(head -1 "$errtmp/stderr")"
        errfail_count=$((errfail_count + 1))
    elif [ -e "$errtmp/out.svg" ]; then
        echo "ERRFAIL $name (abortó con mensaje PERO dejó archivo de salida)"
        errfail_count=$((errfail_count + 1))
    else
        err_ok=$((err_ok + 1))
    fi
    rm -rf "$errtmp"
done

echo "---"
if [ "$MODE" = "capture" ]; then
    echo "capture done. errors: $error_count psfail: $psfail_count c3fail: $c3fail_count imgfail: $imgfail_count errfail: $errfail_count galfail: $galfail_count"
    if [ "$error_count" -ne 0 ] || [ "$psfail_count" -ne 0 ] || [ "$c3fail_count" -ne 0 ] || [ "$imgfail_count" -ne 0 ] || [ "$errfail_count" -ne 0 ] || [ "$galfail_count" -ne 0 ]; then
        exit 1
    fi
    exit 0
else
    echo "check summary: ok=$ok_count fail=$fail_count error=$error_count psfail=$psfail_count c3fail=$c3fail_count imgfail=$imgfail_count errfail=$errfail_count galfail=$galfail_count (err_ok=$err_ok)"
    if [ "$fail_count" -ne 0 ] || [ "$error_count" -ne 0 ] || [ "$psfail_count" -ne 0 ] || [ "$c3fail_count" -ne 0 ] || [ "$imgfail_count" -ne 0 ] || [ "$errfail_count" -ne 0 ] || [ "$galfail_count" -ne 0 ]; then
        exit 1
    fi
    exit 0
fi
