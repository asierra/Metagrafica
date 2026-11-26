% EXPECT: hace falta xmin < xmax y ymin < ymax
% Una ventana del documento INVERTIDA no refleja el eje: la escala uniforme sale
% negativa y GIRA la figura 180° (medido el 2026-11-25: `10 0 0 10` y `0 10 10 0` dan
% la misma salida). Compilaba en silencio, sin siquiera el aviso de figura en blanco.
% Aquí reordenar tampoco da una ventana válida, así que el mensaje no sugiere nada.
% (Dentro de una struct es distinto: ahí invertir SÍ refleja un eje, y es legítimo;
% lo cubre ventana_local_invertida.mg.)
display_size 4 4
world_window 10 0 0 10
polyline { 1 1  3 1 }
