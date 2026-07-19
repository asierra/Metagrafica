% Fig 16.9 — Principio de Franck-Condon: dos potenciales de Morse (estados
% electrónicos fundamental y excitado) con sus funciones de onda vibracionales
% rellenas de gris, niveles no resueltos cerca de la disociación, transición
% vertical y marco de página. Port V3 de examples/fig16-9v1.mg (V1).
%
% Las funciones de onda NO son senoides: cada nivel es una cadena de MEDIOS
% CICLOS DE COSENO (extremos en las uniones, pendiente cero en ambos cabos)
% con envolvente de amplitud por tramo — sine(phase=90/270) como EXPRESIÓN DE
% PATH (§9) da exactamente esa pieza, y concat las suelda C1 gratis (las
% uniones caen en extremos). amplitude = (recorrido pico a pico)/2.
%
% Ventana = cm de página (aspecto 8x10 = display => meet exacto, §3.1). Las
% colocaciones vienen del V1 (dos ventanas + SCST compuestos + docwmin 1.3)
% convertidas a cm y verificadas contra el render V1 a 150 dpi (+-1 px).

display_size 8 10
font_size 8

world_window 0 8 0 10

% ---- piezas: media unidad plana + medios ciclos de coseno ----------------
path plana = { 0 0  .1667 0  .3333 0  .5 0 }

% subeN/bajaN: medio ciclo que sube/baja N decimas de unidad (pico a pico)
path sube10 = sine(half_cycles=1, phase=270, amplitude=.5)  { 0 0  1 0 }
path baja10 = sine(half_cycles=1, phase=90,  amplitude=.5)  { 0 0  1 0 }
path sube20 = sine(half_cycles=1, phase=270, amplitude=1)   { 0 0  1 0 }
path baja20 = sine(half_cycles=1, phase=90,  amplitude=1)   { 0 0  1 0 }
path sube13 = sine(half_cycles=1, phase=270, amplitude=.65) { 0 0  1 0 }
path baja13 = sine(half_cycles=1, phase=90,  amplitude=.65) { 0 0  1 0 }
path sube26 = sine(half_cycles=1, phase=270, amplitude=1.3) { 0 0  1 0 }
path baja26 = sine(half_cycles=1, phase=90,  amplitude=1.3) { 0 0  1 0 }

% ---- funciones de onda: nivel n = n+1 lobulos de signo alterno -----------
% (pw6, la del oso: envolvente anarmonica 2, -2.6, +1.3... del original; el
% cierre exacto es baja20 — el V1 traia 2.028 y quedaba 0.03 fuera de la base)
path pw0 = concat(&plana, &sube10, &baja10, &plana)
path pw1 = concat(&plana, &baja10, &sube20, &baja10, &plana)
path pw2 = concat(&plana, &sube10, &baja20, &sube20, &baja10, &plana)
path pw3 = concat(&plana, &baja10, &sube20, &baja20, &sube20, &baja10, &plana)
path pw4 = concat(&plana, &sube10, &baja20, &sube20, &baja20, &sube20, &baja10, &plana)
path pw5 = concat(&plana, &baja10, &sube20, &baja20, &sube20, &baja20, &sube20, &baja10, &plana)
path pw6 = concat(&plana, &sube20, &baja26, &sube13, &baja13, &sube13, &baja13, &sube26, &baja20, &plana)

% ---- un nivel: base gris tenue + onda rellena de gris con contorno negro --
% Ventana local: x = ancho de la onda (medio + n+2 medios ciclos + medio) —
% es la extension en x del propio path, asi que path_width lo deriva solo
% (plan_struct_params.md, decision 6) —, y simetrica -2..2 para que la base
% quede centrada en el rect del fit.
struct Nivel(&onda, w = path_width(&onda)) {
    world_window 0 w -2 2
    line_width 0.1   color "lightgray"   polyline { 0 0  w 0 }
    fill "lightgray"   outlinefill   color "black"   line_width 0.4
    bezier(&onda)
}

% ---- potencial de Morse (curva digitalizada del V1, control points bezier) --
% Los 32 puntos VERBATIM del V1, incluido el primero duplicado: con el
% agrupamiento 1+3k del motor eso hace c1=p0 en el primer segmento (tangente
% inicial nula) y deja los puntos on-curve donde el original los tenia; el
% punto 32 sobra un grupo y se descarta, igual que en V1.
struct Morse() {
    world_window 0 1 -0.01 0.693976
    line_width 0.6
    bezier { 0.000000 0.693976
             0.000000 0.693976  0.021710 0.357390  0.045592 0.171948
             0.072380 0.079206  0.090888 0.015079  0.109590 0.000000
             0.125407 -0.010000  0.139262 0.000000  0.156276 0.012637
             0.177377 0.041759  0.194267 0.065032  0.224236 0.117444
             0.246641 0.157921  0.291242 0.238452  0.334014 0.309566
             0.368512 0.356172  0.414584 0.418417  0.462263 0.465319
             0.510216 0.499456  0.564208 0.537888  0.615930 0.558881
             0.653521 0.571496  0.701071 0.587438  0.757337 0.598576
             0.790088 0.603735  0.832564 0.610458  0.874534 0.614668
             0.909177 0.617466  0.939518 0.619908  0.970182 0.621763
             1.000000 0.623218 }
}

