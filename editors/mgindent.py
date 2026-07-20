#!/usr/bin/env python3
"""Reindenta un archivo MetaGráfica (.mg) por profundidad de llaves { }.

    python3 mgindent.py archivo.mg           # imprime reindentado a stdout
    python3 mgindent.py -i archivo.mg        # reescribe en el lugar (atómico)
    python3 mgindent.py --check archivo.mg   # sale 1 si NO estaba bien indentado

`-i` escribe de forma ATÓMICA (temporal + os.replace) y solo si algo cambia, así
que una interrupción nunca deja el archivo truncado. NUNCA uses `> archivo.mg`: el
shell trunca el archivo antes de que el script lo lea. Redirige a OTRO nombre.

Regla: la sangría de cada línea = (profundidad de llaves) × 4 espacios. Solo se
toca la sangría INICIAL; el contenido de la línea (incluida la alineación interna
con varios espacios, p. ej. `a = 1   b = 2`) se deja intacto.

Lo que hace robusto esto en MG y que un contador ingenuo de `{}` erra:
  · Las cadenas no cuentan: `text("{/bx}")`, `$x^{2x}$` traen `{}` de markup (§14).
    MG no tiene escapes en cadenas (STRING = "[^"]*"), así que basta alternar en ".
  · Los comentarios no cuentan: todo tras `%` fuera de cadena se ignora — en el
    corpus hay líneas comentadas con una `}` suelta (fig16-10) que si no reventarían.
  · Un bloque de coordenadas en UNA línea (`polyline { 0 0  1 1 }`) es net-cero y
    no altera la profundidad de las líneas siguientes: sale bien solo.

NO reindenta dentro de ( ) ni [ ]: en MG esos no abren bloques visuales, solo
suprimen saltos de línea. Únicamente { } manda la sangría, que es como se lee.
"""
import os
import sys
import tempfile

UNIT = "    "  # 4 espacios (convención del corpus)


def write_atomic(path, text):
    """Escribe `text` en `path` de forma atómica: a un temporal en el MISMO
    directorio y luego os.replace (rename atómico en POSIX). Una interrupción a
    media escritura deja el original intacto en vez de un archivo truncado.
    Conserva los permisos del original."""
    d = os.path.dirname(os.path.abspath(path))
    fd, tmp = tempfile.mkstemp(dir=d, prefix=".mgindent-", suffix=".tmp")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as f:
            f.write(text)
            f.flush()
            os.fsync(f.fileno())        # el contenido llega a disco antes del rename
        try:                            # replica el modo del original si existe
            os.chmod(tmp, os.stat(path).st_mode)
        except OSError:
            pass
        os.replace(tmp, path)           # atómico: el original nunca queda a medias
    except BaseException:
        try:
            os.unlink(tmp)              # limpia el temporal si algo falló antes del replace
        except OSError:
            pass
        raise


def braces(line):
    """(aperturas, cierres, cierres_al_inicio) contando solo { } de CÓDIGO.

    Descarta lo que va dentro de "cadenas" y tras `%` (comentario). cierres_al_inicio
    = número de `}` con que empieza la línea (tras espacios), antes de cualquier otro
    token — son los que dedentan la línea actual (`}` de cierre, o `} else {`).
    """
    opens = closes = 0
    lead_close = 0
    seen_token = False           # ¿ya apareció algo que no sea '}' inicial o espacio?
    in_str = False
    for ch in line:
        if in_str:
            if ch == '"':
                in_str = False
            continue
        if ch == '"':
            in_str = True
            seen_token = True
            continue
        if ch == '%':            # comentario hasta fin de línea
            break
        if ch == '{':
            opens += 1
            seen_token = True
        elif ch == '}':
            closes += 1
            if not seen_token:
                lead_close += 1  # `}` inicial: dedenta la línea actual
        elif not ch.isspace():
            seen_token = True
    return opens, closes, lead_close


def reindent(text):
    out = []
    depth = 0
    for raw in text.split("\n"):
        stripped = raw.strip()
        if not stripped:
            out.append("")       # línea en blanco: sin espacios sobrantes
            continue
        opens, closes, lead = braces(raw)
        level = max(0, depth - lead)
        out.append(UNIT * level + stripped)
        depth = max(0, depth + opens - closes)
    return "\n".join(out)


def main(argv):
    inplace = "-i" in argv
    check = "--check" in argv
    paths = [a for a in argv[1:] if not a.startswith("-")]
    if not paths:
        sys.exit("uso: mgindent.py [-i|--check] archivo.mg ...")
    rc = 0
    for p in paths:
        with open(p, encoding="utf-8") as f:
            src = f.read()
        # Preserva un único salto final si lo había.
        trailing = "\n" if src.endswith("\n") else ""
        body = src[:-1] if trailing else src
        fixed = reindent(body) + trailing
        if check:
            if fixed != src:
                sys.stderr.write("mal indentado: %s\n" % p)
                rc = 1
        elif inplace:
            if fixed != src:            # no reescribas si nada cambia (mtime intacto)
                write_atomic(p, fixed)
        else:
            sys.stdout.write(fixed)
    sys.exit(rc)


if __name__ == "__main__":
    main(sys.argv)
