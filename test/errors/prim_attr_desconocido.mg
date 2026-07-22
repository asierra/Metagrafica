% EXPECT: atributo desconocido en la primitiva
% Un typo en un atributo era un no-op MUDO: el atributo PARECE puesto y no hace
% nada. `colour` por `color` es el caso clásico.
display_size 5 5
polyline(colour="red") { 0 0  1 1 }
