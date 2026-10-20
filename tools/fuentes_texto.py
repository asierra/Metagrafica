#!/usr/bin/env python3
"""Genera include/font_nimbus_ttf.h: las cuatro Nimbus Roman en subset, para el PDF.

    python3 tools/fuentes_texto.py [DIR_OTF]   # default: /usr/share/fonts/opentype/urw-base35

Por qué existe (2026-10-19): el PDF nombraba las base-14 Times-*, que libharu nunca
incrusta, y los sistemas de envío de las revistas rechazan fuentes sin incrustar.
Nimbus Roman (URW) es el clon métrico de Times: sus anchos coinciden EXACTAMENTE con los
de las AFM de Adobe que `mg` usa para medir el texto (src/text.cpp; medido glifo por
glifo), así que incrustarla no mueve ninguna caja de texto. Liberation Serif, la otra
candidata libre, difiere justo en ±, µ, ·, ÷ y ¯.

Qué hace con cada cara:
  1. subset al repertorio de CP1252, que es todo lo que el PDF de `mg` puede pedir (el
     texto viaja con WinAnsiEncoding; las ranuras 1..27 se traducen a CP1252 antes);
  2. convierte los contornos CFF (cúbicos) a glyf (cuadráticos) con cu2qu, tolerancia
     1/1000 em: libharu solo carga y recorta TrueType;
  3. emite un arreglo C por cara. libharu vuelve a recortar al guardar, así que a la
     figura solo llegan los glifos que usa (unos 3-6 KB por cara).

Los OTF de origen NO están en el repo: vienen del paquete `fonts-urw-base35` (Debian/
Ubuntu) o de https://github.com/ArtifexSoftware/urw-base35-fonts. El header generado SÍ
se commitea, como los mapas de lib/. Salida determinista (sin marca de tiempo), así que
regenerar con los mismos OTF da los mismos bytes.

Licencia de las fuentes: AGPL-3 con excepción de fuente («permission is granted to
include these font programs in a Postscript or PDF file ... regardless of the conditions
or license applying to the document itself»). Va copiada en el header.

Requiere fontTools. No es parte de la compilación de `mg`.
"""
import io
import os
import sys

from fontTools import subset
from fontTools.pens.cu2quPen import Cu2QuPen
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.ttLib import TTFont, newTable

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, 'include', 'font_nimbus_ttf.h')
DEFAULT_DIR = '/usr/share/fonts/opentype/urw-base35'

# Orden = índice que usa PDFDisplay (romana, itálica, negrita, negrita itálica).
CARAS = [
    ('regular', 'NimbusRoman-Regular', 'Times-Roman'),
    ('italic', 'NimbusRoman-Italic', 'Times-Italic'),
    ('bold', 'NimbusRoman-Bold', 'Times-Bold'),
    ('bolditalic', 'NimbusRoman-BoldItalic', 'Times-BoldItalic'),
]


def repertorio():
    cps = set()
    for b in range(0x20, 0x100):
        try:
            cps.add(ord(bytes([b]).decode('cp1252')))
        except UnicodeDecodeError:      # 0x81, 0x8D, 0x8F, 0x90, 0x9D: sin asignar
            pass
    return cps


def a_truetype(f):
    gs = f.getGlyphSet()
    glyf = newTable('glyf')
    glyf.glyphOrder = f.getGlyphOrder()
    glyf.glyphs = {}
    for g in glyf.glyphOrder:
        pen = TTGlyphPen(gs)
        gs[g].draw(Cu2QuPen(pen, 1.0, reverse_direction=True))
        glyf.glyphs[g] = pen.glyph()
    f['glyf'] = glyf
    f['loca'] = newTable('loca')
    del f['CFF ']
    maxp = newTable('maxp')
    maxp.tableVersion = 0x00010000
    for k in ('maxZones', 'maxTwilightPoints', 'maxStorage', 'maxFunctionDefs',
              'maxInstructionDefs', 'maxStackElements', 'maxSizeOfInstructions',
              'maxComponentElements', 'maxComponentDepth'):
        setattr(maxp, k, 0)
    maxp.maxZones = 1
    f['maxp'] = maxp
    f['post'].formatType = 3.0          # sin nombres de glifo: el PDF va por cmap
    f['head'].indexToLocFormat = 0
    f['head'].glyphDataFormat = 0
    f.sfntVersion = '\0\1\0\0'


def cara(path):
    f = TTFont(path, recalcTimestamp=False)
    opt = subset.Options()
    opt.layout_features = []            # sin GSUB/GPOS: libharu no los usa
    opt.notdef_outline = True
    s = subset.Subsetter(opt)
    s.populate(unicodes=repertorio())
    s.subset(f)
    a_truetype(f)
    buf = io.BytesIO()
    f.save(buf)
    return buf.getvalue(), len(f.getGlyphOrder())


def arreglo(nombre, data):
    lines = []
    for i in range(0, len(data), 16):
        lines.append('  ' + ','.join(str(b) for b in data[i:i + 16]) + ',')
    return ('static const unsigned char %s[] = {\n%s\n};\n'
            'static const unsigned int %s_len = %d;\n'
            % (nombre, '\n'.join(lines), nombre, len(data)))


def main(argv):
    d = argv[1] if len(argv) > 1 else DEFAULT_DIR
    partes, resumen = [], []
    for clave, archivo, base14 in CARAS:
        path = os.path.join(d, archivo + '.otf')
        if not os.path.exists(path):
            print('fuentes_texto: no encuentro %s (paquete fonts-urw-base35)' % path,
                  file=sys.stderr)
            return 1
        data, n = cara(path)
        partes.append(arreglo('g_nimbus_%s_ttf' % clave, data))
        resumen.append('//   g_nimbus_%-11s %-22s en lugar de %-16s %6d bytes, %d glifos'
                       % (clave, archivo, base14, len(data), n))
    hdr = '''#ifndef MG_FONT_NIMBUS_TTF_H
#define MG_FONT_NIMBUS_TTF_H
// GENERADO por tools/fuentes_texto.py — no editar a mano. Regenerar con:
//   python3 tools/fuentes_texto.py
// a partir de los OTF de fonts-urw-base35 (no están en el repo).
//
// Nimbus Roman (URW), clon métrico de Times, en subset CP1252 y convertida a
// TrueType, para que el PDF INCRUSTE el texto en vez de nombrar las base-14.
// Mismos anchos de avance que las AFM de Adobe con que mide src/text.cpp.
%s
//
// Copyright 2014 (URW)++ Design & Development; 2015 URW Software.
// Licencia: GNU AGPL-3 con la siguiente excepción de fuente:
//   As a special exception, permission is granted to include these font
//   programs in a Postscript or PDF file that consists of a document that
//   contains text to be displayed or printed using this font, regardless
//   of the conditions or license applying to the document itself.
// Fuente original: https://github.com/ArtifexSoftware/urw-base35-fonts
''' % '\n'.join(resumen)
    with open(OUT, 'w') as f:
        f.write(hdr + '\n' + '\n'.join(partes) + '\n#endif\n')
    print('\n'.join(r[3:] for r in resumen))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
