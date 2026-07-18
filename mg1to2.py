#!/usr/bin/env python3
"""mg1to2.py -- traductor MetaGrafica V1 (comandos de dos letras) a V3.

Ver plan_mg1to2.md (raiz del repo) para la especificacion completa del
mapeo, y especificacion_mg.md para la gramatica V3 destino. Uso:

    python3 mg1to2.py entrada.mg [salida.mg]

Sin dependencias externas. Traductor stateful de una pasada: recorre el
stream de tokens V1 y emite lineas V3 directamente, sin construir un AST
intermedio (el mapeo es mayormente 1:1 comando-a-sentencia).
"""
import os
import re
import sys


class TranslateError(Exception):
    pass


# ---------------------------------------------------------------------------
# Scanner: cursor de caracteres sobre el fuente V1. Replica src/mgpp.l:
#   - '%' comenta hasta fin de linea (solo en modo normal).
#   - ':' y ';' son el mismo separador lexico (YSEP); el lector de listas de
#     numeros simplemente se detiene en el primer token no-numerico, asi que
#     aqui son cosmeticos -- se preserva ';' porque por CONVENCION separa
#     sub-trayectos repetidos dentro de un mismo PL/PG/BZ/... (el bucle
#     do-while(YSEP) de Parser.cpp).
#   - '}' (YCLOSE) tampoco tiene manejo en el switch principal del parser
#     real: es enteramente cosmetico.
#   - DT/XYDT/LCOLOR/FCOLOR/TSTYLE capturan el resto de la linea fisica
#     cruda vía capture_line(), replicando el estado <cadena> del lexer.
# ---------------------------------------------------------------------------

class Scanner:
    def __init__(self, text):
        self.s = text
        self.i = 0
        self.n = len(text)

    def _skip_ws_comments(self):
        while self.i < self.n:
            c = self.s[self.i]
            if c in ' \t\r\n':
                self.i += 1
            elif c == '%':
                while self.i < self.n and self.s[self.i] != '\n':
                    self.i += 1
            else:
                break

    def eof(self):
        self._skip_ws_comments()
        return self.i >= self.n

    def next_token(self):
        """Devuelve (kind, value) o None en EOF.
        kind in {'NUM', 'WORD', 'PATH', 'DOLLAR', 'PUNCT'}."""
        self._skip_ws_comments()
        if self.i >= self.n:
            return None
        c = self.s[self.i]
        if c in ':;}':
            self.i += 1
            return ('PUNCT', c)
        if c == '&':
            self.i += 1
            start = self.i
            while self.i < self.n and (self.s[self.i].isalnum() or self.s[self.i] == '_'):
                self.i += 1
            return ('PATH', self.s[start:self.i])
        if c == '$':
            self.i += 1
            start = self.i
            while self.i < self.n and self.s[self.i].isalpha():
                self.i += 1
            return ('DOLLAR', self.s[start:self.i])
        if c == '-' or c.isdigit() or c == '.':
            start = self.i
            self.i += 1
            saw_digit = c.isdigit()
            while self.i < self.n and (self.s[self.i].isdigit() or self.s[self.i] == '.'):
                if self.s[self.i].isdigit():
                    saw_digit = True
                self.i += 1
            text = self.s[start:self.i]
            if saw_digit:
                try:
                    return ('NUM', float(text))
                except ValueError:
                    pass
            # No era realmente un numero (p.ej. un '-' suelto) -- lo tratamos
            # como palabra de un caracter y seguimos.
            return ('WORD', text)
        if c.isalpha() or c == '_':
            start = self.i
            while self.i < self.n and (self.s[self.i].isalnum() or self.s[self.i] == '_'):
                self.i += 1
            return ('WORD', self.s[start:self.i])
        # Caracter desconocido: se ignora (no deberia aparecer en fuentes V1 validos).
        self.i += 1
        return self.next_token()

    def peek_token(self):
        save = self.i
        tok = self.next_token()
        self.i = save
        return tok

    def expect_num(self, what=''):
        tok = self.next_token()
        if tok is None or tok[0] != 'NUM':
            raise TranslateError(f'se esperaba un numero {what}, se obtuvo {tok!r}')
        return tok[1]

    def try_num(self):
        """Consume un NUM si el siguiente token lo es; si no, no avanza."""
        save = self.i
        tok = self.next_token()
        if tok is not None and tok[0] == 'NUM':
            return tok[1]
        self.i = save
        return None

    def expect_string(self, what=''):
        """Nombre/identificador (struct, path, color, etc) -- una palabra."""
        tok = self.next_token()
        if tok is None or tok[0] not in ('WORD', 'PATH'):
            raise TranslateError(f'se esperaba un identificador {what}, se obtuvo {tok!r}')
        return tok[1]

    def expect_filename(self, what=''):
        """Nombre de archivo crudo (p.ej. INPUT arrow.mg): replica el estado
        <incl> del lexer real (src/mgpp.l), que come espacios y luego lee
        todo hasta el siguiente espacio/newline SIN tokenizar -- el '.' de la
        extension no debe cortar el nombre como lo haria next_token()."""
        self._skip_ws_comments()
        start = self.i
        while self.i < self.n and self.s[self.i] not in ' \t\r\n':
            self.i += 1
        if self.i == start:
            raise TranslateError(f'se esperaba un nombre de archivo {what}')
        return self.s[start:self.i]

    def capture_line(self):
        """Captura el resto de la linea fisica cruda (sin saltar espacios ni
        comentarios) y descarta EXACTAMENTE el primer caracter, replicando
        &yylval.s[1] en Parser.cpp (DT/XYDT/LCOLOR/FCOLOR/TSTYLE)."""
        start = self.i
        while self.i < self.n and self.s[self.i] not in '\r\n':
            self.i += 1
        raw = self.s[start:self.i]
        return raw[1:] if raw else raw


def read_coords_or_ref(sc):
    """Lee un bloque de coordenadas: o bien una referencia &nombre, o una
    lista de pares x y (hasta el primer token no-numerico).

    Primero descarta separadores ':'/';' sueltos que hayan quedado sin
    consumir -- p.ej. tras los floats opcionales de CR/EL, donde el ':' que
    "separa params de la lista de puntos" no es mas que el primer token no
    numerico que detuvo ese bucle (ground truth: YSEP es cosmetico, ver
    Scanner)."""
    while True:
        tok = sc.peek_token()
        if tok is not None and tok[0] == 'PUNCT' and tok[1] in (':', ';'):
            sc.next_token()
            continue
        break
    tok = sc.peek_token()
    if tok is not None and tok[0] == 'PATH':
        sc.next_token()
        return ('ref', tok[1])
    pts = []
    while True:
        x = sc.try_num()
        if x is None:
            break
        y = sc.expect_num('(coordenada y que empareja a x)')
        pts.append((x, y))
    return ('pts', pts)


def consume_trailing_punct(sc):
    """Tras leer una lista de puntos, consume un ';' (senal de sub-trayecto
    repetido) o un '}' final si estan presentes. Devuelve True si vio ';'
    (i.e. hay que repetir la primitiva)."""
    tok = sc.peek_token()
    if tok is not None and tok[0] == 'PUNCT' and tok[1] == ';':
        sc.next_token()
        return True
    if tok is not None and tok[0] == 'PUNCT' and tok[1] == '}':
        sc.next_token()
    return False


def fmt(x):
    """Formatea un float sin ceros ni puntos de sobra."""
    if x == int(x):
        return str(int(x))
    s = f'{x:.6f}'.rstrip('0').rstrip('.')
    return s if s not in ('', '-') else '0'


def fmt_pts(pts):
    return '  '.join(f'{fmt(x)} {fmt(y)}' for x, y in pts)


def capitalize(name):
    """Nombres de struct V1 -> V3: Capitalizados, para distinguirlos de
    comandos/palabras reservadas (especificacion_mg.md:1573)."""
    return name[0].upper() + name[1:] if name else name


def combine_scale(pending, factor):
    """Combina la escala ST pendiente (SCST, string "N" o "(sx, sy)") con el
    factor docwmin (§6.1) -- devuelve None si no hace falta emitir scale=."""
    if pending is None:
        return None if factor == 1.0 else fmt(factor)
    if pending.startswith('('):
        sx_s, sy_s = (p.strip() for p in pending[1:-1].split(','))
        return f'({fmt(float(sx_s) * factor)}, {fmt(float(sy_s) * factor)})'
    return fmt(float(pending) * factor)


class Affine:
    """Matriz afin 2x3 (fila-mayor; fila inferior [0,0,1] implicita) que replica
    include/matrix.h EXACTAMENTE: `translate`/`scale` POST-multiplican (M = M.A,
    mismo orden que Matrix::matmat) y `apply` es Matrix::transform. Es el canal PT
    (mtpt) del ground truth: acumula TLPT/SCPT hasta un IDPT. Solo se usa para
    resolver `BZ/PL/PG &nombre` (ver resolve_raw_through_mtpt)."""
    __slots__ = ('a', 'b', 'c', 'd', 'e', 'f')

    def __init__(self, a=1.0, b=0.0, c=0.0, d=0.0, e=1.0, f=0.0):
        self.a, self.b, self.c, self.d, self.e, self.f = a, b, c, d, e, f

    def translate(self, tx, ty):  # M = M . T
        self.c = self.a * tx + self.b * ty + self.c
        self.f = self.d * tx + self.e * ty + self.f

    def scale(self, sx, sy):      # M = M . S
        self.a *= sx
        self.d *= sx
        self.b *= sy
        self.e *= sy

    def apply(self, x, y):        # Matrix::transform
        return (self.a * x + self.b * y + self.c,
                self.d * x + self.e * y + self.f)


