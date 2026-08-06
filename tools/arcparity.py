#!/usr/bin/env python3
"""Paridad GEOMÉTRICA de arcos y elipses entre los tres backends (Capa 3).

    tools/arcparity.py fig.eps fig.svg fig.pdf

Silencioso si los tres dibujan el mismo arco; imprime el diagnóstico y sale con 1
si no. Pensado para correr desde test/run.sh, como las otras invariantes de Capa 3.

POR QUÉ EXISTE
--------------
El golden es autorreferencial: bendice lo que salga. Las dos invariantes viejas de
Capa 3 miran conteo de operaciones de texto y paths de un segmento; ninguna mira
GEOMETRÍA. Entre el 2026-07-26 y el 2026-07-27 vivió ahí un bug que ninguna de las
seis compuertas podía ver: EPS y SVG dibujaban la elipse de `rpstest` como
20.888×13.049 @ 47.27° cuando la verdadera es 21.757×11.541 @ 36.77°, y un arco
parcial reflejado (la antena de `lib/satellite.mg`, de 160°) salía de 350°.

⚠️ EPS y SVG COINCIDÍAN ENTRE SÍ y los dos estaban mal: ambos derivaban los ejes de
las normas de columna de la matriz, que son los semidiámetros CONJUGADOS, no los
ejes. Por eso esta compuerta compara los TRES y no dos: el PDF es la tercera opinión
independiente porque no decide ejes ni ángulos — transforma los puntos de control de
la Bézier, que es exacto para cualquier afín. Una compuerta EPS-vs-SVG habría dado
verde durante todo el bug.

CÓMO
----
Los tres backends comparten el mismo espacio de dispositivo (y hacia arriba, en pt):
el volteo de SVG vive en el `<g transform="scale(1,-1)">`, fuera de las coordenadas
del path. Así que se comparan coordenadas directas, sin normalizar.

El EPS es la lista de referencia (emite UNA operación por arco). De cada arco se
muestrean puntos y se exige que SVG y PDF contengan una curva que pase por todos
ellos. Se comprueba además el CONTEO de comandos `A` del SVG, que si no un backend
podría omitir un arco entero sin que se note (un arco de 360° son 2 comandos `A`,
porque SVG no admite el arco completo).

ALCANCE: solo arcos y elipses, que es donde vivió el bug. NO cubre texto ni tramado
—los dos pendientes de PENDIENTES.md—, a propósito: una compuerta que promete más de
lo que mide es peor que ninguna.
"""

import math
import re
import sys

# Tolerancia. NO es un umbral a ojo: el PDF no dibuja arcos —los aproxima con
# Béziers cúbicas de 90°—, y esa aproximación tiene un error radial máximo conocido
# de ≈2.7e-4·R. Medido en el corpus da 0.029 pt para R=113 y 0.041 para R=176, que
# es justo esa cota. Por eso la tolerancia ESCALA con el radio; un valor constante o
# se pasa de laxo en las figuras grandes o da falsos positivos en ellas.
# El piso absorbe el redondeo de impresión (%g = 6 cifras).
TOL_ABS = 0.02    # pt: piso
TOL_REL = 1e-3    # · radio mayor: cota del error de la Bézier del PDF, con holgura
NSAMP = 64        # puntos muestreados por arco
FLAT = 0.005      # pt: sagita objetivo al aplanar curvas a polilínea. Debe quedar
                  # MUY por debajo de la tolerancia: es error de esta herramienta,
                  # no de los backends, y si no se cuela como si fuera un hallazgo.


def tol_for(ux, uy, vx, vy):
    r = max(math.hypot(ux, uy), math.hypot(vx, vy))
    return max(TOL_ABS, TOL_REL * r)


# ---------------------------------------------------------------- muestreo

def sample_frame(cx, cy, ux, uy, vx, vy, sa, ea, n=NSAMP):
    """Muestrea P(t) = C + u·cos t + v·sin t entre los ángulos sa y ea (grados)."""
    out = []
    for i in range(n + 1):
        t = math.radians(sa + (ea - sa) * i / n)
        c, s = math.cos(t), math.sin(t)
        out.append((cx + ux * c + vx * s, cy + uy * c + vy * s))
    return out


