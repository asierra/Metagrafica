#!/usr/bin/env python3
"""Infla los flujos FlateDecode de un PDF de MetaGráfica, para poder leer sus operadores.

    tools/pdfinflate.py entrada.pdf salida.pdf

Desde el 2026-10-19 `mg` escribe el PDF comprimido (`HPDF_COMP_ALL`: contenido de
página, fuente embebida y su CIDToGIDMap), y la Capa 3 de `test/run.sh` —que cuenta
`Tj` y `/ShN sh` con grep y que alimenta a `arcparity.py` y `fontparity.py`— necesita
los operadores en claro. Esta herramienta deja el archivo como lo escribía libharu sin
compresión: borra la línea `/Filter [ /FlateDecode ]` y sustituye cada flujo por su
contenido inflado. Medido al introducirla: sobre los 33 ejemplos, la salida es
byte-idéntica a la de antes de comprimir salvo los valores de los objetos `/Length`.

⚠️ El resultado NO es un PDF válido (la tabla xref y las longitudes quedan con los
valores del comprimido): es un archivo para leer con grep, no para abrir.

El final de cada flujo lo da el propio zlib (`unused_data`), no `/Length` —que libharu
escribe como referencia indirecta— ni una búsqueda de `endstream`, que puede aparecer
por azar dentro de los bytes comprimidos. Solo biblioteca estándar.
"""
import re
import sys
import zlib

# libharu escribe el filtro en su propia línea, justo antes de cerrar el diccionario
# del flujo. Si algún día cambia ese formato, se aborta en vez de devolver el archivo
# a medio inflar: una Capa 3 que lee binario comprimido no cuenta nada y sale verde.
FILTER = re.compile(rb'/Filter \[ /FlateDecode \]\n(>>\nstream\r?\n)')


def inflate(data):
    out = []
    pos = 0
    n = 0
    for m in FILTER.finditer(data):
        if m.start() < pos:          # dentro de un flujo ya consumido
            continue
        out.append(data[pos:m.start()])
        out.append(m.group(1))
        start = m.end()
        # Página en blanco (`curvas3`): libharu declara el filtro pero no escribe
        # ni un byte de zlib, ni siquiera la cabecera. Se deja vacío, como estaba.
        if data.startswith(b'\nendstream', start):
            pos = start
            n += 1
            continue
        d = zlib.decompressobj()
        out.append(d.decompress(data[start:]))
        if not d.eof:
            raise ValueError('flujo FlateDecode truncado en el byte %d' % start)
        pos = len(data) - len(d.unused_data)
        n += 1
    out.append(data[pos:])
    res = b''.join(out)
    if b'/FlateDecode' in res:
        raise ValueError('quedó un /FlateDecode con un formato que no se reconoce')
    return res, n


def main(argv):
    if len(argv) != 3:
        print(__doc__.strip().splitlines()[2].strip(), file=sys.stderr)
        return 2
    with open(argv[1], 'rb') as f:
        data = f.read()
    try:
        res, _ = inflate(data)
    except (ValueError, zlib.error) as e:
        print('pdfinflate: %s: %s' % (argv[1], e), file=sys.stderr)
        return 1
    with open(argv[2], 'wb') as f:
        f.write(res)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