def current_body_extent(state):
    """(wdx, wdy) de la world_window vigente del cuerpo (unitaria si no hay)."""
    if state.body_ww is None:
        return (1.0, 1.0)
    x0, x1, y0, y1 = state.body_ww
    return (x1 - x0, y1 - y0)


def current_body_origin(state):
    """(wmx, wmy) de la world_window vigente del cuerpo (origen si no hay)."""
    if state.body_ww is None:
        return (0.0, 0.0)
    x0, _x1, y0, _y1 = state.body_ww
    return (x0, y0)


def resolve_raw_through_mtpt(state, pts):
    """Aplica el canal PT (mtpt) a un path CRUDO y lo devuelve en unidades de
    DATO de la ventana vigente, tal como V1 dibuja `BZ &nombre`. En V1
    process_path(mtpt, crudo) da coords NORMALIZADAS [0,1]^2 (mtpt compone
    traslaciones YA normalizadas por WW -- TLPT hace mt.translate(dx/wdx,dy/wdy)
    -- y escalas dimensionales); el punto NO se re-normaliza. Como V3 SI
    re-normaliza lo que emitimos, hay que des-normalizar (x*wd+wm) para que la
    ida y vuelta coincida con el dibujo de V1."""
    wdx, wdy = current_body_extent(state)
    wmx, wmy = current_body_origin(state)
    out = []
    for x, y in pts:
        nx, ny = state.path_mtpt.apply(x, y)
        out.append((nx * wdx + wmx, ny * wdy + wmy))
    return out


def _concat_dist2(a, b):
    return (a[0] - b[0]) ** 2 + (a[1] - b[1]) ** 2


def concat_paths_py(a, b):
    """Replica src/splines.cpp concat_paths(a, b): suelda dos paths por el par
    de extremos MAS CERCANO (invierte a/b si conviene), traslada b para que
    empiece donde termina a y salta el punto duplicado. Sin dependencia de la
    orientacion de los operandos (RPPT tesela N copias identicas asi)."""
    p1, p2 = list(a), list(b)
    if not p1:
        return p2
    if not p2:
        return p1
    a0, a1 = p1[0], p1[-1]
    b0, b1 = p2[0], p2[-1]
    d = [_concat_dist2(a1, b0), _concat_dist2(a1, b1),
         _concat_dist2(a0, b0), _concat_dist2(a0, b1)]
    best = d.index(min(d))       # primer minimo (empata al indice menor, como el for <)
    if best in (2, 3):
        p1.reverse()             # p1 termina en el punto comun
    if best in (1, 3):
        p2.reverse()             # p2 empieza en el punto comun
    tail, head = p1[-1], p2[0]
    dx, dy = tail[0] - head[0], tail[1] - head[1]
    for i in range(1, len(p2)):  # salta el punto duplicado
        p1.append((p2[i][0] + dx, p2[i][1] + dy))
    return p1


def fmt_block(state, ref_or_pts, apply_mtpt=False):
    """Formatea un bloque de coordenadas ya leido por read_coords_or_ref. Una
    referencia &nombre se resuelve a puntos LITERALES si el traductor conoce
    ese path (computed_paths = resultado de PWPT/RPPT, se consume una vez -- ver
    State.computed_paths --, o raw_paths = literal de biblioteca §9); si no,
    se deja como &nombre (referencia real a un path V3, caso no usado hoy).

    `apply_mtpt` (solo lo pasan las primitivas de puntos PL/PG/BZ/SP): un `&ref`
    CRUDO se transforma por el canal PT (mtpt) + des-normalizacion, igual que
    parsePath -> process_path(mtpt, ...) en V1. El buffer (computed_paths) NO se
    transforma: ya viene en coords de dato (PWPT/RPPT lo dejaron listo)."""
    kind, val = ref_or_pts
    if kind == 'ref':
        if val in state.computed_paths:
            return fmt_pts(state.computed_paths.pop(val))
        if val in state.raw_paths:
            pts = state.raw_paths[val]
            if apply_mtpt:
                pts = resolve_raw_through_mtpt(state, pts)
            return fmt_pts(pts)
        return f'&{val}'
    return fmt_pts(val)


# ---------------------------------------------------------------------------
# Estado del traductor
# ---------------------------------------------------------------------------

