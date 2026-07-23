% EXPECT: legend: `disparate=` no existe
display_size 8 6
world_window 0 10 0 10
plot(x=(0,10), y=(0,10)) {
    polyline { 0 0  9 9 }
    legend(at="top-left", disparate=1)
}
