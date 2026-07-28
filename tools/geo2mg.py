#!/usr/bin/env python3
"""geo -> MetaGrafica: genera un `struct` de MAPA icónico (globo/vista satelital, la
proyección ortográfica centrada en cualquier lat/lon = un "full disk" geoestacionario).
Dos modos:

  line-art (default)  costas como polilíneas. Fuente: el polígono de océano (usa su
                      frontera, ya suave a 110m) o un shapefile de líneas de costa.
  --fill              continentes RELLENOS sobre un disco de océano. Fuente: polígonos
                      de tierra.

Dos simplificaciones para el look icónico:
  1. Douglas-Peucker (`--tolerance`, grados): recovecos. A 110m casi no hace falta (0).
  2. Filtro de EXTENSIÓN (`--min-feature`, fracción del radio): elimina features enteras
     con bbox-diagonal menor al umbral (islitas). Se mide en AEQD (equidistante), no en
     ortho, para no penalizar features cerca del limbo (que ortho comprime radialmente).

Salida: un `struct` MG PELADO (para `include`), coords normalizadas a radio 1, y-up. Se
coloca como cualquier struct de biblioteca (ver lib/satellite.mg):
    PolarMap(at=(10,4.5), scale=1)
Es preparación de datos FUERA del compilador, misma categoría que tools/hist2mg.py — NO
se liga a bin/mg ni el lenguaje depende de él.

═══ REQUISITOS (esta herramienta es OPCIONAL; el compilador no la necesita) ═══
· Python:  pip install geopandas pyproj shapely numpy   (más pesado que los demás tools)
· Datos:   Natural Earth 1:110m (dominio público), descargar de naturalearthdata.com:
             - Physical » Ocean   -> ne_110m_ocean/    (line-art y océano de --fill)
             - Physical » Land    -> ne_110m_land/     (tierra de --fill)
           Se pasan con --costas / --land (por defecto se buscan en el dir actual).
           NO se incluyen en el repo.

Ejemplos (las dos vistas de lib/):
    python3 geo2mg.py --fill --lat 90 --lon 0    --nombre PolarMap    --salida polar_map.mg
    python3 geo2mg.py --fill --lat 0  --lon -75  --nombre FullDiskMap --salida fulldisk_map.mg
"""
import argparse
import math
import os
import re
import numpy as np
import geopandas as gpd
from pyproj import Transformer
from shapely.ops import transform
from shapely.geometry import Point, LineString, MultiLineString

ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
ap.add_argument('--lat', type=float, default=90.0, help='Latitud del centro (default 90 = polo norte)')
ap.add_argument('--lon', type=float, default=0.0, help='Longitud del centro (default 0)')
ap.add_argument('--fill', action='store_true', help='Modo RELLENO: continentes rellenos sobre disco de océano (usa --land). Sin esto, line-art.')
ap.add_argument('--tolerance', type=float, default=0.0, help='Douglas-Peucker en grados (default 0; a 110m no hace falta)')
ap.add_argument('--min-feature', type=float, default=0.05, help='Umbral de extensión (bbox en AEQD) como FRACCIÓN del radio; features menores se eliminan (default 0.05)')
ap.add_argument('--grid', action=argparse.BooleanOptionalAction, default=True, help='Meridianos y paralelos (default sí)')
ap.add_argument('--grid-paso', type=float, default=30.0, help='Paso de la retícula en grados (default 30)')
ap.add_argument('--grid-color', type=str, default='gray', help='Color de la retícula (default gray)')
ap.add_argument('--grid-width', type=float, default=0.25, help='Grosor de la retícula en pt (default 0.25)')
ap.add_argument('--costas', type=str, default='ne_110m_ocean/ne_110m_ocean.shp', help='Fuente line-art: líneas de costa o polígono de océano')
ap.add_argument('--land', type=str, default='ne_110m_land/ne_110m_land.shp', help='Polígonos de tierra (modo --fill)')
ap.add_argument('--color-ocean', type=str, default='steelblue', help='Color del océano (modo --fill)')
ap.add_argument('--color-tierra', type=str, default='wheat', help='Color de la tierra (modo --fill)')
ap.add_argument('--nombre', type=str, default=None, help='Nombre del struct (default: derivado de --salida, o "Mapa"). "PolarMap" no aplica a vistas no polares.')
ap.add_argument('--salida', type=str, default=None, help='Archivo .mg (default: <nombre>.mg, o mapa_<lat>_<lon>.mg)')
args = ap.parse_args()