class State:
    def __init__(self):
        self.indent = 0
        # Relleno: V1 separa el booleano FILL/NOFILL del valor (FCOLOR/FGRAY/
        # FPATRN). V3 modela "fill" como un solo valor de estado (presencia =
        # relleno). Se traduce enlazando ambos: FILL/NOFILL emite el valor
        # actual o "none"; FCOLOR/FGRAY solo re-emiten si el relleno esta
        # activo.
        self.fill_on = False
        self.fill_value = '"black"'  # default V1: negro
        self.fill_outline = False    # FCOLOR/FGRAY con signo negativo
        self.outline_on = False      # espejo de dspstate.outlinefill (estado global)
        self.line_color = '"black"'  # espejo de dspstate.linecolor (LCOLOR/LGRAY)
        # hatch: NO se modela como estado global -- `hatch "none"` es un no-op
        # silencioso en el parser V3 (estilo desconocido -> patron vacio ->
        # emitHatch retorna sin emitir nada, asi que el hatch previo se queda
        # pegado). Se ataca en vez como atributo por-primitiva, "one-shot":
        # FPATRN deja pending_hatch, y el siguiente primitivo lo consume.
        self.pending_hatch = None  # (repr_str, gap, outline) o None

        # --- Structs, placements y transforms (paso 4) ---
        self.struct_name_map = {}     # nombre V1 (minuscula) -> Nombre V3 (Capitalizado)
        self.marked_struct = None     # ultimo MKST: struct implicita de LNST/ARCST/PWST/RPST
        # Bloque `{ }` de transform local (TLLC/RTLC/SCLC/SHLC ... IDLC). No es
        # una pila real: en el corpus objetivo (arrow/rpstest/primitives) un
        # bloque TLLC nunca envuelve un OPST, asi que un solo flag alcanza;
        # ver nota en handle_OPST/handle_CLST.
        self.lc_block_open = False
        # Ops del bloque LC vigente (strings 'rotate 90', 'translate 1 2', ...)
        # en orden de emision. Sirven para dos cosas: (1) re-abrir el bloque
        # tras el desvio de un DT (ver lc_pending_ops), y (2) detectar si el
        # bloque esta "fresco" (solo transforms, sin dibujos) para retractarlo
        # limpiamente en handle_DT. lc_block_start_idx = indice en `out` de la
        # linea '{' que abrio el bloque.
        self.lc_block_ops = []
        self.lc_block_start_idx = -1
        # mtlc conceptualmente ACTIVO pero con el bloque `{ }` cerrado: pasa
        # cuando un DT lo interrumpe (emite su propio bloque canonico
        # translate+ops). El siguiente dibujo/transform re-abre el bloque
        # replicando estas ops (reopen_lc_if_pending); IDLC/CLST las limpia.
        self.lc_pending_ops = None
        # $O (orientacion tangente de placements, §C.1): sticky, como
        # orient_next en Parser.cpp. Con $O 0 (default), un identifier
        # placement multipunto son N instancias SIN rotar (StructurePath
        # orient=false) -> N invocaciones sueltas en V3; con $O 1 se orientan
        # a la tangente -> place() (que orienta siempre).
        self.orient = False
        # Canal ST (MTST): escala/rotacion FIJA de cada copia en un place()/
        # repeat()/invocacion directa -- prendida por SCST, reseteada por IDST.
        self.pending_struct_scale = None   # str o None
        self.pending_struct_rotate = None  # num o None
        # Canal RS (MTRS): transform= ACUMULADO de repeat() (RTRS/SCRS/...).
        self.rs_transform_parts = []
        # Canal PP (MTPP): avance de pluma para generadores/RPST (TLPP/IDPP).
        self.pen = (0.0, 0.0)
        self.advance = (0.0, 0.0)
        # Canal PT (MTPT): paso usado por GNPATH (TLPT/IDPT) -- ver §6.4 del
        # plan: GNPATH no genera puntos, guarda los PARAMETROS del generador
        # (x0, y0, dx, dy, n); el consumidor real es `DOT s &nombre`, que se
        # traduce a un for-loop (no existe trail() en el parser V3).
        self.path_step = (0.0, 0.0)
        # Canal PT como MATRIZ afin real (TLPT/SCPT/IDPT que ACUMULAN, ground
        # truth Parser.cpp mtpt): lo usan `BZ/PL/PG &nombre` y RPPT (fig4-10).
        # path_step (arriba) se conserva aparte para el for-loop de GNPATH+DOT
        # (fig6-10), que solo ve TLPT/IDPT (traslacion pura) -- son dos vistas
        # del mismo canal; se actualizan juntas en handle_pt_transform.
        self.path_mtpt = Affine()
        self.named_paths = {}  # nombre -> (x0, y0, dx, dy, n)
        self.gensym_counter = 0
        # &nombre a nivel de sentencia (fuera de OPST): literales de path que
        # las bibliotecas V1 (p.ej. bzsinepaths.mg) definen para que otros
        # archivos los referencien con `BZ &nombre`/`PWPT nombre ...` (§9,
        # ground truth Parser.cpp:751 -- listmap[name] = parsePath()). Se
        # resuelven en PYTHON (coordenadas literales), no vía el `&nombre`/
        # `path` de V3: V3 no tiene todavia scale/translate de un PathExpr
        # (solo transpose/flip_x/flip_y/concat), asi que no hay como delegarle
        # el calculo de PWPT/TLPT+SCPT.
        self.raw_paths = {}
        # Resultado de operaciones de algebra de paths (hoy solo PWPT) que se
        # escriben en el "buffer" V1 (bufferpt) y se consumen -y limpian- con
        # `&buffer` (ground truth Parser.cpp: dereferenciar "buffer" hace
        # bufferpt.clear()).
        self.computed_paths = {}
        self.source_dir = None  # carpeta del .mg de entrada, para resolver INPUT
        # OPPT/CLPT (compound { }): mientras esta abierto, los primitivos
        # internos NO deben repetir hatch/color por su cuenta -- ya lo carga
        # el compound(...) que los envuelve (build_head lo consulta).
        self.in_compound = False
        # §6.1 del plan / ground truth (Parser.cpp:357-362): la escala de un
        # placement/invocacion a nivel de DOCUMENTO (fuera de cualquier
        # struct) se multiplica por docwmin = min(wdx, wdy) del world_window
        # vigente en ese nivel -- en V1 la caja unitaria de la struct media
        # min(canvas); en V3 (isometrico) mide 1 unidad de mundo. Dentro de
        # un cuerpo de struct (depth>0) no aplica (stf=1 alla).
        self.in_struct_depth = 0
        self.doc_ww = None  # (x0, x1, y0, y1) del ultimo world_window a depth 0
        # §6.1bis -- WW a MITAD de cuerpo (fig4-1: `MKST MARCO / PWST 0 0 1 1 /
        # WW 0 2.2 0 1`): en V1 cada coordenada se normaliza con la ventana
        # vigente EN SU LINEA; en V3 la world_window es UNA por cuerpo y
        # aplica a todo el (la ultima gana). Para que coincidan, al llegar un
        # WW se RE-EXPRESAN las coordenadas ya emitidas de este cuerpo (desde
        # body_patch_from) de la ventana anterior (body_ww; None = unitaria,
        # el estado inicial de wdx=wdy=1 en parsePrimitives) a la nueva:
        # x' = nx0 + (x-ox0)/odx * ndx. Ver patch_window_change().
        self.body_ww = None
        self.body_patch_from = 0
        # Pila de estado "local a un cuerpo de struct": ground truth
        # Parser.cpp::parsePrimitives() declara mtst/mtpp/mtrs/pp/st como
        # variables LOCALES de C++ -- cada OPST anidado arranca con matrices
        # identidad/NULL frescas, y al volver (CLST) el frame del padre sigue
        # como estaba (nunca las comparte con el hijo). Aqui state es un solo
        # objeto plano reusado en toda la traduccion, asi que hay que
        # emular eso con push-reset-al-entrar / pop-restaurar-al-salir; sin
        # esto, un RTST/SCST/TLPP/MKST dejado sin resetear DENTRO de un
        # struct se filtraba a structs hermanos posteriores (bug real: en
        # fig4-1.mg, `RTST 90` de OPST FG1 sobrevivia hasta la invocacion de
        # `flechas` en OPST FIG1, mucho despues, sin relacion alguna).
        self.struct_local_stack = []

    def docwmin(self):
        if self.doc_ww is None:
            return 1.0
        x0, x1, y0, y1 = self.doc_ww
        return min(x1 - x0, y1 - y0)

    def out_line(self, out, line):
        out.append(('    ' * self.indent) + line)

    def take_pending_hatch(self, force=False):
        """Ojo: NO es one-shot -- FPATRN es estado PERSISTENTE en V1 (se
        aplica a todos los primitivos hasta que cambia), asi que esto se
        re-emite como atributo por-primitiva en CADA llamada mientras siga
        activo (verificado: primitives.mg dibuja PG+CR+EL+BR bajo un solo
        FPATRN -2, y el oraculo trama las 4 formas, no solo la primera).
        Solo FPATRN (idx 0 o nuevo) y los limites de struct lo modifican.

        Con relleno apagado (NOFILL) NO se emite -- `emitHatch` en
        parserv3.cpp empuja GS_FILL incondicionalmente, asi que adjuntar
        hatch= a un primitivo mientras `fill "none"` esta vigente lo
        rellenaria por accidente (visto en fig6-10.mg: BR con FPATRN activo
        seguido de NOFILL + un segundo BR que debe quedar SIN relleno).

        EXCEPCION -- force=True (polygon): en V1 un PG SIEMPRE se rellena,
        haya o no FILL (ground truth v1-legacy Polyline::draw: `if
        (type==GI_POLYGON && !filled) g.setFilled(true)` temporal), asi que
        el patron vigente SI le aplica aunque fill_on sea False (verificado
        contra el oraculo de fig6-1: `FPATRN 2 / PG ...` sin FILL rinde
        fill="url(#mgpat_2_...)"). El motor V3 conserva el auto-relleno del
        polygon, solo hay que dejarle pasar el hatch."""
        if self.pending_hatch is None or not (self.fill_on or force):
            return []
        rep, gap, outline = self.pending_hatch
        attrs = [f'hatch={rep}', f'hatch_gap={fmt(gap)}']
        if outline:
            # Un color= por-primitiva es lo que dispara GS_OUTLINEFILL
            # automatico (parserv3.cpp emitPrimStyle: color+hatch =>
            # outlinefill), pero el VALOR debe ser el color de linea vigente
            # (LCOLOR/LGRAY), no un negro fijo -- si el V1 traia `LCOLOR
            # green` antes del FPATRN negativo, el contorno es verde.
            attrs.append(f'color={self.line_color}')
        return attrs

    def build_head(self, name, positional_attrs):
        # force: los polygon auto-rellenan en V1/V3 (ver take_pending_hatch).
        extra = [] if self.in_compound else self.take_pending_hatch(force=(name == 'polygon'))
        attrs = list(positional_attrs) + extra
        if attrs:
            return f'{name}({", ".join(attrs)})'
        return name


def emit_fill_state(state, out):
    if state.fill_on:
        state.out_line(out, f'fill {state.fill_value}')
    else:
        state.out_line(out, 'fill "none"')
    # El contorno de un relleno solo se dibuja con el flag global explicito
    # dspstate.outlinefill (verificado: solo poner `color` NO alcanza --
    # SVGDisplay::getStyleAttributes traza solo si `!dspstate.fill ||
    # dspstate.outlinefill`). Se traduce como estado persistente igual que
    # fill, con su propio reset explicito (§6.3). NO se toca `color` aqui:
    # el trazo usa dspstate.linecolor tal cual, que ya viene de LCOLOR/LGRAY
    # (o negro por default si nunca se fijo) -- forzar un valor aqui
    # pisaria un LCOLOR previo (bug real visto en primitives.mg: LCOLOR
    # green + FCOLOR -cyan debia contornear en VERDE, no en negro fijo).
    want_outline = state.fill_on and state.fill_outline
    if want_outline != state.outline_on:
        state.out_line(out, 'outlinefill' if want_outline else 'outlinefill false')
        state.outline_on = want_outline


# ---------------------------------------------------------------------------
# Primitivas
# ---------------------------------------------------------------------------

def handle_point_primitive(sc, state, out, v3name):
    """PL/PG/BZ/SP: lista de puntos (o &ref), repetible con ';'."""
    reopen_lc_if_pending(state, out)
    while True:
        block = read_coords_or_ref(sc)
        if block[0] == 'pts' and not block[1]:
            break
        head = state.build_head(v3name, [])
        state.out_line(out, f'{head} {{ {fmt_block(state, block, apply_mtpt=True)} }}')
        if not consume_trailing_punct(sc):
            break


def handle_BR(sc, state, out):
    """BR: lista generica de puntos, cada par = un rectangulo (igual que en
    V1: Rectangle::draw consume el path de a pares)."""
    reopen_lc_if_pending(state, out)
    while True:
        block = read_coords_or_ref(sc)
        if block[0] == 'pts' and not block[1]:
            break
        head = state.build_head('rectangle', [])
        state.out_line(out, f'{head} {{ {fmt_block(state, block)} }}')
        if not consume_trailing_punct(sc):
            break


def handle_CR(sc, state, out):
    reopen_lc_if_pending(state, out)
    r = [sc.expect_num('(radio de CR)')]
    while len(r) < 8:
        v = sc.try_num()
        if v is None:
            break
        r.append(v)
    k = len(r)
    if k == 1:
        positional = [fmt(r[0])]
        name = 'circle'
    elif k == 2:
        positional = [fmt(r[0]), 'from=0', f'to={fmt(r[1])}']
        name = 'arc'
    else:
        a0 = r[2]
        da = r[1]
        positional = [fmt(r[0]), f'from={fmt(a0)}', f'to={fmt(a0 + da)}']
        name = 'arc'
    block = read_coords_or_ref(sc)
    head = state.build_head(name, positional)
    state.out_line(out, f'{head} {{ {fmt_block(state, block)} }}')
    consume_trailing_punct(sc)


