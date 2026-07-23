% Patrones y anchos de línea — lámina de referencia.
%
% Los seis estilos de `dash` a la izquierda y una escala de `line_width` en
% puntos tipográficos a la derecha, generados con un `for` sobre una lista.
% Hermana de primitives.mg y fill_styles.mg.

display_size 14 7
font_size 8
world_window -1 13 -0.5 6.5

% marco
rectangle { -0.95 -0.45  12.95 6.45 }

% Patrones de línea: los alias de dash.
% dash por primitiva: cada línea usa uno distinto; line_width compartido como estado.

text("Line Patterns with dash") { 1 6.0 }
{
    align "center"
    text("Line Widths with /tline_width") { 8.5 6.0 }
    font_size 6
    text("(units of typographic points)") { 8.5 5.7 }
}
line_width 0.2
pattern = [ "solid", "longdashed", "dashed", "dotted", "dashdot", "dashdotdot" ]
for i = 0 to 5 {
  polyline(dash=pattern[i])      { 0 i  5 i }
  text("Pattern " + pattern[i]) { 0 (i+0.1) }

  % Anchos de línea 
  polyline(line_width=(0.2*i)) { 6 i  11 i }
  text("Width " + str(0.2*i, 1)) { 6 (i+0.1) }
}

