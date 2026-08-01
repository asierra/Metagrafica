% EXPECT_NO_WARN: Error de libharu
% Un arco de BARRIDO CERO (`to == from`) es un resultado LEGÍTIMO, no un error del
% autor: es la salida natural de recortar un arco por visibilidad —`orbita_polar` y
% las figuras pseudo-3D calculan `from`/`to` con trigonometría, y «no se ve nada» es
% una respuesta válida de esa cuenta—. Debe compilar en los TRES backends, y lo que
% dibuja es nada.
%
% Hasta el 2026-08-01 el PDF ABORTABA (`Error de libharu 0x1051`,
% HPDF_PAGE_INVALID_GMODE, exit 1, sin archivo): `arc_bezier` salía antes de emitir
% el MoveTo, dejaba el path vacío, y el Stroke de quien llama reventaba. EPS y SVG lo
% toleraban, así que ninguna otra compuerta podía verlo: sin archivo PDF no hay
% golden que comparar ni tres salidas que confrontar en la Capa 3.
%
% La conducta que este fixture FIJA en los tres es la de PostScript: el arco
% degenerado no traza nada pero SÍ deja la pluma en su punto de inicio. Por eso van
% los tres caminos que pasan por modos distintos del constructor de paths: suelto,
% como PRIMER trazo de un compound (abre con MoveTo) y en MEDIO de uno (se une con
% LineTo).

display_size 6 6
world_window 0 6 0 6

% (1) suelto
arc(1, from=30, to=30) { 2 4 }

% (2) primer trazo de un compound
compound {
  arc(1, from=30, to=30) { 4 4 }
  polyline { 3.2 3.2  4.8 3.2 }
}

% (3) en medio de un compound
compound {
  polyline { 1 1  3 1 }
  arc(0.5, from=90, to=90) { 3 1.5 }
  polyline { 3 2  1 2 }
}

% Un arco de verdad, para que la figura no salga en blanco y el caso no dependa de
% la heurística del lienzo vacío.
arc(1.2, from=200, to=340) { 3 3 }
