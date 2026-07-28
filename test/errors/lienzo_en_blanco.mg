% EXPECT_WARN: la figura sale EN BLANCO
% El caso REAL que motivó el aviso (2026-07-28): un `plot` cuyo `box=` va en
% unidades de MUNDO sobre la ventana default (el cuadrado unitario), porque no se
% declaró `world_window`. Los números 2..15 se leen como centímetros y no lo son,
% así que la caja entera cae fuera del lienzo: compila limpio, sale un SVG válido
% y vacío, y antes no había nada —ni un error, ni un aviso, ni una compuerta— que
% lo distinguiera de un bug del motor.
display_size 16 10

plot(x=(0,1), y=(0,6), box=(2, 1.5, 15, 9.5)) {
    xaxis(step=0.1, tick_labels=true)
    yaxis(step=1, tick_labels=true)
    polyline { 0.1 0  0.5 4.4  1 5.45 }
}
