#!/usr/bin/env python3
"""Paridad de CARA tipográfica entre los tres backends (Capa 3).

    tools/fontparity.py fig.eps fig.svg fig.pdf

Silencioso si las tres salidas dibujan cada trozo de texto con la misma cara;
imprime el primer desacuerdo y sale con 1 si no. Pensado para correr desde
test/run.sh, como tools/arcparity.py.

POR QUÉ EXISTE
--------------
La invariante (a) de la Capa 3 CUENTA operaciones de texto —EPS(show) ==
SVG(<tspan>) == PDF(Tj)— y con eso caza el rótulo en blanco. Lo que no compara es
con qué CARA se dibujó cada una: un backend puede sacar el mismo texto, en el mismo
sitio, en otra fuente, y las otras compuertas siguen verdes. Pasó dos veces en once
días, y las dos con la misma firma —EPS y PDF mal, SVG bien—:

  - 2026-08-19: un run `$…$` dejaba LM Math puesta para el resto del documento.
    Salió PUBLICADO (la leyenda de `quickstart` en itálica, la marca «1» del eje
    log de `fig6-4`). SVG estaba bien por accidente —no tiene `dev_face`— y era
    justo el único formato que publica docs/img, así que imgfail tampoco lo vio.
  - 2026-08-30: un cambio de `font_size` borraba la cara del documento y todo lo
    posterior salía en Times-Roman.

Como arcparity, NO compara contra un golden sino backend contra backend, así que
no hay nada que bendecir: `capture` no puede apagarla. Y compara los TRES: la
firma de esta familia es que dos coinciden entre sí.

CÓMO
----
Los tres emiten el texto en el MISMO orden y, por la invariante (a), el mismo
número de trozos, así que se alinean por posición. De cada backend se saca la
secuencia de caras con que se dibujó cada trozo, siguiendo el estado de la
fuente a través de la pila de estado gráfico (gsave/grestore en EPS, q/Q en PDF:
en los dos la fuente es parte del estado que se restaura). Cada cara se reduce a
un vocabulario común:

    math                   LM Math, en cualquiera de sus nombres (LMMath,
                           LMMathSym, LatinModernMath-Regular, 'MGMath')
    roman|sans|mono        + «-bold» / «-italic» (italic = Italic u Oblique)
    symbol                 el Symbol base-14: en PDF es solo el RESPALDO degradado
                           de cuando el subset de LM Math no cargó, así que
                           verlo en un Tj ya es un desacuerdo

ALCANCE: la cara, no el tamaño. El tamaño tiene otra semántica en cada backend
(EPS lo hornea en `scalefont`, SVG lo pone en el <text>) y no es la clase de fallo
que motivó esto. Una compuerta que promete más de lo que mide es peor que ninguna.
"""

import re
import sys

# Mismo reconocimiento de "operación de texto" que la invariante (a) en
# test/run.sh; si uno cambia, el otro también, o las secuencias dejan de alinear.
EPS_TEXT = re.compile(r'\)\s*(show|cshow|rshow|ashow)$')
EPS_FONT = re.compile(r'^/(\S+) findfont \S+ scalefont setfont$')
PDF_TEXT = re.compile(r'(Tj|TJ)$')
PDF_FONT = re.compile(r'^/(\S+) \S+ Tf$')


def canon(name):
    """Nombre de fuente de cualquier backend → vocabulario común."""
    n = name.strip().strip("'\"").lower()
    n = re.sub(r'^[a-z]{6}\+', '', n)           # prefijo de subset del PDF (HPDFAA+)
    if 'lmmath' in n or 'latinmodernmath' in n or n == 'mgmath':
        return 'math'
    if n.startswith('symbol'):
        return 'symbol'
    if n.startswith('iso'):                     # reencodificadas del EPS
        n = n[3:]
    if 'times' in n:
        fam = 'roman'
    elif 'helvetica' in n or 'arial' in n:
        fam = 'sans'
    elif 'courier' in n:
        fam = 'mono'
    else:
        return '?' + name
    if 'bold' in n:
        fam += '-bold'
    if 'italic' in n or 'oblique' in n:
        fam += '-italic'
    return fam


