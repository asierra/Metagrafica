% EXPECT: solo va al nivel superior del archivo
% EXPECT_AT: 6:12
% `exit` es un control de PARSE-TIME (§18), así que dentro de un `if` no sería
% condicional —la condición se evalúa después— y cortaría el archivo siempre.
display_size 5 5
if 1 > 0 { exit }
polyline { 0 0  1 1 }
