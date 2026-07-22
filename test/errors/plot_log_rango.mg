% EXPECT: un eje log requiere rango
display_size 5 5
plot(x=[0,10], y=[0,10], yscale="log", box=[0,5,0,5]) { }
