% EXPECT: structs dentro de un plot logarítmico
% El mapeo log no es afín, así que matriz(map(p)) != map(matriz(p)). Se detecta en
% la SENTENCIA, no en el item.
display_size 5 5
struct S() { polyline { 0 0  1 1 } }
plot(x=[1,10], y=[1,10], yscale="log", box=[0,5,0,5]) { S() }