def handle_EL(sc, state, out):
    reopen_lc_if_pending(state, out)
    r = [sc.expect_num('(rx de EL)'), sc.expect_num('(ry de EL)')]
    while len(r) < 8:
        v = sc.try_num()
        if v is None:
            break
        r.append(v)
    k = len(r)
    if k == 2:
        positional = [fmt(r[0]), fmt(r[1])]
        name = 'ellipse'
    elif k == 3:
        positional = [f'rx={fmt(r[0])}', f'ry={fmt(r[1])}', 'from=0', f'to={fmt(r[2])}']
        name = 'arc'
    else:
        a0 = r[3]
        da = r[2]
        positional = [f'rx={fmt(r[0])}', f'ry={fmt(r[1])}', f'from={fmt(a0)}', f'to={fmt(a0 + da)}']
        name = 'arc'
    block = read_coords_or_ref(sc)
    head = state.build_head(name, positional)
    state.out_line(out, f'{head} {{ {fmt_block(state, block)} }}')
    consume_trailing_punct(sc)


def emit_gnpath_dot_loop(state, out, name, r):
    """§6.4 del plan: `DOT s &nombre` sobre un path generado por GNPATH no
    tiene equivalente directo (no existe trail() en parserv3.cpp) -- se
    expande a un for-loop, precomputando la coordenada aritmetica en una
    variable (el bloque `{ }` usa parseTerm, sin +/- binario).

    `DOT s &nombre` puede repetirse para el MISMO nombre con un `TLPT dx dy`
    de por medio (visto en fig6-10.mg: dos columnas de ocupacion espejadas) --
    el plan (§6.4) lo describe como "otro bucle desplazado". Aqui el path_step
    VIGENTE al momento de este DOT (normalmente (0,0) recien resetado por un
    IDPT, o el delta de un TLPT posterior) se suma como offset extra sobre la
    base del generador, sin mutar la entrada guardada en named_paths."""
    x0, y0, dx, dy, n = state.named_paths[name]
    ox, oy = state.path_step
    x0, y0 = x0 + ox, y0 + oy
    state.gensym_counter += 1
    k = state.gensym_counter
    iv, xv, yv = f'_gpi{k}', f'_gpx{k}', f'_gpy{k}'
    head = state.build_head('dot', [fmt(r)])
    state.out_line(out, f'for {iv} = 0 to {n - 1} {{')
    state.indent += 1
    state.out_line(out, f'{xv} = {fmt(x0)} + {iv}*{fmt(dx)}')
    state.out_line(out, f'{yv} = {fmt(y0)} + {iv}*{fmt(dy)}')
    state.out_line(out, f'{head} {{ {xv} {yv} }}')
    state.indent -= 1
    state.out_line(out, '}')


def handle_DOT(sc, state, out):
    # ⚠ calibrar factor de tamaño fisico contra el oraculo (plan_mg1to2.md §6.4/§1).
    reopen_lc_if_pending(state, out)
    diam = sc.expect_num('(diametro de DOT)')
    r = diam / 2.0
    tok = sc.peek_token()
    if tok is not None and tok[0] == 'PATH' and tok[1] in state.named_paths:
        sc.next_token()
        emit_gnpath_dot_loop(state, out, tok[1], r)
        consume_trailing_punct(sc)
        return
    block = read_coords_or_ref(sc)
    head = state.build_head('dot', [fmt(r)])
    state.out_line(out, f'{head} {{ {fmt_block(state, block)} }}')
    consume_trailing_punct(sc)


def wrap_math(text):
    """V1 interpreta markup TeX (`_`/`^`/`\\comando`) en CUALQUIER texto; V3
    solo activa sub/superindice y comandos griegos dentro de `$...$` (modo
    matematico, §14 de la spec / CLAUDE.md). Heuristica: si el texto capturado
    trae alguno de esos tres, envolver TODO el string en $...$ -- coincide con
    la convencion ya usada en el corpus V3 a mano (sines.mg, fig6-1.mg:
    `text("$\\psi_I$")`). No perfecto (un `_` literal no-matematico se
    envolveria tambien) pero no se ha visto ese caso en el corpus V1."""
    if any(c in text for c in ('_', '^', '\\')):
        return f'${text}$'
    return text


def handle_DT(sc, state, out):
    """DT texto: dibuja EN LA PLUMA y luego la avanza con mtpp (ground truth
    Parser.cpp case YDT: gs->setPosition(pp) + mtpp.transform(pp)). Un
    text() de V3 SIN bloque de coordenadas no dibuja NADA (TextStmt::exec
    itera los pares de coords), asi que la posicion va siempre explicita.

    Bajo un transform local abierto (XYPP px py / RTLC a / DT texto): V1 NO
    afectaba al texto (la posicion ya estaba fijada por el XYPP previo, y V1
    nunca supo rotar glifos -- los oraculos de fig2-3 "heat capacity" y
    fig6-10 "energy" los muestran HORIZONTALES). En V3 el texto bajo rotate
    SI gira los glifos (§19), que es lo que el autor claramente queria; se
    emite el idioma canonico del plan §5 (¡y del port a mano de fig6-10!):
    `{ translate px py  <ops del bloque>  text("...") { 0 0 } }`.
    DESVIACION DELIBERADA respecto al oraculo V1: la etiqueta sale rotada.
    Si el bloque abierto esta "fresco" (solo transforms) se retracta para no
    dejar un bloque vacio; si ya tenia dibujos, se cierra normal. En ambos
    casos las ops quedan pendientes (lc_pending_ops) por si mas dibujos
    siguen antes del IDLC."""
    text = wrap_math(sc.capture_line())
    px, py = state.pen
    if state.lc_block_open or state.lc_pending_ops is not None:
        if state.lc_block_open:
            ops = state.lc_block_ops
            fresh = len(out) == state.lc_block_start_idx + 1 + len(ops)
            if fresh:
                del out[state.lc_block_start_idx:]
                state.indent -= 1
            else:
                state.indent -= 1
                state.out_line(out, '}')
            state.lc_block_open = False
        else:
            ops = state.lc_pending_ops
        state.out_line(out, '{')
        state.indent += 1
        state.out_line(out, f'translate {fmt(px)} {fmt(py)}')
        for op in ops:
            state.out_line(out, op)
        state.out_line(out, f'text("{text}") {{ 0 0 }}')
        state.indent -= 1
        state.out_line(out, '}')
        state.lc_pending_ops = ops
    else:
        state.out_line(out, f'text("{text}") {{ {fmt(px)} {fmt(py)} }}')
    ax, ay = state.advance
    state.pen = (px + ax, py + ay)


def handle_XYDT(sc, state, out):
    # XYDT cae SIN break en el case YDT del parser V1 (Parser.cpp:816): fija
    # la pluma (pp = parsePoint()) y dibuja ahi; NO la avanza (el avance esta
    # condicionado a lastyylex==YDT).
    x = sc.expect_num('(x de XYDT)')
    y = sc.expect_num('(y de XYDT)')
    state.pen = (x, y)
    text = wrap_math(sc.capture_line())
    state.out_line(out, f'text("{text}") {{ {fmt(x)} {fmt(y)} }}')


# ---------------------------------------------------------------------------
# Estilo
# ---------------------------------------------------------------------------

DASH_NAMES = {0: 'solid', 1: 'longdashed', 2: 'dashed', 3: 'dotted', 4: 'dashdot', 5: 'dashdotdot'}


def handle_LWIDTH(sc, state, out):
    w = sc.expect_num('(LWIDTH)')
    state.out_line(out, f'line_width {fmt(w * 0.2)}')


def handle_LPATRN(sc, state, out):
    n = int(sc.expect_num('(LPATRN)'))
    name = DASH_NAMES.get(n, 'solid')
    state.out_line(out, f'dash "{name}"')


def parse_color_name(sc):
    raw = sc.capture_line().strip()
    name = raw.split()[0] if raw.split() else raw
    outline = name.startswith('-')
    if outline:
        name = name[1:]
    return name, outline


def handle_LCOLOR(sc, state, out):
    name, _outline = parse_color_name(sc)
    state.line_color = f'"{name}"'
    state.out_line(out, f'color {state.line_color}')


def handle_FCOLOR(sc, state, out):
    name, outline = parse_color_name(sc)
    state.fill_value = f'"{name}"'
    state.fill_outline = outline
    if state.fill_on:
        emit_fill_state(state, out)


def handle_LGRAY(sc, state, out):
    g = sc.expect_num('(LGRAY)')
    state.line_color = f'gray({fmt(g / 100)})'
    state.out_line(out, f'color {state.line_color}')


def handle_FGRAY(sc, state, out):
    g = sc.expect_num('(FGRAY)')
    outline = g < 0
    g = abs(g)
    state.fill_value = f'gray({fmt(g / 100)})'
    state.fill_outline = outline
    if state.fill_on:
        emit_fill_state(state, out)


def handle_FILL(sc, state, out):
    state.fill_on = True
    emit_fill_state(state, out)


def handle_NOFILL(sc, state, out):
    state.fill_on = False
    state.out_line(out, 'fill "none"')


def hatch_index_to_angle_gap(idx):
    angle = ((idx - 1) % 4) * 45
    d = (idx - 1) // 4
    gap = 4 // (1 + d)
    return angle, gap


def handle_FPATRN(sc, state, out):
    # hatch NO se modela como estado global (ver State.pending_hatch: `hatch
    # "none"` es un no-op silencioso en el parser V3, no hay forma de
    # apagarlo una vez prendido). FPATRN deja un atributo pendiente que el
    # siguiente primitivo consume una sola vez -- coincide con el uso real
    # del corpus (FPATRN siempre precede inmediatamente a su primitivo).
    n = sc.expect_num('(FPATRN)')
    idx = int(n)
    if idx == 0:
        state.pending_hatch = None
        return
    outline = idx < 0
    idx = abs(idx)
    angle, gap = hatch_index_to_angle_gap(idx)
    state.pending_hatch = (fmt(angle), gap, outline)


