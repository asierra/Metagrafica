#!/usr/bin/env python3
"""
Decima los vértices de los `polygon`/`polyline` LITERALES de un `.mg` generado, para
producir una variante ligera de una figura densa que se va a dibujar pequeña.

PARA QUÉ, Y CUÁNDO NO
---------------------
⚠️ Para los mapas de `lib/` NO se usa esto: `tools/geo2mg.py --simplify` los decima al
generarlos, en la misma corrida, así que su encabezado registra UN comando que reproduce
el archivo. Éste es para el otro caso: sacar una variante ligera de un `.mg` YA generado,
sin volver a pedir geopandas ni los datos de Natural Earth —que no están en el repo—.

El caso real es el LOGO. `lib/fulldisk_map.mg` está decimado a `0.001`, la medida de su
tamaño publicado (7.37 cm en `elevacion_solar.mg`, donde se leen Baja California, Florida
y el Caribe). Dibujado a 1 o 2 cm, la mitad de esos vértices vuelve a caer bajo el píxel
de la impresora. La variante de logo se genera con este tool, donde se use, y no se
committea: un mapa decimado sirve al tamaño para el que se decimó, y committear la
versión de logo invitaría a usarla grande.

⚠️ Ni éste ni `--simplify` son la `--tolerance` de `geo2mg.py`: aquélla va en GRADOS y
ANTES de proyectar. Lo que decide si un vértice se ve es su error DESPUÉS de proyectar, en
fracciones del radio del disco. `--simplify` y `--eps` miden eso; comparten el algoritmo
(`geo2mg.py` importa de aquí) y se diferencian solo en el momento: `--simplify` decima las
coordenadas de plena precisión antes de redondear a 4 decimales, que es el orden correcto,
y `--eps` trabaja sobre las ya redondeadas. Medido: sobre el mismo mapa a 0.001 las dos
rutas dan 1715 y 1722 vértices, con 4 de desvío máximo por bloque.

QUÉ SE MIDIÓ (2026-08-31, disco de 2 cm de diámetro, PDF a 300/600 dpi, criterio de
`ver.sh`: píxeles que difieren con fuzz del 25 %). Partiendo del mapa DENSO de entonces,
de 4047 vértices, que es el que había antes de que `lib/` se decimara a 0.001:

    eps     vértices        PDF     dif. 300 dpi    dif. 600 dpi
    base    4047            51 KB   —               —
    0.002   1247 (31 %)     23 KB   0 px            68 px
    0.004    791 (20 %)     14 KB   46 px           604 px
    0.008    446 (11 %)     8.8 KB  268 px          1557 px

A 1 cm de diámetro, 0.004 sale pixel-idéntico a 300 dpi. **0.004 es el punto dulce para un
logo de 2 cm**: PDF de 51 a 14 KB. Y al revés, para que nadie lo suba de escala pensando
que sale gratis: a los 7.37 cm de `elevacion_solar`, 0.004 aplana Baja California y vuelve
angulosa Centroamérica.

`--eps 0` NO es la identidad, y esa es su gracia: quita los vértices exactamente
colineales y los duplicados consecutivos, o sea un saneo SIN PÉRDIDA. Sobre aquel mapa
denso: 4047 -> 3541 vértices (-12.5 %; 13 de ellos duplicados literales), con el PDF
idéntico y el peor píxel del SVG a 4/255 a escala completa. `geo2mg.py` lo hace ahora en
origen, y por eso su `--simplify 0` tampoco es un no-op.

⚠️ La palanca es la DECIMACIÓN, no tirar features. Se midió sobre el mapa denso: los
vértices estaban en los continentes grandes (941 + 374 + 328 de 2505); los 12 polígonos
más chicos miden entre 0.2 y 1 mm a 2 cm de diámetro —o sea que se VEN— y sumaban apenas
el 12 % del total. Por eso aquí no hay filtro de área: quitaría silueta reconocible a
cambio de casi nada. El filtro que sí tiene sentido es el de `geo2mg.py`
(`--min-feature`), que elimina islas enteras antes de proyectar.

CÓMO
----
Douglas-Peucker sobre cada bloque, con la tolerancia en las unidades del propio archivo
(en los mapas de `lib/`, normalizados a radio 1, `--eps` es una fracción del radio). Los
`polygon` se cierran antes de decimar, para que el vértice de arranque no ancle una arista
falsa. Todo lo demás del archivo —encabezado, `struct`, `if`, colores, el `circle` del
limbo— se copia intacto: la salida sigue siendo el mismo dibujo con menos puntos.

Un bloque cuyo contenido no sean números se copia sin tocar y se reporta: este tool es
para los `.mg` GENERADOS, donde las coordenadas son literales, y no debe adivinar sobre
uno escrito a mano donde el cuerpo puede ser una expresión.

⚠️ No comprueba que el polígono simplificado no se auto-intersecte. A las tolerancias
útiles para un logo no pasa con líneas de costa, pero si algún día se decima algo muy
sinuoso con un eps grande, hay que MIRARLO (`tools/ver.sh`) y no confiar en el conteo.

USO
---
    python3 tools/simplifica_mg.py lib/fulldisk_map.mg --eps 0.004 -o fulldisk_logo.mg
    python3 tools/simplifica_mg.py lib/fulldisk_map.mg --eps 0.004 --eps-polyline 0.002 -o x.mg

Solo biblioteca estándar. Vive fuera del compilador: no se liga a `bin/mg` ni el lenguaje
depende de él.
"""

