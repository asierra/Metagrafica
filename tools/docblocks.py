#!/usr/bin/env python3
"""
Compila los bloques ```octave de la documentación. Novena compuerta de `test/run.sh`.

POR QUÉ EXISTE
--------------
Las otras ocho compuertas vigilan la SALIDA del compilador. Ninguna mira lo que la
documentación AFIRMA, y una afirmación falsa en `docs/referencia.md` es peor que un bug: es
un bug que el lector copia con confianza. Se encontró el 2026-07-29, buscando qué contexto
necesita un agente externo para escribir una figura: el ⚠️ de §10 declaraba que la forma de
partir de un trayecto que ya tienes es la de paréntesis —`smooth(&nodos)`— «igual que en el
resto del álgebra», y esa forma NO compila; el compilador exige el bloque literal. El aviso
era cierto para las primitivas que CONSUMEN un trayecto (`polyline`, `bezier`) y falso justo
para los generadores, que era el párrafo al que estaba pegado.

Un humano tropieza y desconfía del documento. Un modelo de lenguaje obedece, así que la
documentación es su única fuente de verdad y un error ahí se convierte en código roto con
toda seguridad. Esta compuerta cierra eso.

CÓMO DECLARA CADA BLOQUE LO QUE ESPERA
--------------------------------------
En el documento, nunca en una lista aquí dentro (misma política que `test/errors/*.mg`: dos
listas que mantener se desincronizan). Va un comentario HTML —invisible al renderizar—
inmediatamente antes de la cerca:

    <!-- mg-noexec: razón -->   el bloque NO es código MG (notación, firmas). Se omite.
    <!-- mg-expect-error -->    el bloque DEBE fallar: es un contraejemplo ❌ deliberado.
                                Si algún día compila, también falla la compuerta — un
                                contraejemplo que dejó de serlo enseña lo contrario.

FRAGMENTOS
----------
Muchos bloques usan una `struct` o un `path` que el texto definió antes y no repiten. Esos
no pueden compilar solos, y no hace falta marcarlos: `struct no definida` y `path no
definido` son errores de EVALUACIÓN, o sea que el parseo ya pasó — que es justo lo que esta
compuerta comprueba. Se cuentan aparte y no fallan.

Solo biblioteca estándar, como `arcparity.py`.
"""

import pathlib
import re
import shutil
import subprocess
import sys
import tempfile

# Errores que prueban que el bloque parseó bien y solo le faltaba contexto del texto.
# Los tres son de EVALUACIÓN: si el parseo hubiera fallado, el error sería de sintaxis y
# se reportaría en su lugar. Ese es el argumento que hace segura la clasificación.
FRAGMENTO = re.compile(r"Error de evaluación: "
                       r"(struct no definida|path no definido|variable no definida)")

# Lienzo mínimo para los bloques que son un cuerpo suelto (sin `display_size`).
PRELUDIO = "display_size 8 6\nworld_window 0 10 0 8\n"

BLOQUE = re.compile(r"(?:<!--\s*(mg-noexec|mg-expect-error)(?::\s*([^>]*?))?\s*-->\s*\n)?"
                    r"```octave\n(.*?)```", re.S)


def compila(mg, dir_tmp, codigo):
    """(ok, mensaje). Prueba tal cual y, si no, con el lienzo mínimo delante."""
    ultimo = ""
    for fuente in (codigo, PRELUDIO + codigo):
        f = dir_tmp / "bloque.mg"
        f.write_text(fuente, encoding="utf-8")
        r = subprocess.run([mg, str(f), str(dir_tmp / "bloque.svg")],
                           capture_output=True, text=True)
        if r.returncode == 0:
            return True, ""
        ultimo = (r.stderr + r.stdout).strip().split("\n")[0]
    return False, ultimo


def revisa(mg, ruta, dir_tmp):
    texto = ruta.read_text(encoding="utf-8")
    ok = frag = omit = neg = 0
    fallos = []
    for m in BLOQUE.finditer(texto):
        marca, razon, codigo = m.group(1), (m.group(2) or "").strip(), m.group(3)
        linea = texto.count("\n", 0, m.start(3)) + 1
        if marca == "mg-noexec":
            omit += 1
            continue
        bien, msg = compila(mg, dir_tmp, codigo)
        if marca == "mg-expect-error":
            if bien:
                fallos.append((linea, "el contraejemplo YA COMPILA: dejó de enseñar lo que decía"))
            else:
                neg += 1
            continue
        if bien:
            ok += 1
        elif FRAGMENTO.search(msg):
            frag += 1                     # parseó; solo le falta contexto del texto
        else:
            fallos.append((linea, msg))
    return ok, frag, omit, neg, fallos


def main(argv):
    if len(argv) < 2:
        print("uso: docblocks.py <archivo.md> [...]", file=sys.stderr)
        return 2
    raiz = pathlib.Path(__file__).resolve().parent.parent
    mg = raiz / "bin" / "mg"
    if not mg.exists():
        print(f"docblocks: no existe {mg} — corre `make` primero", file=sys.stderr)
        return 2

    total_fallos = 0
    with tempfile.TemporaryDirectory() as d:
        dir_tmp = pathlib.Path(d)
        # Las bibliotecas al lado del bloque: así el `include "pseudo3d.mg"` que documenta
        # la referencia (nombre a secas, sin ruta) se resuelve, y de paso se comprueba.
        for lib in (raiz / "lib").glob("*.mg"):
            shutil.copy(lib, dir_tmp / lib.name)
        for nombre in argv[1:]:
            ruta = pathlib.Path(nombre)
            if not ruta.is_absolute():
                ruta = raiz / ruta
            if not ruta.exists():
                print(f"docblocks: no existe {ruta}", file=sys.stderr)
                return 2
            ok, frag, omit, neg, fallos = revisa(mg, ruta, dir_tmp)
            estado = "FALLA" if fallos else "ok"
            print(f"{estado:5} {ruta.name}: compilan={ok} fragmentos={frag} "
                  f"notación={omit} contraejemplos={neg} fallos={len(fallos)}")
            for linea, msg in fallos:
                print(f"      {ruta.name}:{linea}: {msg}")
            total_fallos += len(fallos)
    return 1 if total_fallos else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
