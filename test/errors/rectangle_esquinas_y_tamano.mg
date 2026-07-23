% EXPECT: no ambos
% rectangle(w,h,at) §4.4: las dos formas (esquinas en el bloque vs centro+tamaño)
% son excluyentes; darlas juntas es ambiguo → error.
display_size 5 5
world_window 0 5 0 5
rectangle(w=2, h=2, at=(2, 2)) { 1 1  3 3 }