lat_0, lon_0 = args.lat, args.lon

# --- Nombre del struct y archivo de salida: uno deriva del otro. Un nombre válido de MG
#     empieza por letra y solo lleva [A-Za-z0-9_]. ---
def _ident(s):
    s = re.sub(r'[^A-Za-z0-9_]', '', s)
    return s if s[:1].isalpha() else 'M' + s
if args.salida is None and args.nombre is None:
    args.nombre = "Mapa"
    args.salida = f"mapa_{lat_0:+.0f}_{lon_0:+.0f}.mg".replace('+', 'p').replace('-', 'n')
elif args.salida is None:
    args.salida = args.nombre.lower() + ".mg"
elif args.nombre is None:
    stem = os.path.splitext(os.path.basename(args.salida))[0]
    args.nombre = _ident(stem[:1].upper() + stem[1:])
args.nombre = _ident(args.nombre)

# --- Proyecciones: ortho = vista final; aeqd = paso intermedio para recortar el
#     hemisferio visible con un círculo perfecto (idéntico a hazmapa.py). ---
cad_ortho = f"+proj=ortho +lat_0={lat_0} +lon_0={lon_0} +x_0=0 +y_0=0 +ellps=GRS80 +units=m +no_defs"
cad_aeqd  = f"+proj=aeqd  +lat_0={lat_0} +lon_0={lon_0} +x_0=0 +y_0=0 +ellps=GRS80 +units=m +no_defs"
tr_aeqd       = Transformer.from_crs("EPSG:4326", cad_aeqd, always_xy=True)
tr_aeqd_ortho = Transformer.from_crs(cad_aeqd, cad_ortho, always_xy=True)

RADIO_TIERRA = 6371000.0
radio_horizonte = math.pi * RADIO_TIERRA / 2 * 0.995
UMBRAL_SALTO_M = 1_500_000
circulo = Point(0, 0).buffer(radio_horizonte, resolution=256)
umbral_aeqd = args.min_feature * radio_horizonte


def a_aeqd(geom):
    try:
        return transform(tr_aeqd.transform, geom)
    except Exception:
        return None


def a_ortho(geom):
    try:
        g = transform(tr_aeqd_ortho.transform, geom)
    except Exception:
        return None
    if g is None or g.is_empty or not all(math.isfinite(v) for v in g.bounds):
        return None
    return g


def explotar(geom):
    """LineStrings individuales de cualquier geometría de líneas."""
    if geom is None or geom.is_empty:
        return
    t = geom.geom_type
    if t == 'LineString':
        if len(geom.coords) >= 2:
            yield geom
    elif t in ('MultiLineString', 'GeometryCollection'):
        for g in geom.geoms:
            yield from explotar(g)


def polis_de(geom):
    """Polygons individuales de Polygon/MultiPolygon/GeometryCollection."""
    if geom is None or geom.is_empty:
        return
    t = geom.geom_type
    if t == 'Polygon':
        yield geom
    elif t in ('MultiPolygon', 'GeometryCollection'):
        for g in geom.geoms:
            yield from polis_de(g)


def bbox_diag(geom):
    x0, y0, x1, y1 = geom.bounds
    return math.hypot(x1 - x0, y1 - y0)


def _vertices(geom):
    t = geom.geom_type
    if t == 'LineString':
        yield from geom.coords
    elif t == 'Polygon':
        yield from geom.exterior.coords
    elif t in ('MultiLineString', 'MultiPolygon', 'GeometryCollection'):
        for g in geom.geoms:
            yield from _vertices(g)


_la0 = math.radians(lat_0)
_lo0 = math.radians(lon_0)

def hay_visible(geom):
    """True si algún vértice está DENTRO del hemisferio visible (a <90° del centro).
    Descarta features del otro hemisferio (Antártida en vista polar norte), que la AEQD
    esparce por el antípoda y produciría un polígono cubriendo todo el disco."""
    for lon, lat in _vertices(geom):
        la, lo = math.radians(lat), math.radians(lon)
        cosd = math.sin(_la0) * math.sin(la) + math.cos(_la0) * math.cos(la) * math.cos(lo - _lo0)
        if cosd > 0.02:
            return True
    return False


def partir_saltos(coords):
    piezas, actual = [], [coords[0]]
    for (x1, y1), (x2, y2) in zip(coords, coords[1:]):
        if math.hypot(x2 - x1, y2 - y1) > UMBRAL_SALTO_M:
            if len(actual) >= 2:
                piezas.append(actual)
            actual = [(x2, y2)]
        else:
            actual.append((x2, y2))
    if len(actual) >= 2:
        piezas.append(actual)
    return piezas


