% EXPECT: necesita un tamaño
% rectangle(w,h,at) §4.4: dar `at` sin w/h no tiene tamaño → error, no un no-op mudo.
display_size 5 5
world_window 0 5 0 5
rectangle(at=(2, 2))