def eps_faces(path):
    faces, texts = [], []
    cur, stack = None, []
    with open(path, encoding='latin-1') as f:
        for line in f:
            line = line.rstrip('\r\n')
            if EPS_TEXT.search(line):
                faces.append(cur)
                texts.append(line[line.find('(') + 1:line.rfind(')')])
                continue
            m = EPS_FONT.match(line)
            if m:
                cur = canon(m.group(1))
                continue
            for tok in re.findall(r'\b(gsave|grestore)\b', line):
                if tok == 'gsave':
                    stack.append(cur)
                elif stack:
                    cur = stack.pop()
    return faces, texts


def pdf_faces(path):
    with open(path, encoding='latin-1') as f:
        data = f.read()
    # Objeto → BaseFont, y recurso /Fn → objeto. ⚠️ Un solo diccionario /Font:
    # es lo que emite PDFDisplay (una página, sin XObjects de forma). Si algún día
    # hay más, el mapa podría ser ambiguo, y se dice en vez de adivinar.
    base = {m.group(1): m.group(2) for m in re.finditer(
        r'(?m)^(\d+) 0 obj\b(?:(?!endobj).)*?/BaseFont /(\S+)', data, re.S)}
    dicts = re.findall(r'/Font <<(.*?)>>', data, re.S)
    if len(dicts) > 1:
        raise ValueError('PDF con %d diccionarios /Font: el mapa de recursos '
                         'sería ambiguo' % len(dicts))
    res = {}
    for d in dicts:
        for m in re.finditer(r'/(\S+) (\d+) 0 R', d):
            res[m.group(1)] = canon(base.get(m.group(2), '?obj' + m.group(2)))

    faces, texts = [], []
    cur, stack = None, []
    for line in data.split('\n'):
        line = line.rstrip('\r')
        if PDF_TEXT.search(line):
            faces.append(cur)
            texts.append(line.rsplit(' ', 1)[0])
            continue
        m = PDF_FONT.match(line)
        if m:
            cur = res.get(m.group(1), '?' + m.group(1))
        elif line == 'q':
            stack.append(cur)
        elif line == 'Q' and stack:
            cur = stack.pop()
    return faces, texts


def svg_faces(path):
    with open(path, encoding='utf-8') as f:
        data = f.read()
    faces, texts = [], []
    for m in re.finditer(r'<tspan ([^>]*)>(.*?)</tspan>', data, re.S):
        attrs = dict(re.findall(r'([\w-]+)="([^"]*)"', m.group(1)))
        fam = attrs.get('font-family', '').split(',')[0]
        face = canon(fam)
        if face not in ('math', 'symbol') and not face.startswith('?'):
            # canon() lee negrita/itálica del NOMBRE; en SVG van en atributos.
            if attrs.get('font-weight') == 'bold':
                face += '-bold'
            if attrs.get('font-style') in ('italic', 'oblique'):
                face += '-italic'
        faces.append(face)
        texts.append(m.group(2))
    return faces, texts


def main(argv):
    if len(argv) != 4:
        print(__doc__.split('\n\n')[1], file=sys.stderr)
        return 2
    try:
        eps, eps_t = eps_faces(argv[1])
        svg, _ = svg_faces(argv[2])
        pdf, _ = pdf_faces(argv[3])
    except (OSError, ValueError) as e:
        print('fontparity: %s' % e)
        return 1
    if not (len(eps) == len(svg) == len(pdf)):
        # Lo reporta ya la invariante (a); aquí no hay alineación posible.
        print('nº de trozos de texto EPS/SVG/PDF = %d/%d/%d: no se pueden alinear'
              % (len(eps), len(svg), len(pdf)))
        return 1
    bad = [i for i in range(len(eps)) if not (eps[i] == svg[i] == pdf[i])]
    if not bad:
        return 0
    i = bad[0]
    print('%d de %d trozos de texto difieren de cara; el primero, nº %d (%r):'
          % (len(bad), len(eps), i + 1, eps_t[i][:40]))
    print('  EPS=%s  SVG=%s  PDF=%s' % (eps[i], svg[i], pdf[i]))
    return 1


if __name__ == '__main__':
    sys.exit(main(sys.argv))