def sin_costura(coords, eps=0.01):
    """Parte una línea lon/lat quitando vértices en el antimeridiano (|lon|≈180), que
    son la costura artificial del polígono de océano, no costa real."""
    piezas, actual = [], []
    for lon, lat in coords:
        if abs(abs(lon) - 180.0) <= eps:
            if len(actual) >= 2:
                piezas.append(LineString(actual))
            actual = []
        else:
            actual.append((lon, lat))
    if len(actual) >= 2:
        piezas.append(LineString(actual))
    return piezas


# --- Retícula (común a ambos modos), por el mismo recorte AEQD->horizonte->ortho ---
grid_ml = []
if args.grid:
    def lineas_grid(paso):
        L = []
        for lon in np.arange(-180, 180, paso):
            L.append(LineString([(lon, lat) for lat in np.arange(-90, 90.01, 1.0)]))
        for lat in np.arange(-90 + paso, 90, paso):
            L.append(LineString([(lon, lat) for lon in np.arange(-180, 180.01, 2.0)]))
        return L
    for linea in lineas_grid(args.grid_paso):
        ga = a_aeqd(linea)
        if ga is None:
            continue
        # ⚠️ PARTIR POR EL ANTÍPODA ANTES DE RECORTAR. En AEQD el antípoda no es un
        # punto: es TODO el círculo de radio πR. Una línea de retícula que pase por
        # él —el paralelo de latitud −lat_0, y solo ese— salta entre dos muestras
        # consecutivas de un borde al opuesto (39 865 km en la vista lat 30), y el
        # segmento recto que las une CRUZA EL DISCO VISIBLE. El recorte lo acorta a
        # una cuerda de 2 puntos de borde a borde: una raya recta atravesando el
        # globo, que es como se descubrió (orbita_polar con grid=true).
        #
        # `partir_saltos` ya existía en este archivo para esto mismo, pero no se
        # llamaba desde ningún lado. Verificado: quita la pieza espuria en las vistas
        # lat=30 (18→17 piezas) y lat=0 (12→11) y no toca la polar, que no tiene
        # ninguna línea de retícula pasando por su antípoda.
        for trozo in partir_saltos(list(ga.coords)):
            for l in explotar(LineString(trozo).intersection(circulo)):
                go = a_ortho(l)
                for l2 in explotar(go):
                    grid_ml.append(list(l2.coords))


# ============================================================================
if args.fill:
    # --- MODO RELLENO: polígonos de tierra, recortados al disco ---
    gdf = gpd.read_file(args.land)
    if args.tolerance > 0:
        gdf['geometry'] = gdf.geometry.simplify(args.tolerance, preserve_topology=True)
    land_rings, ct, cd = [], 0, 0
    for geom in gdf.geometry:
        if not hay_visible(geom):              # descarta Antártida y demás del otro hemisferio
            continue
        ga = a_aeqd(geom)
        if ga is None or ga.is_empty:
            continue
        ga = ga.intersection(circulo)          # recorta el POLÍGONO al hemisferio visible
        for poli in polis_de(ga):
            ct += 1
            if bbox_diag(poli) < umbral_aeqd:  # filtro de extensión en AEQD
                cd += 1
                continue
            go = a_ortho(poli)
            for p2 in polis_de(go):
                land_rings.append(list(p2.exterior.coords))
    n_costa = len(land_rings)
else:
    # --- MODO LINE-ART: costas como líneas. Polígono de océano -> su frontera (sin
    #     costura del antimeridiano); líneas -> tal cual. ---
    gdf = gpd.read_file(args.costas)
    if gdf.geom_type.isin(['Polygon', 'MultiPolygon']).any():
        lineas = []
        for geom in gdf.geometry:
            b = geom.boundary
            for anillo in (b.geoms if b.geom_type == 'MultiLineString' else [b]):
                lineas.extend(sin_costura(anillo.coords))
        gdf = gpd.GeoDataFrame(geometry=lineas, crs=gdf.crs)
    if args.tolerance > 0:
        gdf['geometry'] = gdf.geometry.simplify(args.tolerance, preserve_topology=True)
    costas_ml, ct, cd = [], 0, 0
    for geom in gdf.geometry:
        if not hay_visible(geom):
            continue
        ga = a_aeqd(geom)
        if ga is None or ga.is_empty:
            continue
        ga = ga.intersection(circulo)
        for l in explotar(ga):
            ct += 1
            if bbox_diag(l) < umbral_aeqd:
                cd += 1
                continue
            go = a_ortho(l)
            for l2 in explotar(go):
                costas_ml.append(list(l2.coords))
    n_costa = len(costas_ml)


