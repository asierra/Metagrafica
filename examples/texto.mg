display_size 10 6
font_size 8
world_window -5 5 -3 3

line_width 1
polyline { 0 -2.5 0 2.75 }

text("") { }

text("\hbar{/bk}_0") { 0 -2 } 
text("$x_0 2x_\gamma x^{2x}\delta$") { 0 -1.5 } 
text("Un subíndice $x_0$ y un superíndice $x^2$") { 0 -1 } 

% centrado
align "center"
line_width 0.2
polyline { -2 0 2 0 ; -2 0.5 2 0.5  ; -2 1 2 1 }

text("Una letra griega \alpha y otra no.") { 0 0 }
valign "middle"
text("Aep $2x_0 ek$.") { 0 0.5 } 
%text("Aep $2x_0 ek$.", valign="middle") { 0 0.5 } 
valign "top"
text("\beta empezando por griega.") { 0 1 } 
%text("\beta empezando por griega.", valign="top") { 0 1 } 

% alineado a la derecha
align "right"

text("una {/bnegrita} y /eénfasis el resto.") { 0 2 }
text("un número \infty de posibilidades.") { 0 2.5 }

% --- valign con texto SIMPLE (sin marcado) y align por defecto (left) ---
% La línea horizontal es la pluma (y = 2.7): cada rótulo la ancla distinto.
%   top    → el texto cuelga por debajo de la línea
%   middle → el texto queda centrado sobre la línea
%   bottom → el texto se apoya (su base) en la línea
% Son rótulos de un solo run: ejercitan la ruta de texto simple (Text::draw), no
% la de texto compuesto; y con align=left comprueban que valign no mueve la x.
align "left"
line_width 0.2
polyline { -4.5 2.7 4.5 2.7 }
valign "top"
text("arriba") { -4.5 2.7 }
valign "middle"
text("medio") { -1.5 2.7 }
valign "bottom"
text("abajo") { 1.7 2.7 }


