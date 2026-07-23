% EXPECT: include no encontró
% Antes devolvía {} y seguía: si el archivo perdido solo aportaba cosas
% opcionales, la figura salía A MEDIAS con código de salida 0. Sigue siendo FATAL;
% desde la búsqueda local→lib (§15) el mensaje además lista dónde buscó.
display_size 5 5
include "no_existe_a_proposito.mg"