def bbox(pts):
    xs = [p[0] for p in pts]
    ys = [p[1] for p in pts]
    return min(xs), min(ys), max(xs), max(ys)


# ---------------------------------------------------------------- EPS

# Arco del atajo nativo: "Cx Cy R sa ea arc|arcn" al principio de renglón. El ^ es
# lo que deja fuera a `dot`, que emite "newpath X Y R 0 360 arc" en un solo renglón
# y es un marcador de tamaño FÍSICO, no una forma de mundo.
RE_EPS_ARC = re.compile(
    r'^(-?[\d.]+(?:e[-+]?\d+)?) (-?[\d.]+(?:e[-+]?\d+)?) (-?[\d.]+(?:e[-+]?\d+)?) '
    r'(-?[\d.]+(?:e[-+]?\d+)?) (-?[\d.]+(?:e[-+]?\d+)?) (arcn?)$')
RE_EPS_MGARC = re.compile(
    r'^(\S+) (\S+) \[([^\]]+)\] mgarc$')


def arcs_from_eps(path):
    arcs = []
    with open(path, encoding='latin-1') as fh:
        for line in fh:
            line = line.rstrip('\n')
            m = RE_EPS_MGARC.match(line)
            if m:
                sa, ea = float(m.group(1)), float(m.group(2))
                ux, uy, vx, vy, cx, cy = [float(t) for t in m.group(3).split()]
                arcs.append((cx, cy, ux, uy, vx, vy, sa, ea))
                continue
            m = RE_EPS_ARC.match(line)
            if m:
                cx, cy, r = float(m.group(1)), float(m.group(2)), float(m.group(3))
                sa, ea = float(m.group(4)), float(m.group(5))
                arcs.append((cx, cy, r, 0.0, 0.0, r, sa, ea))
    return arcs


# ---------------------------------------------------------------- SVG

def svg_arc_points(x1, y1, rx, ry, phi_deg, fa, fs, x2, y2):
    """Convierte la parametrización por extremos del comando `A` a forma de centro
    (§ implementation notes del estándar SVG) y devuelve el arco muestreado."""
    phi = math.radians(phi_deg)
    cp, sp = math.cos(phi), math.sin(phi)
    dx2, dy2 = (x1 - x2) / 2.0, (y1 - y2) / 2.0
    x1p, y1p = cp * dx2 + sp * dy2, -sp * dx2 + cp * dy2
    rx, ry = abs(rx), abs(ry)
    if rx == 0 or ry == 0:
        return [(x1, y1), (x2, y2)]
    lam = x1p * x1p / (rx * rx) + y1p * y1p / (ry * ry)
    if lam > 1:
        rx *= math.sqrt(lam)
        ry *= math.sqrt(lam)
    num = rx * rx * ry * ry - rx * rx * y1p * y1p - ry * ry * x1p * x1p
    den = rx * rx * y1p * y1p + ry * ry * x1p * x1p
    co = math.sqrt(max(0.0, num / den)) if den else 0.0
    # lam≈1 ⟺ los extremos son ANTIPODALES: el arco es de 180° y la conversión
    # extremos→centro queda malísimamente condicionada justo ahí. SVGDisplay imprime
    # 6 cifras significativas, así que un extremo real de 155.9055 sale 155.906; ese
    # error de 5e-4 pt se amplifica ~150× y correría el centro 0.075 pt. Es artefacto
    # de precisión de impresión, no desacuerdo entre backends —y son exactamente los
    # arcos de 360°, que SVG parte en dos mitades de 180°—, así que en el caso
    # degenerado el centro se fija en el punto medio de la cuerda, que es lo que el
    # emisor quiso decir. Fuera de esa vecindad (lam < 0.999) no se toca nada.
    if lam > 0.999:
        co = 0.0
    elif fa == fs:
        co = -co
    cxp, cyp = co * rx * y1p / ry, -co * ry * x1p / rx
    cx = cp * cxp - sp * cyp + (x1 + x2) / 2.0
    cy = sp * cxp + cp * cyp + (y1 + y2) / 2.0

    def ang(ux, uy, vx, vy):
        nu, nv = math.hypot(ux, uy), math.hypot(vx, vy)
        if nu == 0 or nv == 0:
            return 0.0
        d = max(-1.0, min(1.0, (ux * vx + uy * vy) / (nu * nv)))
        a = math.acos(d)
        return -a if ux * vy - uy * vx < 0 else a

    ux0, uy0 = (x1p - cxp) / rx, (y1p - cyp) / ry
    vx0, vy0 = (-x1p - cxp) / rx, (-y1p - cyp) / ry
    th1 = ang(1, 0, ux0, uy0)
    dth = ang(ux0, uy0, vx0, vy0)
    if not fs and dth > 0:
        dth -= 2 * math.pi
    elif fs and dth < 0:
        dth += 2 * math.pi
    # Paso angular para que la sagita de la cuerda quede bajo FLAT. El radio de
    # curvatura MÁXIMO de la elipse es max(rx,ry)²/min(rx,ry) (en el extremo del eje
    # menor), que es el caso peor: sagita ≈ R·θ²/8.
    rmax, rmin = max(rx, ry), max(min(rx, ry), 1e-9)
    R = rmax * rmax / rmin
    step = math.sqrt(8 * FLAT / R) if R > 0 else 0.05
    n = max(8, min(20000, int(abs(dth) / max(step, 1e-6)) + 1))
    out = []
    for i in range(n + 1):
        t = th1 + dth * i / n
        a, b = rx * math.cos(t), ry * math.sin(t)
        out.append((cx + a * cp - b * sp, cy + a * sp + b * cp))
    return out