def handle_TSTYLE(sc, state, out):
    name, _ = parse_color_name(sc)
    state.out_line(out, f'font "{name}"')


def handle_THEIGHT(sc, state, out):
    h = sc.expect_num('(THEIGHT/TSIZE)')
    state.out_line(out, f'font_size {fmt(h)}')


TALIGN_NAMES = {0: 'left', 1: 'center', 2: 'right'}


def handle_TALIGN(sc, state, out):
    n = int(sc.expect_num('(TALIGN)'))
    state.out_line(out, f'align "{TALIGN_NAMES.get(n, "left")}"')


# ---------------------------------------------------------------------------
# Pluma y generadores (subconjunto necesario para el hito fill_styles/line_patterns)
# ---------------------------------------------------------------------------

def handle_XYPP(sc, state, out):
    x = sc.expect_num('(x de XYPP)')
    y = sc.expect_num('(y de XYPP)')
    state.pen = (x, y)


def handle_TLPP(sc, state, out):
    # ACUMULA (no sobreescribe): en V1 cada TLPP compone sobre mtpp
    # (oldParseMatrix OPMTL: mt.translate) hasta el IDPP que la resetea; para
    # traslaciones puras la composicion es la suma. En el corpus siempre hay
    # IDPP de por medio, pero el acumulado es el ground truth.
    dx = sc.expect_num('(dx de TLPP)')
    dy = sc.expect_num('(dy de TLPP)')
    ax, ay = state.advance
    state.advance = (ax + dx, ay + dy)


def handle_GNNUM(sc, state, out):
    i0 = sc.expect_num('(GNNUM i0)')
    inc = sc.expect_num('(GNNUM inc)')
    n = sc.expect_num('(GNNUM n)')
    dec = int(sc.expect_num('(GNNUM decimales)'))
    at = getattr(state, 'pen', (0, 0))
    adv = getattr(state, 'advance', (0, 0))
    state.out_line(
        out,
        f'numbers(from={fmt(i0)}, by={fmt(inc)}, count={fmt(n)}, decimals={dec}, '
        f'at=({fmt(at[0])}, {fmt(at[1])}), advance=({fmt(adv[0])}, {fmt(adv[1])}))',
    )


def handle_TICKS(sc, state, out):
    n = sc.expect_num('(TICKS n)')
    dx = sc.expect_num('(TICKS dx)')
    dy = sc.expect_num('(TICKS dy)')
    at = state.pen
    adv = state.advance
    state.out_line(
        out,
        f'ticks({fmt(int(n))}, mark=({fmt(dx)}, {fmt(dy)}), '
        f'at=({fmt(at[0])}, {fmt(at[1])}), advance=({fmt(adv[0])}, {fmt(adv[1])}))',
    )


def handle_GNPATH(sc, state, out):
    # No genera puntos aqui -- guarda los parametros del generador; el
    # consumidor real es `DOT s &nombre` (ver emit_gnpath_dot_loop, §6.4).
    n = sc.expect_num('(GNPATH n)')
    x0 = sc.expect_num('(GNPATH x0)')
    y0 = sc.expect_num('(GNPATH y0)')
    name = sc.expect_string('(GNPATH nombre de path)')
    dx, dy = state.path_step
    state.named_paths[name] = (x0, y0, dx, dy, int(n))


def harvest_named_paths(text):
    """Pre-escanea el fuente V1 de un archivo INPUT-eado en busca de
    literales de path a nivel de sentencia (`&nombre x y ... }`, ground
    truth Parser.cpp:751 -- listmap[name] = parsePath()). Ignora todo lo
    demas token a token: bzsinepaths.mg (unica biblioteca del corpus que usa
    este patron) no tiene mas contenido que estas declaraciones, pero el
    escaneo es seguro en general porque '&' solo aparece en estas
    definiciones o en referencias (que este escaneo simplemente reinterpreta
    como si fueran otra definicion del mismo path -- inofensivo, se
    sobreescribe con los mismos puntos)."""
    sc = Scanner(text)
    paths = {}
    while True:
        tok = sc.next_token()
        if tok is None:
            break
        if tok[0] == 'PATH':
            block = read_coords_or_ref(sc)
            # Solo bloques NO vacios: un &nombre de REFERENCIA (p.ej. `BZ
            # &sinpi` mas abajo en la misma biblioteca) no trae numeros
            # detras y no debe sobreescribir la definicion real con [].
            if block[0] == 'pts' and block[1]:
                consume_trailing_punct(sc)
                paths[tok[1]] = block[1]
    return paths


def handle_INPUT(sc, state, out):
    # INPUT no pasa por keyword_map en V1 (es una regla de lexer aparte que
    # conmuta a un estado <incl> y lee el siguiente token como nombre de
    # archivo) -- aqui basta leer la palabra siguiente.
    fname = sc.expect_filename('(nombre de archivo en INPUT)')
    if not fname.endswith('.mg'):
        fname += '.mg'
    state.out_line(out, f'include "{fname}"')
    if state.source_dir is not None:
        sibling = os.path.join(state.source_dir, fname)
        if os.path.isfile(sibling):
            with open(sibling, 'rb') as f:
                raw = f.read()
            try:
                text = raw.decode('utf-8')
            except UnicodeDecodeError:
                text = raw.decode('latin-1')
            state.raw_paths.update(harvest_named_paths(text))


# ---------------------------------------------------------------------------
# Structs (OPST/CLST/MKST)
# ---------------------------------------------------------------------------

def resolve_struct_name(state, name):
    return state.struct_name_map.get(name, capitalize(name))


def close_lc_block_if_open(state, out):
    if state.lc_block_open:
        state.indent -= 1
        state.out_line(out, '}')
        state.lc_block_open = False
    state.lc_block_ops = []
    state.lc_pending_ops = None


def open_lc_block(state, out):
    state.lc_block_start_idx = len(out)
    state.out_line(out, '{')
    state.indent += 1
    state.lc_block_open = True


def reopen_lc_if_pending(state, out):
    """mtlc sigue conceptualmente activo (un DT cerro el bloque `{ }` para
    emitir su bloque canonico propio, ver handle_DT): el siguiente dibujo lo
    re-abre replicando las ops acumuladas, para que quede bajo el mismo
    transform que en V1 (donde mtlc persiste hasta IDLC)."""
    if state.lc_pending_ops is None:
        return
    ops = state.lc_pending_ops
    state.lc_pending_ops = None
    open_lc_block(state, out)
    for op in ops:
        state.out_line(out, op)
    state.lc_block_ops = ops


def handle_OPST(sc, state, out):
    name = sc.expect_string('(nombre de struct en OPST)')
    # Un TLLC abierto antes de un OPST no tiene un caso vivo en el corpus
    # objetivo; se cierra por seguridad en vez de anidar incorrectamente.
    close_lc_block_if_open(state, out)
    state.pending_hatch = None  # limite de scope: no dejar un hatch fugarse a la struct
    v3name = capitalize(name)
    state.struct_name_map[name] = v3name
    state.out_line(out, f'struct {v3name}() {{')
    state.indent += 1
    state.in_struct_depth += 1
    state.struct_local_stack.append((
        state.marked_struct, state.pending_struct_scale, state.pending_struct_rotate,
        state.rs_transform_parts, state.pen, state.advance, state.path_step,
        state.path_mtpt, state.body_ww, state.body_patch_from,
    ))
    state.marked_struct = None
    state.pending_struct_scale = None
    state.pending_struct_rotate = None
    state.rs_transform_parts = []
    state.pen = (0.0, 0.0)
    state.advance = (0.0, 0.0)
    state.path_step = (0.0, 0.0)
    state.path_mtpt = Affine()
    state.body_ww = None
    state.body_patch_from = len(out)


def handle_CLST(sc, state, out):
    state.in_struct_depth -= 1
    close_lc_block_if_open(state, out)
    state.pending_hatch = None
    state.indent -= 1
    state.out_line(out, '}')
    (state.marked_struct, state.pending_struct_scale, state.pending_struct_rotate,
     state.rs_transform_parts, state.pen, state.advance, state.path_step,
     state.path_mtpt, state.body_ww, state.body_patch_from) = state.struct_local_stack.pop()
    # El marcador de parcheo del padre no puede apuntar ANTES de esta struct
    # (patch_window_change re-escalaria lineas del cuerpo interno, que viven
    # en su propio espacio): se adelanta al presente. Un WW del padre tras
    # esta struct ya no re-expresa coordenadas del padre anteriores a ella --
    # caso inexistente en el corpus (auditado: solo fig4-1 usa WW tras
    # coordenadas, y es dentro de structs, sin defs anidadas de por medio).
    state.body_patch_from = len(out)


def handle_MKST(sc, state, out):
    name = sc.expect_string('(nombre de struct en MKST)')
    state.marked_struct = resolve_struct_name(state, name)


# ---------------------------------------------------------------------------
# Placements (LNST/ARCST/PWST/DPST) e invocacion directa (identifier placement)
# ---------------------------------------------------------------------------

