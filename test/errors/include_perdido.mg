% EXPECT: include no pudo abrir
% Antes devolvía {} y seguía: si el archivo perdido solo aportaba cosas
% opcionales, la figura salía A MEDIAS con código de salida 0.
display_size 5 5
include "no_existe_a_proposito.mg"
