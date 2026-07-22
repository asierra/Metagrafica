% EXPECT: atributo desconocido en la primitiva
% La orientación de un marcador se pide con marker_orient=, NO con el rotate= que
% giran structs y otras primitivas (decisión 2026-07-21). Antes se ignoraba.
display_size 5 5
marker(shape="arrow", rotate=90) { 1 1 }
