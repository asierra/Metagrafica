% EXPECT: 'circle' es un comando de MetaGráfica
% EXPECT_AT: 13:8
% Una struct con el NOMBRE DE UN COMANDO no se puede invocar nunca: parseStatement
% despacha `circle` como primitiva antes de buscar structs, así que el comando gana
% siempre. Hasta el 2026-11-25 la definición se aceptaba en silencio y la invocación
% de abajo dibujaba un CÍRCULO de radio 4 en vez del cuadrado —exit 0, sin una
% palabra—, de la familia de la aridad de structs: una figura plausible y distinta de
% la pedida. Se señala la columna del NOMBRE en la definición, que es lo que hay que
% cambiar.

display_size 5 5
world_window -5 5  -5 5
struct circle(r) {
    rectangle { -r -r  r r }
}

circle(r=4) { 0 0 }
