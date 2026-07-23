% EXPECT: axis: `disparate=` no existe
% Los GENERADORES tragaban nombres desconocidos igual que las primitivas antes del
% 2026-07-22. La lista de `axis` es la más delicada de la familia: es la UNIÓN de lo
% que lee AxisStmt y lo que `plot` le INYECTA (color, line_width, dash, base…), así
% que una lista sacada solo de los accesos de AxisStmt rechazaría `xaxis(color=…)`.
display_size 8 6
world_window 0 10 0 10
axis(disparate=1) { 0 0  10 0 }