RE_SVG_D = re.compile(r'\sd="([^"]*)"')
RE_SVG_TOK = re.compile(r'([MLACZmlacz])([^MLACZmlacz]*)')


def polylines_from_svg(path):
    """Aplana cada atributo d= a una polilínea. Devuelve (polilíneas, nº de `A`)."""
    txt = open(path, encoding='latin-1').read()
    polys, narc = [], 0
    for d in RE_SVG_D.findall(txt):
        cur = None
        start = None
        pts = []
        for op, args in RE_SVG_TOK.findall(d):
            v = [float(t) for t in args.replace(',', ' ').split()] if args.strip() else []
            if op in 'Mm':
                if len(pts) > 1:
                    polys.append(pts)
                cur = (v[0], v[1])
                start = cur
                pts = [cur]
            elif op in 'Ll':
                for i in range(0, len(v) - 1, 2):
                    cur = (v[i], v[i + 1])
                    pts.append(cur)
            elif op in 'Cc':
                for i in range(0, len(v) - 5, 6):
                    pts.extend(flatten_bezier(cur, (v[i], v[i + 1]),
                                              (v[i + 2], v[i + 3]), (v[i + 4], v[i + 5]))[1:])
                    cur = (v[i + 4], v[i + 5])
            elif op in 'Aa':
                for i in range(0, len(v) - 6, 7):
                    narc += 1
                    seg = svg_arc_points(cur[0], cur[1], v[i], v[i + 1], v[i + 2],
                                         int(v[i + 3]), int(v[i + 4]), v[i + 5], v[i + 6])
                    pts.extend(seg[1:])
                    cur = (v[i + 5], v[i + 6])
            elif op in 'Zz':
                if start:
                    pts.append(start)
                    cur = start
        if len(pts) > 1:
            polys.append(pts)
    return polys, narc


# ---------------------------------------------------------------- PDF

