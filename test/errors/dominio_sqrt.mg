% EXPECT: sqrt: argumento negativo
% `sqrt` era el ÚLTIMO hueco por donde un NaN entraba al documento sin ruido: `ln` ya
% abortaba con argumento no positivo, `sqrt` no. Y no es un caso rebuscado — sale solo
% de `sqrt(1 - x*x)` con |x| > 1, que es como se escribían asin y acos antes de que
% existieran.

display_size 5 5
world_window 0 5 0 5
x = 1.2
dot(2) { (sqrt(1 - x*x)) 2 }
