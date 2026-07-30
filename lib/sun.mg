% =====================================================================
%  sun.mg — El Sol como icono: disco y rayos rectos repartidos por igual,
%  el pictograma de uso corriente.
%
%  El disco tiene radio 1, así que `scale=` en la invocación ES el radio en
%  unidades de mundo y los rayos crecen con él. Los rayos van en un anillo
%  aparte (de `ray_in` a `ray_out`, en radios), separados del disco: es lo
%  que hace legible el icono a tamaño chico.
%
%  Ejemplo:  Sun(scale=0.5, at=(2.6, 10.4))
%            Sun(rays=8, disc="white", ray="black", scale=1, at=(0, 0))
% =====================================================================

struct Sun(rays=12, disc="gold", ray="darkorange",
           ray_in=1.35, ray_out=1.9, lw=0.6) {
    circle(1, fill=disc, color=ray) { 0 0 }
    for i = 0 to rays-1 {
        a = i * 2*pi / rays
        polyline(color=ray, line_width=lw) {
            (ray_in*cos(a))   (ray_in*sin(a))
            (ray_out*cos(a))  (ray_out*sin(a)) }
    }
}
