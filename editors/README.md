# Resaltado de sintaxis MG para editores

Configuraciones de resaltado para archivos `.mg` (gramática **V3**). El
vocabulario está extraído del parser real (`src/parserv3.cpp`, `src/lexer.l`),
no de la especificación: **solo aparecen palabras que hoy compilan**. Lo que
`especificacion_mg.md` reserva pero no está construido (`spline`, `trail`,
`cap`, `join`, `rule`, `table`) queda **fuera** a propósito — coloreado, un
nombre reservado se lee como implementado.

## Geany — `geany/filetypes.MG.conf`

```bash
cp geany/filetypes.MG.conf ~/.config/geany/filedefs/
```

y añadir en `~/.config/geany/filetype_extensions.conf`, bajo `[Extensions]`:

```ini
MG=*.mg;
```

Reiniciar Geany. Queda como **MG** en *Documento → Definir tipo de fichero*.

Geany no permite definir un lexer propio: un tipo de fichero personalizado
tiene que reusar uno existente vía `lexer_filetype`, y el carácter de
comentario **lo lleva el lexer cableado**, no el `.conf` (el `comment_single=%`
de `[settings]` solo controla el Ctrl+D). MG comenta con `%` (§1), lo que deja
pocos candidatos; entre ellos, **Erlang** es el único con dos listas de
palabras clave, así que es el que se usa:

| Lista | Estilo | Contenido |
|---|---|---|
| `keywords` | `keyword_1` | control de flujo, declaraciones, configuración, estado (§5–§7, §11.1) |
| `bifs` | `keyword_2` | primitivas, generadores, placements, álgebra de paths, funciones (§4, §9, §10, §13) |

Efecto lateral aprovechado: el lexer de Erlang marca los identificadores que
empiezan con mayúscula como *variable*, y ahí se mapea a `type`. Como el
corpus Capitaliza las structs (`Nivel`, `PanelCurvas`), quedan resaltadas
gratis. Las de `lib/pseudo3d.mg` (`plano`, `prisma`) no, por ir en minúscula.

Los argumentos nombrados (`color=`, `stretch=`) **no** se distinguen: el lexer
de Erlang no tiene esa regla y Geany no permite añadirla.

## Kate / KWrite / KDevelop — `kate/mg.xml`

```bash
mkdir -p ~/.local/share/org.kde.syntax-highlighting/syntax
cp kate/mg.xml ~/.local/share/org.kde.syntax-highlighting/syntax/
```

Reiniciar Kate. Queda como **MetaGrafica** en *Herramientas → Modo → Other*.

KSyntaxHighlighting es un motor de reglas regex propio, no un lexer prestado,
así que este resaltador llega más lejos que el de Geany:

- Seis categorías con color distinto (control / estado / primitivas /
  generadores / funciones / constantes) en vez de dos.
- **Argumentos nombrados** (`stretch=true`) resaltados como atributo, con el
  `(?=\s*=[^=])` que los separa de la comparación `==`.
- **Referencias a path** `&nombre` (§9) como variable.
- Dentro de las cadenas, el **markup de texto** (§14): modo matemático `$…$`,
  comandos TeX `\alpha`, sub/superíndices `_`/`^` y cambios de fuente `/b`,
  `/i`.
- Las cadenas **no cierran en fin de línea**, igual que `STRING`
  (`\"[^\"]*\"`) en `src/lexer.l`: un `"` sin pareja tiñe hacia abajo, que es
  exactamente el error que se quiere ver.

## Mantenimiento

Al añadir una primitiva, generador o función al parser, hay que tocar **las
dos** listas. El vocabulario sale de:

```bash
grep -oE 'name == "[a-z_0-9]+"' src/parserv3.cpp | sort -u   # sentencias
grep -n 'isPrim' -A 4 src/parserv3.cpp                        # primitivas
grep -n 'T_[A-Z]*;' src/lexer.l                               # keywords reales
```