% ---- estado fundamental (pozo inferior): curva + niveles 0..6 --------------
% El V1 dibujaba la curva ~1:1 en unidades de su ventana; este rect es la
% caja de datos ajustada por minimos cuadrados contra el render V1 (rms 1.3 px).
fit(Morse, stretch=true) { 1.96667 0.66008  8.08333 5.34715 }
% pared repulsiva: la figura publicada extiende la rama izquierda mas alla del
% ultimo punto digitalizado; se continua sobre su tangente.
line_width 0.6
polyline { 1.96667 5.34715  1.85106 7.29845 }

fit(Nivel(&pw0), stretch=true) { 1.84615 0.44169  3.46615 1.48169 }
fit(Nivel(&pw1), stretch=true) { 1.66154 1.05856  3.82154 2.09856 }
fit(Nivel(&pw2), stretch=true) { 1.53846 1.59497  4.23846 2.63497 }
fit(Nivel(&pw3), stretch=true) { 1.41538 2.15655  4.33138 3.09255 }
fit(Nivel(&pw4), stretch=true) { 1.35385 2.70609  4.61977 3.45489 }
fit(Nivel(&pw5), stretch=true) { 1.29231 3.14584  4.83816 3.81976 }
fit(Nivel(&pw6), stretch=true) { 1.26154 3.59559  5.05117 4.06734 }

% niveles no resueltos, cerca de la disociacion
line_width 0.1
polyline { 1.26154 4.09967  5.53846 4.09967 }
polyline { 1.26154 4.36787  6.15385 4.36787 }
polyline { 1.26154 4.56044  6.46154 4.56044 }
polyline { 1.26154 4.70849  6.70823 4.70849 }

text("6") { 1.10769 3.83147 }
text("5") { 1.16923 3.48280 }
text("4") { 1.23077 3.08049 }
text("3") { 1.29231 2.62455 }
text("2") { 1.41538 2.11497 }
text("1") { 1.53846 1.57856 }
text("0") { 1.72308 0.96169 }

% ---- estado excitado (pozo superior): curva + niveles 0..5 -----------------
% PWST .02 0 1 .9 del V1: corrimiento x +.02 y escala y 0.9, en su ventana.
fit(Morse, stretch=true) { 2.03077 5.27333  8.18462 9.49719 }

fit(Nivel(&pw0), stretch=true) { 1.90769 5.10836  3.70769 6.14836 }
fit(Nivel(&pw1), stretch=true) { 1.72308 5.72523  4.12308 6.76523 }
fit(Nivel(&pw2), stretch=true) { 1.60000 6.26163  4.60000 7.30163 }
fit(Nivel(&pw3), stretch=true) { 1.47692 6.82322  5.07692 7.75922 }
fit(Nivel(&pw4), stretch=true) { 1.41538 7.37276  5.19538 8.12156 }
fit(Nivel(&pw5), stretch=true) { 1.35385 7.81251  5.67385 8.48643 }

line_width 0.1
polyline { 1.32308 8.40000  5.90769 8.40000 }
polyline { 1.32308 8.66667  6.21538 8.66667 }
polyline { 1.32308 8.86667  6.70769 8.86667 }
polyline { 1.32308 9.00000  7.32308 9.00000 }

text("5") { 1.23077 8.14947 }
text("4") { 1.29231 7.74716 }
text("3") { 1.35385 7.29122 }
text("2") { 1.47692 6.78163 }
text("1") { 1.60000 6.24523 }
text("0") { 1.78462 5.62836 }

% ---- transicion vertical (desde el nivel 0 del fundamental) ----------------
line_width 0.5
polyline(marker_end="arrow", marker_size=6) { 2.61789 0.96169  2.61789 6.80000 }

% ---- marco de pagina -------------------------------------------------------
line_width 0.1
polyline(marker_end="arrow", marker_size=5) { 0.5 0.5  0.5 9.5 }
polyline(marker_end="arrow", marker_size=5) { 0.5 0.5  7.5 0.5 }

text("internuclear distance") { 2.25 0.18 }
{ translate 0.3 5  rotate 90  text("energy") { 0 0 } }
