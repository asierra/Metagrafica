% EXPECT: place: `disparate=` no existe
display_size 8 6
world_window 0 10 0 10
struct Caja() { rectangle { 0 0  1 1 } }
place(Caja, disparate=1) { 0 0  3 0  3 3 }