# --- Radio del disco en ORTHO (para normalizar coords a radio 1) ---
_todos = grid_ml + (land_rings if args.fill else costas_ml)
_pts = [p for c in _todos for p in c]
R = max(max(abs(x), abs(y)) for x, y in _pts)


# --- Emisión ---
def fmt(v):
    return f"{v/R:.4f}".rstrip('0').rstrip('.') or "0"

def bloque(coords):
    return "  ".join(f"{fmt(x)} {fmt(y)}" for x, y in coords)

modo = "relleno" if args.fill else "line-art"
L = []
L.append(f"% Mapa icónico ({modo}) — Natural Earth, proyección ortográfica.")
L.append(f"% Vista lat={lat_0} lon={lon_0}, normalizado a radio 1 (coords [-1,1], y-up).")
L.append(f"% INCLUIBLE (no dibuja hasta invocarse). Colócalo como satellite.mg:")
L.append(f"%     {args.nombre}(at=(10, 4.5), scale=1)              % retícula on")
L.append(f"%     {args.nombre}(at=(10, 4.5), scale=1, grid=false)  % solo tierra/costa")
if args.fill:
    L.append(f"%     {args.nombre}(..., ocean=\"lightblue\", land=\"tan\")   % otros colores")
L.append(f"%")
L.append(f"% GENERADO, no editar a mano. Regenerar con (tools/geo2mg.py, requiere geopandas + datos Natural Earth 110m):")
cmd = f"%   python3 hazmapa_mg.py --lat {lat_0} --lon {lon_0}{' --fill' if args.fill else ''} --tolerance {args.tolerance} --min-feature {args.min_feature} --grid-paso {args.grid_paso}"
if args.fill:
    cmd += f" --color-ocean {args.color_ocean} --color-tierra {args.color_tierra}"
L.append(cmd + f" --salida {args.salida}")
L.append(f"% Simplificación: DP {args.tolerance}° + filtro de extensión bbox<{args.min_feature}·R en AEQD")
L.append(f"%   (eliminó {cd} de {ct} features chicas). {n_costa} {'polígonos de tierra' if args.fill else 'trazos de costa'}, {len(grid_ml)} de retícula.")
L.append("")

if args.fill:
    L.append(f'struct {args.nombre}(grid=true, limb=true, ocean="{args.color_ocean}", land="{args.color_tierra}", grid_color="{args.grid_color}", grid_width={args.grid_width}) {{')
    L.append("  circle(1, fill=ocean) { 0 0 }        % disco de océano")
    L.append("")
    L.append("  % --- continentes rellenos ---")
    for c in land_rings:
        L.append(f"  polygon(fill=land) {{ {bloque(c)} }}")
else:
    L.append(f'struct {args.nombre}(grid=true, limb=true, grid_color="{args.grid_color}", grid_width={args.grid_width}) {{')
    L.append('  color "black"')
    L.append("  line_width 0.8")
    L.append("  if limb {")
    L.append("    circle(1) { 0 0 }        % limbo")
    L.append("  }")
    L.append("")
    L.append("  % --- costas ---")
    L.append("  line_width 0.4")
    for c in costas_ml:
        L.append(f"  polyline {{ {bloque(c)} }}")

if grid_ml:
    L.append("")
    L.append("  % --- retícula (meridianos y paralelos); grosor/color por parámetro ---")
    L.append("  if grid {")
    L.append("    color grid_color")
    L.append("    line_width grid_width")
    for c in grid_ml:
        L.append(f"    polyline {{ {bloque(c)} }}")
    L.append("  }")

if args.fill:
    L.append("")
    L.append("  % limbo (contorno) encima de tierra y retícula; `limb=false` lo quita")
    L.append("  if limb {")
    L.append('    color "black"')
    L.append("    line_width 0.8")
    L.append("    circle(1) { 0 0 }")
    L.append("  }")

L.append("}")

with open(args.salida, 'w') as f:
    f.write("\n".join(L) + "\n")

print(f"Generado: {args.salida}  (modo {modo}, struct incluible)")
print(f"  {n_costa} {'polígonos' if args.fill else 'trazos'} de tierra/costa (eliminadas {cd}/{ct} chicas), {len(grid_ml)} de retícula")