import argparse
import math
import re
import sys

# Un bloque de coordenadas literal: `polygon(attrs) { x y  x y  … }` o `polyline { … }`.
# Los atributos no anidan paréntesis en los .mg generados; si algún día lo hacen, el
# bloque se copiará sin tocar (y se reportará) en vez de salir mal cortado.
BLOQUE = re.compile(r'\b(polygon|polyline)\b(\([^()]*\))?\s*\{([^{}]*)\}')


def douglas_peucker(pts, eps):
    """Los vértices de `pts` cuyo error perpendicular supera `eps`. Iterativo: una línea
    de costa de 941 puntos desborda la recursión por defecto de Python en el peor caso.
    Solo mira las dos primeras componentes, así que un punto puede llevar más (un índice)."""
    if len(pts) < 3:
        return list(pts)
    quedan = [False] * len(pts)
    quedan[0] = quedan[-1] = True
    pila = [(0, len(pts) - 1)]
    while pila:
        a, b = pila.pop()
        if b <= a + 1:
            continue
        ax, ay = pts[a][0], pts[a][1]
        dx, dy = pts[b][0] - ax, pts[b][1] - ay
        largo = math.hypot(dx, dy)
        peor, cual = -1.0, -1
        for i in range(a + 1, b):
            px, py = pts[i][0], pts[i][1]
            if largo == 0.0:
                # Extremos coincidentes: el error es la distancia al punto, no a la recta.
                d = math.hypot(px - ax, py - ay)
            else:
                d = abs(dy * (px - ax) - dx * (py - ay)) / largo
            if d > peor:
                peor, cual = d, i
        if peor > eps:
            quedan[cual] = True
            pila.append((a, cual))
            pila.append((cual, b))
    return [p for p, k in zip(pts, quedan) if k]


def formatea(v, decimales):
    """Como los emite `geo2mg.py`: sin ceros de relleno y sin `-0`."""
    s = f'{v:.{decimales}f}'.rstrip('0').rstrip('.')
    return '0' if s in ('', '-0', '-') else s


def seccion(fuente, inicio):
    """El último comentario de línea completa antes de `inicio`: el rótulo de sección
    que los .mg generados ponen sobre cada grupo de bloques (`% isotermas`)."""
    for linea in reversed(fuente[:inicio].split('\n')):
        if linea.strip().startswith('%'):
            return linea.strip()
    return ''


def simplifica(fuente, eps_poligono, eps_polilinea, decimales, avisos,
               escala=(1.0, 1.0), excluye=()):
    """Devuelve (texto nuevo, vértices antes, vértices después)."""
    salida = []
    pos = antes = despues = 0
    sx, sy = escala
    for m in BLOQUE.finditer(fuente):
        salida.append(fuente[pos:m.start()])
        pos = m.end()
        clase, attrs, cuerpo = m.group(1), m.group(2) or '', m.group(3)
        if excluye and any(e in seccion(fuente, m.start()) for e in excluye):
            salida.append(m.group(0))
            continue
        campos = cuerpo.split()
        try:
            nums = [float(c) for c in campos]
        except ValueError:
            avisos.append(f'{clase} con cuerpo no literal: se copia sin tocar')
            salida.append(m.group(0))
            continue
        if not nums or len(nums) % 2:
            avisos.append(f'{clase} con {len(nums)} números (impar o vacío): se copia sin tocar')
            salida.append(m.group(0))
            continue

        # Se decima en el espacio ESCALADO (el del papel, si --escala lo dice), y cada
        # punto lleva su índice para recuperar después el vértice original.
        pts = [(x * sx, y * sy, i) for i, (x, y) in enumerate(zip(nums[0::2], nums[1::2]))]
        cerrado = (clase == 'polygon')
        eps = eps_poligono if cerrado else eps_polilinea
        if cerrado:
            # Se cierra el anillo para decimar, y se descarta la repetición al final: si
            # no, el primer vértice ancla una arista que el polígono no tiene.
            simple = douglas_peucker(pts + [pts[0]], eps)[:-1]
        else:
            simple = douglas_peucker(pts, eps)

        antes += len(pts)
        despues += len(simple)
        if decimales is None:
            # Sin --decimales, el vértice que sobrevive se copia TAL CUAL: decimar quita
            # puntos, no tiene por qué reescribir los que quedan.
            cuerpo_nuevo = '  '.join(f'{campos[2 * i]} {campos[2 * i + 1]}'
                                     for _, _, i in simple)
        else:
            cuerpo_nuevo = '  '.join(f'{formatea(nums[2 * i], decimales)} '
                                     f'{formatea(nums[2 * i + 1], decimales)}'
                                     for _, _, i in simple)
        salida.append(f'{clase}{attrs} {{ {cuerpo_nuevo} }}')
    salida.append(fuente[pos:])
    return ''.join(salida), antes, despues