def flatten_bezier(p0, p1, p2, p3, n=None):
    if n is None:
        d = (math.dist(p0, p1) + math.dist(p1, p2) + math.dist(p2, p3))
        # n tal que la sagita de cada cuerda quede bajo FLAT: para un tramo de
        # longitud L y n cuerdas, sagita ≈ (L/n)²/(8·R) con R≈L; basta n ∝ √(L/FLAT).
        n = max(8, min(2000, int(math.sqrt(d / max(FLAT, 1e-9)) * 2) or 8))
    out = []
    for i in range(n + 1):
        t = i / n
        mt = 1 - t
        out.append((mt**3 * p0[0] + 3 * mt * mt * t * p1[0] + 3 * mt * t * t * p2[0] + t**3 * p3[0],
                    mt**3 * p0[1] + 3 * mt * mt * t * p1[1] + 3 * mt * t * t * p2[1] + t**3 * p3[1]))
    return out


RE_PDF_OP = re.compile(r'((?:-?[\d.]+\s+)+)(m|l|c|h)\b')

# Un literal de cadena PDF más largo que esto no es un rótulo: es un `(` suelto
# dentro del stream BINARIO de una fuente embebida. Ver strip_pdf_strings.
MAX_PDF_STRING = 8192


def strip_pdf_strings(data):
    """Sustituye por un espacio los literales de cadena `(...)` del PDF.

    ⚠️ Hace falta porque RE_PDF_OP barre el ARCHIVO ENTERO, y ahí dentro un RÓTULO
    puede fabricar un operador de la nada: `text("5 m/s")` viaja al PDF como
    `(5 m/s) Tj`, y `5 m` es exactamente la forma de un `moveto`. Con un operando
    reventaba —`v[-2]`, IndexError— y con dos habría inyectado GEOMETRÍA FALSA, que
    es peor, porque no se nota: un `m` espurio parte una polilínea en dos y el
    cotejo de arcos empieza a comparar trozos.

    Es un fallo PREEXISTENTE, no de los rótulos que lo destaparon (2026-08-06, al
    entrar el escape `\\` a `examples/texto.mg`): cualquier figura con un rótulo
    tipo «5 m» lo habría disparado, y ninguna del corpus lo tenía. Los otros dos
    parsers no lo comparten — el de EPS ancla con `.match` al principio de cada
    línea, y una línea de texto empieza por `(`; el de SVG solo mira atributos `d=`.

    El anidamiento y el `\\` se respetan porque PDF los admite dentro de la cadena.
    Y si un `(` no cierra dentro de MAX_PDF_STRING se deja tal cual en vez de
    tragarse el resto del archivo: en un stream binario de fuente hay bytes 0x28
    sueltos, y sin esa cota una fuente embebida borraría toda la geometría —lo que
    convertiría esta compuerta en verde permanente, que es el modo de fallo que las
    compuertas de este proyecto tienen prohibido.
    """
    out, i, n = [], 0, len(data)
    while i < n:
        k = data.find('(', i)
        if k < 0:
            out.append(data[i:])
            break
        out.append(data[i:k])
        j, depth = k + 1, 1
        while j < n and depth and j - k < MAX_PDF_STRING:
            ch = data[j]
            if ch == '\\':
                j += 2
                continue
            if ch == '(':
                depth += 1
            elif ch == ')':
                depth -= 1
            j += 1
        if depth:                 # sin cierre: no era una cadena
            out.append('(')
            i = k + 1
        else:
            out.append(' ')       # la cadena entera se va, deja un separador
            i = j
    return ''.join(out)