def handle_LNST(sc, state, out):
    sc_val = sc.expect_num('(escala de LNST)')
    both_sides = sc_val < 0
    scale_mag = abs(sc_val)
    sh = sc.try_num()
    n = sc.try_num() if sh is not None else None
    gap = sc.try_num() if n is not None else None
    block = read_coords_or_ref(sc)
    args = [f'scale={fmt(scale_mag)}']
    if sh is not None:
        args.append(f'shift={fmt(1 - sh)}')  # ⚠ shift = 1 - arg2 (plan_mg1to2.md §5)
    if gap is not None:
        args.append(f'gap={fmt(gap)}')
    if both_sides:
        args.append('both_sides=true')
    name = state.marked_struct or 'UNKNOWN'

    # ⚠ n>1 NO es el count= de place() de V3: el ground truth (Parser.cpp
    # YLNST) emite n StructureLine COMPLETAS (linea + struct) desplazando
    # AMBOS extremos por mtpp (el avance de pluma, TLPP) entre copias --
    # flechas repetidas apiladas (fig4-1: TLPP 0 .1 + LNST ... 9). El count=
    # de PlaceStmt en cambio INTERPOLA n instancias A LO LARGO del segmento
    # (StructurePath, sin linea). Se expanden n place() sueltos con los
    # extremos ya desplazados.
    if n is not None and int(n) > 1 and block[0] == 'pts':
        ax, ay = state.advance
        (x1, y1), (x2, y2) = block[1][0], block[1][1]
        for i in range(int(n)):
            pts = f'{fmt(x1 + i * ax)} {fmt(y1 + i * ay)}  {fmt(x2 + i * ax)} {fmt(y2 + i * ay)}'
            state.out_line(out, f'place({name}, {", ".join(args)}) {{ {pts} }}')
        consume_trailing_punct(sc)
        return

    if n is not None:
        args.insert(2 if sh is not None else 1, f'count={fmt(n)}')
    state.out_line(out, f'place({name}, {", ".join(args)}) {{ {fmt_block(state, block)} }}')
    consume_trailing_punct(sc)


def handle_ARCST(sc, state, out):
    sc_val = sc.expect_num('(escala de ARCST)')
    r = sc.expect_num('(radio de ARCST)')
    da = sc.expect_num('(barrido de ARCST)')
    ai = sc.expect_num('(angulo inicial de ARCST)')
    both_sides = sc_val < 0
    scale_mag = abs(sc_val)
    sh = sc.try_num()
    n = sc.try_num() if sh is not None else None
    block = read_coords_or_ref(sc)
    args = [f'scale={fmt(scale_mag)}', f'r={fmt(r)}', f'from={fmt(ai)}', f'to={fmt(ai + da)}']
    if sh is not None:
        args.append(f'shift={fmt(1 - sh)}')
    if n is not None:
        args.append(f'count={fmt(n)}')
    if both_sides:
        args.append('both_sides=true')
    name = state.marked_struct or 'UNKNOWN'
    state.out_line(out, f'place({name}, {", ".join(args)}) {{ {fmt_block(state, block)} }}')
    consume_trailing_punct(sc)


def handle_PWST(sc, state, out):
    block = read_coords_or_ref(sc)
    name = state.marked_struct or 'UNKNOWN'
    state.out_line(out, f'fit({name}, stretch=true) {{ {fmt_block(state, block)} }}')
    consume_trailing_punct(sc)


def handle_DPST(sc, state, out):
    # DPST usa la posicion de pluma vigente (pp) como punto de colocacion --
    # no lee un bloque de puntos (ground truth: Parser.cpp:364-380).
    name = sc.expect_string('(nombre de struct en DPST)')
    v3name = resolve_struct_name(state, name)
    px, py = state.pen
    state.out_line(out, f'{v3name}(at=({fmt(px)}, {fmt(py)}))')


def handle_RPST(sc, state, out):
    n = sc.expect_num('(cuenta de RPST)')
    name = state.marked_struct or 'UNKNOWN'
    args = [f'count={fmt(int(n))}']
    factor = state.docwmin() if state.in_struct_depth == 0 else 1.0
    scale_str = combine_scale(state.pending_struct_scale, factor)
    if scale_str is not None:
        args.append(f'scale={scale_str}')
    if state.pending_struct_rotate is not None:
        args.append(f'rotate={fmt(state.pending_struct_rotate)}')
    args.append(f'at=({fmt(state.pen[0])}, {fmt(state.pen[1])})')
    if state.advance != (0.0, 0.0):
        args.append(f'advance=({fmt(state.advance[0])}, {fmt(state.advance[1])})')
    if state.rs_transform_parts:
        args.append(f'transform={" ".join(state.rs_transform_parts)}')
    state.out_line(out, f'repeat({name}, {", ".join(args)})')


# ---------------------------------------------------------------------------
# Path compuesto (OPPT/CLPT -> compound { }, §9.4)
# ---------------------------------------------------------------------------

def handle_OPPT(sc, state, out):
    # Los atributos (hatch/outline pendientes) van en el propio compound(...);
    # el fill/color YA activos como estado global se heredan solos, igual que
    # cualquier primitivo -- no hace falta repetirlos aqui. build_head() se
    # llama ANTES de fijar in_compound para que el compound() en si SI se
    # quede con el hatch pendiente (solo los primitivos de adentro lo omiten).
    head = state.build_head('compound', [])
    state.out_line(out, f'{head} {{')
    state.indent += 1
    state.in_compound = True


def handle_CLPT(sc, state, out):
    state.in_compound = False
    state.indent -= 1
    state.out_line(out, '}')


# ---------------------------------------------------------------------------
# Literales de path (&nombre a nivel de sentencia) y PWPT (§9)
# ---------------------------------------------------------------------------

def handle_path_literal_decl(name, sc, state, out):
    """`&nombre x y x y ... }` a nivel de sentencia (fuera de cualquier
    comando): declara un path con nombre para referencias `&nombre`
    posteriores (ground truth Parser.cpp:751, listmap[name] = parsePath()).
    Tipico de bibliotecas V1 como bzsinepaths.mg. Se registra en
    state.raw_paths para resolverlo en Python (ver fmt_block) y se emite
    ademas como `path nombre = { ... }` (construccion real de V3, §9) para
    que el archivo traducido siga siendo V3 valido si algo lo `include`."""
    block = read_coords_or_ref(sc)
    pts = block[1] if block[0] == 'pts' else []
    consume_trailing_punct(sc)
    state.raw_paths[name] = pts
    state.out_line(out, f'path {name} = {{ {fmt_pts(pts)} }}')


def handle_PWPT(sc, state, out):
    """PWPT nombre x1 y1 x2 y2: mapea el path YA LITERAL `nombre` (sin pasar
    por el canal TLPT/SCPT -- ground truth Parser.cpp YPVPT usa
    listmap[name] crudo, no mtpt) al rectangulo (x1,y1)-(x2,y2) y dispara
    el resultado al buffer, consumido -y limpiado- por la siguiente
    referencia &buffer (ver fmt_block/State.computed_paths). PWPT en si no
    dibuja nada.

    ⚠ Los 4 numeros (x1 y1 x2 y2) son las mismas coordenadas "de dato" que
    cualquier otro punto en este nivel (struct/documento) -- la normalizacion
    por world_window vigente se cancela exactamente entre el mapeo
    unit-square->rect (con esos MISMOS numeros crudos) y su inverso, asi que
    no hace falta rastrear wmx/wdx aqui (verificado a mano contra
    Matrix::to_rectangle + parsePoint(), ver plan_mg1to2.md)."""
    name = sc.expect_string('(nombre de path en PWPT)')
    x1 = sc.expect_num('(PWPT x1)')
    y1 = sc.expect_num('(PWPT y1)')
    x2 = sc.expect_num('(PWPT x2)')
    y2 = sc.expect_num('(PWPT y2)')
    pts = state.raw_paths.get(name)
    if pts is None:
        raise TranslateError(
            f'PWPT: el path {name!r} no se encontro (falta su INPUT o su '
            f'definicion &{name})')
    state.computed_paths['buffer'] = [
        (x1 + (x2 - x1) * rx, y1 + (y2 - y1) * ry) for rx, ry in pts
    ]


def handle_RPPT(sc, state, out):
    """RPPT nombre N: tesela (repite) el path CRUDO `nombre` N veces al buffer,
    soldando cada copia con la anterior. Ground truth Parser.cpp YRPPT:
    `for i<N: concat_paths(bufferpt, listmap[name], mtpt)` -- CADA copia se
    transforma por el MISMO mtpt vigente (la tesela nace de la soldadura por
    extremo mas cercano de concat_paths, no de mtpt) y se suelda al buffer.
    RPPT no dibuja; lo consume la siguiente `&buffer`.

    Se suelda en coords NORMALIZADAS (donde V1 opera bufferpt) y al final se
    des-normaliza a coords de dato, la convencion de computed_paths['buffer']
    (misma que PWPT)."""
    name = sc.expect_string('(nombre de path en RPPT)')
    n = int(sc.expect_num('(N de RPPT)'))
    pts = state.raw_paths.get(name)
    if pts is None:
        raise TranslateError(
            f'RPPT: el path {name!r} no se encontro (falta su INPUT o &{name})')
    buf = []
    for _ in range(n):
        copy = [state.path_mtpt.apply(x, y) for x, y in pts]  # normalizado
        buf = concat_paths_py(buf, copy)
    wdx, wdy = current_body_extent(state)
    wmx, wmy = current_body_origin(state)
    state.computed_paths['buffer'] = [(x * wdx + wmx, y * wdy + wmy) for x, y in buf]


