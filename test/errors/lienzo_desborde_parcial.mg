% EXPECT_NO_WARN: la figura sale EN BLANCO
% El reverso de lienzo_en_blanco.mg: salirse del lienzo EN PARTE es corriente y
% muchas veces deliberado (un trazo que se sale por el borde, un rótulo que
% asoma), así que el aviso NO debe darse aquí. La condición que lo separa del
% otro caso es que la caja de la tinta siga TOCANDO la página; si alguien la
% endurece a «algo se salió», este fixture se pone rojo.
display_size 8 8
world_window 0 4 0 4

polyline { -3 2  7 2 }        % cruza el lienzo de lado a lado y se sale por los dos
circle(1.2) { 4 4 }           % centrado en la esquina: tres cuartos fuera
text("borde") { 3.8 0.2 }
