% EXPECT: asin: argumento fuera de [-1, 1]
% El gemelo de dominio_acos.mg. Van los dos porque son dos guardas distintas en el
% código, y una de las dos podría perderse sin que la otra lo note.

display_size 5 5
world_window 0 5 0 5
dot(2) { (asin(-1.000001)) 2 }