def handle_identifier_placement(word, sc, state, out):
    """Invocacion directa V1 (`cuadro .1 .75 }`): coloca UNA instancia en un
    punto -- no es un place() (no hay locus), sino Nombre(at=(x, y)) (§10.1
    de la spec). Como cualquier invocacion, hereda la escala/rotacion FIJA
    vigente del canal ST (SCST/IDST) -- ver examples/rpstest.mg: el
    `Cuadro(scale=(0.2, 0.125), at=(0.1, 0.75))` final arrastra la SCST
    de la repeticion anterior, nunca reseteada por un IDST."""
    v3name = resolve_struct_name(state, word)
    block = read_coords_or_ref(sc)
    factor = state.docwmin() if state.in_struct_depth == 0 else 1.0
    scale_str = combine_scale(state.pending_struct_scale, factor)

    # 2+ puntos explicitos: ground truth Parser.cpp YIDENTIFIER hace SIEMPRE
    # parsePath() y un solo StructurePath -- que dibuja UNA INSTANCIA POR
    # PUNTO (structure.cpp StructurePath::draw), rotada por la tangente solo
    # si orient esta activo ($O 1). La traduccion depende de eso:
    # - $O 1 (markers-demo): place(){puntos} -- PlaceStmt con 3+ puntos usa
    #   StructurePath orient=TRUE, exactamente el caso.
    # - sin $O (default): N invocaciones sueltas Nombre(at=(x, y)) -- una
    #   por punto, sin rotar. NO usar place(): con 2 puntos cae en
    #   StructureLine (una instancia estirada a lo largo de la linea, otra
    #   semantica; asi salian mal las columnas de flechas de fig4-1) y con
    #   3+ fuerza orient=true.
    if block[0] == 'pts' and len(block[1]) >= 2:
        if state.orient and len(block[1]) >= 3:
            place_args = [f'scale={scale_str}'] if scale_str is not None else []
            head = f'place({v3name}, {", ".join(place_args)})' if place_args else f'place({v3name})'
            state.out_line(out, f'{head} {{ {fmt_pts(block[1])} }}')
        else:
            args = []
            if scale_str is not None:
                args.append(f'scale={scale_str}')
            if state.pending_struct_rotate is not None:
                args.append(f'rotate={fmt(state.pending_struct_rotate)}')
            for x, y in block[1]:
                inv = args + [f'at=({fmt(x)}, {fmt(y)})']
                state.out_line(out, f'{v3name}({", ".join(inv)})')
        consume_trailing_punct(sc)
        return

    args = []
    if scale_str is not None:
        args.append(f'scale={scale_str}')
    if state.pending_struct_rotate is not None:
        args.append(f'rotate={fmt(state.pending_struct_rotate)}')
    if block[0] == 'pts' and len(block[1]) == 1:
        x, y = block[1][0]
        args.append(f'at=({fmt(x)}, {fmt(y)})')
    elif block[0] == 'pts' and not block[1]:
        pass
    else:
        args.append(f'at=({fmt_block(state, block)})')
    state.out_line(out, f'{v3name}({", ".join(args)})')
    consume_trailing_punct(sc)


# ---------------------------------------------------------------------------
# Operadores de matriz (TL/RT/SC/SH/ID) x (LC/ST/PP/PT/RS) -- ver ground
# truth: cualquier identificador de 4 letras cuyas 2 primeras esten en
# {TL,RT,SC,SH,MT,ID,CP} y las 2 ultimas en {LC,ST,PP,PT,RS} se lexea como
# operador de matriz, SIN importar si se penso como comando o identificador
# (mgpp.l: map_opmat x map_dfmat). MT/CP no tienen caso vivo en el corpus
# objetivo -- se dejan fuera (levantan TranslateError si aparecen).
# ---------------------------------------------------------------------------

MATRIX_MM = {'TL', 'RT', 'SC', 'SH', 'ID'}
MATRIX_DF = {'LC', 'ST', 'PP', 'PT', 'RS'}


def match_matrix_op(word):
    if len(word) == 4:
        mm, df = word[:2], word[2:]
        if mm in MATRIX_MM and df in MATRIX_DF:
            return mm, df
    return None


def handle_lc_transform(mm, args, state, out):
    """LC: bloque `{ }` de transformaciones locales que envuelve las
    primitivas siguientes (§11.1); IDLC lo cierra. Cada op se registra en
    lc_block_ops (composicion, como mtlc en V1) para poder retractar/
    re-abrir el bloque alrededor de un DT (ver handle_DT)."""
    if mm == 'ID':
        close_lc_block_if_open(state, out)
        return
    reopen_lc_if_pending(state, out)
    if not state.lc_block_open:
        open_lc_block(state, out)
        state.lc_block_ops = []
    if mm == 'TL':
        op = f'translate {fmt(args[0])} {fmt(args[1])}'
    elif mm == 'RT':
        op = f'rotate {fmt(args[0])}'
    elif mm == 'SC':
        op = f'scale {fmt(args[0])} {fmt(args[1])}'
    else:  # SH
        op = f'shear {fmt(args[0])} {fmt(args[1])}'
    state.out_line(out, op)
    state.lc_block_ops.append(op)


def handle_st_transform(mm, args, state, out):
    """ST (MTST): escala/rotacion FIJA de la siguiente place()/repeat()/
    invocacion directa -- SCST es el unico caso vivo en el corpus objetivo."""
    if mm == 'ID':
        state.pending_struct_scale = None
        state.pending_struct_rotate = None
        return
    if mm == 'SC':
        sx, sy = args
        state.pending_struct_scale = fmt(sx) if sx == sy else f'({fmt(sx)}, {fmt(sy)})'
    elif mm == 'RT':
        state.pending_struct_rotate = args[0]
    else:
        raise TranslateError(f'{mm}ST: sin caso vivo en el corpus objetivo, falta mapear')


def handle_pp_transform(mm, args, state, out):
    """PP (MTPP): avance de pluma para generadores/RPST. TLPP/XYPP tienen su
    propio handler dedicado (mas comunes); esta rama cubre IDPP (reset) y
    deja el resto sin implementar hasta que aparezca un caso vivo."""
    if mm == 'ID':
        state.advance = (0.0, 0.0)
        return
    if mm == 'TL':
        # Acumula, igual que handle_TLPP (mtpp compone).
        state.advance = (state.advance[0] + args[0], state.advance[1] + args[1])
    else:
        raise TranslateError(f'{mm}PP: sin caso vivo en el corpus objetivo, falta mapear')


def handle_rs_transform(mm, args, state, out):
    """RS (MTRS): transform= ACUMULADO de repeat() -- cada RPST eleva esta
    matriz a la potencia de su indice de copia (§17 de la spec)."""
    if mm == 'ID':
        state.rs_transform_parts = []
        return
    if mm == 'RT':
        state.rs_transform_parts.append(f'rotate({fmt(args[0])})')
    elif mm == 'SC':
        sx, sy = args
        part = f'scale({fmt(sx)})' if sx == sy else f'scale({fmt(sx)}, {fmt(sy)})'
        state.rs_transform_parts.append(part)
    elif mm == 'TL':
        state.rs_transform_parts.append(f'translate({fmt(args[0])}, {fmt(args[1])})')
    elif mm == 'SH':
        state.rs_transform_parts.append(f'shear({fmt(args[0])}, {fmt(args[1])})')


def handle_pt_transform(mm, args, state, out):
    """PT (MTPT): canal de matriz de los paths con nombre. Dos vistas del mismo
    estado (ver State.path_mtpt): `path_step` (dx,dy) para el for-loop de
    GNPATH+DOT (fig6-10, solo TLPT/IDPT) y `path_mtpt` (Affine real) para
    `BZ/PL/PG &nombre` y RPPT (fig4-10). En V1 TLPT normaliza por la WW vigente
    (mt.translate(dx/wdx, dy/wdy)); SCPT es dimensional; ambos POST-multiplican."""
    if mm == 'ID':
        state.path_step = (0.0, 0.0)
        state.path_mtpt = Affine()
        return
    if mm == 'TL':
        # Acumula, igual que mtpt en V1 (compone traslaciones hasta IDPT).
        state.path_step = (state.path_step[0] + args[0], state.path_step[1] + args[1])
        wdx, wdy = current_body_extent(state)
        state.path_mtpt.translate(args[0] / wdx, args[1] / wdy)
        return
    if mm == 'SC':
        state.path_mtpt.scale(args[0], args[1])
        return
    raise TranslateError(f'{mm}PT: sin caso vivo en el corpus objetivo, falta mapear')


def handle_matrix_op(mm, df, sc, state, out):
    if mm == 'TL':
        args = (sc.expect_num(f'(dx de {mm}{df})'), sc.expect_num(f'(dy de {mm}{df})'))
    elif mm == 'RT':
        args = (sc.expect_num(f'(angulo de {mm}{df})'),)
    elif mm in ('SC', 'SH'):
        args = (sc.expect_num(f'(x de {mm}{df})'), sc.expect_num(f'(y de {mm}{df})'))
    else:  # ID
        args = ()

    if df == 'LC':
        handle_lc_transform(mm, args, state, out)
    elif df == 'ST':
        handle_st_transform(mm, args, state, out)
    elif df == 'PP':
        handle_pp_transform(mm, args, state, out)
    elif df == 'RS':
        handle_rs_transform(mm, args, state, out)
    else:  # PT
        handle_pt_transform(mm, args, state, out)


# ---------------------------------------------------------------------------
# Dispatch
# ---------------------------------------------------------------------------