def polylines_from_pdf(path):
    """El PDF de libharu NO va comprimido, así que sus operadores son parseables
    directo — la misma propiedad de la que ya dependen las otras invariantes."""
    data = strip_pdf_strings(open(path, 'rb').read().decode('latin-1'))
    polys, pts, cur, start = [], [], None, None
    for args, op in RE_PDF_OP.findall(data):
        v = [float(t) for t in args.split()]
        # Los operandos se cuentan antes de usarlos. Con strip_pdf_strings ya no
        # deberían llegar operadores mutilados, pero la compuerta no debe morir de
        # IndexError ante una entrada rara: un fallo suyo tiene que ser un
        # diagnóstico, no un traceback.
        if op == 'm':
            if len(v) < 2:
                continue
            if len(pts) > 1:
                polys.append(pts)
            cur = (v[-2], v[-1])
            start = cur
            pts = [cur]
        elif op == 'l' and cur and len(v) >= 2:
            cur = (v[-2], v[-1])
            pts.append(cur)
        elif op == 'c' and cur and len(v) >= 6:
            pts.extend(flatten_bezier(cur, (v[-6], v[-5]), (v[-4], v[-3]), (v[-2], v[-1]))[1:])
            cur = (v[-2], v[-1])
        elif op == 'h' and start:
            pts.append(start)
            cur = start
    if len(pts) > 1:
        polys.append(pts)
    return polys


# ---------------------------------------------------------------- cotejo

def seg_dist(p, a, b):
    ax, ay = a
    bx, by = b
    dx, dy = bx - ax, by - ay
    L = dx * dx + dy * dy
    if L == 0:
        return math.hypot(p[0] - ax, p[1] - ay)
    t = max(0.0, min(1.0, ((p[0] - ax) * dx + (p[1] - ay) * dy) / L))
    return math.hypot(p[0] - (ax + t * dx), p[1] - (ay + t * dy))


def max_dev(samples, poly):
    """Desviación máxima de los puntos muestreados respecto de la polilínea."""
    worst = 0.0
    for p in samples:
        best = min(seg_dist(p, poly[i], poly[i + 1]) for i in range(len(poly) - 1))
        if best > worst:
            worst = best
            if worst > 1e3:
                return worst
    return worst


def best_match(samples, polys, tol):
    """Menor desviación entre las polilíneas candidatas. Se prefiltra por bbox: la
    del arco tiene que caber en la de la polilínea (con holgura), que descarta de un
    golpe las polilíneas largas de datos y deja el cotejo caro para 1 o 2."""
    x0, y0, x1, y1 = bbox(samples)
    m = 1.0
    best = float('inf')
    for poly in polys:
        px0, py0, px1, py1 = bbox(poly)
        if px0 > x0 + m or py0 > y0 + m or px1 < x1 - m or py1 < y1 - m:
            continue
        d = max_dev(samples, poly)
        if d < best:
            best = d
            if best <= tol:
                break
    return best


def main():
    if len(sys.argv) != 4:
        print("uso: arcparity.py fig.eps fig.svg fig.pdf", file=sys.stderr)
        return 2
    eps, svg, pdf = sys.argv[1:4]
    arcs = arcs_from_eps(eps)
    svg_polys, svg_narc = polylines_from_svg(svg)
    pdf_polys = polylines_from_pdf(pdf)

    problems = []

    # Conteo: un arco de 360° son DOS comandos `A` (SVG no admite el completo).
    expected = sum(2 if abs(ea - sa) >= 360.0 else 1
                   for (_, _, _, _, _, _, sa, ea) in arcs)
    if expected != svg_narc:
        problems.append("conteo de arcos EPS→SVG = %d esperados vs %d emitidos "
                        "(un backend omite un arco)" % (expected, svg_narc))

    for i, (cx, cy, ux, uy, vx, vy, sa, ea) in enumerate(arcs):
        samples = sample_frame(cx, cy, ux, uy, vx, vy, sa, ea)
        tol = tol_for(ux, uy, vx, vy)
        ds = best_match(samples, svg_polys, tol)
        dp = best_match(samples, pdf_polys, tol)
        if ds > tol or dp > tol:
            problems.append(
                "arco %d (centro %.3f,%.3f  %.0f°→%.0f°, tol %.3f): "
                "desviación SVG=%s PDF=%s pt"
                % (i, cx, cy, sa, ea, tol,
                   ("%.3f" % ds) if ds < 1e9 else "sin curva",
                   ("%.3f" % dp) if dp < 1e9 else "sin curva"))

    for p in problems:
        print(p)
    return 1 if problems else 0


if __name__ == '__main__':
    sys.exit(main())
