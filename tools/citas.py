#!/usr/bin/env python3
"""
Verifica que los bloques de código que CITAN un archivo del árbol sigan siendo lo que ese
archivo dice. Décima compuerta de `test/run.sh` (novena CLASE de fallo).

POR QUÉ EXISTE, SI YA ESTÁ `docblocks.py`
-----------------------------------------
Son fallos disjuntos, y el de aquí es invisible para aquélla. `docblocks.py` compila lo que
la documentación enseña y caza lo que el lenguaje RECHAZA. Una cita rancia compila
perfectamente —es MetaGráfica válida, solo que ya no es la del archivo—, así que aunque
`docblocks.py` cubriera los README no vería nada.

Nació el 2026-08-05, y del peor modo posible: arreglando un aborto de
`examples/franck_condon.mg` se cambió una línea que los DOS README citaban textualmente
como ejemplo de que «los extremos de un nivel no son coordenadas, son la fórmula». La
figura y su código siguieron compilando; las ocho compuertas siguieron en verde; y la
portada del proyecto quedó enseñando una línea que ya no existía. Nadie lo habría notado
hasta que alguien la copiara.

⚠️ Es la única compuerta que compara la DOCUMENTACIÓN contra el ÁRBOL. Las demás comparan
salida contra golden (bendecible), o salida contra salida. Aquí no hay nada que bendecir:
o la cita está en el archivo o no está.

CÓMO DECLARA UN BLOQUE QUE ES UNA CITA
--------------------------------------
En el documento, nunca en una lista aquí dentro (misma política que `test/errors/*.mg` y
que `docblocks.py`). Un comentario HTML —invisible al renderizar— antes de la cerca:

    <!-- mg-cita: examples/franck_condon.mg -->

Un bloque sin marcador no se revisa: la mayoría son fragmentos ilustrativos de sintaxis que
no salen de ningún archivo. Para que el marcador no se olvide, un bloque SIN marcar cuyas
líneas resulten estar todas en algún `.mg` del árbol sale como AVISO (no falla): es casi
seguro una cita a la que le falta la declaración.

⚠️ Y por eso el marcador es explícito y no se detecta solo: una cita se detectaría por
coincidir con el archivo, o sea que el día que se pudre dejaría de parecer una cita y la
compuerta se apagaría sola, justo cuando tiene que sonar.

QUÉ SE COMPARA
--------------
No las líneas: los README reformatean y tienen derecho a hacerlo. Se comprueba que cada
línea del bloque aparezca en el archivo, **en orden**, sobre el texto normalizado (sin
comentarios y con los espacios colapsados). Eso tolera las tres libertades legítimas de una
cita y no tolera la cuarta:

  · re-indentar                    ✓ (los espacios se colapsan)
  · re-partir una línea larga      ✓ (la búsqueda es por subcadena, no por línea:
                                      `quickstart` parte en dos el `entry("Experimental")`)
  · saltarse líneas de en medio    ✓ (el orden se respeta, la contigüidad no: el README
                                      cita E, s, rm y rp, y omite el Es de en medio)
  · cambiar lo que el archivo dice ✗ — que es lo único que hay que cazar.

Solo biblioteca estándar, como `arcparity.py` y `docblocks.py`.
"""

import pathlib
import re
import sys

BLOQUE = re.compile(r"(?:<!--\s*mg-cita:\s*([^>]*?)\s*-->\s*\n)?```octave\n(.*?)```", re.S)


def sin_comentario(linea):
    """Quita el comentario `%` final. Respeta las comillas: `text("50%")` no es comentario."""
    fuera, i = True, 0
    while i < len(linea):
        c = linea[i]
        if c == '"':
            fuera = not fuera
        elif c == "%" and fuera:
            return linea[:i]
        i += 1
    return linea


def normaliza(texto):
    """Texto -> una sola línea sin comentarios y con los espacios colapsados."""
    partes = [sin_comentario(l).strip() for l in texto.splitlines()]
    return " ".join(" ".join(p.split()) for p in partes if p)


def revisa(bloque, fuente):
    """Devuelve la primera línea del bloque que NO está en la fuente (o None si todas)."""
    aguja = normaliza(fuente)
    desde = 0
    for linea in bloque.splitlines():
        pedazo = normaliza(linea)
        if not pedazo:
            continue
        pos = aguja.find(pedazo, desde)
        if pos < 0:
            return linea.strip()
        desde = pos + len(pedazo)      # el orden importa; la contigüidad no
    return None


def main(argv):
    raiz = pathlib.Path(__file__).resolve().parent.parent
    docs = argv[1:]
    if not docs:
        print("uso: citas.py <doc.md> [doc.md …]", file=sys.stderr)
        return 2

    # Para el aviso de «cita sin marcar». Se lee una vez y se reusa.
    arbol = {p: p.read_text(encoding="utf-8")
             for d in ("examples", "lib") for p in (raiz / d).glob("*.mg")}

    fallos, avisos, revisados = [], [], 0
    for nombre in docs:
        ruta = raiz / nombre
        if not ruta.exists():
            fallos.append(f"{nombre}: no existe")
            continue
        for marcador, bloque in BLOQUE.findall(ruta.read_text(encoding="utf-8")):
            if not marcador:
                # Sin marcador: solo se avisa si PARECE una cita. Un bloque de tres
                # líneas triviales coincide con cualquier cosa, así que se exige algo
                # de sustancia antes de molestar.
                cuerpo = [l for l in bloque.splitlines() if normaliza(l)]
                if len(cuerpo) >= 4:
                    for p, texto in arbol.items():
                        if revisa(bloque, texto) is None:
                            avisos.append(f"{nombre}: hay un bloque que coincide entero con "
                                          f"{p.relative_to(raiz)} y no lo declara; "
                                          f"añade <!-- mg-cita: {p.relative_to(raiz)} -->")
                            break
                continue

            revisados += 1
            fuente = raiz / marcador
            if not fuente.exists():
                fallos.append(f"{nombre}: cita a {marcador}, que no existe")
                continue
            perdida = revisa(bloque, fuente.read_text(encoding="utf-8"))
            if perdida is not None:
                fallos.append(f"{nombre}: la cita a {marcador} está RANCIA.\n"
                              f"    esta línea ya no está en el archivo (o quedó fuera de orden):\n"
                              f"        {perdida}\n"
                              f"    re-cita del archivo, o corrige el archivo si el que se "
                              f"equivocó fue él.")

    for a in avisos:
        print(f"AVISO {a}", file=sys.stderr)
    for f in fallos:
        print(f)
    if fallos:
        return 1
    print(f"citas: {revisados} bloque(s) al día en {len(docs)} documento(s)")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