WORD_HANDLERS = {
    'PL': lambda sc, st, out: handle_point_primitive(sc, st, out, 'polyline'),
    'PG': lambda sc, st, out: handle_point_primitive(sc, st, out, 'polygon'),
    'BZ': lambda sc, st, out: handle_point_primitive(sc, st, out, 'bezier'),
    'SP': lambda sc, st, out: handle_point_primitive(sc, st, out, 'spline'),
    'BR': handle_BR,
    'CR': handle_CR,
    'EL': handle_EL,
    'DOT': handle_DOT,
    'DT': handle_DT,
    'XYDT': handle_XYDT,
    'LWIDTH': handle_LWIDTH,
    'LPATRN': handle_LPATRN,
    'LCOLOR': handle_LCOLOR,
    'FCOLOR': handle_FCOLOR,
    'LGRAY': handle_LGRAY,
    'FGRAY': handle_FGRAY,
    'FILL': handle_FILL,
    'NOFILL': handle_NOFILL,
    'FPATRN': handle_FPATRN,
    'TSTYLE': handle_TSTYLE,
    'THEIGHT': handle_THEIGHT,
    'TSIZE': handle_THEIGHT,
    'TALIGN': handle_TALIGN,
    'XYPP': handle_XYPP,
    'TLPP': handle_TLPP,
    'GNNUM': handle_GNNUM,
    'TICKS': handle_TICKS,
    'GNPATH': handle_GNPATH,
    'INPUT': handle_INPUT,
    'OPST': handle_OPST,
    'CLST': handle_CLST,
    'MKST': handle_MKST,
    'LNST': handle_LNST,
    'ARCST': handle_ARCST,
    'PWST': handle_PWST,
    'DPST': handle_DPST,
    'RPST': handle_RPST,
    'OPPT': handle_OPPT,
    'CLPT': handle_CLPT,
    # Bug de V1 (auditoria §21 de la spec / ground truth de mgpp.l): LSTYLE
    # mapea internamente a AT_LWIDTH, NO a AT_LSTYLE (ese es LPATRN) -- es un
    # typo del lexer original que el traductor debe reproducir para no
    # desviarse del oraculo (confirmado en fig6-10.mg: LSTYLE aparece
    # exactamente donde FIG1's espejo usa LWIDTH).
    'LSTYLE': handle_LWIDTH,
    'PWPT': handle_PWPT,
    'RPPT': handle_RPPT,
}

# Comandos V1 reales (keyword_map de src/mgpp.l) que busca_key SI reconoce
# pero que mg1to2.py aun no traduce -- distinto de una palabra desconocida
# (que busca_key trata como YIDENTIFIER = invocacion de struct, ver el
# despacho de mas abajo). Mantiene el error honesto en vez de mal-interpretar
# el comando como un nombre de struct. Son las op de "algebra de paths"
# completa (§9) que ni el propio parser V3 tiene cableadas del todo (tile/
# reverse/normalize) mas los realmente obsoletos/no usados en el corpus.
KNOWN_UNIMPLEMENTED_COMMANDS = {
    'EXIT', 'MAXDEEP', 'MKMR', 'MR', 'INTXT', 'XYTXT',
    'GNBZPATH', 'INVPT', 'NORMPT', 'CTPT',
}


def patch_window_change(state, out, new_ww):
    """WW a MITAD de cuerpo (§6.1bis, ver State.body_ww): re-expresa las
    coordenadas YA emitidas de este cuerpo (desde body_patch_from) en
    unidades de la ventana nueva. En V1 cada coordenada se normalizaba con
    la ventana vigente en su linea; en V3 hay UNA world_window por cuerpo
    que aplica a todo el, asi que las lineas previas al WW quedarian mal
    escaladas si no se convierten (visto en fig4-1: el `PWST 0 0 1 1` de
    MARCO, bajo ventana unitaria, precede al `WW 0 2.2 0 1` -- sin
    conversion, el fit salia a 1/2.2 del tamano).

    Solo parchea los dos formatos de coordenadas que el traductor emite en
    posicion pre-WW: bloque final `{ x y ... }` y `at=(x, y)`. Otros
    acarreos (advance=/mark=, variables de for) no aparecen antes de un WW
    en el corpus (auditado 2026-07-12: fig4-1 x2, unico caso)."""
    old = state.body_ww or (0.0, 1.0, 0.0, 1.0)
    ox0, ox1, oy0, oy1 = old
    nx0, nx1, ny0, ny1 = new_ww
    odx, ody = ox1 - ox0, oy1 - oy0
    ndx, ndy = nx1 - nx0, ny1 - ny0

    def conv_pair(x, y):
        return (nx0 + (x - ox0) / odx * ndx, ny0 + (y - oy0) / ody * ndy)

    def conv_pts(text):
        nums = [float(t) for t in text.split()]
        pts = list(zip(nums[0::2], nums[1::2]))
        return '  '.join(f'{fmt(cx)} {fmt(cy)}' for cx, cy in (conv_pair(x, y) for x, y in pts))

    block_re = re.compile(r'\{ ([-\d.eE+ ]+) \}\s*$')
    at_re = re.compile(r'at=\(([-\d.eE+]+), ([-\d.eE+]+)\)')
    for i in range(state.body_patch_from, len(out)):
        line = out[i]
        m = block_re.search(line)
        if m:
            line = line[:m.start()] + '{ ' + conv_pts(m.group(1)) + ' }'
        line = at_re.sub(
            lambda m: 'at=({}, {})'.format(*map(fmt, conv_pair(float(m.group(1)), float(m.group(2))))),
            line)
        out[i] = line


def handle_WW(sc, state, out):
    x0 = sc.expect_num('(WW x0)')
    x1 = sc.expect_num('(WW x1)')
    y0 = sc.expect_num('(WW y0)')
    y1 = sc.expect_num('(WW y1)')
    patch_window_change(state, out, (x0, x1, y0, y1))
    state.out_line(out, f'world_window {fmt(x0)} {fmt(x1)} {fmt(y0)} {fmt(y1)}')
    state.body_ww = (x0, x1, y0, y1)
    state.body_patch_from = len(out)
    if state.in_struct_depth == 0:
        state.doc_ww = (x0, x1, y0, y1)


def handle_dollar(letter, sc, state, out):
    if letter == 'D':
        w = sc.expect_num('($D ancho)')
        h = sc.expect_num('($D alto)')
        state.out_line(out, f'display_size {fmt(w)} {fmt(h)}')
    elif letter == 'P':
        p = sc.expect_num('($P)')
        state.out_line(out, f'font_size {fmt(p)}')
    elif letter == 'S':
        v = sc.expect_num('($S)')
        state.out_line(out, f'% TODO(mg1to2): $S {fmt(v)} -- spline(mode=...)')
    elif letter == 'O':
        # $O n: orientacion tangente de placements (§C.1) -- sticky, como
        # orient_next en Parser.cpp. Lo consume handle_identifier_placement
        # (con $O 1, un placement multipunto va a place(), que orienta).
        v = sc.expect_num('($O)')
        state.orient = int(v) != 0
    else:
        raise TranslateError(f'directiva desconocida ${letter}')


def translate(src, source_dir=None):
    sc = Scanner(src)
    state = State()
    state.source_dir = source_dir
    out = []
    while True:
        tok = sc.next_token()
        if tok is None:
            break
        kind, val = tok
        if kind == 'DOLLAR':
            handle_dollar(val, sc, state, out)
        elif kind == 'PUNCT':
            continue  # ';' ':' '}' sueltos entre statements: cosmeticos
        elif kind == 'PATH':
            handle_path_literal_decl(val, sc, state, out)
        elif kind == 'WORD':
            mop = match_matrix_op(val)
            if val == 'WW':
                handle_WW(sc, state, out)
            elif val in WORD_HANDLERS:
                WORD_HANDLERS[val](sc, state, out)
            elif mop is not None:
                handle_matrix_op(mop[0], mop[1], sc, state, out)
            elif val in KNOWN_UNIMPLEMENTED_COMMANDS:
                raise TranslateError(
                    f'{val}: comando V1 reconocido pero su algebra de paths '
                    f'aun no esta soportada por mg1to2.py (ver plan_mg1to2.md '
                    f'§5/§8; cerca de offset {sc.i} del fuente)'
                )
            else:
                # Invocacion directa de struct (identifier placement): ground
                # truth busca_key (src/mgpp.l) -- CUALQUIER palabra que no
                # matchee keyword_map ni un operador de matriz de 4 letras es
                # YIDENTIFIER (struct), SIN importar mayus/minus (ver
                # fig6-10.mg: `OPST DETECTOR` + `DETECTOR 7 -.2 }`, y
                # fig4-1.mg: `OPST SNDVTSQ` definido en curvas3.mg via
                # INPUT, invocado en MAYUSCULAS). La antigua heuristica
                # "alguna minuscula" fallaba justo con este caso.
                handle_identifier_placement(val, sc, state, out)
        else:
            raise TranslateError(f'token inesperado a nivel de sentencia: {tok}')
    close_lc_block_if_open(state, out)
    return '\n'.join(out) + '\n'


def main(argv):
    if len(argv) < 2:
        print('uso: python3 mg1to2.py entrada.mg [salida.mg]', file=sys.stderr)
        return 2
    with open(argv[1], 'rb') as f:
        raw = f.read()
    try:
        src = raw.decode('utf-8')
    except UnicodeDecodeError:
        # Parte del corpus V1 historico esta en latin-1 (p.ej. curvas3.mg).
        src = raw.decode('latin-1')
    source_dir = os.path.dirname(os.path.abspath(argv[1]))
    try:
        result = translate(src, source_dir=source_dir)
    except TranslateError as e:
        print(f'error de traduccion: {e}', file=sys.stderr)
        return 1
    if len(argv) >= 3:
        with open(argv[2], 'w', encoding='utf-8') as f:
            f.write(result)
    else:
        sys.stdout.write(result)
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
