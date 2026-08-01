% EXPECT_WARN: \hat requiere un grupo
% `\hat` y `\vec` dibujan su marca sobre una BASE, así que el grupo no es opcional: sin
% él la marca no tiene sobre qué ir ni de qué ancho. El diagnóstico NO es fatal, por
% consistencia con `\frac`, que ya avisaba y seguía — son errores de markup de texto,
% no del documento.
%
% Lo que este fixture protege es que el aviso siga saliendo. Es la clase de regresión
% más barata de introducir: `\hat n` se parsearía como la palabra `hat` seguida de `n`,
% compilaría sin una queja, y en la página faltaría justo lo que se quiso decir — un
% versor que deja de serlo.

display_size 5 5
world_window 0 5 0 5
text("$\hat n$") { 1 2 }
