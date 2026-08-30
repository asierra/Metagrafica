% EXPECT_WARN: necesita llaves
% `\frac`, `\hat` y `\vec` piden grupos, y en un sub/superíndice SIN LLAVES no hay
% dónde ponerlos: el índice se descarga por textflush con el estado del texto,
% mientras que Fraction y Accent son ítems 2-D que van a `text_line` y no saben del
% índice. La forma con llaves (`$x^{\frac{a}{b}}$`) sí puede, porque a ésa la atiende
% el escáner principal — y eso es lo que el aviso dice.
%
% Hasta el 2026-08-30 esta rama contestaba «symbol name unknown frac», que es falso
% —`\frac` existe y funciona a dos caracteres de distancia— y además DERRAMABA los
% grupos: `$x^\frac{p}{q}$` dibujaba «pq» como texto suelto. Un mensaje que miente
% sobre la causa cuesta más que no decir nada: manda a buscar el símbolo que falta en
% vez de a poner las llaves.
%
% ⚠️ Lo que esta compuerta ve es el MENSAJE. Que los grupos se consuman —o sea que
% no quede basura dibujada— lo fija el golden de `examples/texto.mg`, porque aquí
% solo se mira stderr y el código de salida.

display_size 6 4
world_window 0 6 0 4

text("$x^\frac{p}{q}$") { 0.5 2 }
