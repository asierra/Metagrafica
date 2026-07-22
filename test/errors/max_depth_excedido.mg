% EXPECT: profundidad de recursión excedida
% Sin la guarda esto agotaba la pila y MORÍA POR SEÑAL (exit 139). La aserción de
% `exit == 1` exacto es justo la que lo caza; un "!= 0" lo habría bendecido.
display_size 5 5
struct r(n) { polyline { 0 0  1 1 }  r(n+1) }
r(0)
