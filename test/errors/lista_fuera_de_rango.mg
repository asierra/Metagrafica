% EXPECT: índice fuera de rango en xs
display_size 5 5
xs = [1,2]
polyline { 0 0  (xs[7]) 1 }
