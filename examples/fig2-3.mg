% Fig. 2.3 — La variación de la capacidad calorífica del diamante con la
% temperatura, comparada con la predicción de la teoría cuántica de Einstein.
% Puerto a plot { } (plan_plot.md Fase 4, ruta LINEAL). Debe igualar por vista
% examples/v1/reference/fig2-3.svg (oráculo V1); NO es byte-idéntico al render
% anterior: ahora el mapeo datos→caja lo hace plot (stretch), no la world_window.
%
% Modelo: los datos viven EN UNIDADES DE DATOS (temperatura 0..1, capacidad
% 0..6); plot los mapea a la caja física box= (world coords) con stretch, y los
% ejes se piden con xaxis/yaxis DENTRO del bloque (heredan from/to de x=/y=).
% La retícula es grid= (§13.7): en V1 no había comando de retícula, eran TICKS
% con el vector del tick tan largo como el campo (TICKS 10 0 6 / TICKS 6 10 0)
% en LGRAY 50; grid=gray(0.5) hace lo mismo por dentro (axis(ticks="grid") con
% los step de xaxis/yaxis), mismo gris y mismos pasos.

display_size 12 8
font_size 8

% Lienzo exterior en cm (1 unidad = 1 cm; aspecto = display ⇒ isométrico, sin
% letterbox). La anisotropía temperatura/capacidad la produce ahora el fit
% interno de plot (datos→box), NO la world_window (antes: -0.1 1.1 -1 7 stretch).
world_window 0 12 0 8

% x = temperatura relativa (0..1) ; y = capacidad calorífica (0..6).
% box = rectángulo FÍSICO de datos, en cm; el margen para rótulos/títulos queda
% AFUERA de la caja.
plot(x=(0,1), y=(0,6), box=(1.3,1.1, 11.5,7.5), grid=gray(0.5)) {

    % ── Curva teórica (Einstein), EN DATOS ──────────────────────────────
    % V1 LWIDTH 4 → 0.8 pt (regla LWIDTH·0.2, §4.10). LPATRN 2 → "dashed".
    line_width 0.8
    dash "dashed"
    polyline { 0 0  0.076 0 }                                 % tramo recto inicial
    bezier   { 0.076 0  0.26602 0.0247  0.17879 5.0734  0.99809 5.4533 }
    dash "solid"                                              % la trama solo aplica a la curva

    % ── Puntos experimentales (marcador físico dot, EN DATOS) ───────────
    % dot(r): posición mapeada por el fit del plot, radio (pt) inmune al stretch
    % → círculos redondos en su sitio (V1 CR 0.1 cm ≈ 2.83 pt).
    dot(2.83, color="black") {
        0.16011 0.7290  0.19421 1.1410  0.21008 1.3624  0.22753 1.5638
        0.24370 1.8089  0.26464 2.0984  0.30605 2.6922  0.35098 3.3768
        0.38756 3.6056  0.65876 5.3581  0.80844 5.3956  0.94408 5.5524
    }

    % ── Ejes (from/to HEREDADOS de x=/y=; sin bloque de coords) ─────────
    % El estilo va SOBRE el eje (line_width/label_font), no como estado del bloque:
    % los ejes se dibujan fuera del envoltorio de contenido, así que un line_width/
    % font suelto aquí NO les llegaría. Etiquetas en itálica; títulos en romana 10.
    % La x omite el 0 con start=0.1 (V1: 0.1..1.0).
    xaxis(line_width=0.2, label_font="italic",
          step=0.1, start=0.1, decimals=1,
          title="relative temperature", title_font="roman", title_size=10)
    yaxis(line_width=0.2, label_font="italic",
          step=1, decimals=0,
          title="heat capacity", title_font="roman", title_size=10)
}
