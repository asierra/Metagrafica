% EXPECT_WARN: no queda nada que dibujar con ella
% El `m/s` (E2b, 2026-08-06). `/s` es el código de cara SANSERIF, así que `5 m/s`
% salía como `5 m`: la barra se quiso literal, el compilador la leyó como sintaxis y
% se llevó la `s` por delante. En silencio, con salida byte-estable y plausible —la
% clase de fallo más cara que tiene este proyecto—. Y no era un caso raro: `m/s`,
% `J/g`, `cal/g`, `1/r`, `1/e`, `W/cm`, toda unidad con barra.
%
% El aviso se dispara SOLO cuando el cambio de cara queda al final de la cadena, que
% es donde no puede haber falsos positivos: cambiar de cara y no escribir nada
% después siempre es inútil. La salida que propone —`\/`— es el escape E1.
%
% ⚠️ Lo que este aviso NO alcanza, dicho aquí para que no se confunda con una
% regresión: el `m/s` en MEDIO de una cadena (`v (m/s)`), donde sí queda texto
% después y es indistinguible de un cambio de cara intencional. Para ése la defensa
% es documental —la referencia lista las letras reservadas tras `/` y enseña `\/`—.
% Su fixture es `cara_legitima.mg`, que fija el otro lado: lo que NO debe avisar.

display_size 6 4
world_window 0 6 0 4
text("5 m/s") { 0.5 2 }