def procedencia(origen, eps_poligono, eps_polilinea, decimales, antes, despues,
                escala=(1.0, 1.0), excluye=()):
    """El encabezado del archivo derivado. Los `.mg` generados de este proyecto llevan su
    comando de regeneración; uno derivado, con más razón: es el único rastro de que no es
    el mapa bueno."""
    eps = (f'--eps {eps_poligono}' if eps_poligono == eps_polilinea
           else f'--eps {eps_poligono} --eps-polyline {eps_polilinea}')
    if decimales is not None:
        eps += f' --decimales {decimales}'
    if escala != (1.0, 1.0):
        eps += f' --escala {escala[0]:g} {escala[1]:g}'
    for e in excluye:
        eps += f' --excluye-seccion "{e}"'
    pct = f'{100.0 * despues / antes:.0f} %' if antes else '—'
    return (
        f'% DERIVADO de {origen} por decimación Douglas-Peucker.\n'
        f'% {antes} -> {despues} vértices ({pct}).\n'
        f'% GENERADO, no editar a mano. Regenerar con:\n'
        f'%   python3 tools/simplifica_mg.py {origen} {eps} -o <salida>\n'
        f'% ⚠️ Sirve al tamaño para el que se decimó. Ampliado se le ven las aristas;\n'
        f'%    para la figura a escala completa usa {origen}.\n'
        f'%\n'
    )


def main():
    ap = argparse.ArgumentParser(
        description='Decima los polygon/polyline literales de un .mg generado.')
    ap.add_argument('entrada')
    ap.add_argument('-o', '--salida', help='archivo de salida (default: stdout)')
    ap.add_argument('--eps', type=float, required=True,
                    help='tolerancia Douglas-Peucker en unidades del archivo '
                         '(en los mapas de lib/, fracción del radio; 0.004 para un logo de 2 cm)')
    ap.add_argument('--eps-polyline', type=float, default=None,
                    help='tolerancia distinta para los polyline (default: la de --eps)')
    ap.add_argument('--decimales', type=int, default=None,
                    help='reescribe las coordenadas con N decimales (default: los vértices '
                         'que quedan se copian tal cual)')
    ap.add_argument('--escala', type=float, nargs=2, metavar=('SX', 'SY'), default=(1.0, 1.0),
                    help='mide la tolerancia tras multiplicar x por SX e y por SY; para un '
                         'plot cuyas unidades no son isótropas (p.ej. cm de papel por unidad)')
    ap.add_argument('--excluye-seccion', action='append', default=[], metavar='TEXTO',
                    help='no toca los bloques cuyo comentario de sección más cercano contenga '
                         'TEXTO (repetible)')
    ap.add_argument('--sin-encabezado', action='store_true',
                    help='no anteponer el bloque de procedencia')
    args = ap.parse_args()

    if args.eps < 0 or (args.eps_polyline is not None and args.eps_polyline < 0):
        sys.exit('simplifica_mg: la tolerancia no puede ser negativa')
    eps_polilinea = args.eps if args.eps_polyline is None else args.eps_polyline

    with open(args.entrada, encoding='utf-8') as f:
        fuente = f.read()

    avisos = []
    escala = tuple(args.escala)
    if escala[0] <= 0 or escala[1] <= 0:
        sys.exit('simplifica_mg: la escala tiene que ser positiva')
    texto, antes, despues = simplifica(fuente, args.eps, eps_polilinea,
                                       args.decimales, avisos, escala,
                                       args.excluye_seccion)
    if antes == 0:
        sys.exit(f'simplifica_mg: {args.entrada} no tiene ningún polygon/polyline literal')

    if not args.sin_encabezado:
        texto = procedencia(args.entrada, args.eps, eps_polilinea,
                            args.decimales, antes, despues, escala,
                            args.excluye_seccion) + texto

    if args.salida:
        with open(args.salida, 'w', encoding='utf-8') as f:
            f.write(texto)
    else:
        sys.stdout.write(texto)

    for a in avisos:
        print(f'AVISO: {a}', file=sys.stderr)
    pct = 100.0 * despues / antes
    print(f'{args.entrada}: {antes} -> {despues} vértices ({pct:.1f} %), '
          f'eps={args.eps}/{eps_polilinea}', file=sys.stderr)


if __name__ == '__main__':
    main()
