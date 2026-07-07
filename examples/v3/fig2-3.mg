% Fig. 2.3 — La variación de la capacidad calorífica del diamante con la
% temperatura, comparada con la predicción de la teoría cuántica de Einstein.
% Traducción V3 (BORRADOR) de examples/v1/fig2-3.mg. El render debe igualar
% examples/v1/reference/fig2-3.svg.
%
% Traducción con marco de datos (world_window en datos + stretch, §13.7). Las
% decisiones de axis/grid/marcador quedaron resueltas; queda solo CALIBRAR el
% tamaño físico del marcador (dot 0.1) contra el oráculo al verificar el render.

display_size 12 8
font_size 8

% ── Marco de datos (§13.7) ──────────────────────────────────────────────────
% Realización simple: para un plot que ES toda la figura NO hace falta ventana
% anidada; basta que la world_window esté EN UNIDADES DE DATOS, con stretch
% (temperatura y capacidad no guardan proporción). El ×10 manual del V1
% desaparece. (La ventana anidada de §13.7 se reserva para paneles: fig4-10.)
% V1: WW -1 11 -1 7 con temp×10  →  aquí x/10, y igual; margen para ejes/rótulos.
world_window -0.1 1.1  -1 7   stretch=true

% ── Curva teórica (Einstein) ────────────────────────────────────────────────
% V1 LWIDTH 4 → 0.8 pt (¡no 4!, regla LWIDTH·0.2, §4.10 — el borrador de §13.7
% tenía este bug). LPATRN 2 → "dashed".
line_width 0.8
dash "dashed"
polyline { 0 0  0.076 0 }                                  % tramo recto inicial
bezier   { 0.076 0  0.26602 0.0247  0.17879 5.0734  0.99809 5.4533 }

% ── Puntos experimentales ───────────────────────────────────────────────────
% dot es marcador FÍSICO (§4.6): la posición la transforma el marco stretch, el
% tamaño NO → círculos redondos en su sitio. color= (sin fill) = abierto, como
% los CR 0.1 del V1. El tamaño es físico EN PT (§4.6); el CR 0.1 del V1 medía
% 0.1 cm de radio ≈ 2.8 pt, así que el "0.1" aquí es provisional. ⚠ calibrar el
% valor (y radio-vs-diámetro) contra el oráculo cuando exista el parser V3.
dot(0.1, color="black") {
    0.16011 0.7290  0.19421 1.1410  0.21008 1.3624  0.22753 1.5638
    0.24370 1.8089  0.26464 2.0984  0.30605 2.6922  0.35098 3.3768
    0.38756 3.6056  0.65876 5.3581  0.80844 5.3956  0.94408 5.5524
}

% ── Retícula gris (V1: TICKS que barren todo el alto/ancho = gridlines) ───────
% V1 LGRAY 50 → gray(0.5); LWIDTH 0 → 0.1 pt. grid incluye las líneas del cero,
% pero coinciden con los ejes (dibujados encima) → sin diferencia visible.
grid(xstep=0.1, ystep=1, color=gray(0.5), line_width=0.1) { 0 0  1 6 }

% ── Ejes ──────────────────────────────────────────────────────────────────
% V1 LWIDTH 1 → 0.2 pt. Sin marco anidado: from/to explícitos (no heredados).
% Etiquetas en itálica 8 (estado de texto ambiente, font_size 8 vigente); títulos
% en romana 10 (title_font/title_size). La x omite el 0 con start=0.1 (V1: 0.1..1.0).
line_width 0.2
font "italic"
axis(from=0, to=1, step=0.1, start=0.1, decimals=1,
     title="relative temperature", title_font="roman", title_size=10) { 0 0  1 0 }
axis(from=0, to=6, step=1, decimals=0,
     title="heat capacity", title_font="roman", title_size=10) { 0 0  0 6 }
