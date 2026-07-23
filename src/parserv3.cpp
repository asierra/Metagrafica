// Parser V3 (descenso recursivo). Compila la gramática V3 (§1-§18) a un árbol
// MetaGrafica. Post-cutover (§22.6): es el ÚNICO front-end de bin/mg (Parser.cpp
// V1 sigue en el árbol solo como referencia/insumo de mg1to2.py, fuera del
// build). El entry point (main()) vive en src/main.cpp; este archivo expone
// buildFromSource()/g_baseDir/g_flags vía include/parserv3.h.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

#include "ast.h"
#include "tokens.h"

// Motor reutilizado (árbol dibujable + backends)
#include "structures.h"    // MetaGrafica, GraphicsItemList, point, Path, primitivas
#include "splines.h"       // concat_paths/process_path: álgebra de paths (§9)
#include "markers.h"        // markerIdForName: marcadores físicos de dot (§4.11)
#include "text_parser.h"   // parse_text: markup de texto (super/subíndices, TeX, math)
#include "mgflags.h"       // MGFlags: banderas de uso para el backend (reencode, cmmi…)
#include "SVGDisplay.h"
#include "EPSDisplay.h"
#include "PDFDisplay.h"
#include "parserv3.h"       // buildFromSource()/g_baseDir/g_flags: interfaz pública para main.cpp

// Definición de las globales declaradas `extern` en parserv3.h. Banderas de uso
// recogidas durante la evaluación (parse_text las activa); se vuelcan al Display
// antes de dibujar, igual que main.cpp V1 (g.flags = parser.flags).
MGFlags g_flags;

// --- Interfaz al scanner generado por Flex (src/lexer.l) ---------------------
extern int   yylex();
extern char *yytext;
struct yy_buffer_state;
typedef yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *);
extern void            yy_delete_buffer(YY_BUFFER_STATE);

// --- Buffer de tokens con lookahead -----------------------------------------
struct Tok {
  int type = T_EOF;
  double num = 0.0;
  std::string str;
  int line = 0, col = 0;
};

struct Lexer {
  std::vector<Tok> toks;
  size_t pos = 0;

  void tokenize(const char *src) {
    YY_BUFFER_STATE buf = yy_scan_string(src);
    int t;
    while ((t = yylex()) != 0) {
      Tok tk;
      tk.type = t;
      tk.line = yytokline;
      tk.col = yytokcol;
      if (t == T_NUMBER) {
        tk.num = yylval.num;
      } else if (t == T_IDENTIFIER || t == T_STRING) {
        tk.str = yylval.str;
        std::free(yylval.str);   // el lexer hizo strdup (malloc)
      }
      toks.push_back(tk);
    }
    Tok eof;
    toks.push_back(eof);
    yy_delete_buffer(buf);
  }

  const Tok &peek(int k = 0) const {
    size_t i = pos + (size_t)k;
    return i < toks.size() ? toks[i] : toks.back();
  }
  const Tok &next() {
    const Tok &t = toks[pos];
    if (pos + 1 < toks.size()) pos++;
    return t;
  }
  bool accept(int type) {
    if (peek().type == type) { next(); return true; }
    return false;
  }
};

// Describe el token que el parser tiene DELANTE, para que el error diga no solo
// qué se esperaba sino qué se encontró. El lexer ya nombraba al culpable
// («carácter inesperado '@'»); el parser se había quedado atrás, y eso duele
// justo en el caso más común: una primitiva mal escrita —`polylnie { … }`—
// señalaba al '{' siguiente sin mencionar nunca la palabra, que es lo único que
// el autor necesita ver para arreglarla.
static std::string describeTok(const Tok &t) {
  switch (t.type) {
    case T_EOF:        return "el fin del archivo";
    case T_NEWLINE:    return "un fin de línea";
    case T_IDENTIFIER: return "'" + t.str + "'";
    case T_STRING:     return "la cadena \"" + t.str + "\"";
    case T_NUMBER:   { char b[64]; std::snprintf(b, sizeof b, "el número %g", t.num); return b; }
    case T_LBRACE:     return "'{'";
    case T_RBRACE:     return "'}'";
    case T_LPAREN:     return "'('";
    case T_RPAREN:     return "')'";
    case T_LBRACKET:   return "'['";
    case T_RBRACKET:   return "']'";
    case T_COMMA:      return "','";
    case T_SEMICOLON:  return "';'";
    case T_AMP:        return "'&'";
    case T_ASSIGN:     return "'='";
    case T_PLUS:       return "'+'";
    case T_MINUS:      return "'-'";
    case T_STAR:       return "'*'";
    case T_SLASH:      return "'/'";
    case T_CARET:      return "'^'";
    case T_EQ:         return "'=='";
    case T_NE:         return "'!='";
    case T_LT:         return "'<'";
    case T_GT:         return "'>'";
    case T_LE:         return "'<='";
    case T_GE:         return "'>='";
    case T_PLUSASSIGN: return "'+='";
    case T_STRUCT:     return "'struct'";
    case T_FOR:        return "'for'";
    case T_TO:         return "'to'";
    case T_STEP:       return "'step'";
    case T_IF:         return "'if'";
    case T_ELSE:       return "'else'";
    case T_AND:        return "'and'";
    case T_OR:         return "'or'";
    case T_NOT:        return "'not'";
    case T_INCLUDE:    return "'include'";
    default:           return "otra cosa";
  }
}

static void parseError(const Lexer &lx, const char *what) {
  const Tok &t = lx.peek();
  std::fprintf(stderr, "Error de sintaxis en %d:%d: se esperaba %s, pero se encontró %s\n",
               t.line, t.col, what, describeTok(t).c_str());
  std::exit(1);
}

// Paridad de un bloque de coordenadas (§4): van en pares `x y`, y cada subtrayecto
// —los que abre ';'— se valida por separado. Sin esto una coord suelta se descarta
// EN SILENCIO y ni se evalúa (los lazos por pares hacen `i + 1 < coords.size()`),
// que es justo lo que esconde el footgun del identificador desnudo ante '(':
// `polyline { 1/u (u*u-u) }` colapsa a UNA coord —`1/u(u*u-u)`, una llamada— y la
// polilínea desaparece sin decir nada. Se valida en parse-time: es propiedad
// estática del bloque, así el mensaje sale con línea y antes de evaluar nada.
// `breaks` guarda los índices donde ';' corta; llamar con el lexer en '}'.
static void checkCoordPairs(const Lexer &lx, const std::string &what,
                            const std::vector<ExprPtr> &coords,
                            const std::vector<size_t> &breaks,
                            bool allowsPoints = false) {
  // allowsPoints: un término puede evaluar a un PUNTO [x,y] (una lista de 2, p.ej.
  // point_at(&p,t)), que vale por DOS coordenadas siendo UN término. Cuando eso es
  // posible, la paridad por término deja de ser válida en parse-time (un punto en
  // variable no es distinguible aquí), así que se difiere al eval-time de
  // PrimStmt::coordsToPath, que sí conoce el tipo. Los bloques que NO aceptan puntos
  // (smooth/place/literal de path) conservan el chequeo estricto y su línea:columna.
  if (allowsPoints) return;
  size_t start = 0;
  for (size_t i = 0; i <= breaks.size(); i++) {
    size_t end = (i < breaks.size()) ? breaks[i] : coords.size();
    if ((end - start) % 2 != 0) {
      const Tok &t = lx.peek();
      std::fprintf(stderr, "Error de sintaxis en %d:%d: %s recibió un número impar de "
                   "coordenadas (%zu); van en pares x y\n",
                   t.line, t.col, what.c_str(), end - start);
      std::exit(1);
    }
    start = end;
  }
}

// `bezier { … }`: los puntos son de CONTROL y van en grupos de 3k+1 (ancla, dos
// tiradores, ancla, dos tiradores, …), porque Polyline::draw los agrupa de 3 en 3
// para emitir cada `curveto`. Un conteo que no cierre se DESCARTABA EN SILENCIO
// (5 o 6 puntos dibujaban lo mismo que 4), que es el mismo hueco que motivó
// checkCoordPairs: lo descartado esconde el error de verdad, y la salida sale
// plausible. Se valida por subtrayecto, en parse-time, antes de evaluar nada.
// NO aplica a `smooth` (sus puntos son nodos: cualquier cantidad ≥2 vale) ni a
// la forma `bezier(&path)` (el álgebra §9 preserva la aritmética 3k+1).
static void checkBezierControlCount(const Lexer &lx, const std::vector<ExprPtr> &coords,
                                    const std::vector<size_t> &breaks) {
  size_t start = 0;
  for (size_t i = 0; i <= breaks.size(); i++) {
    size_t end = (i < breaks.size()) ? breaks[i] : coords.size();
    size_t n = (end - start) / 2;                 // nº de puntos del subtrayecto
    if (n != 0 && (n < 4 || (n - 1) % 3 != 0)) {
      const Tok &t = lx.peek();
      std::fprintf(stderr, "Error de sintaxis en %d:%d: bezier recibió %zu puntos de "
                   "control; van en grupos de 3k+1 (4, 7, 10, …): un ancla inicial más "
                   "dos tiradores de tangente y un ancla por cada curva\n",
                   t.line, t.col, n);
      std::exit(1);
    }
    start = end;
  }
}

// --- Gramática de expresiones (§2) ------------------------------------------
static ExprPtr parseExpression(Lexer &);
static ExprPtr parseUnary(Lexer &);   // adelantada: parsePower la necesita
// path_width(&p) (§8.x/§9): puente Expr↔PathExpr. parseAtom la necesita (más
// abajo) pero PathExpr/parsePathExpr viven mucho más adelante en el archivo
// (álgebra de paths, §9); declarada aquí, definida junto a parsePathExpr.
static ExprPtr parsePathWidthCall(Lexer &);
// path_x_min_at_y/path_x_max_at_y(&p, y [, expand]): misma clase de puente.
static ExprPtr parsePathXBoundCall(Lexer &, bool wantMax);
// point_at(&p, t [, curve=b]) / angle_at(&p, t [, curve=b]): familia de muestreo §9.
static ExprPtr parsePathSampleCall(Lexer &, bool wantAngle);

static ExprPtr parseAtom(Lexer &lx) {
  const Tok &t = lx.peek();
  switch (t.type) {
    case T_NUMBER: { double v = lx.next().num; return ExprPtr(new NumExpr(v)); }
    case T_STRING: { std::string s = lx.next().str; return ExprPtr(new StrExpr(s)); }
    case T_LBRACKET: {                       // lista [a, b, c]
      lx.next();
      auto *arr = new ArrayExpr();
      if (lx.peek().type != T_RBRACKET) {
        arr->elems.push_back(parseExpression(lx));
        while (lx.accept(T_COMMA)) arr->elems.push_back(parseExpression(lx));
      }
      if (!lx.accept(T_RBRACKET)) parseError(lx, "']'");
      return ExprPtr(arr);
    }
    case T_LPAREN: {                          // (Expr) agrupación o (a,b) tupla
      lx.next();
      ExprPtr first = parseExpression(lx);
      if (lx.accept(T_COMMA)) {
        auto *arr = new ArrayExpr();          // tupla = lista
        arr->elems.push_back(std::move(first));
        arr->elems.push_back(parseExpression(lx));
        while (lx.accept(T_COMMA)) arr->elems.push_back(parseExpression(lx));
        if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
        return ExprPtr(arr);
      }
      if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
      return first;
    }
    case T_IDENTIFIER: {
      std::string name = lx.next().str;
      if (lx.accept(T_LPAREN)) {              // llamada a función
        // path_width(&p) (§8.x/§9): reducción path→número, no una función de
        // CallExpr (su argumento es un PathExpr, no un Expr). Antes de armar
        // el CallExpr genérico para que no lo intercepte como nombre desconocido.
        if (name == "path_width") return parsePathWidthCall(lx);
        if (name == "path_x_min_at_y") return parsePathXBoundCall(lx, false);
        if (name == "path_x_max_at_y") return parsePathXBoundCall(lx, true);
        if (name == "point_at") return parsePathSampleCall(lx, false);
        if (name == "angle_at") return parsePathSampleCall(lx, true);
        auto *call = new CallExpr(name);
        if (lx.peek().type != T_RPAREN) {
          call->args.push_back(parseExpression(lx));
          while (lx.accept(T_COMMA)) call->args.push_back(parseExpression(lx));
        }
        if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
        return ExprPtr(call);
      }
      if (lx.accept(T_LBRACKET)) {            // indexado lista[i]
        ExprPtr idx = parseExpression(lx);
        if (!lx.accept(T_RBRACKET)) parseError(lx, "']'");
        return ExprPtr(new IndexExpr(name, std::move(idx)));
      }
      return ExprPtr(new IdentExpr(name));    // variable (o constante pi)
    }
    default:
      parseError(lx, "una expresión (número, variable, '(' o '[')");
      return nullptr;
  }
}

static ExprPtr parsePower(Lexer &lx) {        // Atom ["^" Unary]  (der-asociativo)
  ExprPtr base = parseAtom(lx);
  if (lx.peek().type == T_CARET) {
    lx.next();
    return ExprPtr(new BinExpr(T_CARET, std::move(base), parseUnary(lx)));
  }
  return base;
}

static ExprPtr parseUnary(Lexer &lx) {        // "-" Unary | Power
  if (lx.peek().type == T_MINUS) {
    lx.next();
    return ExprPtr(new UnExpr(parseUnary(lx)));
  }
  return parsePower(lx);
}

static ExprPtr parseTerm(Lexer &lx) {         // Unary (("*"|"/") Unary)*
  ExprPtr e = parseUnary(lx);
  while (lx.peek().type == T_STAR || lx.peek().type == T_SLASH) {
    int op = lx.next().type;
    e = ExprPtr(new BinExpr(op, std::move(e), parseUnary(lx)));
  }
  return e;
}

static ExprPtr parseAdditive(Lexer &lx) {     // Term (("+"|"-") Term)*
  ExprPtr e = parseTerm(lx);
  while (lx.peek().type == T_PLUS || lx.peek().type == T_MINUS) {
    int op = lx.next().type;
    e = ExprPtr(new BinExpr(op, std::move(e), parseTerm(lx)));
  }
  return e;
}

// Comparación (§6.1): Additive [ (== != < > <= >=) Additive ]. No asociativa
// (un solo comparador; encadenar se hace con `and`).
static bool isCmpOp(int t) {
  return t == T_EQ || t == T_NE || t == T_LT || t == T_GT || t == T_LE || t == T_GE;
}
static ExprPtr parseComparison(Lexer &lx) {
  ExprPtr e = parseAdditive(lx);
  if (isCmpOp(lx.peek().type)) {
    int op = lx.next().type;
    e = ExprPtr(new BinExpr(op, std::move(e), parseAdditive(lx)));
  }
  return e;
}
// `not` (§6.1): liga MÁS que `and`/`or` y MENOS que la comparación, la convención de
// los lenguajes con operadores en palabra — `not a > b` es `not (a > b)`, no
// `(not a) > b`. Es prefijo y se puede encadenar (`not not x`).
static ExprPtr parseNot(Lexer &lx) {          // "not" Not | Comparison
  if (lx.peek().type == T_NOT) {
    lx.next();
    return ExprPtr(new NotExpr(parseNot(lx)));
  }
  return parseComparison(lx);
}
static ExprPtr parseAnd(Lexer &lx) {          // Not ("and" Not)*
  ExprPtr e = parseNot(lx);
  while (lx.peek().type == T_AND) {
    lx.next();
    e = ExprPtr(new BinExpr(T_AND, std::move(e), parseNot(lx)));
  }
  return e;
}
// Entrada de expresión: `or` es el operador de menor precedencia (§6.1).
static ExprPtr parseExpression(Lexer &lx) {   // And ("or" And)*
  ExprPtr e = parseAnd(lx);
  while (lx.peek().type == T_OR) {
    lx.next();
    e = ExprPtr(new BinExpr(T_OR, std::move(e), parseAnd(lx)));
  }
  return e;
}

// ============================================================================
// Slice 2: sentencias -> árbol dibujable. Subconjunto lineal: config + primitivas
// + asignación (sin estado/color todavía, sin structs/for/if). Regla: posiciones
// separadas por espacio (config, coordenadas) = Term; args nombrados (…) = Expression.
// ============================================================================
// --- Colores V3: nombre o hex "#RRGGBB" -> int RGB (tabla del motor V1) ------
// Nombres CSS/X11 estándar, los 148, SIN EXCEPCIONES (2026-07-21).
//
// Hasta esa fecha dos conservaban el valor histórico de V1 y mentían: `orange` era
// 0xcc3232 (un rojo ladrillo, no un naranja) y `green` era 0x00ff00 (el verde puro,
// que en CSS se llama `lime`). Se corrigieron con el argumento de la 4ª condición
// para el 1.0 (§22.7): que un usuario nuevo encuentre lo que espera. Es la misma
// clase de defecto que el renombre de §13 —un nombre que significa otra cosa que en
// el resto del mundo— y mientras la beta dure cuesta un `sed`.
//
// No se pierde nada: el verde puro sigue disponible bajo su nombre CSS correcto,
// `lime`. Y que la tabla sea CSS sin excepciones es una regla menos que recordar:
// antes había que saberse cuáles dos no lo eran.
static const std::map<std::string, int> map_color = {
  {"aliceblue", 0xf0f8ff}, {"antiquewhite", 0xfaebd7}, {"aqua", 0x00ffff},
  {"aquamarine", 0x7fffd4}, {"azure", 0xf0ffff}, {"beige", 0xf5f5dc},
  {"bisque", 0xffe4c4}, {"black", 0x000000}, {"blanchedalmond", 0xffebcd},
  {"blue", 0x0000ff}, {"blueviolet", 0x8a2be2}, {"brown", 0xa52a2a},
  {"burlywood", 0xdeb887}, {"cadetblue", 0x5f9ea0}, {"chartreuse", 0x7fff00},
  {"chocolate", 0xd2691e}, {"coral", 0xff7f50}, {"cornflowerblue", 0x6495ed},
  {"cornsilk", 0xfff8dc}, {"crimson", 0xdc143c}, {"cyan", 0x00ffff},
  {"darkblue", 0x00008b}, {"darkcyan", 0x008b8b}, {"darkgoldenrod", 0xb8860b},
  {"darkgray", 0xa9a9a9}, {"darkgreen", 0x006400}, {"darkgrey", 0xa9a9a9},
  {"darkkhaki", 0xbdb76b}, {"darkmagenta", 0x8b008b}, {"darkolivegreen", 0x556b2f},
  {"darkorange", 0xff8c00}, {"darkorchid", 0x9932cc}, {"darkred", 0x8b0000},
  {"darksalmon", 0xe9967a}, {"darkseagreen", 0x8fbc8f}, {"darkslateblue", 0x483d8b},
  {"darkslategray", 0x2f4f4f}, {"darkslategrey", 0x2f4f4f}, {"darkturquoise", 0x00ced1},
  {"darkviolet", 0x9400d3}, {"deeppink", 0xff1493}, {"deepskyblue", 0x00bfff},
  {"dimgray", 0x696969}, {"dimgrey", 0x696969}, {"dodgerblue", 0x1e90ff},
  {"firebrick", 0xb22222}, {"floralwhite", 0xfffaf0}, {"forestgreen", 0x228b22},
  {"fuchsia", 0xff00ff}, {"gainsboro", 0xdcdcdc}, {"ghostwhite", 0xf8f8ff},
  {"gold", 0xffd700}, {"goldenrod", 0xdaa520}, {"gray", 0x808080},
  {"green", 0x008000}, {"greenyellow", 0xadff2f}, {"grey", 0x808080},
  {"honeydew", 0xf0fff0}, {"hotpink", 0xff69b4}, {"indianred", 0xcd5c5c},
  {"indigo", 0x4b0082}, {"ivory", 0xfffff0}, {"khaki", 0xf0e68c},
  {"lavender", 0xe6e6fa}, {"lavenderblush", 0xfff0f5}, {"lawngreen", 0x7cfc00},
  {"lemonchiffon", 0xfffacd}, {"lightblue", 0xadd8e6}, {"lightcoral", 0xf08080},
  {"lightcyan", 0xe0ffff}, {"lightgoldenrodyellow", 0xfafad2}, {"lightgray", 0xd3d3d3},
  {"lightgreen", 0x90ee90}, {"lightgrey", 0xd3d3d3}, {"lightpink", 0xffb6c1},
  {"lightsalmon", 0xffa07a}, {"lightseagreen", 0x20b2aa}, {"lightskyblue", 0x87cefa},
  {"lightslategray", 0x778899}, {"lightslategrey", 0x778899}, {"lightsteelblue", 0xb0c4de},
  {"lightyellow", 0xffffe0}, {"lime", 0x00ff00}, {"limegreen", 0x32cd32},
  {"linen", 0xfaf0e6}, {"magenta", 0xff00ff}, {"maroon", 0x800000},
  {"mediumaquamarine", 0x66cdaa}, {"mediumblue", 0x0000cd}, {"mediumorchid", 0xba55d3},
  {"mediumpurple", 0x9370db}, {"mediumseagreen", 0x3cb371}, {"mediumslateblue", 0x7b68ee},
  {"mediumspringgreen", 0x00fa9a}, {"mediumturquoise", 0x48d1cc}, {"mediumvioletred", 0xc71585},
  {"midnightblue", 0x191970}, {"mintcream", 0xf5fffa}, {"mistyrose", 0xffe4e1},
  {"moccasin", 0xffe4b5}, {"navajowhite", 0xffdead}, {"navy", 0x000080},
  {"oldlace", 0xfdf5e6}, {"olive", 0x808000}, {"olivedrab", 0x6b8e23},
  {"orange", 0xffa500}, {"orangered", 0xff4500}, {"orchid", 0xda70d6},
  {"palegoldenrod", 0xeee8aa}, {"palegreen", 0x98fb98}, {"paleturquoise", 0xafeeee},
  {"palevioletred", 0xdb7093}, {"papayawhip", 0xffefd5}, {"peachpuff", 0xffdab9},
  {"peru", 0xcd853f}, {"pink", 0xffc0cb}, {"plum", 0xdda0dd},
  {"powderblue", 0xb0e0e6}, {"purple", 0x800080}, {"rebeccapurple", 0x663399},
  {"red", 0xff0000}, {"rosybrown", 0xbc8f8f}, {"royalblue", 0x4169e1},
  {"saddlebrown", 0x8b4513}, {"salmon", 0xfa8072}, {"sandybrown", 0xf4a460},
  {"seagreen", 0x2e8b57}, {"seashell", 0xfff5ee}, {"sienna", 0xa0522d},
  {"silver", 0xc0c0c0}, {"skyblue", 0x87ceeb}, {"slateblue", 0x6a5acd},
  {"slategray", 0x708090}, {"slategrey", 0x708090}, {"snow", 0xfffafa},
  {"springgreen", 0x00ff7f}, {"steelblue", 0x4682b4}, {"tan", 0xd2b48c},
  {"teal", 0x008080}, {"thistle", 0xd8bfd8}, {"tomato", 0xff6347},
  {"turquoise", 0x40e0d0}, {"violet", 0xee82ee}, {"wheat", 0xf5deb3},
  {"white", 0xffffff}, {"whitesmoke", 0xf5f5f5}, {"yellow", 0xffff00},
  {"yellowgreen", 0x9acd32},
};
static int get_color(const std::string &c) {
  if (!c.empty() && c[0] == '#') return (int)std::strtol(c.c_str() + 1, nullptr, 16);
  auto it = map_color.find(c);
  if (it != map_color.end()) return it->second;
  warn("color desconocido (uso negro): ", c);   // aviso NO fatal; la salida sigue
  return 0;
}
// Un color puede darse como cadena (nombre/hex) o como número (RGB directo, p.ej.
// el resultado de gray()/un entero 0xRRGGBB).
static int colorFromValue(const Value &v) {
  return v.type == Value::STRING ? get_color(v.str) : (int)v.num;
}

static bool emitStyleAttr(const std::string &, const Value &, GraphicsItemList &);   // adelantada

// §4.11: construye el FillPattern de un tramado a partir del valor de `hatch`
// (sobrecargado, como `dash`): número = ángulo libre de UNA familia; cadena =
// estilo nombrado (à la Asymptote) — "hatch"→45°, "hatchback"→135°,
// "crosshatch"→45°+135° (dos familias). Estilo desconocido → sin trama (vector
// vacío, silencioso, igual que el hatchIndex→0 de la tabla FPATRN anterior).
//
// `hatch_angle` (angleGiven) DESACOPLA la orientación del tipo: `hatch` = qué
// trama, `hatch_angle` = a qué ángulo, igual que `hatch_gap` = a qué paso. En un
// estilo simple fija el ángulo de las líneas; en `crosshatch` gira TODA la familia
// (base y base+90 siguen perpendiculares) → `hatch_angle=0` da la rejilla RECTA.
// El `hatch=<número>` sobrecargado sigue siendo el atajo de la familia simple.
static FillPattern buildHatchPattern(const Value &v, double gap,
                                     double angle, bool angleGiven) {
  if (v.type == Value::STRING) {
    if (v.str == "hatch")      return { HatchLine{angleGiven ? angle :  45.0, gap} };
    if (v.str == "hatchback")  return { HatchLine{angleGiven ? angle : 135.0, gap} };
    if (v.str == "crosshatch") {
      double b = angleGiven ? angle : 45.0;
      return { HatchLine{b, gap}, HatchLine{b + 90.0, gap} };
    }
    return {};
  }
  return { HatchLine{v.num, gap} };
}

// §4.11: emite el HatchAttr (FillPattern completo) + activa el relleno.
// Compartido por el atributo por-primitiva (emitPrimStyle) y la sentencia de
// estado (StateStmt), para que ambos registros se comporten idéntico. El ángulo
// base es opcional (la forma posicional `hatch <a> [paso]` no lo pasa).
static void emitHatch(const Value &v, double gap, GraphicsItemList &out,
                      double angle = 0.0, bool angleGiven = false) {
  FillPattern fp = buildHatchPattern(v, gap, angle, angleGiven);
  if (fp.empty()) return;
  g_flags.using_hatcher = true;
  out.push_back(std::make_unique<HatchAttr>(fp));
  out.push_back(std::make_unique<GraphicsState>(GS_FILL));
}

// Atributos nombrados que una primitiva reconoce (§7.5 + los propios de cada
// forma). Lo que no esté aquí se DESCARTABA EN SILENCIO: `marker(rotate=90)`,
// `dot(tamano=5)`, `polyline(colour="red")` compilaban sin hacer nada y sin avisar,
// que es el peor destino de un typo — el atributo *parece* estar puesto.
//
// La lista es COMÚN a todas las primitivas, no por forma: `circle(closed=true)`
// sigue pasando. Se cierra el caso que duele (un nombre que no existe en NINGUNA)
// sin arriesgar falsos positivos en el corpus; afinar por primitiva es una vuelta
// posterior, cuando exista la referencia que diga qué acepta cada una.
static bool isKnownPrimAttr(const std::string &k) {
  static const char *ok[] = {
    // estilo (emitPrimStyle / emitStyleAttr)
    "color", "fill", "line_width", "dash", "hatch", "hatch_gap", "hatch_angle",
    // forma
    "closed",                                   // polyline/polygon §4.1
    "w", "h", "at",                             // rectangle(w,h,at) §4.4 (centro+tamaño)
    "from", "to",                               // arc §4.5
    "width", "dir",                             // polybar §4.12
    "shape", "size",                            // marker/dot §4.6
    // marcadores sobre la ruta (§B)
    "marker", "marker_orient", "marker_size", "marker_color", "marker_fill",
    "marker_start", "marker_mid", "marker_end",
    "marker_start_shift", "marker_end_shift",
    "marker_start_orient", "marker_end_orient",   // §4.5/§B: sobreescriben el global
  };
  for (const char *n : ok) if (k == n) return true;
  return false;
}

// Compuerta contra el typo silencioso en los GENERADORES (§13) y en los
// constructos de colocación (place/repeat/fit). Es el mismo defecto que se cerró
// en las primitivas y en text() el 2026-07-22: `axis(disparate=1)` compilaba, no
// hacía nada y no decía una palabra.
//
// ⚠️ CÓMO SE ARMARON LAS LISTAS, que es la parte delicada. NO salen de un grep de
// los accesos directos: eso ya falló una vez (`marker_start_orient=`, que existe y
// está en la spec, se pasa a un helper y no aparecía). Salen de cruzar TRES
// fuentes: (1) `docs/referencia.md` §11, que enumera qué acepta cada constructo;
// (2) un barrido de los accesos a `named` en el cuerpo de cada Stmt, con
// cualquier nombre de variable —`PlotStmt` lee `m.find("grid")`, no
// `named.find`—; y (3) los nombres que un constructo INYECTA en otro: `plot`
// escribe `from`/`to`/`scale`/`field`/`color`/`line_width`… en el `AxisStmt` del
// usuario y luego lo ejecuta, así que la lista de `axis` es la unión de lo que
// `axis` lee y lo que `plot` le pone.
//
// Los nombres consumidos en PARSE-time (`transform=` de repeat, `stretch=` de fit)
// nunca llegan a `named`, así que no pueden dar falso positivo aquí.
template <size_t N>
static void checkKnownArgs(const char *who,
                           const std::map<std::string, ExprPtr> &named,
                           const char *const (&ok)[N]) {
  for (const auto &kv : named) {
    bool found = false;
    for (size_t i = 0; i < N && !found; ++i) found = (kv.first == ok[i]);
    if (!found)
      evalError((std::string(who) + ": `" + kv.first +
                 "=` no existe (¿mal escrito?)").c_str());
  }
}

// axis / xaxis / yaxis. La unión que explica el comentario de arriba: lo que lee
// AxisStmt más lo que plot le inyecta (from/to/scale/field/color/line_width/dash/
// grid/grid_dash/base).
static const char *const kAxisArgs[] = {
  "from", "to", "step", "start", "ticks", "tick_size", "minor",
  "tick_labels", "decimals", "strip_zero",
  "tick_label_gap", "tick_label_size", "tick_label_font",
  "tick_label_align", "tick_label_valign",
  "label", "label_at", "label_gap", "label_size", "label_font",
  "base", "extend", "scale", "field",
  "color", "line_width", "dash", "grid", "grid_dash",
};
static const char *const kNumbersArgs[] = {
  "from", "by", "count", "at", "advance", "decimals", "prefix", "suffix",
};
static const char *const kTicksArgs[]  = { "count", "mark", "at", "advance" };
static const char *const kGridArgs[]   = { "xstep", "ystep" };
static const char *const kPlotArgs[]   = {
  "x", "y", "box", "xscale", "yscale", "grid", "grid_dash", "frame",
};
static const char *const kRuleArgs[]   = {
  "x", "y", "to", "label", "label_at", "label_size", "color", "dash", "line_width",
};
static const char *const kLegendArgs[] = {
  "at", "margin", "sample_width", "sample_height", "gap", "row_gap", "font_size",
};
static const char *const kTableArgs[]  = {
  "at", "col_widths", "row_height", "decimals", "align", "border", "fill",
  "label_col", "label_font", "font_size", "margin",
};
// ⚠️ NO lleva amplitude/half_cycles/phase/squared. Los tuve un rato: mi barrido de
// `named` delimitaba cada Stmt hasta el SIGUIENTE `struct`, y entre PlaceStmt y el
// siguiente vive el helper de `sine`, así que le atribuyó cuatro nombres ajenos.
// Lo destapó una prueba de COMPORTAMIENTO, no de lectura de código:
// `place(P, count=6, half_cycles=2, amplitude=1)` da coordenadas byte-idénticas a
// `place(P, count=6)` — se aceptaban y no hacían nada, que es justo el bug que esta
// compuerta cierra. Un locus de onda para `place` no existe.
static const char *const kPlaceArgs[]  = {
  "scale", "shift", "count", "gap", "both_sides", "r", "from", "to",
};
static const char *const kRepeatArgs[] = { "count", "scale", "rotate", "at", "advance" };

// Estilo por-primitiva (§7.5) compartido por PrimStmt y compound (§9.4):
// color/fill/line_width + tramado (hatch/hatch_gap, §4.11) + contorno (color con
// relleno). Emite en `attrs` los GraphicsItem que el llamador acota con push/pop.
static void emitPrimStyle(const std::map<std::string, ExprPtr> &named, Scope &s,
                          GraphicsItemList &attrs) {
  for (const char *k : {"color", "fill", "line_width", "dash"}) {
    auto it = named.find(k);
    if (it != named.end()) emitStyleAttr(k, it->second->eval(s), attrs);
  }
  if (named.count("hatch")) {   // §4.11: hatch=ángulo|"estilo" [+ hatch_gap/hatch_angle]
    Value hv = named.at("hatch")->eval(s);
    double gap = named.count("hatch_gap") ? named.at("hatch_gap")->eval(s).num : 4.0;
    bool angGiven = named.count("hatch_angle");
    double ang = angGiven ? named.at("hatch_angle")->eval(s).num : 0.0;
    emitHatch(hv, gap, attrs, ang, angGiven);
  }
  // color= junto a un relleno (fill= o hatch) = contornear (§4).
  if (named.count("color") && (named.count("fill") || named.count("hatch")))
    attrs.push_back(std::make_unique<GraphicsState>(GS_OUTLINEFILL));
}

// Emite el/los GraphicsItem de un atributo de estilo (color/fill/line_width).
// Compartido por la sentencia de estado (§7) y el atributo por-primitiva (§7.5).
// El color acepta cadena (nombre/hex) o número (RGB directo, p.ej. gray()).
// Devuelve true si reconoció el atributo.
static bool emitStyleAttr(const std::string &name, const Value &v, GraphicsItemList &out) {
  if (name == "color") {
    if (v.type == Value::STRING && v.str == "none") return true;   // sin trazo: pendiente
    auto a = std::make_unique<Attribute>();
    a->set(AT_LCOLOR, colorFromValue(v));
    out.push_back(std::move(a));
    return true;
  }
  if (name == "fill") {
    if (v.type == Value::STRING && v.str == "none") { out.push_back(std::make_unique<GraphicsState>(GS_NOFILL)); return true; }
    auto a = std::make_unique<Attribute>();
    a->set(AT_FCOLOR, colorFromValue(v));
    out.push_back(std::move(a));
    out.push_back(std::make_unique<GraphicsState>(GS_FILL));        // activar relleno (§4)
    return true;
  }
  if (name == "line_width") {
    auto a = std::make_unique<Attribute>();
    a->setF(AT_LWIDTH, v.num);                                      // pt directo (§4.10)
    out.push_back(std::move(a));
    return true;
  }
  if (name == "font_size") {                                        // §14: alto de texto en pt
    auto a = std::make_unique<Attribute>();
    a->set(AT_THEIGHT, (int)v.num);   // estado en el flujo (V1 THEIGHT): conmutable mid-doc
    out.push_back(std::move(a));
    return true;
  }
  if (name == "font") {                                             // §14.3: cara base (roman/italic/…)
    FontFace ff = get_font_face_from_string(v.type == Value::STRING ? v.str : "", g_flags.using_fontcmmi);
    if (ff != FN_NOFACE) { auto a = std::make_unique<Attribute>(); a->set(AT_TSTYLE, ff); out.push_back(std::move(a)); }
    return true;
  }
  if (name == "align") {                                            // §4.8: horizontal (left/center/right)
    int a = 0;                                                      // 0=left(start) default
    if (v.type == Value::STRING) { if (v.str == "center") a = 1; else if (v.str == "right") a = 2; }
    if (a != 0) g_flags.using_textalign = true;   // EPS: activa el prólogo /cshow /rshow (si no, cae en el operador cshow real de PS y truena typecheck)
    auto at = std::make_unique<Attribute>(); at->set(AT_TALIGN, a); out.push_back(std::move(at));
    return true;
  }
  if (name == "valign") {
    int a = 0;  // baseline
    if (v.type == Value::STRING) {
        if (v.str == "top") a = 1;
        else if (v.str == "middle") a = 2;
        else if (v.str == "bottom") a = 3;
    }
    auto at = std::make_unique<Attribute>(); at->set(AT_TVALIGN, a); out.push_back(std::move(at));
    return true;
}
  if (name == "dash") {                             // §4.10: patrón de línea
    int idx = 0;                                    // solid/none/continuous → 0
    if (v.type == Value::STRING) {
      if (v.str == "longdashed") idx = 1;          // dashArrayForIndex: {4,2}
      else if (v.str == "dashed") idx = 2;
      else if (v.str == "dotted") idx = 3;                          // {2,1.6}
      else if (v.str == "dashdot" || v.str == "dash-dot") idx = 4;  // {4,2,1,2}
      else if (v.str == "dashdotdot") idx = 5;                      // {4,2,2,2,2,2}
      else if (v.str == "shortdashed") idx = 6;
    }
    auto a = std::make_unique<Attribute>(); a->set(AT_LSTYLE, idx); out.push_back(std::move(a));
    return true;
  }
  return false;   // hatch (manejado aparte por emitHatch) / transform… → pendientes
}

struct Stmt {
  virtual ~Stmt() {}
  virtual void exec(Scope &, MetaGrafica &, GraphicsItemList &) = 0;
  // true solo en las sentencias de transformación local (§11.1): apilan una
  // matriz de mundo (OPMPUSH) que el bloque/struct contenedor debe desapilar.
  virtual bool isTransform() const { return false; }
};
using StmtPtr = std::unique_ptr<Stmt>;

// Nº de sentencias de transformación DIRECTAS en un cuerpo: el bloque/struct que
// lo ejecuta emite ese número de OPMPOP al final para restaurar la matriz (§11.1).
// Las anidadas (dentro de for/if/{}) las restaura su propio contenedor.
static int countTransforms(const std::vector<StmtPtr> &body) {
  int n = 0;
  for (const auto &s : body) if (s->isTransform()) n++;
  return n;
}
static void popTransforms(int n, GraphicsItemList &out) {
  for (int i = 0; i < n; i++) {
    auto t = std::make_unique<Transform>();
    t->setOperation(OPMPOP);
    out.push_back(std::move(t));
  }
}

// Argumento de sentencia de estado: número (Term) o cadena (color).
struct SArg { bool isStr = false; ExprPtr num; std::string str; };

struct StateStmt : Stmt {
  std::string name;
  std::vector<SArg> args;
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    if (name == "outlinefill") {              // §4.11: contornea los rellenos del bloque
      // Como siempre hay color de trazo, el contorno no puede inferirse de `color=`
      // (eso solo aplica al registro por-primitiva, §4); aquí es explícito.
      // `outlinefill` / `outlinefill true` = on; `outlinefill false` = off.
      bool on = args.empty() || (args[0].isStr ? args[0].str != "false"
                                                : args[0].num->eval(s).num != 0.0);
      out.push_back(std::make_unique<GraphicsState>(on ? GS_OUTLINEFILL : GS_NOOUTLINEFILL));
      return;
    }
    if (args.empty()) return;
    if (name == "hatch") {          // §4.11: hatch <ángulo>|"estilo" [paso] (posicionales)
      Value v = args[0].isStr ? Value(args[0].str) : Value(args[0].num->eval(s).num);
      double gap = (args.size() > 1 && !args[1].isStr) ? args[1].num->eval(s).num : 4.0;
      emitHatch(v, gap, out);
      return;
    }
    Value v = args[0].isStr ? Value(args[0].str) : Value(args[0].num->eval(s).num);
    // emitStyleAttr YA devolvía si reconoció el nombre; el retorno se descartaba,
    // así que una sentencia de estado mal escrita (`colour 0.5`) compilaba, no
    // hacía nada y NO avisaba: figura plausible pero equivocada, la misma clase
    // de fallo que motivó checkCoordPairs y la validación 3k+1 de bezier. Es
    // fatal por el mismo argumento que hizo fatal a evalError: un documento
    // inválido no debe salir con código 0. Aquí solo llegan los nombres que no
    // atendieron las ramas de arriba (outlinefill/hatch), así que `false` es
    // inequívocamente "no lo conozco".
    if (!emitStyleAttr(name, v, out))
      evalError("sentencia de estado desconocida: ", name);
  }
};

// Transformación local como sentencia de estado (§11.1): translate/rotate/scale/
// shear. Post-multiplica la matriz de mundo (OPMPUSH) → afecta a lo que sigue en
// el bloque; el contenedor la desapila al salir (countTransforms/popTransforms).
// La rotación es alrededor del origen del marco vigente (matriz), no de la pluma.
struct TransformStmt : Stmt {
  int mop;                    // OPMTL/OPMRT/OPMSC/OPMSH
  std::vector<ExprPtr> args;
  bool isTransform() const override { return true; }
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    auto n = [&](size_t i) { return i < args.size() ? args[i]->eval(s).num : 0.0; };
    Matrix m;
    switch (mop) {
      case OPMTL: m.translate(n(0), n(1)); break;
      case OPMRT: m.rotate(n(0)); break;
      case OPMSC: {
        double sx = n(0), sy = args.size() >= 2 ? n(1) : sx;
        if (sx != sy) g_flags.using_ellipse = true;   // círculo bajo escala anisótropa → elipse
        m.scale(sx, sy);
        break;
      }
      case OPMSH: g_flags.using_ellipse = true; m.shear(n(0), n(1)); break;
    }
    auto t = std::make_unique<Transform>();
    t->setOperation(OPMPUSH); t->setMatrix(m);
    out.push_back(std::move(t));
  }
};

struct AssignStmt : Stmt {
  std::string name;
  ExprPtr val;
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &) override {
    // Alcance tipo bloque (C/JS): si la variable ya existe en un ámbito ancestro,
    // se modifica AHÍ; si no, se crea local. Así un `w = …` dentro de un if/else
    // altera el `w` declarado afuera (no se necesita el else para dar un default).
    // MG no tiene declaración explícita (let/var), así que la asignación es el
    // único mecanismo y esta es la semántica intuitiva. (Las lecturas ya subían
    // vía Scope::find.) Evalúa antes de asignar.
    Value v = val->eval(s);
    if (Value *p = s.find(name)) *p = std::move(v);
    else                         s.vars[name] = std::move(v);
  }
};

// Tope de expansión de structs recursivas (§18) y contador vigente. Declarados
// aquí porque ConfigStmt (`max_depth n`) los fija; se usan en execStructBody,
// junto a las structs, donde está el razonamiento.
static int g_maxDepth = 32;
static int g_structDepth = 0;

struct ConfigStmt : Stmt {
  std::string which;
  std::vector<ExprPtr> args;
  ExprPtr stretchE;                        // world_window ... stretch= (null = meet)
  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &) override {
    auto n = [&](size_t i) { return args[i]->eval(s).num; };
    if (which == "display_size")      mg.setDimension(n(0), n(1));
    else if (which == "max_depth") {           // §18: tope de expansión recursiva
      int d = (int)n(0);
      if (d < 1) { evalError("max_depth debe ser al menos 1, se dio: ", std::to_string(d)); return; }
      g_maxDepth = d;
    }
    else if (which == "world_window") {
      mg.setWindow(n(0), n(2), n(1) - n(0), n(3) - n(2));
      mg.setStretch(stretchE && stretchE->eval(s).num != 0.0);    // §13.7 marco de datos
    }
  }
};

// Directorio del archivo principal (para resolver includes relativos, §15).
// Definición de la global declarada `extern` en parserv3.h; main.cpp la fija.
std::string g_baseDir;

// include "archivo.mg" (§15): trae las structs/definiciones de otro archivo al
// espacio global. Se lee y parsea en tiempo de parseo (parseInclude); su cuerpo
// se ejecuta aquí en orden, antes de que el archivo principal lo use.
struct IncludeStmt : Stmt {
  std::vector<StmtPtr> prog;
  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    for (auto &st : prog) st->exec(s, mg, out);
  }
};

// Bloque anónimo (§7.1): apila/restaura el estado gráfico (gsave/grestore) y
// abre un ámbito léxico hijo para las variables.
struct BlockStmt : Stmt {
  std::vector<StmtPtr> body;
  void exec(Scope &parent, MetaGrafica &mg, GraphicsItemList &out) override {
    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
    Scope child(&parent);                  // reads y writes suben al padre (AssignStmt); nombres nuevos = locales
    for (auto &st : body) st->exec(child, mg, out);
    popTransforms(countTransforms(body), out);   // restaura transforms locales (§11.1)
    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// --- Structs (§8): definición reutilizable + invocación paramétrica/recursiva -
struct StructDef {
  std::string name;
  std::vector<std::string> params;
  std::vector<ExprPtr> defaults;   // paralelo a params; null si no hay default
  std::vector<bool> paramIsPath;   // paralelo a params (§8.x): true si se declaró `&nombre`
  std::vector<StmtPtr> body;
  // Ventana local (world_window DENTRO del cuerpo): define la caja de coordenadas
  // que place/fit normalizan al locus/rectángulo (§10, §16). 4 exprs
  // [xmin xmax ymin ymax]; vacío = caja unitaria (sin normalizar).
  std::vector<ExprPtr> localWindow;
};

// Caja de coordenadas de un struct (origen + tamaño) para place/fit.
struct Box { double x = 0, y = 0, dx = 1, dy = 1; };
static Box structBox(Scope &s, const StructDef *def) {
  if (def->localWindow.size() == 4) {
    double a = def->localWindow[0]->eval(s).num, b = def->localWindow[1]->eval(s).num;
    double c = def->localWindow[2]->eval(s).num, d = def->localWindow[3]->eval(s).num;
    return Box{a, c, b - a, d - c};
  }
  return Box{0, 0, 1, 1};
}

// Bbox de un path (§9): caja fuente de fit(<PathExpr>) y de path_width (paso 5
// de plan_struct_params.md, factorizado de FitStmt::exec). Vacío = caja nula;
// el llamador decide qué hacer (fit-de-path ya comprueba path.empty() antes).
static Box path_bbox(const Path &path) {
  if (path.empty()) return Box{0, 0, 0, 0};
  double minx = path[0].x, miny = path[0].y, maxx = path[0].x, maxy = path[0].y;
  for (const point &p : path) {
    if (p.x < minx) minx = p.x; if (p.x > maxx) maxx = p.x;
    if (p.y < miny) miny = p.y; if (p.y > maxy) maxy = p.y;
  }
  return Box{minx, miny, maxx - minx, maxy - miny};
}

// Registro global (§15: nombres globales). Posee las definiciones.
static std::map<std::string, std::unique_ptr<StructDef>> g_structs;

// --- Profundidad de recursión (§18) -----------------------------------------
// Una struct puede invocarse a sí misma (§8.1). Sin límite, una recursión sin
// condición de paro agota la pila del proceso y el compilador MUERE POR SEÑAL,
// que es el único modo de falla que no pasa por evalError. `max_depth n` fija el
// tope; 32 de default, como pide la spec.
//
// El límite es también SEMÁNTICA, no solo defensa: el listado V0 del árbol
// fractal (Ciencias 21, 1991, Apéndice 1; ver examples/fractal_tree.mg) NO tiene
// condición de paro —V0 no tenía condicionales— así que la profundidad era lo
// único que lo detenía. Con V1 la palabra sobrevivió en el léxico (MAXDEEP,
// src/mgpp.l) pero nadie la aplicaba. (g_maxDepth/g_structDepth se declaran
// arriba, junto a ConfigStmt, que es quien fija el tope.)
//
// Ejecuta el CUERPO de una struct llevando la cuenta de profundidad. Único punto
// por el que pasan los cinco sitios que expanden un cuerpo (invoke, repeat, fit,
// buildStructure y el volcado temporal de plot-log), para que ninguno se salte la
// guarda. Save/restore explícito y no RAII: el proyecto compila sin excepciones,
// así que el único camino que se salta el decremento es evalError, que sale del
// proceso.
static void execStructBody(const StructDef *def, Scope &local, MetaGrafica &mg,
                           GraphicsItemList &out) {
  if (g_structDepth >= g_maxDepth) {
    evalError("profundidad de recursión excedida (§18) en la struct: ",
              def->name + " — max_depth = " + std::to_string(g_maxDepth) +
                  ". ¿Falta la condición de paro (if, §8.1)?");
    return;                                   // inalcanzable: evalError es fatal
  }
  ++g_structDepth;
  for (auto &st : def->body) st->exec(local, mg, out);
  --g_structDepth;
}

// Bandera de contexto: activa mientras PlotStmt ejecuta contenido en modo LOG
// (plan_plot.md Fase 4 Paso 5). Los colocadores de structs (invoke/repeat/fit/
// place) la consultan y erroran: una struct dibuja en coords LOCALES y el motor
// aplica su matriz en draw-time, así que el mapper puntual del plot (no afín)
// daría matriz(map(p)) ≠ map(matriz(p)). Detección en la SENTENCIA, no en el item
// (evita falsos negativos de structs sin Transform y falsos positivos de rotate).
static bool g_plotLogContext = false;
static bool logPlotBlocksStruct(const char *what) {
  if (!g_plotLogContext) return false;
  evalError("structs dentro de un plot logarítmico: aún no soportado (", std::string(what) + ")");
  return true;
}

struct StructDefStmt : Stmt {
  std::unique_ptr<StructDef> def;
  void exec(Scope &, MetaGrafica &, GraphicsItemList &) override {
    g_structs[def->name] = std::move(def);   // registra; el registro pasa a poseerla
  }
};

// ── Álgebra de paths (§9) ────────────────────────────────────────────────────
// Un PathExpr evalúa a un Path (= std::vector<point>). Vive APARTE de la jerarquía
// Expr/Value (que solo transporta escalares/cadenas/listas), igual que g_structs
// vive aparte de Scope::vars: un Value no puede llevar un Path. Todas las ops son
// funcionales (no mutan su operando).
struct PathExpr {
  virtual ~PathExpr() {}
  virtual Path evalPath(Scope &s) const = 0;
};
using PathExprPtr = std::unique_ptr<PathExpr>;

// path_width(&p) (§8.x/§9, decisión 6 de plan_struct_params.md): reducción
// path→número (extensión en x del bbox), no un CallExpr más — su argumento es
// un PathExpr, no un Expr (la misma razón por la que Path vive aparte de
// Value, ver el comentario de arriba; aquí la fuga va al revés: lo que sale
// es un double corriente). Nombre PROVISIONAL: primer miembro construido de
// una familia reservada (path_height, path_x_bounds — decisión 7), se refina
// cuando todo funcione.
struct PathWidthExpr : Expr {
  PathExprPtr arg;
  Value eval(Scope &s) const override {
    return Value(path_bbox(arg->evalPath(s)).dx);
  }
};

// path_x_min_at_y/path_x_max_at_y(&p, y [, expand]): abscisas donde el path
// cruza la horizontal y — los puntos de retorno de una curva de potencial.
// Van DOS expresiones porque una expresión de MG devuelve un número (§5.2) y
// aquí salen dos; cada una calcula ambas y devuelve su lado.
//
// `expand` ensancha el par por FRACCIÓN del tramo (no por una cantidad
// absoluta): a-f·(b-a), b+f·(b-a). La fracción es lo que permite escribirlo
// sin conocer de antemano el ancho en cm — es justo lo que se necesita para
// alojar una onda con colas planas, donde f = 0.5/(path_width(&onda)-1).
struct PathXBoundExpr : Expr {
  PathExprPtr arg;
  ExprPtr level;
  ExprPtr expand;                    // opcional (nulo = 0)
  bool wantMax = false;
  Value eval(Scope &s) const override {
    const double y = level->eval(s).num;
    double xmin = 0, xmax = 0;
    if (!path_x_bounds_at_y(arg->evalPath(s), y, xmin, xmax))
      evalError(wantMax ? "path_x_max_at_y: el path no cruza y = "
                        : "path_x_min_at_y: el path no cruza y = ",
                std::to_string(y));
    if (expand) {
      const double f = expand->eval(s).num * (xmax - xmin);
      xmin -= f;
      xmax += f;
    }
    return Value(wantMax ? xmax : xmin);
  }
};

// point_at(&p, t [, curve=b]) / angle_at(&p, t [, curve=b]) — familia de muestreo
// (§9, α+β). point_at devuelve un PUNTO ([x,y], una lista de 2: funciona en at=/box=,
// aún no en un bloque {}); angle_at devuelve el ángulo de la tangente (un número).
// El flag `curve` (default false) interpreta el path como vértices (lineal) o como
// controles bézier (evalúa la cúbica): ver splines.h. El parámetro t recorre el path
// por longitud de arco, así que t=0.5 es el medio GEOMÉTRICO.
struct PathPointExpr : Expr {
  PathExprPtr arg;
  ExprPtr param;
  ExprPtr curve;                     // opcional (nulo = false)
  bool wantAngle = false;
  Value eval(Scope &s) const override {
    double t = param->eval(s).num;
    bool c = curve ? curve->eval(s).num != 0.0 : false;
    Path p = arg->evalPath(s);
    if (wantAngle) return Value(path_angle(p, t, c));
    point q = path_point(p, t, c);
    Value v; v.type = Value::LIST;
    v.items.push_back(Value(q.x));
    v.items.push_back(Value(q.y));
    return v;
  }
};

// Argumento de invocación de struct (§8.x): exactamente uno de los dos se
// llena, nunca ambos. Un solo tipo para posicionales y nombrados, en vez de
// una lista de paths aparte, para que el índice posicional siga
// correspondiendo 1:1 con StructDef::params/paramIsPath (decisión 4 de
// plan_struct_params.md) — con listas paralelas separadas, `Nivel(&pw0, 3)`
// perdería la correspondencia entre el path y el parámetro 0.
struct Arg { ExprPtr e; PathExprPtr p; };

// Liga los parámetros de una struct al ámbito local — compartido por
// InvokeStmt y FitStmt (§8.x, §10.2): ambos colocan una invocación, solo
// cambia cómo se posiciona el resultado después. Un parámetro &path liga
// {expr, ámbito del LLAMADOR} en pathBindings en vez de evaluarse aquí
// (decisión 2 de plan_struct_params.md: el PathExpr no se evalúa al ligar,
// PathRef::evalPath lo resuelve diferido cada vez que el cuerpo lo usa); el
// resto evalúa a Value en el ámbito del llamador, o el default en el ámbito
// local (los defaults ya pueden referirse a parámetros anteriores).
//
// Dos pasadas, no una: TODOS los params path se ligan antes de evaluar
// CUALQUIER default (decisión 6/paso 5), sin importar el orden de
// declaración — un default como `w = path_width(&onda)` necesita encontrar
// `onda` ya ligado aunque `&onda` no sea el parámetro anterior.
static void bindStructParams(Scope &local, Scope &caller, StructDef *def,
                             std::vector<Arg> &pos, std::map<std::string, Arg> &named) {
  // --- Aridad (§8) ---------------------------------------------------------
  // Los parámetros PATH ya se validaban; los ESCALARES no, en ningún sentido: un
  // parámetro sin argumento y sin default caía a `Value(0)` EN SILENCIO y los
  // argumentos de más se descartaban igual de callados, los dos con código 0. Es
  // la peor variante de la familia porque el resultado es PLAUSIBLE: sobre
  // `struct S(a,b)`, `S(1)` dibujaba con b=0 y no había nada que lo delatara.
  //
  // Los tres casos son el mismo defecto —un argumento que no liga con nada, o un
  // parámetro que no recibe nada— así que se cierran juntos y aquí, que es el
  // punto por el que pasan la invocación directa y la de fit.
  if (pos.size() > def->params.size()) {
    evalError("demasiados argumentos en la struct: ",
              def->name + " — recibió " + std::to_string(pos.size()) +
                  " posicionales y declara " + std::to_string(def->params.size()));
    return;
  }
  for (const auto &kv : named) {
    const std::string &k = kv.first;
    // at/rotate/scale son modificadores de COLOCACIÓN (§8), no parámetros: los
    // consume InvokeStmt, y dentro de fit ya son error antes de llegar aquí.
    if (k == "at" || k == "rotate" || k == "scale") continue;
    bool isParam = false;
    for (size_t i = 0; i < def->params.size(); i++)
      if (def->params[i] == k) { isParam = true; break; }
    if (!isParam) {
      evalError("argumento nombrado desconocido en la struct: ",
                def->name + " — `" + k + "=` no es un parámetro suyo");
      return;
    }
  }
  auto argFor = [&](size_t i, const std::string &pn) -> const Arg * {
    if (i < pos.size()) {
      // Dar el mismo parámetro por posición y por nombre es ambiguo; antes ganaba
      // el posicional sin decir nada.
      if (named.count(pn)) {
        evalError("argumento duplicado (posicional y nombrado) en la struct: ",
                  def->name + " — parámetro " + pn);
        return nullptr;
      }
      return &pos[i];
    }
    auto nit = named.find(pn);
    return nit != named.end() ? &nit->second : nullptr;
  };
  for (size_t i = 0; i < def->params.size(); i++) {
    if (!(i < def->paramIsPath.size() && def->paramIsPath[i])) continue;
    const std::string &pn = def->params[i];
    const Arg *arg = argFor(i, pn);
    if (!arg)    { evalError("falta el argumento path del parámetro &", pn); return; }
    if (!arg->p) { evalError("se esperaba un path (&nombre) para el parámetro ", pn); return; }
    local.pathBindings[pn] = PathBinding{arg->p.get(), &caller};
  }
  for (size_t i = 0; i < def->params.size(); i++) {
    if (i < def->paramIsPath.size() && def->paramIsPath[i]) continue;
    const std::string &pn = def->params[i];
    const Arg *arg = argFor(i, pn);
    if (arg) {
      if (!arg->e) { evalError("no se esperaba un path para el parámetro ", pn); return; }
      local.vars[pn] = arg->e->eval(caller);
    } else if (def->defaults[i]) {
      local.vars[pn] = def->defaults[i]->eval(local);
    } else {
      evalError("falta el argumento del parámetro en la struct: ",
                def->name + " — parámetro " + pn + " (sin valor por default)");
      return;
    }
  }
}

// Registro global de paths nombrados (§9), calcado de g_structs.
static std::map<std::string, PathExprPtr> g_paths;

// Literal de puntos { x y  x y … }: los pares se evalúan a la hora de usar el path.
struct PathLiteral : PathExpr {
  std::vector<ExprPtr> coords;                 // pares x y (Term)
  Path evalPath(Scope &s) const override {
    Path p;
    for (size_t i = 0; i + 1 < coords.size(); i += 2)
      p.push_back(point(coords[i]->eval(s).num, coords[i + 1]->eval(s).num));
    return p;
  }
};

// Referencia &ID a un path nombrado; se resuelve en g_paths al evaluar (diferido,
// para que un path pueda referir a otro declarado antes). Un parámetro path de
// struct (§8.x) ENSOMBRECE un path global del mismo nombre dentro del cuerpo
// (decisión 3 de plan_struct_params.md): por eso pathBindings se consulta
// PRIMERO, subiendo por la cadena de ámbitos como las variables.
struct PathRef : PathExpr {
  std::string name;
  Path evalPath(Scope &s) const override {
    if (PathBinding *b = s.findPath(name)) return b->expr->evalPath(*b->scope);
    auto it = g_paths.find(name);
    if (it == g_paths.end()) { evalError("path no definido: ", name); return Path(); }
    return it->second->evalPath(s);
  }
};

// Ops unarias (§9): transpose/flip_x/flip_y/reverse. La geometría vive en
// splines.cpp/.h (álgebra de datos, no de parseo); aquí solo se despacha sobre
// el path evaluado.
struct PathUnary : PathExpr {
  enum Op { TRANSPOSE, FLIP_X, FLIP_Y, REVERSE } op = TRANSPOSE;
  PathExprPtr arg;
  Path evalPath(Scope &s) const override {
    Path p = arg->evalPath(s);
    if (op == TRANSPOSE) return transpose_path(p);
    if (op == FLIP_X)    return flip_x_path(p);
    if (op == FLIP_Y)    return flip_y_path(p);
    return reverse_path(p);
  }
};

// concat(a, b, …): suelda paths en uno continuo (§9, variádico). La soldadura
// vive en concat_paths() (splines.cpp) —álgebra de datos, no de parseo—: cada
// operando se traslada para que su INICIO continúe desde el final del acumulado,
// sin auto-reversión (el autor orienta con reverse()). Así una media curva H y
// su espejo forman un perfil simétrico con `concat(reverse(flip_x(&H)), &H)`.
struct PathConcat : PathExpr {
  std::vector<PathExprPtr> parts;
  Path evalPath(Scope &s) const override {
    Path acc;
    for (auto &e : parts) acc = concat_paths(acc, e->evalPath(s));
    return acc;
  }
};

// sample(&p, n [, curve=b]) (§9, α+β): n puntos equiespaciados por longitud de arco
// (t=0 y t=1 incluidos). Devuelve un PATH, así que es álgebra §9 como concat/reverse
// —se dibuja con polyline(sample(…)) o se marca con dot(sample(…))—. El flag `curve`
// interpreta el path como vértices (default) o controles bézier (ver splines.h).
struct PathSample : PathExpr {
  PathExprPtr arg;
  ExprPtr count;
  ExprPtr curve;                     // opcional (nulo = false)
  Path evalPath(Scope &s) const override {
    int n = (int)count->eval(s).num;
    bool c = curve ? curve->eval(s).num != 0.0 : false;
    return path_sample(arg->evalPath(s), n, c);
  }
};

// smooth { pts } (§9.2): Bézier suave que pasa EXACTAMENTE por los puntos
// dados (tangentes automáticas, path_to_bezier de splines.cpp). El motor
// consume el primer y último punto como ayudas de tangente, así que aquí se
// extienden los extremos por REFLEXIÓN (p0′ = 2·p0 − p1): la curva pasa
// también por los extremos con tangente natural, y no se degenera la
// parametrización por distancias (un duplicado daría distancia cero → NaN).
// (V1: GNBZPATH, que exigía el mismo truco a mano en los datos.)
// Compartida por las DOS formas de smooth (§9.2): la expresión de path
// (PathSmooth, para componer con concat/fit) y la primitiva de dibujo
// (PrimStmt, `smooth { nodos }`), igual que sine tiene ambas.
static Path smoothPath(const Path &p) {
  if (p.size() < 2) { evalError("smooth requiere al menos 2 puntos"); return Path(); }
  size_t n = p.size();
  Path ext;
  ext.push_back(point(2 * p[0].x - p[1].x, 2 * p[0].y - p[1].y));
  for (auto &q : p) ext.push_back(q);
  ext.push_back(point(2 * p[n-1].x - p[n-2].x, 2 * p[n-1].y - p[n-2].y));
  return path_to_bezier(ext);
}

struct PathSmooth : PathExpr {
  std::vector<ExprPtr> coords;                 // pares x y
  Path evalPath(Scope &s) const override {
    Path p;
    for (size_t i = 0; i + 1 < coords.size(); i += 2)
      p.push_back(point(coords[i]->eval(s).num, coords[i + 1]->eval(s).num));
    return smoothPath(p);
  }
};

// Delegado NO-propietario: g_paths[name] apunta al árbol que POSEE un PathDeclStmt,
// sin moverlo. Así la declaración es RE-EJECUTABLE (necesario cuando `path w = …`
// vive dentro de un `for` — p.ej. el sembrado del acumulador §9). El AST sobrevive a
// todo el exec/draw, así que el puntero crudo es seguro.
struct PathAlias : PathExpr {
  PathExpr *target;
  explicit PathAlias(PathExpr *t) : target(t) {}
  Path evalPath(Scope &s) const override { return target->evalPath(s); }
};

// path ID = <PathExpr>: registra la definición (diferida) en g_paths. El Stmt POSEE
// el árbol; g_paths guarda un alias, para no moverlo y poder re-ejecutar (ver arriba).
struct PathDeclStmt : Stmt {
  std::string name;
  PathExprPtr expr;
  void exec(Scope &, MetaGrafica &, GraphicsItemList &) override {
    g_paths[name] = std::make_unique<PathAlias>(expr.get());
  }
};

// Path YA evaluado a puntos concretos (sin dependencia de ámbito). Lo produce `+=`.
struct FrozenPath : PathExpr {
  Path pts;
  Path evalPath(Scope &) const override { return pts; }
};

// path ID += <PathExpr>: acumulación (§9). Evalúa YA (no diferido, a diferencia de
// `=`): lee el path actual, le concatena la pieza en el ámbito vigente y CONGELA el
// resultado en g_paths. Así un `for` construye una curva pieza a pieza con su propia
// variable de lazo (envolvente WKB, amplitud por lóbulo…). El empalme es el de
// concat_paths (join C0: traslada cada pieza al final de la anterior). Si el nombre no
// existe aún, `+=` lo CREA con la pieza (permite sembrar sin un `=` previo).
struct PathAppendStmt : Stmt {
  std::string name;
  PathExprPtr expr;
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &) override {
    Path base;
    auto it = g_paths.find(name);
    if (it != g_paths.end()) base = it->second->evalPath(s);
    Path add = expr->evalPath(s);
    auto fr = std::make_unique<FrozenPath>();
    fr->pts = concat_paths(base, add);
    g_paths[name] = std::move(fr);
  }
};

struct InvokeStmt : Stmt {
  std::string name;
  std::vector<Arg> pos;
  std::map<std::string, Arg> named;      // incluye at/rotate/scale (aún ignorados)
  void exec(Scope &caller, MetaGrafica &mg, GraphicsItemList &out) override {
    if (logPlotBlocksStruct("invoke")) return;
    auto it = g_structs.find(name);
    if (it == g_structs.end()) { evalError("struct no definida: ", name); return; }
    StructDef *def = it->second.get();
    // Ámbito de la struct: hijo del GLOBAL (léxico), con los parámetros ligados
    // (bindStructParams: args en el ámbito del llamador, defaults en el local).
    Scope local(caller.root());
    bindStructParams(local, caller, def, pos, named);
    // Modificadores de colocación (§8): at/rotate/scale → marco canónico fijo
    // T·R·S (post-multiplicación: escala/gira en el marco local, luego traslada;
    // el orden de escritura es irrelevante). Se evalúan en el ámbito del llamador.
    // Nombres reservados: no son parámetros de la struct (nunca paths).
    Matrix frame;
    bool hasFrame = false;
    auto ait = named.find("at"), rit = named.find("rotate"), sit = named.find("scale");
    if (ait != named.end() && ait->second.e) {
      Value v = ait->second.e->eval(caller);
      if (v.type == Value::LIST && v.items.size() >= 2) { frame.translate(v.items[0].num, v.items[1].num); hasFrame = true; }
    }
    if (rit != named.end() && rit->second.e) { frame.rotate(rit->second.e->eval(caller).num); hasFrame = true; }
    if (sit != named.end() && sit->second.e) {
      Value v = sit->second.e->eval(caller);
      if (v.type == Value::LIST && v.items.size() >= 2) frame.scale(v.items[0].num, v.items[1].num);
      else                                              frame.scale(v.num, v.num);
      hasFrame = true;
    }

    // Ventana local (world_window del cuerpo): normaliza las coords del cuerpo a
    // la caja unitaria, igual que fit/place/buildStructure. Va DENTRO del marco
    // de colocación (device = … · frame · N · coord) para que el at/rotate/scale
    // posicione la caja ya centrada/normalizada. Sin esto, la invocación directa
    // ignoraba world_window (el detector de fig2-5 no se centraba ni agrandaba).
    // Ámbito LOCAL (no el del llamador): un world_window que use un parámetro de
    // la struct (p.ej. `world_window 0 w -2 2`) necesita verlo ya ligado.
    Box w = structBox(local, def);
    bool needN = (w.x != 0.0 || w.y != 0.0 || w.dx != 1.0 || w.dy != 1.0);

    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));  // aísla estado (como structure())
    if (hasFrame) {
      auto t = std::make_unique<Transform>();
      t->setOperation(OPMPUSH); t->setMatrix(frame);
      out.push_back(std::move(t));
    }
    if (needN) {
      Matrix N; N.scale(1.0 / w.dx, 1.0 / w.dy); N.translate(-w.x, -w.y);   // ventana→unidad
      auto t = std::make_unique<Transform>();
      t->setOperation(OPMPUSH); t->setMatrix(N);
      out.push_back(std::move(t));
    }
    execStructBody(def, local, mg, out);   // §18: cuenta la profundidad
    popTransforms(countTransforms(def->body), out);   // transforms locales del cuerpo (§11.1)
    if (needN) {
      auto t = std::make_unique<Transform>();
      t->setOperation(OPMPOP);
      out.push_back(std::move(t));
    }
    if (hasFrame) {
      auto t = std::make_unique<Transform>();
      t->setOperation(OPMPOP);
      out.push_back(std::move(t));
    }
    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// Bucle (§6): for var = a to b [step s] { … }. La variable es local al bucle
// (restaura el valor previo al salir: vive en un Scope hijo que se descarta).
// El cuerpo es un BlockStmt → cada iteración apila/restaura el estado gráfico
// y abre su propio ámbito léxico, igual que un bloque { } (§7.1). Inclusivo en
// ambos extremos; step puede ser negativo (§6).
struct ForStmt : Stmt {
  std::string var;
  ExprPtr start, end, step;      // step null = 1
  StmtPtr body;                  // BlockStmt
  void exec(Scope &parent, MetaGrafica &mg, GraphicsItemList &out) override {
    double a = start->eval(parent).num;
    double b = end->eval(parent).num;
    double s = step ? step->eval(parent).num : 1.0;
    if (s == 0.0) { evalError("step 0 en for (bucle infinito)"); return; }
    Scope loop(&parent);         // aloja la variable del for; se descarta al salir
    const double eps = 1e-9;
    for (double v = a; s > 0 ? v <= b + eps : v >= b - eps; v += s) {
      loop.vars[var] = Value(v);
      body->exec(loop, mg, out);
    }
  }
};

// Condicional (§6.1): if cond { … } [else { … }] | [else if …]. Los cuerpos son
// BlockStmt → ámbito léxico + estado gráfico como los bloques { } (§7.1). Su uso
// principal es la condición de paro de structs recursivas (§8.1).
struct IfStmt : Stmt {
  ExprPtr cond;
  StmtPtr thenB;                 // BlockStmt
  StmtPtr elseB;                 // BlockStmt, IfStmt (else if), o null
  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    if (cond->eval(s).num != 0.0) thenB->exec(s, mg, out);
    else if (elseB)               elseB->exec(s, mg, out);
  }
};

// --- Helpers para leer args nombrados (número, punto, escala) ----------------
static double namedNum(Scope &s, const std::map<std::string, ExprPtr> &m, const char *k, double def) {
  auto it = m.find(k);
  return it == m.end() ? def : it->second->eval(s).num;
}
static point namedPoint(Scope &s, const std::map<std::string, ExprPtr> &m, const char *k, double dx, double dy) {
  auto it = m.find(k);
  if (it == m.end()) return point(dx, dy);
  Value v = it->second->eval(s);
  if (v.type == Value::LIST && v.items.size() >= 2) return point(v.items[0].num, v.items[1].num);
  return point(dx, dy);
}
static point namedScale(Scope &s, const std::map<std::string, ExprPtr> &m, const char *k) {
  auto it = m.find(k);
  if (it == m.end()) return point(1, 1);
  Value v = it->second->eval(s);
  if (v.type == Value::LIST && v.items.size() >= 2) return point(v.items[0].num, v.items[1].num);
  return point(v.num, v.num);   // scale=s uniforme
}
static std::string namedStr(Scope &s, const std::map<std::string, ExprPtr> &m, const char *k) {
  auto it = m.find(k);
  if (it == m.end()) return "";
  Value v = it->second->eval(s);
  return v.type == Value::STRING ? v.str : "";
}

// Constructor de matriz para transform= (§17): rotate(a)/scale(a[,b])/translate(a,b)/
// shear(a,b), yuxtapuestos y compuestos en orden de escritura (post-multiplicación).
struct MatrixCtor { int op = OPMID; std::vector<ExprPtr> args; };
static Matrix buildTransform(Scope &s, const std::vector<MatrixCtor> &ctors) {
  Matrix m;   // identidad
  for (const auto &c : ctors) {
    double a0 = !c.args.empty() ? c.args[0]->eval(s).num : 0.0;
    double a1 = c.args.size() > 1 ? c.args[1]->eval(s).num : a0;   // 1 arg → uniforme
    switch (c.op) {
      case OPMTL: m.translate(a0, a1); break;
      case OPMRT: m.rotate(a0);        break;
      case OPMSC: m.scale(a0, a1);     break;
      case OPMSH: m.shear(a0, a1);     break;
    }
  }
  return m;
}

// Repetición de structs (§17): repeat(Struct, count=N, [scale=], [rotate=], [at=],
// [advance=], [transform=]). Instancia k = marco T(at+k·advance) · A^k · R_fijo · S_fijo,
// con A = transform acumulado (MTRS de V1). scale/rotate fijos = iguales en toda copia
// (MTST). Reutiliza OPMPUSH/OPMPOP como la invocación directa (§8).
struct RepeatStmt : Stmt {
  std::string structName;
  std::map<std::string, ExprPtr> named;   // count, scale, rotate, at, advance
  std::vector<MatrixCtor> xform;          // transform= (vacío = identidad)
  void exec(Scope &caller, MetaGrafica &mg, GraphicsItemList &out) override {
    checkKnownArgs("repeat", named, kRepeatArgs);
    if (logPlotBlocksStruct("repeat")) return;
    auto it = g_structs.find(structName);
    if (it == g_structs.end()) { evalError("struct no definida (repeat): ", structName); return; }
    StructDef *def = it->second.get();

    int   count = (int)namedNum(caller, named, "count", 1);
    point at    = namedPoint(caller, named, "at", 0, 0);
    point adv   = namedPoint(caller, named, "advance", 0, 0);
    double rot  = namedNum(caller, named, "rotate", 0);
    point scl   = namedScale(caller, named, "scale");
    Matrix A    = buildTransform(caller, xform);        // acumulado por paso

    // Ámbito de la struct con sus parámetros en el valor por defecto (repeat no
    // pasa args propios de la struct; hijo del GLOBAL, léxico como la invocación).
    Scope local(caller.root());
    for (size_t i = 0; i < def->params.size(); i++)
      local.vars[def->params[i]] = def->defaults[i] ? def->defaults[i]->eval(local) : Value(0);

    Matrix Ak;   // A^k, empieza en identidad
    for (int k = 0; k < count; k++) {
      // Orden verificado contra el oráculo (rpstest caso 3): el transform
      // ACUMULADO A^k es el más interno (transforma la geometría cruda primero),
      // luego el marco FIJO R·S (canónico §8), y por fuera la traslación.
      Matrix M;
      M.translate(at.x + k * adv.x, at.y + k * adv.y);   // T (posición + avance)
      M.rotate(rot);                                     // R fijo
      M.scale(scl.x, scl.y);                             // S fijo
      M *= Ak;                                           // A^k innermost → M = T·R·S·A^k
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      { auto t = std::make_unique<Transform>(); t->setOperation(OPMPUSH); t->setMatrix(M); out.push_back(std::move(t)); }
      execStructBody(def, local, mg, out);   // §18: cuenta la profundidad
      { auto t = std::make_unique<Transform>(); t->setOperation(OPMPOP); out.push_back(std::move(t)); }
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
      Ak = Ak * A;
    }
  }
};

// fit (§10.2): ajusta una struct a un rectángulo (NO repite). Mapea la caja de
// coordenadas del struct (su world_window local, o unidad) al rectángulo del
// bloque. stretch=true deforma al rectángulo exacto; default = escala uniforme
// (meet) centrada. El orden de las esquinas refleja (escala con signo, como
// Matrix::to_rectangle). Inline con OPMPUSH/OPMPOP, como repeat/invocación.
struct FitStmt : Stmt {
  std::string structName;
  PathExprPtr pathArg;                    // fit de un path (§9) en vez de struct
  ExprPtr stretchE;                       // stretch= (null = proporcional/meet)
  std::vector<ExprPtr> coords;            // 4 Terms = 2 esquinas del rectángulo
  // fit(Struct(args), …) (§10.2, decisión de plan_struct_params.md): args de la
  // invocación interior, vacíos si se escribió `fit(Struct, …)` sin paréntesis.
  std::vector<Arg> invokePos;
  std::map<std::string, Arg> invokeNamed;

  // Construye la matriz window→rectángulo (misma lógica para struct y path). El
  // orden de las esquinas refleja (escala con signo); stretch deforma, si no meet.
  static Matrix fitMatrix(const Box &w, double x1, double y1, double x2, double y2,
                          bool stretch) {
    double tdx = x2 - x1, tdy = y2 - y1;
    double wdx = (w.dx != 0.0) ? w.dx : 1.0, wdy = (w.dy != 0.0) ? w.dy : 1.0;
    Matrix M;
    if (stretch) {
      M.translate(x1, y1);
      M.scale(tdx / wdx, tdy / wdy);
      M.translate(-w.x, -w.y);
    } else {
      double sx = tdx / wdx, sy = tdy / wdy;
      double s = (std::fabs(sx) < std::fabs(sy)) ? std::fabs(sx) : std::fabs(sy);
      double ssx = (sx < 0 ? -s : s), ssy = (sy < 0 ? -s : s);
      double ox = x1 + (tdx - ssx * wdx) / 2, oy = y1 + (tdy - ssy * wdy) / 2;
      M.translate(ox, oy);
      M.scale(ssx, ssy);
      M.translate(-w.x, -w.y);
    }
    return M;
  }

  void exec(Scope &caller, MetaGrafica &mg, GraphicsItemList &out) override {
    if (coords.size() < 4) { evalError("fit requiere un rectángulo (2 puntos)"); return; }
    double x1 = coords[0]->eval(caller).num, y1 = coords[1]->eval(caller).num;
    double x2 = coords[2]->eval(caller).num, y2 = coords[3]->eval(caller).num;
    bool stretch = stretchE && stretchE->eval(caller).num != 0.0;

    // fit de un path (§9): usa el bbox REAL del path como caja fuente y hornea la
    // matriz en los puntos (Polyline), sin OPMPUSH/OPMPOP (nada que se fugue).
    if (pathArg) {
      Path path = pathArg->evalPath(caller);
      if (path.empty()) return;
      Box w = path_bbox(path);
      Matrix M = fitMatrix(w, x1, y1, x2, y2, stretch);
      auto p = std::make_unique<Polyline>(GI_POLYLINE);
      p->setPath(process_path(M, path));
      out.push_back(std::move(p));
      return;
    }

    if (logPlotBlocksStruct("fit")) return;   // solo la rama struct (fit-de-path arriba ya retornó)
    auto it = g_structs.find(structName);
    if (it == g_structs.end()) { evalError("struct no definida (fit): ", structName); return; }
    StructDef *def = it->second.get();
    // at=/rotate=/scale= de la invocación interior competirían con la matriz del
    // propio fit (decisión 5 de plan_struct_params.md): error, no se ignoran.
    if (invokeNamed.count("at") || invokeNamed.count("rotate") || invokeNamed.count("scale"))
      { evalError("at=/rotate=/scale= no van dentro de fit(Struct(...)): compiten con la matriz del fit"); return; }
    // Ámbito LOCAL (con params ligados) ANTES de structBox: el world_window del
    // cuerpo puede referirse a un parámetro (p.ej. `world_window 0 w -2 2`).
    Scope local(caller.root());
    bindStructParams(local, caller, def, invokePos, invokeNamed);
    Box w = structBox(local, def);
    Matrix M = fitMatrix(w, x1, y1, x2, y2, stretch);   // window → rectángulo (esquinas firmadas)
    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
    { auto t = std::make_unique<Transform>(); t->setOperation(OPMPUSH); t->setMatrix(M); out.push_back(std::move(t)); }
    execStructBody(def, local, mg, out);   // §18: cuenta la profundidad
    popTransforms(countTransforms(def->body), out);   // transforms locales del cuerpo (§11.1): sin esto se fugan al fit hermano siguiente
    { auto t = std::make_unique<Transform>(); t->setOperation(OPMPOP); out.push_back(std::move(t)); }
    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// Construye un objeto Structure (motor) desde un StructDef, para que place lo
// replique con StructureLine/Arc (mismo código que generó el oráculo V1). El
// cuerpo se ejecuta con params por defecto; si el struct tiene ventana local
// != unidad, se hornea la normalización ventana→unidad como OPMPUSH/OPMPOP.
static int g_structSeq = 0;
static Structure *buildStructure(StructDef *def, MetaGrafica &mg, Scope &caller) {
  Scope local(caller.root());
  for (size_t i = 0; i < def->params.size(); i++)
    local.vars[def->params[i]] = def->defaults[i] ? def->defaults[i]->eval(local) : Value(0);
  GraphicsItemList prlist;
  Box w = structBox(local, def);   // ámbito LOCAL: el world_window del cuerpo puede usar un parámetro
  bool needN = (w.x != 0.0 || w.y != 0.0 || w.dx != 1.0 || w.dy != 1.0);
  if (needN) {
    Matrix N; N.scale(1.0 / w.dx, 1.0 / w.dy); N.translate(-w.x, -w.y);   // ventana→unidad
    auto t = std::make_unique<Transform>(); t->setOperation(OPMPUSH); t->setMatrix(N);
    prlist.push_back(std::move(t));
  }
  execStructBody(def, local, mg, prlist);   // §18: cuenta la profundidad
  popTransforms(countTransforms(def->body), prlist);   // §11.1: cierra los transforms del cuerpo antes de la ventana→unidad
  if (needN) { auto t = std::make_unique<Transform>(); t->setOperation(OPMPOP); prlist.push_back(std::move(t)); }
  auto s = std::make_unique<Structure>();
  std::string nm = "__" + def->name + "_" + std::to_string(g_structSeq++);
  s->setName(nm, mg.structure_map);
  s->setGraphicsItems(std::move(prlist));
  Structure *ptr = s.get();
  mg.structure_map[nm] = std::move(s);
  return ptr;
}

// place (§10.1): repite una struct a lo largo de un locus, orientándola con la
// tangente. Locus inferido: 2 puntos = línea; args de arco (r/from/to) = arco;
// (3+ puntos / &path = path, diferido). Reusa StructureLine/StructureArc del
// motor. count reparte N instancias uniformes sobre el locus (línea/arco).
struct PlaceStmt : Stmt {
  std::string structName;
  std::map<std::string, ExprPtr> named;   // scale, shift, count, gap, both_sides, r, from, to
  std::vector<ExprPtr> coords;
  void exec(Scope &caller, MetaGrafica &mg, GraphicsItemList &out) override {
    checkKnownArgs("place", named, kPlaceArgs);
    if (logPlotBlocksStruct("place")) return;
    auto it = g_structs.find(structName);
    if (it == g_structs.end()) { evalError("struct no definida (place): ", structName); return; }
    Structure *s = buildStructure(it->second.get(), mg, caller);

    double scale = namedNum(caller, named, "scale", 1.0);
    double shift = namedNum(caller, named, "shift", 1.0);   // default: instancia en el 2º punto (§10.1)
    double gap   = namedNum(caller, named, "gap", 0.0);
    int    count = (int)namedNum(caller, named, "count", 1);
    bool   both  = named.count("both_sides") && named.at("both_sides")->eval(caller).num != 0.0;

    if (named.count("r")) {                                // locus = arco
      double r = namedNum(caller, named, "r", 1);
      double from = namedNum(caller, named, "from", 0), to = namedNum(caller, named, "to", 360);
      point c(coords.size() > 1 ? coords[0]->eval(caller).num : 0,
              coords.size() > 1 ? coords[1]->eval(caller).num : 0);
      auto sr = std::make_unique<StructureArc>();
      sr->setStructure(s); sr->setScale(scale, scale); sr->setRadius(r);
      sr->setAngles(from, to); sr->setPoint(c); sr->setShift(shift); sr->setBothSides(both);
      out.push_back(std::move(sr));
      return;
    }
    // locus = path (3+ puntos): coloca la struct en cada punto, orientada a la
    // tangente local (§10.1). Mismo StructurePath que count=, pero los puntos los
    // da el bloque directamente (no se interpolan). Precede al caso línea/count.
    if (coords.size() >= 6) {
      Path pth;
      for (size_t i = 0; i + 1 < coords.size(); i += 2)
        pth.push_back(point(coords[i]->eval(caller).num, coords[i + 1]->eval(caller).num));
      auto sp = std::make_unique<StructurePath>();
      sp->setStructure(s); sp->setScale(scale); sp->setPath(std::move(pth)); sp->setOrient(true);
      out.push_back(std::move(sp));
      return;
    }

    // locus = línea (2 puntos)
    if (coords.size() < 4) { evalError("place sobre línea requiere 2 puntos"); return; }
    point p1(coords[0]->eval(caller).num, coords[1]->eval(caller).num);
    point p2(coords[2]->eval(caller).num, coords[3]->eval(caller).num);

    // count>1: reparte count instancias UNIFORMES a lo largo del segmento (§10.1).
    // Cada instancia se posiciona directo del locus (sin acumular transformación,
    // a diferencia de repeat §17) → más barato. Reusa StructurePath, que coloca la
    // struct en cada punto del path orientándola a la tangente local.
    if (count > 1) {
      Path pth;
      for (int i = 0; i < count; i++) {
        double t = (double)i / (count - 1);              // extremos incluidos
        pth.push_back(point(p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y)));
      }
      auto sp = std::make_unique<StructurePath>();
      sp->setStructure(s); sp->setScale(scale); sp->setPath(std::move(pth)); sp->setOrient(true);
      out.push_back(std::move(sp));
      return;
    }

    auto sr = std::make_unique<StructureLine>();          // count≤1: marcador(es) + línea
    sr->setStructure(s); sr->setScale(scale, scale); sr->setPoints(p1, p2);
    sr->setShift(shift); sr->setGap(gap); sr->setBothSides(both);
    out.push_back(std::move(sr));
  }
};

// sine (§4.13): onda senoidal aproximada por bezier a lo largo de la base p1→p2,
// oscilando perpendicular. Puntos de control = los de V1 (bzsinepaths.mg) →
// coincide con el oráculo. Núcleo: phase=0 (jorobas &sinpi con signo alterno) +
// squared (bumps sin², &cos2pi invertido) + phase múltiplo de 90° (cuartos de
// ciclo). half_cycles fraccionario: diferido (plan_sine.md).
// La GEOMETRÍA vive aquí, compartida por la sentencia de dibujo (SineStmt) y la
// forma de expresión de path §9 (PathSine): devuelve el path de CONTROL bezier
// (3k+1 puntos), vacío si los parámetros no dan onda.
static Path sineBezierPath(point p1, point p2, double n, double A,
                           double phase, bool squared) {
  Path path;
  if (n <= 0) return path;
  double dx = p2.x - p1.x, dy = p2.y - p1.y, L = std::sqrt(dx * dx + dy * dy);
  if (L == 0.0) return path;
  double ux = dx / L, uy = dy / L, vx = -uy, vy = ux;   // base y perpendicular (+90°)
  double w = L / n;                                     // ancho de una joroba (unidades de base)
  int ni = (int)(n + 0.5);                              // nº entero de jorobas (fraccionario diferido)
  auto W = [&](double lx, double ly) {                 // local (base, perp) → mundo
    return point(p1.x + lx * ux + ly * vx, p1.y + lx * uy + ly * vy);
  };

  // phase≠0 (no squared): descomposición en CUARTOS de ciclo (§4.13, plan_sine.md).
  // Cada cuarto (90°) es una cúbica derivada de &sinpi2; la fase (múltiplo de 90°)
  // rota el cuarto de arranque. half_cycles=n → 2n cuartos. phase=90 = coseno
  // (arranca en el máximo). fig6-1 (WKB).
  if (phase != 0.0 && !squared) {
    // {C1x,C1y, C2x,C2y, Px,Py, startY} por cuarto (x∈[0,1] del cuarto; y = seno)
    static const double Q[4][7] = {
      {0.417,  0.6667, 0.693,  1.0,    1.0,  1.0,  0.0},   // Q0 sube 0→+1
      {0.307,  1.0,    0.583,  0.6667, 1.0,  0.0,  1.0},   // Q1 baja +1→0
      {0.417, -0.6667, 0.693, -1.0,    1.0, -1.0,  0.0},   // Q2 baja 0→−1
      {0.307, -1.0,    0.583, -0.6667, 1.0,  0.0, -1.0},   // Q3 sube −1→0
    };
    int startQ = (int)std::lround(phase / 90.0) % 4;
    if (startQ < 0) startQ += 4;
    double wq = L / (2.0 * ni);                          // ancho de un cuarto
    path.push_back(W(0.0, Q[startQ][6] * A));            // moveto en la y inicial del 1er cuarto
    for (int i = 0; i < 2 * ni; i++) {
      const double *q = Q[(startQ + i) % 4];
      double base = i * wq;
      path.push_back(W(base + q[0] * wq, q[1] * A));
      path.push_back(W(base + q[2] * wq, q[3] * A));
      path.push_back(W(base + q[4] * wq, q[5] * A));
    }
    return path;
  }

  path.push_back(W(0.0, 0.0));                          // moveto (arranque)
  for (int k = 0; k < ni; k++) {
    double x0 = k * w;
    if (squared) {                                      // bump sin²(πt): &cos2pi invertido (todas +)
      path.push_back(W(x0 + 0.1825 * w, 0)); path.push_back(W(x0 + 0.3175 * w, A)); path.push_back(W(x0 + 0.5 * w, A));
      path.push_back(W(x0 + 0.6825 * w, A)); path.push_back(W(x0 + 0.8175 * w, 0)); path.push_back(W(x0 + 1.0 * w, 0));
    } else {                                            // joroba &sinpi, signo alterno (pico = A)
      double cy = ((k % 2 == 0) ? 1.0 : -1.0) * A * 1.335;
      path.push_back(W(x0 + 0.41 * w, cy)); path.push_back(W(x0 + 0.59 * w, cy)); path.push_back(W(x0 + 1.0 * w, 0));
    }
  }
  return path;
}

// Evaluación común de los argumentos de sine (sentencia y expresión de path):
// half_cycles/amplitude/phase/squared + base de 2 puntos en el bloque.
static Path sinePathFromArgs(Scope &s, const std::map<std::string, ExprPtr> &named,
                             const std::vector<ExprPtr> &coords) {
  double n = namedNum(s, named, "half_cycles", 1);
  double A = namedNum(s, named, "amplitude", 1);
  double phase = namedNum(s, named, "phase", 0);
  bool squared = named.count("squared") && named.at("squared")->eval(s).num != 0.0;
  if (coords.size() < 4) { evalError("sine requiere una base (2 puntos)"); return Path(); }
  if (phase != 0.0 && squared)
    std::fprintf(stderr, "sine: phase!=0 con squared aún no implementado; se dibuja phase=0\n");
  point p1(coords[0]->eval(s).num, coords[1]->eval(s).num);
  point p2(coords[2]->eval(s).num, coords[3]->eval(s).num);
  return sineBezierPath(p1, p2, n, A, phase, squared);
}

// sine como SENTENCIA (§4.13): dibuja la onda — un Polyline(GI_BEZIER), sin
// clase de motor.
struct SineStmt : Stmt {
  std::map<std::string, ExprPtr> named;   // half_cycles, amplitude, phase, squared
  std::vector<ExprPtr> coords;            // 2 puntos = base
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    Path path = sinePathFromArgs(s, named, coords);
    if (path.empty()) return;
    auto p = std::make_unique<Polyline>(GI_BEZIER);
    p->setPath(path);
    out.push_back(std::move(p));
  }
};

// sine como EXPRESIÓN DE PATH (§9): la misma onda, como datos de control bezier
// para el álgebra (concat/reverse/…). Con phase=90/270 cada llamada es un medio
// ciclo de coseno que cae/sube entre extremos con pendiente cero — la pieza con
// la que se ensamblan funciones de onda con envolvente por tramo (fig16-9).
struct PathSine : PathExpr {
  std::map<std::string, ExprPtr> named;
  std::vector<ExprPtr> coords;
  Path evalPath(Scope &s) const override {
    return sinePathFromArgs(s, named, coords);
  }
};

// Marcador definido por una struct del usuario (§B): ejecuta el cuerpo del struct
// a una lista temporal y recoge los subtrayectos de sus polyline/polygon como
// geometría del marcador (caja del propio struct; ancla = su origen, sin recentrar
// —se ajusta editando las coords del struct). Solo geometría poligonal por ahora;
// arcos/texto/nested vienen después. false si el nombre no es una struct o no
// aporta geometría. (world_window del cuerpo va a localWindow, no muta mg.)
static bool markerShapeFromStruct(Scope &s, MetaGrafica &mg, const std::string &name,
                                  std::vector<Path> &subpaths, bool &fillable) {
  auto it = g_structs.find(name);
  if (it == g_structs.end()) return false;
  StructDef *def = it->second.get();
  Scope local(s.root());
  for (size_t i = 0; i < def->params.size(); i++)
    local.vars[def->params[i]] = def->defaults[i] ? def->defaults[i]->eval(local) : Value(0);
  GraphicsItemList tmp;
  execStructBody(def, local, mg, tmp);   // §18: cuenta la profundidad
  fillable = false;
  for (auto &gi : tmp) {
    GraphicsItemType t = gi->getType();
    if (t != GI_POLYLINE && t != GI_POLYGON) continue;   // bezier = puntos de control, no on-curve
    subpaths.push_back(static_cast<GraphicsItemWithPath *>(gi.get())->getPath());
    if (t == GI_POLYGON) fillable = true;                 // polygon = relleno; polyline = contorno
  }
  return !subpaths.empty();
}

struct PrimStmt : Stmt {
  std::string name;
  std::vector<ExprPtr> pos;               // args posicionales
  std::map<std::string, ExprPtr> named;   // args nombrados
  std::vector<ExprPtr> coords;            // coordenadas (Term), en pares x y
  std::vector<size_t> breaks;             // índices en coords donde ';' abre subtrayecto
  PathExprPtr pathArg;                    // polyline(<PathExpr>): un path del álgebra §9

  double namedOr(Scope &s, const char *k, double def) const {
    auto it = named.find(k);
    return it == named.end() ? def : it->second->eval(s).num;
  }
  double posOr(Scope &s, size_t i, double def) const {
    return i < pos.size() ? pos[i]->eval(s).num : def;
  }

  // Evalúa las coords [from, to) a un Path. Cada término es un ESCALAR (que se
  // empareja con el siguiente para formar un punto x-y) o un PUNTO ya hecho —una
  // lista de 2, como devuelve point_at(&p,t) o un literal [x,y]—, que aporta el par
  // entero de una vez. Así `marker { point_at(...) }`, `polyline { 0 0  p  5 5 }`
  // (mezcla) y `dot { point_at(...)  point_at(...) }` componen sin ceremonia.
  // La paridad se valida AQUÍ (no en parse-time): un escalar sin pareja al final es
  // el error, y un punto en variable solo se distingue de un número al evaluar.
  Path evalPath(Scope &s, size_t from, size_t to) const {
    Path path;
    bool pending = false;
    double px = 0;
    for (size_t i = from; i < to; i++) {
      Value v = coords[i]->eval(s);
      if (v.type == Value::LIST) {              // un punto ya hecho
        if (v.items.size() != 2)
          evalError("una coordenada-lista debe ser un punto [x,y] (2 valores), en ", name);
        if (pending)
          evalError("coordenada suelta sin pareja antes de un punto [x,y], en ", name);
        path.push_back(point(v.items[0].num, v.items[1].num));
      } else {                                  // un escalar: se empareja
        if (!pending) { px = v.num; pending = true; }
        else { path.push_back(point(px, v.num)); pending = false; }
      }
    }
    if (pending)
      evalError("número impar de coordenadas (una quedó sin pareja), en ", name);
    return path;
  }

  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    // Un atributo que la primitiva no conoce era un no-op MUDO (§7.5): el typo
    // parecía puesto y no hacía nada. Se valida antes de dibujar nada.
    for (const auto &kv : named)
      if (!isKnownPrimAttr(kv.first))
        evalError("atributo desconocido en la primitiva: ",
                  name + " — `" + kv.first + "=` no existe (¿mal escrito?)");
    // polyline/polygon/bezier admiten subtrayectos disjuntos separados por ';'
    // (§4): un item por subtrayecto, mismo estilo (distinto de compound, §9.4,
    // que combina en un solo relleno par-impar). El resto de primitivas ignora
    // los ';' (no aplican: rectangle=pares, dot=posiciones, circle/arc=centros,
    // polybar=centros superiores).
    std::vector<std::unique_ptr<GraphicsItem>> items;
    // Marcadores en polyline (§4.11): si se piden, guardamos una copia de cada
    // subtrayecto para dispersar Dot(s) sobre el mismo path tras dibujar la ruta.
    // (polygon/bezier: segunda pasada; bezier además tiene puntos de control.)
    bool wantMarkers = (name == "polyline") &&
        (named.count("marker") || named.count("marker_start") ||
         named.count("marker_mid") || named.count("marker_end"));
    std::vector<Path> markerPaths;
    GraphicsItemType poly = GI_POLYLINE;
    bool isPoly = true;
    if      (name == "polyline") poly = GI_POLYLINE;
    else if (name == "polygon")  poly = GI_POLYGON;
    else if (name == "bezier")   poly = GI_BEZIER;   // path = p0 c1 c2 p1 [c1 c2 p2…]; Polyline::draw agrupa de 3 en 3 → curveto
    else if (name == "smooth")   poly = GI_BEZIER;   // §9.2: mismo item que bezier, pero el bloque son NODOS y las tangentes las deriva el compilador
    else isPoly = false;
    // `smooth { nodos }` como primitiva de dibujo (§9.2), hermana de `bezier`.
    // La diferencia es de quién calcula las tangentes: en `bezier` el bloque son
    // puntos de CONTROL y el autor las pone (puede hacer picos, tiradores
    // asimétricos — es estrictamente más expresivo); en `smooth` el bloque son
    // NODOS y el compilador emite los controles, garantizando que la curva pase
    // por todos y empalme sin picos. Antes solo existía la forma de expresión de
    // path, así que dibujar exigía `bezier(smooth { … })`, que filtraba el
    // detalle de que smooth produce puntos de control.
    const bool doSmooth = (name == "smooth");

    // polyline/polygon/bezier(<PathExpr>): un path del álgebra §9 (§9) como único
    // subtrayecto. El resultado del álgebra es una secuencia plana de puntos.
    if (pathArg && isPoly) {
      bool closed = named.count("closed") && named.at("closed")->eval(s).num != 0.0;
      Path path = pathArg->evalPath(s);
      if (doSmooth) path = smoothPath(path);     // smooth(&p): suaviza los nodos de un path del álgebra §9
      if (!path.empty()) {
        if (wantMarkers) markerPaths.push_back(path);
        auto p = std::make_unique<Polyline>(poly);
        p->setPath(std::move(path));
        p->setClosed(closed);
        items.push_back(std::move(p));
      }
    } else if (isPoly) {
      // §4.1: closed=true cierra cada subtrayecto (closepath) sin repetir el
      // punto inicial. Default false = polilínea abierta. En polygon es redundante
      // (el relleno ya cierra); solo cambia algo si se pide explícitamente.
      bool closed = named.count("closed") && named.at("closed")->eval(s).num != 0.0;
      size_t start = 0;
      auto flush = [&](size_t end) {
        Path path = evalPath(s, start, end);
        start = end;
        if (doSmooth && !path.empty()) path = smoothPath(path);  // cada subtrayecto ';' se suaviza por separado
        if (path.empty()) return;
        if (wantMarkers) markerPaths.push_back(path);
        auto p = std::make_unique<Polyline>(poly);
        p->setPath(std::move(path));
        p->setClosed(closed);
        items.push_back(std::move(p));
      };
      for (size_t b : breaks) flush(b);
      flush(coords.size());
    } else {
      // Resto de primitivas: el path es la FUENTE DE PUNTOS, venga de un bloque de
      // coordenadas o de una expresión del álgebra §9 (que es justo eso, una
      // secuencia de puntos). Cada una los interpreta a su manera —centros para
      // circle/dot, esquinas para rectangle, topes de barra para polybar— y esa
      // lectura no depende de cómo se escribieron. Antes esta rama era un error
      // ("un path solo puede dibujarse con polyline/polygon/bezier"), lo que dejaba
      // fuera el caso natural de datos generados: `polybar(&hist, width=w)`.
      Path path = pathArg ? pathArg->evalPath(s) : evalPath(s, 0, coords.size());
      std::unique_ptr<GraphicsItem> item;
      if      (name == "rectangle") {
        auto p = std::make_unique<Rectangle>();
        // Forma alterna (§4.4): rectangle(w=, h=, at=) — CENTRO + tamaño, en vez
        // de las dos esquinas del bloque. `at` es el centro (como el de circle/dot);
        // omitirlo lo pone en el origen. Cómodo para colocar sin calcular esquinas.
        bool atForm = named.count("w") || named.count("h") || named.count("at");
        if (atForm) {
          if (!path.empty())
            evalError("rectangle: da las esquinas O (w,h,at), no ambos (§4.4)");
          if (!named.count("w") && !named.count("h"))
            evalError("rectangle con at= necesita un tamaño: w= y h= (§4.4)");
          double w = named.count("w") ? named.at("w")->eval(s).num
                                      : named.at("h")->eval(s).num;   // solo h → cuadrado
          double h = named.count("h") ? named.at("h")->eval(s).num : w;
          double cx = 0, cy = 0;
          if (named.count("at")) {
            Value a = named.at("at")->eval(s);
            if (a.type != Value::LIST || a.items.size() < 2)
              evalError("rectangle: at= debe ser un punto (x, y) (§4.4)");
            cx = a.items[0].num; cy = a.items[1].num;
          }
          Path r;
          r.push_back(point(cx - w / 2.0, cy - h / 2.0));
          r.push_back(point(cx + w / 2.0, cy + h / 2.0));
          p->setPath(std::move(r));
        } else {
          p->setPath(path);
        }
        item = std::move(p);
      }
      else if (name == "dot" || name == "marker") {
        // §4.6: `marker(r, shape=…)` es la primitiva; `dot(r)` es su atajo para el
        // caso común (el disco), y NO lleva shape= — un `dot` que dibuja un cuadrado
        // sería un nombre mentiroso. Misma clase (Dot) para las dos.
        auto p = std::make_unique<Dot>();
        p->setRadius(posOr(s, 0, namedOr(s, "size", 1)));
        // shape= (§4.6): la forma física. Acepta un builtin del catálogo
        // (circle/square/diamond/cross/x/arrow) O EL NOMBRE DE UN STRUCT del usuario,
        // igual que marker_start/mid/end de polyline (§B). No hay forma "disk": el
        // disco es `circle` RELLENO — la forma y el relleno son ejes independientes
        // (§4), y duplicar el catálogo por relleno sería redundante.
        if (named.count("marker"))
          evalError(name == "dot"
                    ? "dot no lleva forma; usa `marker(r, shape=…)` (§4.6)"
                    : "marker: `marker=` se renombró a `shape=` (§4.6)");
        if (name == "dot" && named.count("shape"))
          evalError("dot es el atajo del disco y no lleva `shape=`; usa `marker(r, shape=…)` (§4.6)");
        MarkerId mid = MK_CIRCLE;
        bool builtin = true;
        auto mit = named.find("shape");
        if (mit != named.end()) {
          std::string mname = mit->second->eval(s).str;
          builtin = markerIdForName(mname, mid);
          if (builtin) p->setMarker(mid);
          else {
            std::vector<Path> subs; bool fillable = false;
            if (!markerShapeFromStruct(s, mg, mname, subs, fillable))
              evalError("marcador desconocido (ni builtin ni struct): ", mname);
            p->setCustomShape(subs, fillable);
          }
        }
        // Orientación (§B.3): la flecha se orienta a la tangente local por default;
        // el resto queda fijo —incluido un struct, que no se sabe "flecha", igual
        // criterio que polyline. Sobreescribible con marker_orient="auto"|"fixed".
        bool orient = builtin && (mid == MK_ARROW);
        auto oit = named.find("marker_orient");
        if (oit != named.end()) {
          Value ov = oit->second->eval(s);
          if (ov.type == Value::NUMBER) { orient = false; p->setFixedAngle(ov.num); }  // ángulo fijo en grados
          else if (ov.str == "auto") orient = true;
          else if (ov.str == "fixed") orient = false;
          else evalError("marker_orient debe ser \"auto\", \"fixed\" o un ángulo en grados: ", ov.str);
        }
        p->setOrient(orient);
        p->setPath(path); item = std::move(p);
      }
      else if (name == "circle") { auto p = std::make_unique<Arc>(); double r = posOr(s, 0, 1); p->setRadius(r, r); p->setAngles(0, 360); p->setPath(path); item = std::move(p); }
      else if (name == "ellipse") { auto p = std::make_unique<Arc>(); double rx = posOr(s, 0, 1), ry = posOr(s, 1, rx); if (rx != ry) g_flags.using_ellipse = true; p->setRadius(rx, ry); p->setAngles(0, 360); p->setPath(path); item = std::move(p); }
      else if (name == "arc") { auto p = std::make_unique<Arc>(); double r = posOr(s, 0, 1); p->setRadius(r, r); p->setAngles(namedOr(s, "from", 0), namedOr(s, "to", 360)); p->setPath(path); item = std::move(p); }
      else if (name == "polybar") {
        // §4.12: cada coord es el centro superior de una barra; Polybar::draw la
        // expande a un rect() desde la base común 0. `width` va en unidades de la
        // ventana, es miembro aparte (no del path) → un eje log lo deja mal.
        auto p = std::make_unique<Polybar>();
        auto wit = named.find("width");
        if (wit == named.end() && pos.empty())
          evalError("polybar requiere `width` (polybar(w) o polybar(width=w))");
        p->setWidth(posOr(s, 0, namedOr(s, "width", 1)));
        auto dit = named.find("dir");
        if (dit != named.end()) {
          std::string dv = dit->second->eval(s).str;
          if (dv == "horizontal") p->setHorizontal(true);
          else if (dv != "vertical") evalError("dir debe ser \"vertical\" u \"horizontal\": ", dv);
        }
        p->setPath(path); item = std::move(p);
      }
      if (item) items.push_back(std::move(item));
    }
    if (items.empty()) return;

    // Atributos de estilo por-primitiva (§7.5): color/fill/line_width/hatch/dash,
    // acotados a esta primitiva (push/pop de estado alrededor).
    GraphicsItemList attrs;
    emitPrimStyle(named, s, attrs);
    // dot: relleno por defecto = disco (§4.6). `color=` SIN `fill=` = círculo
    // ABIERTO (trazo en color de línea): se apaga el relleno. `dot(r)` a secas =
    // disco negro; `fill=` ya activó el relleno en emitPrimStyle.
    if (name == "dot" || name == "marker") {
      if (named.count("color") && !named.count("fill")) {
        attrs.push_back(std::make_unique<GraphicsState>(GS_NOFILL));
      } else if (!named.count("fill")) {
        auto a = std::make_unique<Attribute>(); a->set(AT_FCOLOR, 0); attrs.push_back(std::move(a));   // negro
        attrs.push_back(std::make_unique<GraphicsState>(GS_FILL));
      }
    }
    if (attrs.empty()) {
      for (auto &it : items) out.push_back(std::move(it));
    } else {
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      for (auto &a : attrs) out.push_back(std::move(a));
      for (auto &it : items) out.push_back(std::move(it));
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
    }
    emitMarkers(s, mg, out, markerPaths);   // §4.11: marcadores sobre la ruta (encima)
    // §4.11 en arco: marcador en el extremo, con el punto y la tangente derivados
    // de los parámetros del arco (no del path, que son centros). Solo start/end.
    if (name == "arc") {
      double r = posOr(s, 0, 1);
      emitArcMarkers(s, mg, out, evalPath(s, 0, coords.size()), r, r,
                     namedOr(s, "from", 0), namedOr(s, "to", 360));
    }
  }

  // Marcadores (§4.11) sobre polyline: tras la ruta, dispersa Dot(s) en los
  // vértices de cada subtrayecto según el canal (start/mid/end, o `marker`=todos),
  // con el MISMO path (la tangente se calcula completa). Heredan estado ambiente y
  // relleno negro por default (como dot); su tamaño es FÍSICO (marker_size, def 4).
  void emitMarkers(Scope &s, MetaGrafica &mg, GraphicsItemList &out, const std::vector<Path> &paths) const {
    if (paths.empty()) return;
    // Orientación (§B.3), global + override por extremo (paridad con arc). Modo:
    // 0=default (por forma: arrow→auto, resto→fixed), 1=auto (tangente), 2=fixed
    // (+x), 3=reverse (tangente+180°), 4=ángulo absoluto en grados. marker_orient
    // fija el global; marker_start_orient/marker_end_orient lo sobreescriben por extremo.
    auto parseSpec = [&](const char *key, int &mode, double &angle) {
      auto it = named.find(key);
      if (it == named.end()) { mode = 0; return; }
      Value ov = it->second->eval(s);
      if (ov.type == Value::NUMBER)  { mode = 4; angle = ov.num; }
      else if (ov.str == "auto")     mode = 1;
      else if (ov.str == "fixed")    mode = 2;
      else if (ov.str == "reverse")  mode = 3;
      else evalError("marker_orient debe ser \"auto\", \"fixed\", \"reverse\" o un ángulo en grados: ", ov.str);
    };
    int gMode, sMode, eMode; double gAng = 0, sAng = 0, eAng = 0;
    parseSpec("marker_orient", gMode, gAng);
    parseSpec("marker_start_orient", sMode, sAng);
    parseSpec("marker_end_orient", eMode, eAng);
    double msize = namedOr(s, "marker_size", 4);
    // §B: desplazamiento del marcador de extremo SOBRE su segmento (el `shift` de
    // LNST). shift∈[0,1]: 1 (default) = en el vértice; 0 = en el vértice adyacente
    // interior; intermedio = lerp. Solo start/end (mid/all no tienen "segmento").
    bool hasStartShift = named.count("marker_start_shift") != 0;
    bool hasEndShift   = named.count("marker_end_shift") != 0;
    double startShift = namedOr(s, "marker_start_shift", 1.0);
    double endShift   = namedOr(s, "marker_end_shift", 1.0);
    GraphicsItemList mk;
    auto emit = [&](MarkerRange range, const Expr *e, bool useShift, double shift, int omode, double oang) {
      std::string nm = e->eval(s).str;
      MarkerId id;
      bool builtin = markerIdForName(nm, id);
      std::vector<Path> subs; bool fillable = false;
      if (!builtin && !markerShapeFromStruct(s, mg, nm, subs, fillable)) {
        evalError("marcador desconocido (ni builtin ni struct): ", nm); return;
      }
      // Resuelve el modo a orient/reverse/ángulo. Default por forma: arrow→auto,
      // resto (incl. struct, que no se sabe "flecha")→fixed (§B.3).
      bool orient, reverse = false, hasAngle = false; double fixedA = 0;
      switch (omode) {
        case 1:  orient = true;  break;                                 // auto (tangente)
        case 2:  orient = false; break;                                 // fixed (+x)
        case 3:  orient = true; reverse = true; break;                  // reverse (tangente+180°)
        case 4:  orient = false; hasAngle = true; fixedA = oang; break; // ángulo absoluto
        default: orient = builtin && id == MK_ARROW; break;             // por forma
      }
      for (const Path &p : paths) {
        if (useShift && p.size() >= 2) {
          // Marcador desplazado sobre el segmento del extremo: en vez de dispersar
          // en el vértice, un Dot de un punto en lerp(adj, ancla, shift) con el
          // ángulo del segmento (mismo patrón que los marcadores de arco).
          point anchor, adj;
          if (range == MR_LAST) { anchor = p[p.size()-1]; adj = p[p.size()-2]; }
          else                  { anchor = p[0];          adj = p[1]; }
          point P(adj.x + shift*(anchor.x-adj.x), adj.y + shift*(anchor.y-adj.y));
          // Tangente en el sentido de avance (v0→v1 al inicio, v[n-2]→v[n-1] al fin).
          point fwd = (range == MR_LAST) ? point(anchor.x-adj.x, anchor.y-adj.y)
                                         : point(adj.x-anchor.x, adj.y-anchor.y);
          double useAng;
          if (hasAngle)    useAng = fixedA;                                            // absoluto
          else if (orient) { useAng = std::atan2(fwd.y, fwd.x)*180.0/M_PI; if (reverse) useAng += 180.0; }
          else             useAng = reverse ? 180.0 : 0.0;                             // fixed: +x, o -x si reverse
          auto d = std::make_unique<Dot>();
          d->setPath({P});
          if (builtin) d->setMarker(id); else d->setCustomShape(subs, fillable);
          d->setRadius(msize); d->setFixedAngle(useAng);
          mk.push_back(std::move(d));
          continue;
        }
        auto d = std::make_unique<Dot>();
        d->setPath(p);          // copia: comparte la forma del subtrayecto
        if (builtin) d->setMarker(id);
        else         d->setCustomShape(subs, fillable);
        d->setRange(range); d->setOrient(orient); d->setRadius(msize);
        d->setReverse(reverse);
        if (hasAngle) d->setFixedAngle(fixedA);
        mk.push_back(std::move(d));
      }
    };
    bool hasSpecific = named.count("marker_start") || named.count("marker_mid") || named.count("marker_end");
    auto all = named.find("marker");
    if (all != named.end() && !hasSpecific) {
      emit(MR_ALL, all->second.get(), false, 1.0, gMode, gAng);   // atajo: un Dot(MR_ALL) por subtrayecto
    } else {
      const Expr *fallback = (all != named.end()) ? all->second.get() : nullptr;  // `marker` rellena ausentes
      // El modo por extremo cae al override si se dio (mode!=0), si no al global.
      auto chan = [&](const char *key, MarkerRange range, bool useShift, double shift, int cmode, double cang) {
        int m = (cmode != 0) ? cmode : gMode;
        double a = (cmode != 0) ? cang : gAng;
        auto it = named.find(key);
        if (it != named.end())  emit(range, it->second.get(), useShift, shift, m, a);
        else if (fallback)      emit(range, fallback, useShift, shift, m, a);
      };
      chan("marker_start", MR_FIRST, hasStartShift, startShift, sMode, sAng);
      chan("marker_mid",   MR_MID,   false,         1.0,        0,     0);   // mid: solo global
      chan("marker_end",   MR_LAST,  hasEndShift,   endShift,   eMode, eAng);
    }
    wrapMarkers(s, out, mk);
  }

  // Envuelve los Dots de marcador (mk) en su alcance de estado (§B.4): marker_color
  // fija trazo+relleno (un valor sirve para formas rellenas y para cruz/x);
  // marker_fill sobreescribe solo el relleno ("none" = abierto). Sin ninguno:
  // relleno negro por default (como dot); el trazo (cruz/x) hereda el ambiente.
  // Compartido por los marcadores de polyline (emitMarkers) y de arco (emitArcMarkers).
  void wrapMarkers(Scope &s, GraphicsItemList &out, GraphicsItemList &mk) const {
    if (mk.empty()) return;
    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
    bool haveFill = named.count("marker_fill") != 0;
    if (named.count("marker_color")) {
      int c = colorFromValue(named.at("marker_color")->eval(s));
      auto lc = std::make_unique<Attribute>(); lc->set(AT_LCOLOR, c); out.push_back(std::move(lc));
      auto fc = std::make_unique<Attribute>(); fc->set(AT_FCOLOR, c); out.push_back(std::move(fc));
    } else if (!haveFill) {
      auto fc = std::make_unique<Attribute>(); fc->set(AT_FCOLOR, 0); out.push_back(std::move(fc));   // negro por default
    }
    bool open = false;
    if (haveFill) {
      Value fv = named.at("marker_fill")->eval(s);
      if (fv.type == Value::STRING && fv.str == "none") open = true;   // abierto
      else { auto fc = std::make_unique<Attribute>(); fc->set(AT_FCOLOR, colorFromValue(fv)); out.push_back(std::move(fc)); }
    }
    out.push_back(std::make_unique<GraphicsState>(open ? GS_NOFILL : GS_FILL));
    for (auto &it : mk) out.push_back(std::move(it));
    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }

  // §4.11 en `arc`: marcador en el EXTREMO del arco (start/end), orientado a su
  // TANGENTE. A diferencia de polyline —donde el path SON los vértices— el path de
  // un arco son CENTROS; el punto y su tangente se calculan de (centro, rx, ry,
  // a0, a1). Solo start/end tienen sentido (un arco no tiene vértices interiores);
  // `marker` a secas = ambos extremos. `marker_orient` ajusta el sentido: ausente/
  // "auto" = tangente del barrido; "reverse" = tangente + 180° (flecha al revés);
  // "fixed" = +x; un número = ángulo absoluto en grados.
  void emitArcMarkers(Scope &s, MetaGrafica &mg, GraphicsItemList &out,
                      const Path &centers, double rx, double ry, double a0, double a1) const {
    const Expr *start = named.count("marker_start") ? named.at("marker_start").get()
                      : named.count("marker")       ? named.at("marker").get() : nullptr;
    const Expr *end   = named.count("marker_end")   ? named.at("marker_end").get()
                      : named.count("marker")       ? named.at("marker").get() : nullptr;
    if (!start && !end) return;
    double msize = namedOr(s, "marker_size", 4);
    double dir = (a1 >= a0) ? 1.0 : -1.0;             // sentido del barrido
    // Override de orientación (§B.3): "auto"=tangente, "reverse"=+180°, "fixed"=+x,
    // número=absoluto. Modos: 0=tangente, 1=reverse, 2=fixed, 3=absoluto. `marker_orient`
    // fija el global; `marker_start_orient`/`marker_end_orient` lo sobreescriben por
    // extremo (para invertir SOLO uno — la flecha curva bidireccional).
    auto parseOrient = [&](const char *key, int gMode, double gAbs, int &mode, double &abs) {
      auto it = named.find(key);
      if (it == named.end()) { mode = gMode; abs = gAbs; return; }
      Value ov = it->second->eval(s);
      mode = 0; abs = 0;
      if (ov.type == Value::NUMBER)  { mode = 3; abs = ov.num; }
      else if (ov.str == "auto")     mode = 0;
      else if (ov.str == "reverse")  mode = 1;
      else if (ov.str == "fixed")    mode = 2;
      else evalError("marker_orient debe ser \"auto\", \"reverse\", \"fixed\" o un ángulo en grados: ", ov.str);
    };
    int gMode, eMode, sMode; double gAbs, eAbs, sAbs;
    parseOrient("marker_orient", 0, 0, gMode, gAbs);          // global (default tangente)
    parseOrient("marker_start_orient", gMode, gAbs, sMode, sAbs);
    parseOrient("marker_end_orient",   gMode, gAbs, eMode, eAbs);
    GraphicsItemList mk;
    auto add = [&](const Expr *e, double angDeg, int mode, double abs) {
      std::string nm = e->eval(s).str;
      MarkerId id; bool builtin = markerIdForName(nm, id);
      std::vector<Path> subs; bool fillable = false;
      if (!builtin && !markerShapeFromStruct(s, mg, nm, subs, fillable)) {
        evalError("marcador desconocido (ni builtin ni struct): ", nm); return;
      }
      double th = angDeg * M_PI / 180.0;
      // Tangente = sentido · d/dθ (rx cosθ, ry sinθ) = sentido · (-rx sinθ, ry cosθ).
      double tang = std::atan2(dir * ry * std::cos(th), dir * (-rx) * std::sin(th)) * 180.0 / M_PI;
      double useAng = tang;
      if      (mode == 1) useAng = tang + 180.0;   // reverse
      else if (mode == 2) useAng = 0.0;            // fixed (+x)
      else if (mode == 3) useAng = abs;            // absoluto
      for (const point &c : centers) {
        auto d = std::make_unique<Dot>();
        d->setPath({ point(c.x + rx * std::cos(th), c.y + ry * std::sin(th)) });
        if (builtin) d->setMarker(id); else d->setCustomShape(subs, fillable);
        d->setRadius(msize); d->setFixedAngle(useAng);
        mk.push_back(std::move(d));
      }
    };
    if (start) add(start, a0, sMode, sAbs);
    if (end)   add(end, a1, eMode, eAbs);
    wrapMarkers(s, out, mk);
  }
};

// Texto (§4.8): text("s") { p1 p2 … } estampa la cadena en cada coordenada.
// El contenido pasa por parse_text (mismo markup que V1); una llamada por punto
// porque el GraphicsItem resultante es no-copiable (siembra múltiple, §4).
struct TextStmt : Stmt {
  ExprPtr content;
  std::map<std::string, ExprPtr> named;   // §7.5: estilo por-primitiva (font_size/size, font, align, valign, color)
  std::vector<ExprPtr> coords;            // coordenadas (Term), en pares x y

  // Atributos que text() reconoce. Lista PROPIA y no la de las primitivas
  // (isKnownPrimAttr): los ejes no se solapan —`align`/`valign`/`font` no tienen
  // sentido en un polyline, ni `hatch`/`closed`/`marker_*` en un text— y
  // compartirla dejaría pasar sinsentidos en las dos direcciones.
  static bool isKnownAttr(const std::string &k) {
    return k == "font_size" || k == "size" || k == "color" ||
           k == "align" || k == "valign" || k == "font";
  }

  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    // Mismo silencio que tenían las primitivas: `text("h", tamano=20)` o un
    // `colour=` compilaban sin hacer nada y sin avisar. (Una cara de fuente
    // inválida —`font="Helvetika"`— ya avisaba con warn y cae a Times, que es el
    // criterio no-fatal que ya usaban los colores desconocidos.)
    for (const auto &kv : named)
      if (!isKnownAttr(kv.first))
        evalError("atributo desconocido en text(): ",
                  "`" + kv.first + "=` no existe (¿mal escrito?)");
    Value c = content->eval(s);
    std::string str;
    if (c.type == Value::STRING) str = c.str;
    else { char buf[64]; std::snprintf(buf, sizeof buf, "%g", c.num); str = buf; }

    // Estilo por-primitiva (§7.5), acotado a este text() con push/pop de estado
    // (como PrimStmt); sin esto los parámetros nombrados se parsean pero se
    // ignoran. En el contexto de text(), `size` es alias de `font_size`. `color`
    // tiñe el texto (color de trazo, que es con el que `show` pinta).
    GraphicsItemList attrs;
    for (const char *k : {"font_size", "size", "color", "align", "valign"}) {
      auto it = named.find(k);
      if (it != named.end())
        emitStyleAttr(std::string(k) == "size" ? "font_size" : k, it->second->eval(s), attrs);
    }
    bool scoped = !attrs.empty();
    if (scoped) {
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      for (auto &a : attrs) out.push_back(std::move(a));
    }
    // `font=` (cara de fuente, §14.3): no va por el estado del Display sino como
    // cara INICIAL de parse_text (queda horneada en el Text; Text::draw la emite
    // salvo FN_NOFACE). Default FN_DEFAULT (Times) cuando no se da o es inválida,
    // idéntico al comportamiento previo. El markup interno del string ($…$, /i…)
    // sigue pudiendo cambiarla desde ahí.
    FontFace tface = FN_DEFAULT;
    auto fit = named.find("font");
    if (fit != named.end()) {
      Value fv = fit->second->eval(s);
      if (fv.type == Value::STRING) {
        FontFace f = get_font_face_from_string(fv.str, g_flags.using_fontcmmi);
        if (f != FN_NOFACE) tface = f;
      }
    }
    for (size_t i = 0; i + 1 < coords.size(); i += 2) {
      point p(coords[i]->eval(s).num, coords[i + 1]->eval(s).num);
      auto gs = std::make_unique<GraphicsState>();
      gs->setPosition(p);                                  // GS_PLUMEPOSITION
      out.push_back(std::move(gs));
      out.push_back(parse_text(str, tface, g_flags.using_reencode, g_flags.using_fontcmmi));
    }
    if (scoped)
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// numbers (§13.2): serie de etiquetas numéricas. El valor arranca en `from` y
// crece `by` por paso; `count` etiquetas con `decimals` decimales, envueltas por
// `prefix`/`suffix` (cadenas literales). La POSICIÓN arranca en `at` y avanza
// `advance` por paso (§13; forma compacta at/advance, sin pluma). Las etiquetas
// heredan el estado de texto vigente, como text(). V1: GNNUM i0 inc n decimals.
// Formatea un número para un rótulo y MATA el "-0" (y "-0.00"). Un cero con signo
// no es correcto en ningún eje, y sale en cuanto el paso es fraccionario: con
// `yaxis(step=0.5)` los rótulos eran «-1 -1 -0 0 1 1». Lo destapó una prueba en
// frío de la referencia (2026-07-23) — es lo primero que escribe quien quiere
// marcas cada medio.
static void formatLabel(char *out, size_t n, const char *fmt, double v) {
  std::snprintf(out, n, fmt, v);
  if (out[0] != '-') return;
  for (const char *p = out + 1; *p; ++p)
    if (*p != '0' && *p != '.') return;          // hay un dígito significativo
  std::memmove(out, out + 1, std::strlen(out));  // era -0, -0.0, -0.00…
}

// Decimales por default de un eje: los que hacen falta para que el PASO se
// distinga. Con step=0.5 y el default viejo (0), dos marcas vecinas se rotulaban
// IGUAL —«-1 -1 -0 0 1 1»: seis marcas, tres parejas repetidas—. Solo se aplica
// cuando el documento no dijo `decimals=`; un `decimals=` explícito manda siempre.
static int decimalsForStep(double step) {
  double a = std::fabs(step);
  if (!(a > 0)) return 0;
  double p = 1.0;
  for (int d = 0; d <= 6; ++d, p *= 10.0) {
    double escalado = a * p;
    if (std::fabs(escalado - (double)std::llround(escalado)) < 1e-9 * (escalado > 1 ? escalado : 1))
      return d;
  }
  return 6;
}

struct NumbersStmt : Stmt {
  std::map<std::string, ExprPtr> named;
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    checkKnownArgs("numbers", named, kNumbersArgs);
    double from = namedNum(s, named, "from", 0);
    double by   = namedNum(s, named, "by", 1);
    int    count = (int)namedNum(s, named, "count", 0);
    int    dec  = (int)namedNum(s, named, "decimals", 0);
    std::string prefix = namedStr(s, named, "prefix");
    std::string suffix = namedStr(s, named, "suffix");
    point at  = namedPoint(s, named, "at", 0, 0);
    point adv = namedPoint(s, named, "advance", 0, 0);
    char fmt[8];
    std::snprintf(fmt, sizeof fmt, "%%.%df", dec < 0 ? 0 : dec);
    for (int i = 0; i < count; i++) {
      char num[64];
      formatLabel(num, sizeof num, fmt, from + i * by);
      std::string label = prefix + num + suffix;
      auto gs = std::make_unique<GraphicsState>();
      gs->setPosition(point(at.x + i * adv.x, at.y + i * adv.y));
      out.push_back(std::move(gs));
      out.push_back(parse_text(label, FN_NOFACE, g_flags.using_reencode, g_flags.using_fontcmmi));  // hereda font ambiente
    }
  }
};

// ticks (§13.1): genera `count` marcas; cada una es un segmento mark=(dx,dy)
// dibujado en la posición de pluma, que avanza `advance` por paso (forma
// compacta at/advance, sin pluma). V1: TICKS n dx dy. Motor: Polyline(GI_TICKS)
// con path = [mark, pos0, pos1, …]; Polyline::draw traza pos→pos+mark por marca.
struct TicksStmt : Stmt {
  std::vector<ExprPtr> pos;                // count posicional (§13.1)
  std::map<std::string, ExprPtr> named;
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    checkKnownArgs("ticks", named, kTicksArgs);
    int count = pos.empty() ? (int)namedNum(s, named, "count", 0) : (int)pos[0]->eval(s).num;
    point mark = namedPoint(s, named, "mark", 0, 0);
    point at   = namedPoint(s, named, "at", 0, 0);
    point adv  = namedPoint(s, named, "advance", 0, 0);
    if (count <= 0) return;
    Path p;
    p.push_back(mark);                                // path[0] = vector de marca
    for (int i = 0; i < count; i++)
      p.push_back(point(at.x + i * adv.x, at.y + i * adv.y));
    auto pl = std::make_unique<Polyline>(GI_TICKS);
    pl->setPath(p);
    out.push_back(std::move(pl));
  }
};

// grid (§13.6): retícula sobre el rectángulo dado por el bloque (esquina
// inf-izq, esquina sup-der), con líneas verticales cada xstep y horizontales
// cada ystep (inclusivo en ambos extremos, incluye la línea del cero). El
// estilo (color/line_width) queda acotado a la retícula (push/pop). Cada haz
// es un Polyline(GI_TICKS) cuyo mark barre todo el alto/ancho del rectángulo
// (V1: TICKS con dx/dy = tamaño del campo).
struct GridStmt : Stmt {
  std::map<std::string, ExprPtr> named;
  std::vector<ExprPtr> coords;              // 2 puntos: (x0 y0) (x1 y1)
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    checkKnownArgs("grid", named, kGridArgs);
    if (coords.size() < 4) return;
    double x0 = coords[0]->eval(s).num, y0 = coords[1]->eval(s).num;
    double x1 = coords[2]->eval(s).num, y1 = coords[3]->eval(s).num;
    double xstep = namedNum(s, named, "xstep", 0);
    double ystep = namedNum(s, named, "ystep", 0);
    GraphicsItemList attrs;
    for (const char *k : {"color", "line_width"}) {
      auto it = named.find(k);
      if (it != named.end()) emitStyleAttr(k, it->second->eval(s), attrs);
    }
    bool wrap = !attrs.empty();
    if (wrap) {
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      for (auto &a : attrs) out.push_back(std::move(a));
    }
    const double eps = 1e-9;
    if (xstep > 0) {                        // verticales: mark = alto del campo
      Path p; p.push_back(point(0, y1 - y0));
      for (double x = x0; x <= x1 + eps; x += xstep) p.push_back(point(x, y0));
      auto pl = std::make_unique<Polyline>(GI_TICKS); pl->setPath(p); out.push_back(std::move(pl));
    }
    if (ystep > 0) {                        // horizontales: mark = ancho del campo
      Path p; p.push_back(point(x1 - x0, 0));
      for (double y = y0; y <= y1 + eps; y += ystep) p.push_back(point(x0, y));
      auto pl = std::make_unique<Polyline>(GI_TICKS); pl->setPath(p); out.push_back(std::move(pl));
    }
    if (wrap) out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// axis (§13.5): eje declarativo = línea + marcas + etiquetas + título. El bloque
// da los dos extremos físicos { p1 p2 }; los argumentos, el rango numérico y la
// decoración. pos(v) = p1 + (v-from)/(to-from)·(p2-p1). La orientación de marcas
// y etiquetas se deriva de la tangente de la línea; el LADO "out" por heurística:
// eje ~horizontal → abajo, ~vertical → izquierda (cubre X/Y). tick_size y
// label_gap son FÍSICOS (pt), inmunes a la ventana/stretch (como line_width,
// §3.2): se convierten a unidades de usuario con la escala device de mg.
// ticks: "out"|"in"|"both"|"none"|"grid" (grid barre el campo, §13.6-como-eje).
// Título vertical SIN rotar aún (2º corte). title_font (roman/italic) pendiente
// del estado font §7.3; title_size sí se aplica (AT_THEIGHT). V1: TICKS+GNNUM+PL.
//
// scale="log" (§13.5 "Propuesta para estudiar", plan_plot.md Fase 1): from/to
// siguen siendo VALORES de datos (p.ej. 1e-15/1e5), no exponentes — mismo
// contrato que el eje lineal. Las marcas MAYORES caen en cada década (o cada
// `step` décadas, step entero); las etiquetas son "10^n" vía markup matemático
// (§14), n=0 → "1" (estilo del libro). `minor=true` añade marcas 2·10ⁿ..9·10ⁿ
// sin rótulo. `strip_zero=true` (solo eje lineal) pule "0.30"→".30".

// Mapeo de nombre → código de alineación de texto (mismos códigos que las
// sentencias `align`/`valign`, parserv3.cpp §4.8): sirven para el override
// label_align=/label_valign= de axis (plan_plot.md Fase 1.5). -1 = no reconocido
// (deja el default derivado del lado).
static int alignCodeFromStr(const std::string &v) {
  if (v == "left")   return 0;
  if (v == "center") return 1;
  if (v == "right")  return 2;
  return -1;
}
static int valignCodeFromStr(const std::string &v) {
  if (v == "baseline") return 0;
  if (v == "top")      return 1;
  if (v == "middle")   return 2;
  if (v == "bottom")   return 3;
  return -1;
}

// Renombre a la nomenclatura de §13.0 (2026-07-16). Los nombres viejos FALLAN en
// compilación apuntando al nuevo, para que nadie los use por inercia ni queden
// aceptándose en silencio dos vocabularios.
//
// ⚠️ No se puede hacer lo mismo con `label_font`/`label_size`/`label_gap`: son válidos
// ANTES y DESPUÉS con DISTINTO significado (antes = rótulos de marca; ahora = nombre del
// eje). Un `label_gap=2` viejo no es un error, es un cambio de sentido silencioso — el
// parser no puede verlo. Ahí la red golden es la única guardia (§13.0, "trampa del
// renombre"): el renombre es puro, así que cualquier golden que se mueva delata un sitio
// mal portado.
static void checkRenamedAxisArgs(const std::map<std::string, ExprPtr> &named) {
  static const struct { const char *from, *to; } moved[] = {
    { "title",         "label"             },   // era el NOMBRE DEL EJE, no el encabezado
    { "title_font",    "label_font"        },
    { "title_size",    "label_size"        },
    { "title_gap",     "label_gap"         },
    { "labels",        "tick_labels"       },
    { "label_align",   "tick_label_align"  },
    { "label_valign",  "tick_label_valign" },
  };
  for (const auto &m : moved)
    if (named.count(m.from))
      evalError((std::string("axis: `") + m.from + "=` se renombró a `" + m.to +
                 "=` (§13.0 de la spec)").c_str());
}

struct AxisStmt : Stmt {
  std::map<std::string, ExprPtr> named;
  std::vector<ExprPtr> coords;              // 2 puntos: p1 p2
  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    // ORDEN IMPORTANTE: el de renombrados va PRIMERO. Los nombres viejos
    // (`title=`, `labels=`…) tampoco están en kAxisArgs, así que si la compuerta
    // genérica corre antes, se traga el mensaje que dice a qué se renombraron —
    // que es justo el que hace barata la migración. Lo cazó test/errors.
    checkRenamedAxisArgs(named);
    checkKnownArgs("axis", named, kAxisArgs);
    if (coords.size() < 4) return;
    point p1(coords[0]->eval(s).num, coords[1]->eval(s).num);
    point p2(coords[2]->eval(s).num, coords[3]->eval(s).num);
    double from = namedNum(s, named, "from", 0), to = namedNum(s, named, "to", 1);
    double step = namedNum(s, named, "step", 1);
    double startv = named.count("start") ? namedNum(s, named, "start", from) : from;
    std::string tdir = namedStr(s, named, "ticks"); if (tdir.empty()) tdir = "out";
    bool tickLabels = named.count("tick_labels") ? namedNum(s, named, "tick_labels", 1) != 0.0 : true;
    int decimals = named.count("decimals")
                 ? (int)namedNum(s, named, "decimals", 0)
                 : decimalsForStep(namedNum(s, named, "step", 1));
    // label = NOMBRE DEL EJE (§13.0; el `xlabel`/`ylabel` de matplotlib). NO es el
    // encabezado del plot: ese nombre (`title`) queda reservado para `plot`.
    std::string axisLabel = namedStr(s, named, "label");
    double labelSize = namedNum(s, named, "label_size", 0);
    double tickSize = namedNum(s, named, "tick_size", 3.0);   // pt
    // tick_label_gap: distancia FÍSICA (pt) del eje al BORDE cercano del rótulo de marca
    // (arriba para el eje X, derecha para el Y), tras la auto-alineación (Fase 1.5).
    // Default 4: libra las marcas de 3 pt de `ticks="out"` con ~1 pt de margen sin quedar lejos.
    double tickLabelGap = namedNum(s, named, "tick_label_gap", 4.0);   // pt

    // extend=: alarga SOLO la línea del eje más allá de sus extremos, en unidades
    // del eje (las mismas del bloque {p1 p2}), sin mover marcas/etiquetas. Escalar
    // = ambos extremos por igual; lista [lo hi] = lo antes de p1, hi después de p2.
    // Reproduce el "sobrante estilo libro" (eje que rebasa la primera/última marca,
    // p.ej. fig6-4: la vertical baja hasta el eje x). Default 0 → cero churn.
    double extLo = 0, extHi = 0;
    { auto it = named.find("extend");
      if (it != named.end()) {
        Value ev = it->second->eval(s);
        if (ev.type == Value::LIST && ev.items.size() >= 2) { extLo = ev.items[0].num; extHi = ev.items[1].num; }
        else extLo = extHi = ev.num;
      } }

    // scale="log": from/to en valores (no exponentes); step = décadas enteras
    // entre marcas mayores (default 1). minor/strip_zero: ver comentario arriba.
    bool isLog = namedStr(s, named, "scale") == "log";
    bool minorTicks = named.count("minor") ? namedNum(s, named, "minor", 0) != 0.0 : false;
    bool stripZero = named.count("strip_zero") ? namedNum(s, named, "strip_zero", 0) != 0.0 : false;
    int stepDec = 1;
    if (isLog) {
      if (from <= 0 || to <= 0) {
        evalError("axis(scale=\"log\") requiere from/to positivos"); return;
      }
      double r = std::round(step);
      if (std::fabs(step - r) > 1e-9 || r <= 0) {
        evalError("axis(scale=\"log\"): step debe ser un entero positivo (décadas)"); return;
      }
      stepDec = (int)r;
    }

    double dx = p2.x - p1.x, dy = p2.y - p1.y, L = std::sqrt(dx * dx + dy * dy);
    if (L <= 0 || step == 0) return;
    double ux = dx / L, uy = dy / L;
    bool horiz = std::fabs(ux) >= std::fabs(uy);
    double px = uy, py = -ux;                 // una perpendicular; ajusta el lado "out"
    if (horiz) { if (py > 0) { px = -px; py = -py; } }   // abajo (y<0)
    else       { if (px > 0) { px = -px; py = -py; } }   // izquierda (x<0)

    // Escala física: cm por unidad de usuario en cada eje (meet o stretch).
    double dcx, dcy; mg.getDimension(dcx, dcy);
    double wx, wy, wdx, wdy; mg.getWindow(wx, wy, wdx, wdy);
    double scx = dcx / wdx, scy = dcy / wdy;
    if (!mg.getStretch()) { double m = scx < scy ? scx : scy; scx = scy = m; }
    const double PT_PER_CM = 72.0 / 2.54;
    auto physOut = [&](double pt) {           // offset físico (pt) en dirección out → usuario
      double cm = pt / PT_PER_CM;
      return point(px * (cm / scx), py * (cm / scy));
    };
    auto posOf = [&](double v) {
      double t;
      if (isLog) {
        double lf = std::log10(from), lt = std::log10(to);
        t = (lt == lf) ? 0 : (std::log10(v) - lf) / (lt - lf);
      } else {
        t = (to == from) ? 0 : (v - from) / (to - from);
      }
      return point(p1.x + t * dx, p1.y + t * dy);
    };
    const double eps = 1e-9;

    // Valores de las marcas MAYORES, precomputados UNA vez y reusados por
    // marcas/etiquetas/grid (mismo cómputo que antes en el caso lineal ⇒ cero
    // churn: los `v` resultantes son bit-idénticos a los tres loops previos).
    std::vector<double> majors;
    if (isLog) {
      double lTo = std::log10(to), lStart = std::log10(startv);
      bool asc = to >= from;
      int n = asc ? (int)std::ceil(lStart - eps) : (int)std::floor(lStart + eps);
      int dn = asc ? stepDec : -stepDec;
      for (; asc ? (n <= lTo + eps) : (n >= lTo - eps); n += dn)
        majors.push_back(std::pow(10.0, n));
      if (majors.empty())
        warn("axis(scale=\"log\"): el rango no contiene ninguna década completa, sin marcas mayores");
    } else {
      for (double v = startv; v <= to + eps; v += step) majors.push_back(v);
    }

    // Estilo por-eje (line_width/color): acotado con push/pop alrededor de TODO el
    // eje (línea + marcas + etiquetas + título), como GridStmt. Desacopla el ancho
    // del eje del line_width del contenido — necesario dentro de plot, donde los
    // ejes se dibujan FUERA del envoltorio de contenido (plan_plot.md Fase 4). Sin
    // estos args, estado ambiente como antes → cero churn en ejes sueltos.
    GraphicsItemList styleAttrs;
    for (const char *k : {"color", "line_width", "dash"}) {
      auto it = named.find(k);
      if (it != named.end()) emitStyleAttr(k, it->second->eval(s), styleAttrs);
    }
    bool styleWrap = !styleAttrs.empty();
    if (styleWrap) {
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      for (auto &a : styleAttrs) out.push_back(std::move(a));
    }

    // 1. línea del eje (con el sobrante opcional extend=, a lo largo de la tangente)
    { point a1(p1.x - extLo * ux, p1.y - extLo * uy);
      point a2(p2.x + extHi * ux, p2.y + extHi * uy);
      auto pl = std::make_unique<Polyline>(GI_POLYLINE); Path pp;
      pp.push_back(a1); pp.push_back(a2); pl->setPath(pp); out.push_back(std::move(pl)); }

    // 2. marcas
    if (tdir == "grid") {                     // barren el campo perpendicular (ventana o field=)
      double field = named.count("field") ? namedNum(s, named, "field", 0) : (horiz ? wdy : wdx);
      // La retícula arranca en la LÍNEA del eje (coord perpendicular de p1), no en el
      // origen de ventana: así barre el campo desde el eje (dentro de plot, desde el
      // borde de la caja). Coinciden cuando el eje está en el origen (cero churn en
      // ejes sueltos; ningún ejemplo usa ticks="grid").
      double base  = horiz ? p1.y : p1.x;
      Path pp; pp.push_back(horiz ? point(0, field) : point(field, 0));
      for (double v : majors) {
        point q = posOf(v); pp.push_back(horiz ? point(q.x, base) : point(base, q.y));
      }
      auto pl = std::make_unique<Polyline>(GI_TICKS); pl->setPath(pp); out.push_back(std::move(pl));
    } else if (tdir != "none") {
      point o = physOut(tickSize), mark = o, off(0, 0);
      if (tdir == "in")        mark = point(-o.x, -o.y);
      else if (tdir == "both") { off = point(-o.x, -o.y); mark = point(2 * o.x, 2 * o.y); }
      Path pp; pp.push_back(mark);
      for (double v : majors) {
        point q = posOf(v); pp.push_back(point(q.x + off.x, q.y + off.y));
      }
      auto pl = std::make_unique<Polyline>(GI_TICKS); pl->setPath(pp); out.push_back(std::move(pl));
    }

    // 2b. marcas MENORES (solo log; sin rótulo, mitad de tamaño; §13.5 "minor=").
    // Barren TODAS las décadas del rango (2·10ⁿ..9·10ⁿ), no solo entre marcas
    // mayores (stepDec puede saltarse décadas). En "grid" no barren el campo:
    // siguen siendo marcas cortas sobre la línea, como en "out"/"in"/"both".
    if (isLog && minorTicks && tdir != "none") {
      point o = physOut(tickSize * 0.5), mark = o, off(0, 0);
      if (tdir == "in")        mark = point(-o.x, -o.y);
      else if (tdir == "both") { off = point(-o.x, -o.y); mark = point(2 * o.x, 2 * o.y); }
      Path pp; pp.push_back(mark);
      double lo = std::min(from, to), hi = std::max(from, to);
      int nStart = (int)std::floor(std::log10(lo) - eps);
      int nEnd   = (int)std::ceil(std::log10(hi) + eps);
      for (int n = nStart; n <= nEnd; n++) {
        for (int m = 2; m <= 9; m++) {
          double v = m * std::pow(10.0, n);
          if (v < lo - eps || v > hi + eps) continue;
          point q = posOf(v); pp.push_back(point(q.x + off.x, q.y + off.y));
        }
      }
      if (pp.size() > 1) {
        auto pl = std::make_unique<Polyline>(GI_TICKS); pl->setPath(pp); out.push_back(std::move(pl));
      }
    }

    // 3. etiquetas numéricas (heredan el estado de texto vigente, §14)
    if (tickLabels) {
      // Alineación de los rótulos de marca, derivada del lado del eje, para que NO se
      // encimen con la línea (plan_plot.md Fase 1.5): eje ~horizontal → centrados
      // y colgando bajo la marca (center/top); eje ~vertical → pegados por la
      // derecha y centrados en vertical (right/middle). Override explícito con
      // tick_label_align=/tick_label_valign= (nombres = sentencias align/valign, §4.8).
      int alignCode  = horiz ? 1 : 2;    // 1=center, 2=right
      int valignCode = horiz ? 1 : 2;    // 1=top,    2=middle
      if (named.count("tick_label_align")) {
        int c = alignCodeFromStr(namedStr(s, named, "tick_label_align")); if (c >= 0) alignCode = c;
      }
      if (named.count("tick_label_valign")) {
        int c = valignCodeFromStr(namedStr(s, named, "tick_label_valign")); if (c >= 0) valignCode = c;
      }
      // tick_label_font/tick_label_size: cara y tamaño de los RÓTULOS DE MARCA, hermanos
      // de label_font/label_size (que son los del NOMBRE DEL EJE, §13.0). Sin
      // tick_label_font, FN_NOFACE = hereda la cara ambiente. tick_label_size se acota con
      // el push/pop de alineación de abajo (AT_THEIGHT), como label_size.
      std::string tickLabelFont = namedStr(s, named, "tick_label_font");
      FontFace lff = tickLabelFont.empty() ? FN_NOFACE
                     : get_font_face_from_string(tickLabelFont, g_flags.using_fontcmmi);
      double tickLabelSize = namedNum(s, named, "tick_label_size", 0);
      // El prólogo EPS /cshow /rshow solo se emite si esta bandera se activó en
      // parse-time — exec() corre ANTES de EPSDisplay::start(), que la lee. Sin
      // ella el EPS llamaría al operador cshow REAL de PS y truena en typecheck
      // (misma clase de trampa que el proc /ellipse). Solo aplica a center/right.
      if (alignCode != 0) g_flags.using_textalign = true;

      // Estado de alineación acotado con push/pop, igual que el nombre del eje aísla
      // label_size (AT_THEIGHT): no se filtra a los text() que sigan al eje.
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      { auto a = std::make_unique<Attribute>(); a->set(AT_TALIGN, alignCode);  out.push_back(std::move(a)); }
      { auto a = std::make_unique<Attribute>(); a->set(AT_TVALIGN, valignCode); out.push_back(std::move(a)); }
      if (tickLabelSize > 0) { auto a = std::make_unique<Attribute>(); a->set(AT_THEIGHT, (int)tickLabelSize); out.push_back(std::move(a)); }

      point g = physOut(tickLabelGap);
      char fmt[8]; std::snprintf(fmt, sizeof fmt, "%%.%df", decimals < 0 ? 0 : decimals);
      for (double v : majors) {
        point q = posOf(v); char num[64];
        if (isLog) {
          int n = (int)std::llround(std::log10(v));
          if (n == 0) std::snprintf(num, sizeof num, "1");
          else        std::snprintf(num, sizeof num, "$10{^%d}$", n);
        } else {
          formatLabel(num, sizeof num, fmt, v);
          if (stripZero) {
            size_t len = std::strlen(num);
            if (len >= 2 && num[0] == '0' && num[1] == '.')
              std::memmove(num, num + 1, len);
            else if (len >= 3 && num[0] == '-' && num[1] == '0' && num[2] == '.')
              std::memmove(num + 1, num + 2, len - 1);
          }
        }
        auto gs = std::make_unique<GraphicsState>(); gs->setPosition(point(q.x + g.x, q.y + g.y));
        out.push_back(std::move(gs));
        out.push_back(parse_text(num, lff, g_flags.using_reencode, g_flags.using_fontcmmi));  // label_font, o FN_NOFACE=ambiente
      }
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
    }

    // 4. NOMBRE DEL EJE (§13.0: `label`, el xlabel/ylabel de matplotlib — no el
    //    encabezado del plot). Eje vertical → texto girado con el ángulo de la línea
    //    (MTLC), acotado con save/restore del dispositivo para cerrar el grupo de rotación.
    if (!axisLabel.empty()) {
      // label_at (§13.0): posición del nombre A LO LARGO del eje. Elige entre las dos
      // convenciones reales, ambas en el corpus: "center" (default) = centrado, lo de
      // matplotlib/asymptote (fig2-3); "start"/"end" = al extremo, estilo de libro
      // (los rótulos `x` de fig4-5). La alineación se deriva: al extremo el texto
      // ARRANCA en p2 (end, align=left) o TERMINA en p1 (start, align=right), para que
      // crezca alejándose del eje en vez de encimarse con él.
      std::string labelAt = namedStr(s, named, "label_at");
      if (labelAt.empty()) labelAt = "center";
      double lfrac = 0.5; int lalign = 1;                          // center
      if (labelAt == "start")    { lfrac = 0.0; lalign = 2; }      // align=right
      else if (labelAt == "end") { lfrac = 1.0; lalign = 0; }      // align=left
      else if (labelAt != "center")
        evalError("axis: label_at debe ser \"center\", \"start\" o \"end\": ", labelAt);

      // label_gap: distancia FÍSICA (pt) del eje a la LÍNEA BASE de su nombre,
      // DESACOPLADA de tick_label_gap. El default depende de label_at porque **lo que
      // estorba depende de dónde lo pongas**: centrado tiene que librar la fila de
      // rótulos de marca (tick_label_gap + 20 ≈ 1 alto de fuente + aire); al extremo NO
      // —va más allá de ellos a lo largo del eje— y basta el hueco pequeño. Override propio.
      double labelGap = namedNum(s, named, "label_gap",
                                 labelAt == "center" ? tickLabelGap + 20.0 : tickLabelGap);
      point m(p1.x + lfrac * dx, p1.y + lfrac * dy);
      point t = physOut(labelGap);
      point tp(m.x + t.x, m.y + t.y);
      double ang = horiz ? 0.0 : std::atan2(uy, ux) * 57.29577951308232;   // rad→grados
      std::string labelFont = namedStr(s, named, "label_font");
      // Cara del nombre: label_font la hornea en el Text (override propio, §13.5);
      // sin él, FN_NOFACE → hereda la ambiente. label_size se aísla con push/pop.
      FontFace tff = labelFont.empty() ? FN_NOFACE
                     : get_font_face_from_string(labelFont, g_flags.using_fontcmmi);
      // El anclaje opera sobre la base ROTADA en el eje vertical (cshow/text-anchor),
      // así que center/start/end miden a lo largo de la línea en ambas orientaciones.
      // El estado se acota con push/pop (no se filtra). using_textalign activa el
      // prólogo /cshow de EPS (misma trampa que los rótulos de marca, §Fase 1.5);
      // seguro en parse-time, y se deja incondicional para no mover goldens.
      g_flags.using_textalign = true;
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      { auto a = std::make_unique<Attribute>(); a->set(AT_TALIGN, lalign); out.push_back(std::move(a)); }
      if (labelSize > 0) { auto a = std::make_unique<Attribute>(); a->set(AT_THEIGHT, (int)labelSize); out.push_back(std::move(a)); }
      if (ang != 0.0) out.push_back(std::make_unique<GraphicsState>(GS_DEVSAVE));
      { auto gs = std::make_unique<GraphicsState>(); gs->setPosition(tp); out.push_back(std::move(gs)); }
      if (ang != 0.0) {
        auto tr = std::make_unique<Transform>();
        tr->setOperation(OPMRT); tr->setRotation(ang); tr->setPredefinedMatrix(MTLC);
        out.push_back(std::move(tr));
      }
      out.push_back(parse_text(axisLabel, tff, g_flags.using_reencode, g_flags.using_fontcmmi));
      if (ang != 0.0) out.push_back(std::make_unique<GraphicsState>(GS_DEVRESTORE));
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
    }

    if (styleWrap) out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));   // cierra el estilo por-eje
  }
};

// compound (§9.4): une varias primitivas de trazo en UN path (V1 OPPT…CLPT). Se
// abre el path (GS_OPENPATH: las sub-primitivas acumulan sin trazar por separado),
// se ejecutan las sub-primitivas, y al cerrar (GS_CLOSEPATH) el path acumulado se
// rellena/traza con el estilo del compound (color/fill/hatch). fill_rule="even-odd"
// → pendiente (default non-zero; ningún ejemplo lo usa aún).
struct CompoundStmt : Stmt {
  std::map<std::string, ExprPtr> named;
  std::vector<StmtPtr> body;               // sub-primitivas (polyline/arc/…)
  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    GraphicsItemList attrs;
    emitPrimStyle(named, s, attrs);
    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
    for (auto &a : attrs) out.push_back(std::move(a));
    out.push_back(std::make_unique<GraphicsState>(GS_OPENPATH));
    for (auto &st : body) st->exec(s, mg, out);
    out.push_back(std::make_unique<GraphicsState>(GS_CLOSEPATH));   // cierra → rellena/traza
    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// rule (§13.8, el VALOR NOTABLE): hijo del `plot`, coords EXTERIORES como
// xaxis/yaxis/legend. Es la otra mitad de "las marcas de un eje son DOS conceptos":
// la malla regular se LEE (sale del rango, uniforme, rotulada con su número), el
// valor notable SEÑALA UN HECHO (sale de la física, arbitrario, con nombre propio).
//
// Es hijo del PLOT y no del AXIS porque el eje no sabe dónde está la curva —así que
// no podría dar la extensión `to=`— y porque la leyenda es del plot. El costo
// aceptado de esa elección: el rótulo no hereda gratis el acomodo del eje, así que
// PlotStmt le pasa dónde pondría el eje ese rótulo (labelAnchor/labelGap). Es la
// misma cuenta que el eje ya hace para sus marcas: acoplamiento, no maquinaria nueva.
//
// Toda la geometría llega YA MAPEADA desde PlotStmt (que es quien tiene el mapper
// datos→caja): con eso, la escala log sale gratis, igual que le salió a `base=`.
struct RuleStmt : Stmt {
  std::map<std::string, ExprPtr> named;

  // Resuelto por PlotStmt antes de exec(), como la caja de legend (§13.9).
  bool   vertical = true;      // x=v → línea vertical (barre y); y=v → horizontal
  double at = 0;               // posición ya mapeada, sobre el eje propio
  double lo = 0, hi = 1;       // extensión ya mapeada, sobre el eje perpendicular
  double labelAnchor = 0;      // coord perpendicular de la LÍNEA DEL EJE (respeta base=)
  double labelGap = 4.0;       // tick_label_gap del eje al que pertenece (pt)

  bool toLegend(Scope &s) const {
    std::string la = namedStr(s, named, "label_at");
    return la == "legend";
  }
  std::string labelOf(Scope &s) const { return namedStr(s, named, "label"); }

  // Estilo propio (§13.8: "rojo punteado"). Mismo mecanismo que el estilo por-eje de
  // AxisStmt, más `dash` — que es justo lo que distingue visualmente a un notable de
  // la malla. Lo comparte la muestra de la leyenda, para que muestra y línea no
  // puedan divergir (el defecto que la leyenda explícita sí tiene).
  void emitStyle(Scope &s, GraphicsItemList &attrs) const {
    for (const char *k : {"color", "line_width", "dash"}) {
      auto it = named.find(k);
      if (it != named.end()) emitStyleAttr(k, it->second->eval(s), attrs);
    }
  }

  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    checkKnownArgs("rule", named, kRuleArgs);
    GraphicsItemList attrs;
    emitStyle(s, attrs);
    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
    for (auto &a : attrs) out.push_back(std::move(a));

    { auto pl = std::make_unique<Polyline>(GI_POLYLINE); Path pp;
      pp.push_back(vertical ? point(at, lo) : point(lo, at));
      pp.push_back(vertical ? point(at, hi) : point(hi, at));
      pl->setPath(pp); out.push_back(std::move(pl)); }

    // label_at="axis" (default): el nombre va DONDE IRÍA EL RÓTULO DE MARCA de ese
    // valor — misma perpendicular, mismo tick_label_gap y misma auto-alineación por
    // lado que AxisStmt (center/top en el horizontal, right/middle en el vertical).
    // Con label_at="legend" no se emite aquí: lo recoge la leyenda (§13.9).
    std::string label = labelOf(s);
    if (!label.empty() && !toLegend(s)) {
      double dcx, dcy; mg.getDimension(dcx, dcy);
      double wx, wy, wdx, wdy; mg.getWindow(wx, wy, wdx, wdy);
      double scx = dcx / wdx, scy = dcy / wdy;
      if (!mg.getStretch()) { double m = scx < scy ? scx : scy; scx = scy = m; }
      const double PT_PER_CM = 72.0 / 2.54;
      double gapW = (labelGap / PT_PER_CM) / (vertical ? scy : scx);
      int alignCode  = vertical ? 1 : 2;    // 1=center, 2=right
      int valignCode = vertical ? 1 : 2;    // 1=top,    2=middle
      g_flags.using_textalign = true;       // prólogo /cshow de EPS (misma trampa que el eje)
      point tp = vertical ? point(at, labelAnchor - gapW)
                          : point(labelAnchor - gapW, at);
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      { auto a = std::make_unique<Attribute>(); a->set(AT_TALIGN, alignCode);   out.push_back(std::move(a)); }
      { auto a = std::make_unique<Attribute>(); a->set(AT_TVALIGN, valignCode); out.push_back(std::move(a)); }
      double sz = namedNum(s, named, "label_size", 0);
      if (sz > 0) { auto a = std::make_unique<Attribute>(); a->set(AT_THEIGHT, (int)sz); out.push_back(std::move(a)); }
      { auto gs = std::make_unique<GraphicsState>(); gs->setPosition(tp); out.push_back(std::move(gs)); }
      out.push_back(parse_text(label, FN_NOFACE, g_flags.using_reencode, g_flags.using_fontcmmi));
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
    }

    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// legend (§13.9, forma EXPLÍCITA de series): hijo de `plot`, coords EXTERIORES
// (física, la caja) — nunca mapeada por datos, así que su tamaño no se deforma
// con plot log ni con el stretch (mismo trato que xaxis/yaxis). Cada `entry()`
// declara su propia muestra: un bloque arbitrario en caja unitaria 0..1, ajustado
// con el fitMatrix MEET-centrado de `fit`/colocación de struct (stretch=false) —
// preserva forma, así un marker circular no sale elipse.
//
// at= ancla una ESQUINA de la caja del plot con margin= (pt) de inset. El lado
// ("left"/"right") fija el borde de la COLUMNA DE MUESTRAS, no del texto: el
// compilador no puede medir el ancho de una cadena en parse-time (ningún backend
// lo expone ahí — EPS usa `stringwidth`, PDF `HPDF_Page_TextWidth`, SVG
// `text-anchor`, los tres en DRAW-TIME). "-left" arranca la muestra en el margen
// y el texto CRECE a la derecha (align=left, natural); "-right" TERMINA la
// muestra en el margen y el texto CRECE a la izquierda desde ahí (align=right,
// nativo del backend) — cero necesidad de medir texto en ningún caso.
struct LegendEntry { ExprPtr label; std::vector<StmtPtr> sample; };
struct LegendStmt : Stmt {
  std::map<std::string, ExprPtr> named;
  std::vector<LegendEntry> entries;
  // Entradas AUTOMÁTICAS (§13.9 fuente 1): los `rule` con label_at="legend" del plot
  // que contiene esta leyenda. PlotStmt los recoge y los apunta aquí antes de exec()
  // (no los posee). Aquí la autocolección SÍ es limpia —correspondencia 1:1 entre
  // valor notable y entrada, y la muestra ES su propia línea—, a diferencia de las
  // series, donde una entrada puede ser varias primitivas (las 3 pasadas de un
  // polybar) y por eso hay que declararla a mano.
  std::vector<RuleStmt *> autoRules;
  double boxX0 = 0, boxY0 = 0, boxX1 = 1, boxY1 = 1;   // caja del plot; PlotStmt la fija antes de exec()

  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    checkKnownArgs("legend", named, kLegendArgs);
    if (entries.empty() && autoRules.empty()) return;
    std::string at = namedStr(s, named, "at"); if (at.empty()) at = "top-right";
    bool top    = at.compare(0, 3, "top") == 0;
    bool bottom = at.compare(0, 6, "bottom") == 0;
    bool center = at.compare(0, 6, "center") == 0;   // centrado en vertical (el loc='center right')
    bool right  = at.size() >= 5 && at.compare(at.size() - 5, 5, "right") == 0;
    bool left   = at.size() >= 4 && at.compare(at.size() - 4, 4, "left") == 0;
    if ((!top && !bottom && !center) || (!left && !right)) {
      evalError("legend: at= debe ser \"top\"/\"bottom\"/\"center\" + \"-left\"/\"-right\": ", at);
      return;
    }

    double margin       = namedNum(s, named, "margin", 6.0);        // pt
    double sampleWidth  = namedNum(s, named, "sample_width", 18.0); // pt
    double sampleHeight = namedNum(s, named, "sample_height", 10.0);// pt
    double gap          = namedNum(s, named, "gap", 4.0);           // pt
    double rowGap        = namedNum(s, named, "row_gap", 4.0);       // pt (aire extra entre renglones)
    double fontSize      = namedNum(s, named, "font_size", 8.0);     // pt

    // Escala física (cm por unidad de usuario), como axis (§13.5): pt→mundo por eje.
    double dcx, dcy; mg.getDimension(dcx, dcy);
    double wx, wy, wdx, wdy; mg.getWindow(wx, wy, wdx, wdy);
    double scx = dcx / wdx, scy = dcy / wdy;
    if (!mg.getStretch()) { double m = scx < scy ? scx : scy; scx = scy = m; }
    const double PT_PER_CM = 72.0 / 2.54;
    auto ptX = [&](double pt) { return (pt / PT_PER_CM) / scx; };
    auto ptY = [&](double pt) { return (pt / PT_PER_CM) / scy; };

    double rowH = ptY(std::max(sampleHeight, fontSize * 1.2) + rowGap);
    double marginX = ptX(margin), marginY = ptY(margin);
    size_t nAuto = autoRules.size();
    double n = (double)(nAuto + entries.size());
    // El margen solo aplica al borde con el que se ancla; centrado no toca ningún
    // borde horizontal, así que el bloque se reparte sobre el medio de la caja.
    double topEdgeY = top    ? (boxY1 - marginY)
                    : center ? ((boxY0 + boxY1) / 2 + n * rowH / 2)
                             : (boxY0 + marginY + n * rowH);

    double sw = ptX(sampleWidth), sh = ptY(sampleHeight), gp = ptX(gap);
    double sampleLeft, sampleRight;
    if (right) { sampleRight = boxX1 - marginX; sampleLeft = sampleRight - sw; }
    else       { sampleLeft  = boxX0 + marginX; sampleRight = sampleLeft + sw; }
    double textX = right ? (sampleLeft - gp) : (sampleRight + gp);
    int alignCode = right ? 2 : 0;   // 0=left, 2=right (nativo del backend, sin medir texto)

    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
    { auto a = std::make_unique<Attribute>(); a->set(AT_TALIGN, alignCode); out.push_back(std::move(a)); }
    { auto a = std::make_unique<Attribute>(); a->set(AT_TVALIGN, 2); out.push_back(std::move(a)); }   // middle
    { auto a = std::make_unique<Attribute>(); a->set(AT_THEIGHT, (int)fontSize); out.push_back(std::move(a)); }
    if (alignCode != 0) g_flags.using_textalign = true;   // prólogo /cshow de EPS (align=right)

    Box unit{0, 0, 1, 1};
    // Las automáticas van primero, luego las explícitas; dentro de cada grupo, en
    // orden de escritura. Ambas comparten renglonado y emisión de texto: la única
    // diferencia es de dónde sale la muestra.
    for (size_t i = 0; i < nAuto + entries.size(); i++) {
      double cy = topEdgeY - (i + 0.5) * rowH;
      std::string str;

      if (i < nAuto) {
        // Muestra AUTOMÁTICA: la línea del propio `rule`, con su propio estilo. No
        // se declara ni se repite — por eso esta fuente no puede divergir de lo que
        // señala, que es el defecto que la explícita acepta a cambio de control.
        RuleStmt *r = autoRules[i];
        GraphicsItemList attrs;
        r->emitStyle(s, attrs);
        out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
        for (auto &a : attrs) out.push_back(std::move(a));
        { auto pl = std::make_unique<Polyline>(GI_POLYLINE); Path pp;
          pp.push_back(point(sampleLeft, cy)); pp.push_back(point(sampleRight, cy));
          pl->setPath(pp); out.push_back(std::move(pl)); }
        out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
        str = r->labelOf(s);
      } else {
        // Muestra: fit MEET-centrado (preserva forma) de la caja unitaria al rectángulo
        // de esta fila. Inline con OPMPUSH/OPMPOP, como fit de struct.
        LegendEntry &e = entries[i - nAuto];
        Matrix M = FitStmt::fitMatrix(unit, sampleLeft, cy - sh / 2, sampleRight, cy + sh / 2, /*stretch=*/false);
        out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
        { auto t = std::make_unique<Transform>(); t->setOperation(OPMPUSH); t->setMatrix(M); out.push_back(std::move(t)); }
        for (auto &st : e.sample) st->exec(s, mg, out);
        popTransforms(countTransforms(e.sample), out);
        { auto t = std::make_unique<Transform>(); t->setOperation(OPMPOP); out.push_back(std::move(t)); }
        out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));

        Value lv = e.label->eval(s);
        if (lv.type == Value::STRING) str = lv.str;
        else { char buf[64]; std::snprintf(buf, sizeof buf, "%g", lv.num); str = buf; }
      }

      // Texto de la entrada (hereda align/valign/font_size del push de arriba).
      auto gs = std::make_unique<GraphicsState>(); gs->setPosition(point(textX, cy));
      out.push_back(std::move(gs));
      out.push_back(parse_text(str, FN_NOFACE, g_flags.using_reencode, g_flags.using_fontcmmi));
    }
    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// plot (§13.7): marco de datos declarativo. Bloque de contenido (sentencias de
// datos, EN UNIDADES DE DATOS) + rangos como args (x=/y=) + caja física box=
// (world coords; default = ventana vigente). El contenido se mapea datos→caja;
// los ejes se piden con xaxis(...)/yaxis(...) DENTRO del bloque (interceptados por
// table (§13.10): rejilla de celdas de texto con anchos DECLARADOS.
//
// No es una leyenda: una leyenda lista muestras gráficas con su nombre, y su ancho
// es incognoscible en parse-time (la muestra es un bloque arbitrario) — por eso
// `legend` se quedó sin marco. Una tabla declara `col_widths=`, así que su caja mide
// sum(anchos) × (filas·alto) y bordes, fondos y centrado son aritmética conocida.
// Esa es la inversión que hace viable el constructo: tener que dar los anchos es
// justo lo que compra el marco.
//
// NO es hija exclusiva de plot, a diferencia de rule (§13.8): `rule` no puede existir
// fuera porque su significado ES un valor en unidades de datos, mientras que una tabla
// no depende del mapeo ni del rango ni de los ejes — solo necesita un rectángulo. Las
// dos formas de `at=` resuelven ese rectángulo y el resto del código es común.
struct TableRow { std::vector<ExprPtr> cells; };
struct TableStmt : Stmt {
  std::map<std::string, ExprPtr> named;
  std::vector<TableRow> rows;
  // Caja de la que colgar cuando va dentro de un plot; PlotStmt la fija antes de exec().
  double boxX0 = 0, boxY0 = 0, boxX1 = 1, boxY1 = 1;
  bool inPlot = false;

  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    checkKnownArgs("table", named, kTableArgs);
    if (rows.empty()) return;

    // Anchos de columna (pt). Su tamaño fija el nº de columnas.
    std::vector<double> cw;
    { auto it = named.find("col_widths");
      if (it == named.end()) { evalError("table: requiere col_widths=(…) en puntos"); return; }
      Value v = it->second->eval(s);
      if (v.type == Value::LIST) for (auto &e : v.items) cw.push_back(e.num);
      else cw.push_back(v.num);
    }
    if (cw.empty()) { evalError("table: col_widths= vacío"); return; }
    for (auto &r : rows)
      if (r.cells.size() > cw.size()) {
        evalError("table: una fila tiene más celdas que columnas declaradas en col_widths");
        return;
      }

    double fontSize  = namedNum(s, named, "font_size", 8.0);
    double rowH      = namedNum(s, named, "row_height", fontSize * 1.8);
    double margin    = namedNum(s, named, "margin", 6.0);
    int    decimals  = (int)namedNum(s, named, "decimals", 3);

    // Escala física pt→mundo por eje, idéntica a la de legend/axis (§13.5).
    double dcx, dcy; mg.getDimension(dcx, dcy);
    double wx, wy, wdx, wdy; mg.getWindow(wx, wy, wdx, wdy);
    double scx = dcx / wdx, scy = dcy / wdy;
    if (!mg.getStretch()) { double m = scx < scy ? scx : scy; scx = scy = m; }
    const double PT_PER_CM = 72.0 / 2.54;
    auto ptX = [&](double pt) { return (pt / PT_PER_CM) / scx; };
    auto ptY = [&](double pt) { return (pt / PT_PER_CM) / scy; };

    double totW = 0; for (double w : cw) totW += w;
    double W = ptX(totW), H = ptY(rowH * (double)rows.size());

    // --- Esquina superior izquierda -----------------------------------------
    // at= sobrecargado (§13.10): cadena = ancla a la caja que contiene; tupla =
    // punto en coordenadas de mundo. La primera solo tiene sentido dentro de un plot.
    double x0, y1;
    auto itAt = named.find("at");
    Value atv = itAt != named.end() ? itAt->second->eval(s) : Value();
    if (itAt != named.end() && atv.type == Value::LIST && atv.items.size() >= 2) {
      x0 = atv.items[0].num; y1 = atv.items[1].num;
    } else {
      std::string at = (itAt != named.end() && atv.type == Value::STRING) ? atv.str : "top-right";
      if (!inPlot) {
        evalError("table: fuera de un plot, at= debe ser un punto (x,y); "
                  "las esquinas nombradas anclan a la caja del plot: ", at);
        return;
      }
      bool top    = at.compare(0, 3, "top") == 0;
      bool bottom = at.compare(0, 6, "bottom") == 0;
      bool center = at.compare(0, 6, "center") == 0;
      bool right  = at.size() >= 5 && at.compare(at.size() - 5, 5, "right") == 0;
      bool left   = at.size() >= 4 && at.compare(at.size() - 4, 4, "left") == 0;
      if ((!top && !bottom && !center) || (!left && !right)) {
        evalError("table: at= debe ser \"top\"/\"bottom\"/\"center\" + \"-left\"/\"-right\", "
                  "o un punto (x,y): ", at);
        return;
      }
      double mx = ptX(margin), my = ptY(margin);
      x0 = right ? (boxX1 - mx - W) : (boxX0 + mx);
      y1 = top   ? (boxY1 - my) : center ? ((boxY0 + boxY1) / 2 + H / 2) : (boxY0 + my + H);
    }
    double x1 = x0 + W, y0 = y1 - H;

    // --- Estilo --------------------------------------------------------------
    int  bgColor = 0xffffff; bool hasBg = true;          // fondo opaco por default
    { auto it = named.find("fill");
      if (it != named.end()) { Value v = it->second->eval(s);
        if (v.type == Value::STRING && v.str == "none") hasBg = false;
        else bgColor = colorFromValue(v); } }
    int  gridColor = 0x000000; bool hasGrid = true;
    { auto it = named.find("border");
      if (it != named.end()) { Value v = it->second->eval(s);
        if (v.type == Value::STRING) gridColor = colorFromValue(v);
        else if (v.num == 0.0) hasGrid = false; } }
    int  labColor = 0xf0f0f0; bool hasLab = false;       // 1ª columna, como el original
    { auto it = named.find("label_col");
      if (it != named.end()) { Value v = it->second->eval(s);
        if (v.type == Value::STRING) { hasLab = true; labColor = colorFromValue(v); }
        else if (v.num != 0.0) { hasLab = true; if (v.num != 1.0) labColor = (int)v.num; } } }

    auto rect = [&](double a, double b, double c, double d, int color) {
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      { auto at2 = std::make_unique<Attribute>(); at2->set(AT_FCOLOR, color); out.push_back(std::move(at2)); }
      out.push_back(std::make_unique<GraphicsState>(GS_FILL));
      { auto p = std::make_unique<Rectangle>(); Path pp;
        pp.push_back(point(a, b)); pp.push_back(point(c, d));
        p->setPath(pp); out.push_back(std::move(p)); }
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
    };

    // Fondos primero (el texto y la rejilla van encima).
    if (hasBg)  rect(x0, y0, x1, y1, bgColor);
    if (hasLab) rect(x0, y0, x0 + ptX(cw[0]), y1, labColor);

    // --- Rejilla -------------------------------------------------------------
    if (hasGrid) {
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      { auto a = std::make_unique<Attribute>(); a->set(AT_LCOLOR, gridColor); out.push_back(std::move(a)); }
      auto line = [&](double ax, double ay, double bx, double by) {
        auto pl = std::make_unique<Polyline>(GI_POLYLINE); Path pp;
        pp.push_back(point(ax, ay)); pp.push_back(point(bx, by));
        pl->setPath(pp); out.push_back(std::move(pl));
      };
      for (size_t r = 0; r <= rows.size(); r++) { double y = y1 - ptY(rowH * (double)r); line(x0, y, x1, y); }
      double xacc = x0;
      for (size_t c = 0; c <= cw.size(); c++) {
        line(xacc, y0, xacc, y1);
        if (c < cw.size()) xacc += ptX(cw[c]);
      }
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
    }

    // --- Texto ---------------------------------------------------------------
    std::string alignS = namedStr(s, named, "align");
    int alignCode = alignS.empty() ? 1 : alignCodeFromStr(alignS);   // 1 = center (cellLoc del original)
    if (alignCode < 0) alignCode = 1;
    if (alignCode != 0) g_flags.using_textalign = true;
    std::string labFont = namedStr(s, named, "label_font");
    FontFace labFace = labFont.empty() ? FN_NOFACE
                       : get_font_face_from_string(labFont, g_flags.using_fontcmmi);

    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
    { auto a = std::make_unique<Attribute>(); a->set(AT_TALIGN, alignCode);  out.push_back(std::move(a)); }
    { auto a = std::make_unique<Attribute>(); a->set(AT_TVALIGN, 2);         out.push_back(std::move(a)); }  // middle
    { auto a = std::make_unique<Attribute>(); a->set(AT_THEIGHT, (int)fontSize); out.push_back(std::move(a)); }
    char fmt[8]; std::snprintf(fmt, sizeof fmt, "%%.%df", decimals < 0 ? 0 : decimals);
    for (size_t r = 0; r < rows.size(); r++) {
      double cy = y1 - ptY(rowH * ((double)r + 0.5));
      double xacc = x0;
      for (size_t c = 0; c < rows[r].cells.size(); c++) {
        double w = ptX(cw[c]);
        // Ancla según la alineación: centro de celda, o su borde con un respiro.
        double pad = ptX(3.0);
        double tx = (alignCode == 1) ? (xacc + w / 2)
                  : (alignCode == 2) ? (xacc + w - pad) : (xacc + pad);
        Value v = rows[r].cells[c]->eval(s);
        std::string str;
        if (v.type == Value::STRING) str = v.str;
        else { char buf[64]; std::snprintf(buf, sizeof buf, fmt, v.num); str = buf; }
        // La cara de la 1ª columna se ACOTA con push/pop: `parse_text` la hornea en el
        // Text y `Text::draw` la deja puesta en el dispositivo, así que sin esto un
        // label_font="bold" se fugaría a las celdas de valor de la misma fila (que van
        // con FN_NOFACE = heredar la ambiente, y la ambiente ya sería la negrita).
        bool isLabel = (c == 0 && labFace != FN_NOFACE);
        if (isLabel) out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
        auto gs = std::make_unique<GraphicsState>(); gs->setPosition(point(tx, cy));
        out.push_back(std::move(gs));
        out.push_back(parse_text(str, (c == 0 ? labFace : FN_NOFACE),
                                 g_flags.using_reencode, g_flags.using_fontcmmi));
        if (isLabel) out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
        xacc += w;
      }
    }
    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// el parser) y se dibujan en coords exteriores, heredando from/to/scale de plot.
//
// Mecanismo (plan_plot.md Fase 4): plot TRANSFORMA las coordenadas de su
// contenido. Ruta 100% LINEAL → matriz envolvente datos→box (= FitStmt::fitMatrix
// con stretch, la misma idea que fit de struct). La ruta LOG (mapper puntual,
// porque log no es afín) se añade después; por ahora xscale/yscale="log" es error.
// Huella en el motor: CERO elementos nuevos (igual que axis).
struct PlotStmt : Stmt {
  std::map<std::string, ExprPtr> named;    // x, y, xscale, yscale, box
  std::vector<StmtPtr> content;            // sentencias de datos (mapeadas)
  std::unique_ptr<AxisStmt> xaxis, yaxis;  // ejes interceptados (coords exteriores)
  std::unique_ptr<LegendStmt> legend;      // leyenda interceptada (coords exteriores, §13.9)
  std::vector<std::unique_ptr<TableStmt>> tables;   // tablas §13.10, coords exteriores
  std::vector<std::unique_ptr<RuleStmt>> rules;   // valores notables (§13.8), coords exteriores

  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    checkKnownArgs("plot", named, kPlotArgs);
    // Rangos de datos (x=(from,to), y=(from,to)). Deben ser TUPLAS, igual que box=:
    // un escalar (`x=5`) es error, no se acepta en silencio cayendo al default (0,1).
    auto getRange = [&](const char *k, point &r) -> bool {
      auto it = named.find(k);
      if (it == named.end()) { r = point(0, 1); return true; }   // omitido = default
      Value v = it->second->eval(s);
      if (v.type != Value::LIST || v.items.size() < 2) {
        evalError("plot: requiere una tupla (from,to) en el arg ", k); return false;
      }
      r = point(v.items[0].num, v.items[1].num); return true;
    };
    point xr, yr;
    if (!getRange("x", xr) || !getRange("y", yr)) return;
    double x0 = xr.x, x1 = xr.y, y0 = yr.x, y1 = yr.y;
    std::string xscale = namedStr(s, named, "xscale");
    std::string yscale = namedStr(s, named, "yscale");
    bool xlog = xscale == "log", ylog = yscale == "log";

    // Caja física en world coords: box=(bx0,by0, bx1,by1); default = ventana vigente.
    double bx0, by0, bx1, by1;
    if (named.count("box")) {
      Value bv = named.at("box")->eval(s);
      if (bv.type != Value::LIST || bv.items.size() < 4) {
        evalError("plot: box= requiere 4 valores (x0,y0, x1,y1)"); return;
      }
      bx0 = bv.items[0].num; by0 = bv.items[1].num;
      bx1 = bv.items[2].num; by1 = bv.items[3].num;
    } else {
      double wx, wy, wdx, wdy; mg.getWindow(wx, wy, wdx, wdy);
      bx0 = wx; by0 = wy; bx1 = wx + wdx; by1 = wy + wdy;
    }

    // grid=: retícula opcional. `true` (número 1) → gris default; un color (cadena
    // "azul"/hex, o RGB de gray()/entero) → ese color; `false`/0/ausente → sin
    // retícula. (grid=gray(0)=0 cae en "sin retícula": una malla negra no se usa.)
    //
    // Aquí es el DEFAULT DE AMBOS EJES; cada uno puede pedir el suyo con
    // xaxis(grid=…)/yaxis(grid=…), que gana sobre este (§13.6). El caso corriente
    // —malla solo horizontal, como en cualquier gráfica de barras— es
    // `yaxis(grid=true)` sin tocar el plot.
    //
    // ⚠️ Y por eso NO se hizo `grid="y"`: `grid=` ya está sobrecargado con color,
    // así que la "y" se leería como nombre de color, avisaría y daría una malla
    // NEGRA en los dos ejes. Además la retícula ES la marca del eje barrida por el
    // campo (este código reusa axis(ticks="grid") con su step y su start), así que
    // pedirla por eje es donde conceptualmente vive.
    auto readGrid = [&](const std::map<std::string, ExprPtr> &m, bool &on, int &color) {
      auto it = m.find("grid");
      if (it == m.end()) return;                       // no dice nada: hereda
      Value gv = it->second->eval(s);
      if (gv.type == Value::STRING) { on = true; color = colorFromValue(gv); }
      else if (gv.num != 0.0) { on = true; if (gv.num != 1.0) color = (int)gv.num; }
      else on = false;                                 // grid=false apaga explícitamente
    };
    bool hasGrid = false;
    int gridColor = 0x808080;
    readGrid(named, hasGrid, gridColor);
    bool gridX = hasGrid, gridY = hasGrid;
    int  gcolX = gridColor, gcolY = gridColor;
    if (xaxis) readGrid(xaxis->named, gridX, gcolX);
    if (yaxis) readGrid(yaxis->named, gridY, gcolY);
    // grid_dash: el original de casi cualquier gráfica de barras es punteado.
    std::string gridDash = namedStr(s, named, "grid_dash");
    std::string gdX = gridDash, gdY = gridDash;
    if (xaxis && xaxis->named.count("grid_dash")) gdX = namedStr(s, xaxis->named, "grid_dash");
    if (yaxis && yaxis->named.count("grid_dash")) gdY = namedStr(s, yaxis->named, "grid_dash");

    // Rango de datos ≤ 0 en un eje log → error (el axis lo revalida, pero aquí el
    // mensaje es específico del plot).
    if ((xlog && (x0 <= 0 || x1 <= 0)) || (ylog && (y0 <= 0 || y1 <= 0))) {
      evalError("plot: un eje log requiere rango (x=/y=) positivo"); return;
    }

    // Mapper puntual valor→caja por eje (lineal o log). Lo usa la ruta LOG (abajo);
    // la lineal usa una matriz envolvente. t = fracción a lo largo del eje; en log
    // t interpola en el exponente (log no es afín → ninguna Matrix 3×3 lo expresa).
    auto mapAxis = [](double v, double from, double to, double b0, double b1, bool log) {
      double t;
      if (log) { double lf = std::log10(from), lt = std::log10(to);
                 t = (lt == lf) ? 0 : (std::log10(v) - lf) / (lt - lf); }
      else     { t = (to == from) ? 0 : (v - from) / (to - from); }
      return b0 + t * (b1 - b0);
    };
    auto mapPt = [&](point p) {
      return point(mapAxis(p.x, x0, x1, bx0, bx1, xlog),
                   mapAxis(p.y, y0, y1, by0, by1, ylog));
    };

    // --- Contenido de datos → caja ------------------------------------------
    // Retícula (grid=): capa de FONDO, ANTES del contenido → z-order correcto
    // (contenido y ejes encima). Reusa axis(ticks="grid"): los pasos salen del
    // `step` de xaxis/yaxis, así se auto-alinea con las marcas (y log sale gratis
    // cuando llegue). Estilo propio (gris 0.1 pt), desacoplado de ejes/contenido.
    // La pasada de grid dibuja también su línea de eje (gris, sobre el borde de la
    // caja), pero el eje real —negro, pintado después— la cubre → sin artefacto.
    if (gridX || gridY) {
      auto emitGrid = [&](AxisStmt *src, bool isX, int gcol, const std::string &gdash) {
        if (!src) return;                       // sin eje en esa dirección, sin retícula
        double from = isX ? x0 : y0, to = isX ? x1 : y1;
        const std::string &scale = isX ? xscale : yscale;
        double field = isX ? (by1 - by0) : (bx1 - bx0);
        double gx2 = isX ? bx1 : bx0, gy2 = isX ? by0 : by1;
        AxisStmt g;
        g.coords.push_back(ExprPtr(new NumExpr(bx0)));
        g.coords.push_back(ExprPtr(new NumExpr(by0)));
        g.coords.push_back(ExprPtr(new NumExpr(gx2)));
        g.coords.push_back(ExprPtr(new NumExpr(gy2)));
        g.named["from"] = ExprPtr(new NumExpr(from));
        g.named["to"]   = ExprPtr(new NumExpr(to));
        if (!scale.empty()) g.named["scale"] = ExprPtr(new StrExpr(scale));
        g.named["step"]       = ExprPtr(new NumExpr(namedNum(s, src->named, "step", 1)));
        // `start` va junto con `step`: la retícula ES la marca barrida por el campo,
        // así que tiene que nacer donde nacen las marcas. Sin esto arrancaba en
        // `from` y bastaba un start= fuera de la malla `from + k·step` para dejarla
        // en contrafase con su propio eje (p.ej. step=2 start=1 → marcas en 1,3,5…
        // y líneas en 0,2,4…). En fig2-3/fig6-4 alineaba de casualidad: su start=
        // cae a un número entero de pasos de from.
        if (src->named.count("start"))
          g.named["start"] = ExprPtr(new NumExpr(namedNum(s, src->named, "start", from)));
        g.named["ticks"]      = ExprPtr(new StrExpr("grid"));
        g.named["tick_labels"] = ExprPtr(new NumExpr(0));          // sin rótulos de marca
        g.named["field"]      = ExprPtr(new NumExpr(field));
        g.named["line_width"] = ExprPtr(new NumExpr(0.1));
        g.named["color"]      = ExprPtr(new NumExpr((double)gcol));
        if (!gdash.empty()) g.named["dash"] = ExprPtr(new StrExpr(gdash));
        g.exec(s, mg, out);
      };
      if (gridX) emitGrid(xaxis.get(), true,  gcolX, gdX);
      if (gridY) emitGrid(yaxis.get(), false, gcolY, gdY);
    }

    if (!xlog && !ylog) {
      // Ruta LINEAL: matriz envolvente datos→box (stretch), como fit de struct. Se
      // ejecuta el contenido dentro del envoltorio OPMPUSH/OPMPOP; los transforms
      // locales §11.1 del cuerpo se cierran antes del OPMPOP (como en FitStmt).
      // Ruta general: hasta structs invocadas dentro funcionan (matrices componen).
      Box w{x0, y0, x1 - x0, y1 - y0};
      Matrix M = FitStmt::fitMatrix(w, bx0, by0, bx1, by1, /*stretch=*/true);
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      { auto t = std::make_unique<Transform>(); t->setOperation(OPMPUSH); t->setMatrix(M); out.push_back(std::move(t)); }
      for (auto &st : content) st->exec(s, mg, out);
      popTransforms(countTransforms(content), out);
      { auto t = std::make_unique<Transform>(); t->setOperation(OPMPOP); out.push_back(std::move(t)); }
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
    } else {
      // Ruta LOG: mapper puntual. El contenido se ejecuta a una lista temporal (con
      // la bandera de contexto activa → bloquea colocación de structs, cuya matriz
      // de draw-time NO compone con el mapper no afín) y luego se remapean SOLO los
      // PUNTOS de cada item. Radios/anchos/line_width/font son miembros aparte o
      // Attributes → el mapper no los alcanza (invariante físico gratis, Lección 4).
      GraphicsItemList tmp;
      bool prevLogCtx = g_plotLogContext;   // save/restore (soporta plots anidados)
      g_plotLogContext = true;
      for (auto &st : content) st->exec(s, mg, tmp);
      g_plotLogContext = prevLogCtx;
      // Acota el estado del contenido (fill/color/line_width/dash) con push/pop, como
      // la ruta lineal lo hace vía el envoltorio de matriz. Sin esto un `fill "black"`
      // del contenido se FUGA a los ejes (que se dibujan después) → en PDF/SVG las
      // líneas del eje salen rellenas sin trazo = invisibles (EPS lo toleraba).
      out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
      bool sawBareTicks = false;
      for (auto &it : tmp) {
        switch (it->getType()) {
          case GI_POLYLINE: case GI_POLYGON: case GI_BEZIER: case GI_DOT:
          case GI_RECTANGLE: case GI_ARC: case GI_CIRCLE: case GI_ELLIPSE:
          case GI_POLYBAR: {
            auto *gp = static_cast<GraphicsItemWithPath *>(it.get());
            Path p = gp->getPath();
            for (point &pt : p) pt = mapPt(pt);
            gp->setPath(std::move(p));
            break;
          }
          case GI_STATE: {                       // ancla de text(): solo GS_PLUMEPOSITION lleva punto
            auto *gs = static_cast<GraphicsState *>(it.get());
            if (gs->getGraphicsStateType() == GS_PLUMEPOSITION)
              gs->setPosition(mapPt(gs->getPosition()));
            break;
          }
          case GI_TICKS:
            // grid()/ticks()/axis() PELADO en el contenido: su path[0] es un VECTOR,
            // no un punto → el mapper lo destruiría. La retícula va con grid= (arg de
            // plot); marcas/ejes con xaxis/yaxis. Se reporta UNA vez (tras el loop) y
            // el item corrupto NO se emite (abajo se salta al volcar).
            sawBareTicks = true;
            break;
          default: break;                        // Attribute/HatchAttr/Transform/glifos: intactos
        }
      }
      if (sawBareTicks)
        evalError("plot log: grid()/ticks()/axis() en el contenido corrompe coords; usa grid= o xaxis/yaxis");
      for (auto &it : tmp) {
        if (it->getType() == GI_TICKS) continue;   // no emitir el item corrupto (ya se avisó)
        out.push_back(std::move(it));
      }
      popTransforms(countTransforms(content), out);   // §11.1 del cuerpo (raro en log)
      out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
    }

    // --- frame=: caja completa alrededor del plot (§13.7) --------------------
    // Reusa el `box=` que el plot ya tiene, en vez de obligar a repetir sus cuatro
    // números en un `rectangle` por panel — que es lo que había que hacer antes y
    // se desincroniza en cuanto un panel se mueve. `true` = negro, un color = ese,
    // ausente/`false` = sin marco. Va DESPUÉS del contenido y ANTES de los ejes:
    // las líneas de eje caen sobre el borde de la caja y deben quedar encima.
    {
      auto it = named.find("frame");
      if (it != named.end()) {
        Value fv = it->second->eval(s);
        bool on = true; int fcol = 0x000000;
        if (fv.type == Value::STRING) fcol = colorFromValue(fv);
        else if (fv.num == 0.0) on = false;
        else if (fv.num != 1.0) fcol = (int)fv.num;
        if (on) {
          out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
          { auto a = std::make_unique<Attribute>(); a->set(AT_LCOLOR, fcol); out.push_back(std::move(a)); }
          auto pl = std::make_unique<Polyline>(GI_POLYLINE); Path pp;
          pp.push_back(point(bx0, by0)); pp.push_back(point(bx1, by0));
          pp.push_back(point(bx1, by1)); pp.push_back(point(bx0, by1));
          pl->setPath(pp); pl->setClosed(true);        // cierra sin repetir el vértice
          out.push_back(std::move(pl));
          out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
        }
      }
    }

    // --- Ejes: coords exteriores, NO mapeados -------------------------------
    // p1 p2 = borde de la caja; from/to/scale heredados de los args de plot. axis
    // ya sabe log, alinear, centrar título, extend → reuso directo.
    auto setup = [](AxisStmt &ax, double px1, double py1, double px2, double py2,
                    double from, double to, const std::string &scale) {
      ax.coords.clear();
      ax.coords.push_back(ExprPtr(new NumExpr(px1)));
      ax.coords.push_back(ExprPtr(new NumExpr(py1)));
      ax.coords.push_back(ExprPtr(new NumExpr(px2)));
      ax.coords.push_back(ExprPtr(new NumExpr(py2)));
      ax.named["from"] = ExprPtr(new NumExpr(from));
      ax.named["to"]   = ExprPtr(new NumExpr(to));
      if (!scale.empty()) ax.named["scale"] = ExprPtr(new StrExpr(scale));
    };
    // base= (opcional): valor —en unidades de datos del eje PERPENDICULAR— donde
    // cruza este eje. Default = borde de la caja (by0 para x, bx0 para y). Es lo que
    // permite el eje V centrado y el eje x sobre V=0 de las figuras de potenciales
    // (fig4-5); sin él todo eje queda pegado al borde. NO mueve la retícula: el grid
    // barre la caja completa, cruce donde cruce el eje.
    auto baseOf = [&](AxisStmt &ax, double from, double to, double b0, double b1,
                      bool log, double dflt, const char *which) {
      auto it = ax.named.find("base");
      if (it == ax.named.end()) return dflt;
      double v = it->second->eval(s).num;
      if (log && v <= 0) {
        evalError("plot: base= requiere un valor positivo en eje log, en ", which);
        return dflt;
      }
      return mapAxis(v, from, to, b0, b1, log);
    };
    // La posición de cada línea de eje se calcula ANTES de dibujarlas porque los
    // `rule` también la necesitan (su `to=` mide desde la línea del eje, y su rótulo
    // cuelga de donde el eje pondría el de esa marca). Sin eje en esa dirección, el
    // borde de la caja: es donde el eje habría caído sin `base=`.
    double yb = xaxis ? baseOf(*xaxis, y0, y1, by0, by1, ylog, by0, "xaxis") : by0;
    double xb = yaxis ? baseOf(*yaxis, x0, x1, bx0, bx1, xlog, bx0, "yaxis") : bx0;
    if (xaxis) { setup(*xaxis, bx0, yb, bx1, yb, x0, x1, xscale); xaxis->exec(s, mg, out); }
    if (yaxis) { setup(*yaxis, xb, by0, xb, by1, y0, y1, yscale); yaxis->exec(s, mg, out); }

    // --- rule (§13.8): valores notables ---------------------------------------
    // Coords EXTERIORES como los ejes: se resuelven con el mapper del plot (log
    // incluido) y se dibujan ENCIMA del contenido, debajo de la leyenda.
    for (auto &r : rules) {
      bool hasX = r->named.count("x") != 0, hasY = r->named.count("y") != 0;
      if (hasX == hasY) {
        evalError("rule: requiere exactamente uno de x= o y= (el valor, y de qué eje es hijo)");
        continue;
      }
      std::string la = namedStr(s, r->named, "label_at");
      if (!la.empty() && la != "axis" && la != "legend") {
        evalError("rule: label_at debe ser \"axis\" o \"legend\": ", la);
        continue;
      }
      double v = namedNum(s, r->named, hasX ? "x" : "y", 0);
      bool selfLog = hasX ? xlog : ylog, perpLog = hasX ? ylog : xlog;
      if (selfLog && v <= 0) {
        evalError("rule: en un eje log el valor debe ser positivo");
        continue;
      }
      r->vertical = hasX;
      r->at = hasX ? mapAxis(v, x0, x1, bx0, bx1, xlog) : mapAxis(v, y0, y1, by0, by1, ylog);
      // Extensión: sin `to=`, la caja entera (el axvline de matplotlib). Con `to=`,
      // desde la LÍNEA DEL EJE hasta ese valor — la línea corta de cortesía que va
      // del dato al eje, que es lo que pide una figura de libro.
      double axisLine = hasX ? yb : xb;
      if (r->named.count("to")) {
        double tv = namedNum(s, r->named, "to", 0);
        if (perpLog && tv <= 0) {
          evalError("rule: en un eje log el `to=` debe ser positivo");
          continue;
        }
        r->lo = axisLine;
        r->hi = hasX ? mapAxis(tv, y0, y1, by0, by1, ylog) : mapAxis(tv, x0, x1, bx0, bx1, xlog);
      } else {
        r->lo = hasX ? by0 : bx0;
        r->hi = hasX ? by1 : bx1;
      }
      r->labelAnchor = axisLine;
      AxisStmt *owner = hasX ? xaxis.get() : yaxis.get();
      r->labelGap = owner ? namedNum(s, owner->named, "tick_label_gap", 4.0) : 4.0;
      r->exec(s, mg, out);
      if (la == "legend" && legend) legend->autoRules.push_back(r.get());
    }
    if (!legend) {
      for (auto &r : rules)
        if (namedStr(s, r->named, "label_at") == "legend") {
          warn("rule: label_at=\"legend\" pero el plot no tiene legend(...); el rótulo no se dibuja");
          break;
        }
    }
    // Leyenda: ENCIMA de contenido+ejes (última en pintarse), en coords exteriores
    // (la caja física, no mapeada por datos).
    if (legend) {
      legend->boxX0 = bx0; legend->boxY0 = by0; legend->boxX1 = bx1; legend->boxY1 = by1;
      legend->exec(s, mg, out);
    }
    // Tablas (§13.10): como la leyenda, coords exteriores y encima del contenido.
    for (auto &t : tables) {
      t->boxX0 = bx0; t->boxY0 = by0; t->boxX1 = bx1; t->boxY1 = by1;
      t->inPlot = true;
      t->exec(s, mg, out);
    }
  }
};

// Controles del documento (§18): valores sueltos separados por espacio, sin
// paréntesis ni bloque. max_depth va aquí y no con las sentencias de estado
// porque no es estado gráfico: no se acota por bloque ni se restaura con
// gsave/grestore — es un tope del compilador, como display_size.
static bool isConfig(const std::string &n) {
  return n == "display_size" || n == "world_window" || n == "max_depth";
}
static int configArity(const std::string &n) {
  if (n == "display_size") return 2;
  if (n == "max_depth")    return 1;
  return 4;                                   // world_window
}
// Sentencia de transformación local (§11.1) → operación de matriz, o -1 si no lo es.
static int transformOp(const std::string &n) {
  if (n == "translate") return OPMTL;
  if (n == "rotate")    return OPMRT;
  if (n == "scale")     return OPMSC;
  if (n == "shear")     return OPMSH;
  return -1;
}
static bool isPrim(const std::string &n) {
  return n == "polyline" || n == "polygon" || n == "circle" || n == "rectangle" ||
         n == "dot" || n == "marker" || n == "arc" || n == "ellipse" || n == "bezier" ||
         n == "polybar" || n == "smooth";   // smooth: primitiva §9.2 Y expresión de path (como sine)
}

static StmtPtr parseBlock(Lexer &);        // adelantadas (mutuamente recursivas)
static StmtPtr parseStructDef(Lexer &);
static StmtPtr parseFor(Lexer &);
static StmtPtr parseIf(Lexer &);
static StmtPtr parseRepeat(Lexer &);
static StmtPtr parseFit(Lexer &);
static StmtPtr parsePlace(Lexer &);
static StmtPtr parseSine(Lexer &);
static StmtPtr parsePlot(Lexer &);
static std::unique_ptr<TableStmt> parseTableBody(Lexer &);   // §13.10 (también suelta)
static StmtPtr parseInclude(Lexer &);
static StmtPtr parseInvoke(Lexer &, const std::string &);
static PathExprPtr parsePathExpr(Lexer &);   // álgebra de paths (§9)
static void parseSineArgs(Lexer &, std::map<std::string, ExprPtr> &,
                          std::vector<ExprPtr> &);   // `(args) { base }` de sine

// Un nombre de atributo puede coincidir con una palabra de control: `to`/`step`
// son keywords (for i = a to b) pero también nombres nombrados (arc(from=, to=)).
// Dentro de (…) se desambigua por contexto: keyword seguida de '=' = nombre.
// ¿El punto actual empieza un PathExpr (§9)? Una referencia `&nombre` o una op de
// path (`transpose`/`flip_x`/`flip_y`/`concat`). Sirve para bifurcar polyline/fit
// entre su forma clásica y la de álgebra de paths.
static bool startsPathExpr(const Lexer &lx) {
  int tt = lx.peek().type;
  if (tt == T_AMP) return true;
  if (tt == T_IDENTIFIER && lx.peek(1).type != T_ASSIGN) {  // `sine=` sería arg nombrado
    const std::string &n = lx.peek().str;
    return n == "transpose" || n == "flip_x" || n == "flip_y" || n == "reverse" ||
           n == "concat" || n == "sine" || n == "smooth" || n == "sample";
  }
  return false;
}

static bool attrNameHere(const Lexer &lx, std::string &out) {
  int tt = lx.peek().type;
  if (lx.peek(1).type != T_ASSIGN) return false;
  if (tt == T_IDENTIFIER) { out = lx.peek().str; return true; }
  if (tt == T_TO)   { out = "to";   return true; }
  if (tt == T_STEP) { out = "step"; return true; }
  return false;
}

// Lista de args ( … ): posicionales (Expression) y nombrados (nombre = Expression).
// Asume que '(' ya se consumió; consume hasta ')'.
static void parseArgList(Lexer &lx, std::vector<ExprPtr> &pos,
                         std::map<std::string, ExprPtr> &named) {
  if (lx.peek().type != T_RPAREN) {
    do {
      std::string k;
      if (attrNameHere(lx, k)) {
        lx.next(); lx.next();                 // nombre y '='
        named[k] = parseExpression(lx);
      } else {
        pos.push_back(parseExpression(lx));
      }
    } while (lx.accept(T_COMMA));
  }
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
}

// Lista de args mixta (§8.x): como parseArgList, pero cada arg es Expression O
// PathExpr (nunca ambos, ver `Arg`) — solo la usan la invocación de struct
// (InvokeStmt) y la invocación interior de fit (§10.2), que son las únicas
// formas que aceptan un parámetro &path.
static void parseMixedArgList(Lexer &lx, std::vector<Arg> &pos,
                              std::map<std::string, Arg> &named) {
  if (lx.peek().type != T_RPAREN) {
    do {
      std::string k;
      bool isNamed = attrNameHere(lx, k);
      if (isNamed) { lx.next(); lx.next(); }      // nombre y '='
      Arg a;
      if (startsPathExpr(lx)) a.p = parsePathExpr(lx);
      else                    a.e = parseExpression(lx);
      if (isNamed) named[k] = std::move(a);
      else         pos.push_back(std::move(a));
    } while (lx.accept(T_COMMA));
  }
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
}

static StmtPtr parseStatement(Lexer &lx) {
  const Tok &t = lx.peek();
  if (t.type == T_LBRACE) return parseBlock(lx);      // bloque anónimo (§7.1)
  if (t.type == T_STRUCT) return parseStructDef(lx);  // struct Nombre(...) { … }
  if (t.type == T_FOR)    return parseFor(lx);        // for v = a to b [step s] { … }
  if (t.type == T_IF)     return parseIf(lx);         // if cond { … } [else { … }]
  if (t.type == T_INCLUDE) return parseInclude(lx);   // include "archivo.mg" (§15)
  if (t.type != T_IDENTIFIER) parseError(lx, "un comando, asignación, struct o '{'");
  std::string name = t.str;
  const int nameLine = t.line, nameCol = t.col;   // para señalar AL NOMBRE, no a lo que le sigue

  if (lx.peek(1).type == T_ASSIGN) {          // asignación: ID = Expr
    lx.next(); lx.next();
    auto st = std::make_unique<AssignStmt>();
    st->name = name;
    st->val = parseExpression(lx);
    return st;
  }
  lx.next();                                   // consume el nombre del comando

  if (name == "path") {                        // path ID = <PathExpr>  (§9)
    if (lx.peek().type != T_IDENTIFIER) parseError(lx, "el nombre del path");
    std::string pname = lx.next().str;
    if (lx.accept(T_PLUSASSIGN)) {             // path ID += <PathExpr>: acumular (§9)
      auto st = std::make_unique<PathAppendStmt>();
      st->name = pname;
      st->expr = parsePathExpr(lx);
      return st;
    }
    if (!lx.accept(T_ASSIGN)) parseError(lx, "'=' o '+=' en la declaración de path");
    auto st = std::make_unique<PathDeclStmt>();
    st->name = pname;
    st->expr = parsePathExpr(lx);
    return st;
  }
  if (isConfig(name)) {
    auto st = std::make_unique<ConfigStmt>();
    st->which = name;
    int n = configArity(name);
    for (int i = 0; i < n; i++) st->args.push_back(parseTerm(lx));   // valores por espacio -> Term
    std::string k;                              // world_window ... stretch=true (§13.7)
    if (attrNameHere(lx, k) && k == "stretch") { lx.next(); lx.next(); st->stretchE = parseExpression(lx); }
    return st;
  }
  if (isPrim(name)) {
    auto st = std::make_unique<PrimStmt>();
    st->name = name;
    if (lx.accept(T_LPAREN)) {
      if (startsPathExpr(lx)) {                 // polyline(<PathExpr>): álgebra §9
        st->pathArg = parsePathExpr(lx);
        // De dónde sale el path y CÓMO SE DIBUJA son cosas ortogonales, así que tras
        // la expresión se admiten los mismos args nombrados que en la forma con
        // bloque: `polybar(&hist, width=w, fill=…)`, `polyline(&p, color="red")`.
        // Antes se exigía ')' aquí mismo, lo que obligaba a envolver el path en un
        // bloque solo para poder darle estilo (o a rodearlo con sentencias de estado).
        if (lx.accept(T_COMMA)) parseArgList(lx, st->pos, st->named);
        else if (!lx.accept(T_RPAREN)) parseError(lx, "')' o ',' tras la expresión de path");
      } else {
        parseArgList(lx, st->pos, st->named);   // args (Expression)
      }
    }
    if (lx.accept(T_LBRACE)) {                  // { coords }  (Term; ';' abre subtrayecto §4; newline se salta)
      while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
        if (lx.accept(T_SEMICOLON)) { st->breaks.push_back(st->coords.size()); continue; }
        if (lx.accept(T_NEWLINE)) continue;
        st->coords.push_back(parseTerm(lx));
      }
      // allowsPoints=true: una coordenada puede ser un punto [x,y] (point_at, un
      // literal [a,b] o una variable-punto). La paridad se valida en eval-time
      // (coordsToPath). bezier NO acepta puntos: sus controles son escalares y el
      // conteo 3k+1 se sigue validando en parse-time.
      checkCoordPairs(lx, name, st->coords, st->breaks, /*allowsPoints=*/name != "bezier");
      if (name == "bezier") checkBezierControlCount(lx, st->coords, st->breaks);
      if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
    }
    return st;
  }
  if (name == "repeat") return parseRepeat(lx);   // repeat(Struct, count=…, …) (§17)
  if (name == "fit")    return parseFit(lx);      // fit(Struct[, stretch=]) { rect } (§10.2)
  if (name == "place")  return parsePlace(lx);    // place(Struct, …) { locus } (§10.1)
  if (name == "sine")   return parseSine(lx);     // sine(half_cycles=, …) { base } (§4.13)
  if (name == "text") {                       // text("s") { coords }  (§4.8)
    auto st = std::make_unique<TextStmt>();
    if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras text");
    std::vector<ExprPtr> pos;
    parseArgList(lx, pos, st->named);          // 1er posicional = contenido; resto nombrado
    if (pos.empty()) parseError(lx, "el contenido del texto");
    st->content = std::move(pos[0]);
    if (lx.accept(T_LBRACE)) {
      while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
        if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
        st->coords.push_back(parseTerm(lx));
      }
      checkCoordPairs(lx, "text", st->coords, {});   // bloque vacío = pluma (§12.1); 0 es par
      if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
    }
    return st;
  }
  if (name == "numbers") {                    // numbers(from=, by=, count=, …) (§13.2)
    auto st = std::make_unique<NumbersStmt>();
    if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras numbers");
    std::vector<ExprPtr> pos;
    parseArgList(lx, pos, st->named);
    return st;
  }
  if (name == "ticks") {                       // ticks(count, mark=, at=, advance=) (§13.1)
    auto st = std::make_unique<TicksStmt>();
    if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras ticks");
    parseArgList(lx, st->pos, st->named);
    return st;
  }
  if (name == "compound") {                    // compound(…) { sub-primitivas } (§9.4)
    auto st = std::make_unique<CompoundStmt>();
    if (lx.accept(T_LPAREN)) { std::vector<ExprPtr> p; parseArgList(lx, p, st->named); }
    if (lx.accept(T_LBRACE)) {
      while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
        if (lx.accept(T_NEWLINE)) continue;
        st->body.push_back(parseStatement(lx));
      }
      if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
    }
    return st;
  }
  if (name == "axis") {                        // axis(from=, to=, step=, …) { p1 p2 } (§13.5)
    auto st = std::make_unique<AxisStmt>();
    if (lx.accept(T_LPAREN)) { std::vector<ExprPtr> p; parseArgList(lx, p, st->named); }
    if (lx.accept(T_LBRACE)) {
      while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
        if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
        st->coords.push_back(parseTerm(lx));
      }
      if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
    }
    return st;
  }
  if (name == "plot") return parsePlot(lx);    // plot(x=, y=, box=, …) { contenido + xaxis/yaxis } (§13.7)
  // table (§13.10) NO es hija exclusiva de plot: fuera de él se posiciona con at=(x,y).
  // (Dentro, parsePlot la intercepta antes de llegar aquí, para anclarla a su caja.)
  if (name == "table") return parseTableBody(lx);
  if (name == "grid") {                        // grid(xstep=, ystep=, …) { rect } (§13.6)
    auto st = std::make_unique<GridStmt>();
    if (lx.accept(T_LPAREN)) { std::vector<ExprPtr> p; parseArgList(lx, p, st->named); }
    if (lx.accept(T_LBRACE)) {
      while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
        if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
        st->coords.push_back(parseTerm(lx));
      }
      if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
    }
    return st;
  }
  // Transformación local como sentencia (§11.1): translate/rotate/scale/shear sin
  // paréntesis, args por espacio hasta fin de línea. Con paréntesis sería el
  // constructor de valor de transform= (§17), que no aparece en posición de sentencia.
  int top = transformOp(name);
  if (top >= 0 && lx.peek().type != T_LPAREN) {
    auto st = std::make_unique<TransformStmt>();
    st->mop = top;
    // Aridad FIJA (no "hasta newline"): la spec §11.1 permite varias sentencias
    // por línea (`translate 5 5  rotate 30  Flecha(...)`), así que cada transform
    // consume solo sus argumentos. scale: 1, con 2º opcional si sigue un número.
    if (top == OPMSC) {
      // `scale` es el único transform de aridad VARIABLE (1 = uniforme, 2 = por eje),
      // y eso choca de frente con «varias sentencias por línea»: en `scale s  Flecha()`
      // o `scale s  color "red"`, lo que sigue es OTRA sentencia, no el factor en y.
      // Un identificador suelto NO se puede desambiguar ahí —cualquier nombre puede
      // ser un comando o una struct—, así que la regla es sintáctica:
      //   - número o '-': segundo factor (como siempre).
      //   - '(' : segundo factor. Es INEQUÍVOCO porque ninguna sentencia empieza con
      //     '(' → `scale sx (sy)` es la forma de dar dos factores con variables.
      //   - identificador: NO se toma. Antes se descartaba en silencio y el escalado
      //     quedaba uniforme con el primer factor (bug de 2026-07-22).
      st->args.push_back(parseTerm(lx));
      int t = lx.peek().type;
      bool second = (t == T_NUMBER || t == T_MINUS || t == T_LPAREN);
      if (!second && t == T_IDENTIFIER) {
        // Un identificador seguido de FIN DE SENTENCIA no puede SER una sentencia: las
        // de estado piden argumento y una invocación pide '('. Luego se escribió como
        // segundo factor, y se toma. La deducción es lo que permite `scale sx sy` sin
        // romper `scale s  color "red"` ni `scale s  Flecha()`, donde lo que sigue NO
        // termina ahí.
        int after = lx.peek(1).type;
        bool endsStmt = (after == T_NEWLINE || after == T_SEMICOLON ||
                         after == T_RBRACE  || after == T_EOF);
        // `outlinefill` es la ÚNICA sentencia de estado de cero argumentos (§4.11), o
        // sea el único identificador que legítimamente aparece solo y termina ahí.
        second = endsStmt && lx.peek().str != "outlinefill";
      }
      if (second) st->args.push_back(parseTerm(lx));
    } else {
      int arity = (top == OPMRT) ? 1 : 2;      // rotate=1; translate/shear=2
      for (int i = 0; i < arity; i++) st->args.push_back(parseTerm(lx));
    }
    return st;
  }

  // `rule` (§13.8) es hijo del plot y solo parsePlot lo intercepta; suelto caería en
  // parseInvoke y fallaría como "struct no definida", que no dice nada del problema.
  // Mensaje directo (no parseError) por lo mismo que la rama de `polylnie` de abajo:
  // la plantilla "se esperaba X, pero se encontró Y" no aplica a una frase completa,
  // y hay que señalar al NOMBRE, no al '(' que le sigue.
  // `exit` (§18) lo consume parseProgram, que es el nivel de archivo. Si llega aquí
  // es que está ANIDADO, y eso no puede funcionar: es un control de PARSE-TIME, así
  // que dentro de un `if` cortaría el archivo siempre —la condición se evalúa
  // después— y dentro de un bloque o un struct dejaría las llaves sin cerrar.
  if (name == "exit") {
    std::fprintf(stderr, "Error de sintaxis en %d:%d: 'exit' solo va al nivel superior "
                 "del archivo (§18). Es un control de parse-time: dentro de un if no "
                 "sería condicional, y dentro de un bloque dejaría las llaves abiertas.\n",
                 nameLine, nameCol);
    std::exit(1);
  }
  if (name == "rule") {
    std::fprintf(stderr, "Error de sintaxis en %d:%d: 'rule' solo es válido dentro de "
                 "un plot { } (es hijo del plot, §13.8)\n", nameLine, nameCol);
    std::exit(1);
  }
  // `row` (§13.10) solo existe dentro de table { }; suelto caeria en parseInvoke y
  // fallaria como "struct no definida", que no dice nada del problema real.
  if (name == "row") {
    std::fprintf(stderr, "Error de sintaxis en %d:%d: 'row' solo es válido dentro de "
                 "un table { } (§13.10)\n", nameLine, nameCol);
    std::exit(1);
  }

  if (lx.peek().type == T_LPAREN) return parseInvoke(lx, name);   // invocación: Nombre( … )

  // Resto: sentencia de estado (color, line_width, …). ARIDAD ACOTADA por nombre
  // (antes leía "hasta T_NEWLINE", lo que se tragaba una sentencia escrita en el
  // mismo renglón: `line_width 0.5  P()` → P() era arg de line_width). Así varias
  // sentencias por línea funcionan, igual que translate/rotate (§11.1). Casi todas
  // toman 1 arg; `hatch` 1-2 (ángulo/estilo + paso opcional); `outlinefill` 0-1.
  auto st = std::make_unique<StateStmt>();
  st->name = name;
  auto parseArg = [&](SArg &a) {
    if (lx.peek().type == T_STRING) { a.isStr = true; a.str = lx.next().str; }
    else                            { a.isStr = false; a.num = parseTerm(lx); }
  };
  if (name == "outlinefill") {
    // 0 o 1: bare = on; también `outlinefill true|false` (bareword → cadena) o un
    // número/cadena. Un identificador que NO sea true/false se deja como la
    // siguiente sentencia (no se consume).
    int t = lx.peek().type;
    if (t == T_IDENTIFIER && (lx.peek().str == "true" || lx.peek().str == "false")) {
      SArg a; a.isStr = true; a.str = lx.next().str; st->args.push_back(std::move(a));
    } else if (t == T_NUMBER || t == T_MINUS || t == T_STRING) {
      SArg a; parseArg(a); st->args.push_back(std::move(a));
    }
  } else {
    // Un '{' NO puede ser el argumento de una sentencia de estado (todas toman
    // número o cadena), así que el identificador no era un comando: el caso
    // típico es una PRIMITIVA MAL ESCRITA, `polylnie { … }`. Sin esta rama el
    // error señalaba al '{' pidiendo "una expresión" y no nombraba nunca la
    // palabra, que es lo único que hay que corregir. Se apunta a la posición del
    // NOMBRE, no a la del '{'. `outlinefill { … }` sí es legítimo (0 args +
    // bloque), por eso la comprobación vive en esta rama y no antes.
    if (lx.peek().type == T_LBRACE) {
      std::fprintf(stderr, "Error de sintaxis en %d:%d: '%s' no es un comando conocido "
                   "(¿primitiva mal escrita?)\n", nameLine, nameCol, name.c_str());
      std::exit(1);
    }
    SArg a; parseArg(a); st->args.push_back(std::move(a));           // 1er arg (obligatorio)
    if (name == "hatch" && (lx.peek().type == T_NUMBER || lx.peek().type == T_MINUS)) {
      SArg g; parseArg(g); st->args.push_back(std::move(g));         // hatch: paso opcional
    }
  }
  return st;
}

static StmtPtr parseBlock(Lexer &lx) {
  lx.next();                                 // consume '{'
  auto blk = std::make_unique<BlockStmt>();
  while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
    if (lx.accept(T_NEWLINE)) continue;      // saltar líneas en blanco dentro del bloque
    blk->body.push_back(parseStatement(lx));
  }
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
  return blk;
}

// plot(args) { contenido… xaxis(...) yaxis(...) }  (§13.7). Calcado de parseCompound:
// args nombrados + loop de parseStatement hasta '}', INTERCEPTANDO xaxis/yaxis (que
// solo son válidos aquí) → AxisStmt sin bloque de coords (plot fija p1 p2 y from/to).
// legend( ... ) { entry("texto") { <bloque de la muestra, coords 0..1> } ... }
// Solo válido dentro de plot (como xaxis/yaxis); `entry` solo dentro de legend.
// table( … ) { row(a, b, …)  row(…) … }   (§13.10)
// Válida dentro de plot (ancla a su caja) y suelta (at=(x,y)); `row` solo aquí.
static std::unique_ptr<TableStmt> parseTableBody(Lexer &lx) {
  auto st = std::make_unique<TableStmt>();
  if (lx.accept(T_LPAREN)) { std::vector<ExprPtr> p; parseArgList(lx, p, st->named); }
  if (!lx.accept(T_LBRACE)) parseError(lx, "'{' tras table(...)");
  while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
    if (lx.accept(T_NEWLINE)) continue;
    if (lx.peek().type != T_IDENTIFIER || lx.peek().str != "row")
      parseError(lx, "'row(...)' dentro de table");
    lx.next();
    if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras row");
    TableRow r;
    if (lx.peek().type != T_RPAREN)
      do { r.cells.push_back(parseExpression(lx)); } while (lx.accept(T_COMMA));
    if (!lx.accept(T_RPAREN)) parseError(lx, "')' de row");
    st->rows.push_back(std::move(r));
  }
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}' de table");
  return st;
}

static std::unique_ptr<LegendStmt> parseLegendBody(Lexer &lx) {
  auto st = std::make_unique<LegendStmt>();
  if (lx.accept(T_LPAREN)) { std::vector<ExprPtr> p; parseArgList(lx, p, st->named); }
  // Sin bloque: leyenda 100% AUTOMÁTICA (§13.9 fuente 1) — `legend(at="top-left")`
  // solo declara DÓNDE va, y sus entradas las ponen los `rule` con label_at="legend".
  // Es la forma que quiere una figura de varios paneles, donde lo único que cambia
  // de panel a panel es la esquina que queda libre.
  if (lx.peek().type != T_LBRACE) return st;
  if (!lx.accept(T_LBRACE)) parseError(lx, "'{' tras legend(...)");
  while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
    if (lx.accept(T_NEWLINE)) continue;
    if (lx.peek().type != T_IDENTIFIER || lx.peek().str != "entry")
      parseError(lx, "'entry(...)' dentro de legend");
    lx.next();
    LegendEntry e;
    if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras entry");
    e.label = parseExpression(lx);
    if (!lx.accept(T_RPAREN)) parseError(lx, "')' tras el texto de entry");
    while (lx.accept(T_NEWLINE)) {}              // '{' puede ir en la línea siguiente
    if (!lx.accept(T_LBRACE)) parseError(lx, "'{' del cuerpo de entry");
    while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
      if (lx.accept(T_NEWLINE)) continue;
      e.sample.push_back(parseStatement(lx));
    }
    if (!lx.accept(T_RBRACE)) parseError(lx, "'}' del cuerpo de entry");
    st->entries.push_back(std::move(e));
  }
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}' de legend");
  return st;
}

static StmtPtr parsePlot(Lexer &lx) {
  auto st = std::make_unique<PlotStmt>();
  if (lx.accept(T_LPAREN)) { std::vector<ExprPtr> p; parseArgList(lx, p, st->named); }
  if (!lx.accept(T_LBRACE)) parseError(lx, "'{' tras plot(...)");
  while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
    if (lx.accept(T_NEWLINE)) continue;
    const Tok &tk = lx.peek();
    if (tk.type == T_IDENTIFIER && (tk.str == "xaxis" || tk.str == "yaxis")) {
      bool isX = tk.str == "xaxis";
      lx.next();
      auto ax = std::make_unique<AxisStmt>();
      if (lx.accept(T_LPAREN)) { std::vector<ExprPtr> p; parseArgList(lx, p, ax->named); }
      // Sin bloque { }: plot suministra coords y from/to. Un bloque aquí es error.
      if (lx.peek().type == T_LBRACE) parseError(lx, "xaxis/yaxis dentro de plot no llevan bloque { }");
      if (isX) st->xaxis = std::move(ax); else st->yaxis = std::move(ax);
      continue;
    }
    if (tk.type == T_IDENTIFIER && tk.str == "legend") {
      lx.next();
      st->legend = parseLegendBody(lx);
      continue;
    }
    if (tk.type == T_IDENTIFIER && tk.str == "table") {
      lx.next();
      st->tables.push_back(parseTableBody(lx));
      continue;
    }
    // rule (§13.8): como xaxis/yaxis, sin bloque — sus args lo dicen todo.
    if (tk.type == T_IDENTIFIER && tk.str == "rule") {
      lx.next();
      auto r = std::make_unique<RuleStmt>();
      if (lx.accept(T_LPAREN)) { std::vector<ExprPtr> p; parseArgList(lx, p, r->named); }
      if (lx.peek().type == T_LBRACE) parseError(lx, "rule no lleva bloque { } (el valor va en x=/y=)");
      st->rules.push_back(std::move(r));
      continue;
    }
    st->content.push_back(parseStatement(lx));
  }
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
  return st;
}

static StmtPtr parseStructDef(Lexer &lx) {
  lx.next();                                   // 'struct'
  if (lx.peek().type != T_IDENTIFIER) parseError(lx, "el nombre de la struct");
  auto def = std::make_unique<StructDef>();
  def->name = lx.next().str;
  if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras el nombre de la struct");
  if (lx.peek().type != T_RPAREN) {            // parámetros: [&]ID [= default]  (§8.x: & = tipo path)
    do {
      bool isPath = lx.accept(T_AMP);
      if (lx.peek().type != T_IDENTIFIER) parseError(lx, "un nombre de parámetro");
      def->params.push_back(lx.next().str);
      def->paramIsPath.push_back(isPath);
      def->defaults.push_back(lx.accept(T_ASSIGN) ? parseExpression(lx) : nullptr);
    } while (lx.accept(T_COMMA));
  }
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
  while (lx.accept(T_NEWLINE)) {}              // '{' puede ir en la línea siguiente
  if (!lx.accept(T_LBRACE)) parseError(lx, "'{' del cuerpo de la struct");
  while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
    if (lx.accept(T_NEWLINE)) continue;
    // world_window dentro del cuerpo = ventana LOCAL del struct (§16): la captura
    // place/fit para normalizar; NO fija la ventana del documento ni va al cuerpo.
    if (lx.peek().type == T_IDENTIFIER && lx.peek().str == "world_window" &&
        lx.peek(1).type != T_ASSIGN) {
      lx.next();
      for (int i = 0; i < 4; i++) def->localWindow.push_back(parseTerm(lx));
      continue;
    }
    def->body.push_back(parseStatement(lx));
  }
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
  auto stmt = std::make_unique<StructDefStmt>();
  stmt->def = std::move(def);
  return stmt;
}

static StmtPtr parseFor(Lexer &lx) {
  lx.next();                                   // 'for'
  if (lx.peek().type != T_IDENTIFIER) parseError(lx, "la variable del for");
  auto st = std::make_unique<ForStmt>();
  st->var = lx.next().str;
  if (!lx.accept(T_ASSIGN)) parseError(lx, "'=' en el for");
  st->start = parseExpression(lx);
  if (!lx.accept(T_TO)) parseError(lx, "'to' en el for");
  st->end = parseExpression(lx);
  if (lx.accept(T_STEP)) st->step = parseExpression(lx);
  while (lx.accept(T_NEWLINE)) {}              // '{' puede ir en la línea siguiente
  if (lx.peek().type != T_LBRACE) parseError(lx, "'{' del cuerpo del for");
  st->body = parseBlock(lx);                   // BlockStmt: estado + ámbito por iteración
  return st;
}

static StmtPtr parseIf(Lexer &lx) {
  lx.next();                                   // 'if'
  auto st = std::make_unique<IfStmt>();
  st->cond = parseExpression(lx);
  while (lx.accept(T_NEWLINE)) {}              // '{' puede ir en la línea siguiente
  if (lx.peek().type != T_LBRACE) parseError(lx, "'{' del cuerpo del if");
  st->thenB = parseBlock(lx);
  // else opcional: puede llevar newlines entre el '}' y el 'else'. Si no hay
  // else, restaura la posición para que el newline siga siendo terminador.
  size_t save = lx.pos;
  while (lx.accept(T_NEWLINE)) {}
  if (lx.accept(T_ELSE)) {
    while (lx.accept(T_NEWLINE)) {}
    if (lx.peek().type == T_IF)          st->elseB = parseIf(lx);      // else if …
    else if (lx.peek().type == T_LBRACE) st->elseB = parseBlock(lx);
    else parseError(lx, "'{' o 'if' tras else");
  } else {
    lx.pos = save;
  }
  return st;
}

// transform= (§17): secuencia de constructores de matriz yuxtapuestos
// (rotate(a) scale(b) …), compuestos en orden de escritura. Se detiene al ver
// algo que no sea uno de los cuatro constructores (típicamente ',' o ')').
static int ctorOp(const std::string &fn) {
  if (fn == "translate") return OPMTL;
  if (fn == "rotate")    return OPMRT;
  if (fn == "scale")     return OPMSC;
  if (fn == "shear")     return OPMSH;
  return -1;
}
static void parseTransformValue(Lexer &lx, std::vector<MatrixCtor> &out) {
  while (lx.peek().type == T_IDENTIFIER) {
    int op = ctorOp(lx.peek().str);
    if (op < 0) break;
    lx.next();
    if (!lx.accept(T_LPAREN)) parseError(lx, "'(' en el constructor de transform");
    MatrixCtor c; c.op = op;
    if (lx.peek().type != T_RPAREN) {
      c.args.push_back(parseExpression(lx));
      while (lx.accept(T_COMMA)) c.args.push_back(parseExpression(lx));
    }
    if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
    out.push_back(std::move(c));
  }
  if (out.empty()) parseError(lx, "un constructor de transform (rotate/scale/translate/shear)");
}

// Parsea un PathExpr (§9): literal `{ x y … }`, referencia `&nombre`, u op de path
// `transpose/flip_x/flip_y(<PathExpr>)` / `concat(<PathExpr>, <PathExpr>)`. Anidable.
static PathExprPtr parsePathExpr(Lexer &lx) {
  const Tok &t = lx.peek();
  if (t.type == T_LBRACE) {                      // literal { x y  x y … }
    lx.next();
    auto lit = std::make_unique<PathLiteral>();
    while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
      if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
      lit->coords.push_back(parseTerm(lx));
    }
    checkCoordPairs(lx, "el literal de path", lit->coords, {});
    if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
    return lit;
  }
  if (t.type == T_AMP) {                         // &ID
    lx.next();
    if (lx.peek().type != T_IDENTIFIER) parseError(lx, "el nombre del path tras '&'");
    auto r = std::make_unique<PathRef>();
    r->name = lx.next().str;
    return r;
  }
  if (t.type == T_IDENTIFIER) {
    std::string n = t.str;
    if (n == "transpose" || n == "flip_x" || n == "flip_y" || n == "reverse") {
      lx.next();
      if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras la operación de path");
      auto u = std::make_unique<PathUnary>();
      u->op = (n == "transpose") ? PathUnary::TRANSPOSE
            : (n == "flip_x")    ? PathUnary::FLIP_X
            : (n == "flip_y")    ? PathUnary::FLIP_Y : PathUnary::REVERSE;
      u->arg = parsePathExpr(lx);
      if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
      return u;
    }
    if (n == "concat") {                         // variádico (§9): concat(a, b, c, …)
      lx.next();
      if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras concat");
      auto c = std::make_unique<PathConcat>();
      c->parts.push_back(parsePathExpr(lx));
      while (lx.accept(T_COMMA)) c->parts.push_back(parsePathExpr(lx));
      if (c->parts.size() < 2) parseError(lx, "al menos dos paths en concat(a, b, …)");
      if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
      return c;
    }
    if (n == "sine") {                           // sine(args) { base } como path (§9/§4.13)
      lx.next();
      auto ps = std::make_unique<PathSine>();
      parseSineArgs(lx, ps->named, ps->coords);
      return ps;
    }
    if (n == "sample") {                         // sample(&p, n [, curve=b]) (§9, α+β)
      lx.next();
      if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras sample");
      auto sp = std::make_unique<PathSample>();
      sp->arg = parsePathExpr(lx);
      if (!lx.accept(T_COMMA)) parseError(lx, "',' y el número de puntos");
      sp->count = parseExpression(lx);
      if (lx.accept(T_COMMA)) {                  // curve= nombrado, o posicional
        std::string k;
        if (attrNameHere(lx, k) && k == "curve") { lx.next(); lx.next(); }
        sp->curve = parseExpression(lx);
      }
      if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
      return sp;
    }
    if (n == "smooth") {                         // smooth { pts } (§9.2)
      lx.next();
      if (!lx.accept(T_LBRACE)) parseError(lx, "'{' de los puntos de smooth");
      auto sm = std::make_unique<PathSmooth>();
      while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
        if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
        sm->coords.push_back(parseTerm(lx));
      }
      checkCoordPairs(lx, "smooth", sm->coords, {});
      if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
      return sm;
    }
  }
  parseError(lx, "un path: '{…}', &nombre, transpose/flip_x/flip_y/reverse/concat/sine/smooth(…)");
  return nullptr;                                // inalcanzable (parseError hace exit)
}

// path_width(&p) (paso 5 de plan_struct_params.md): puente Expr↔PathExpr,
// declarada junto a parseUnary (parseAtom la necesita) y definida aquí, donde
// PathWidthExpr y parsePathExpr ya son visibles. El '(' inicial ya se consumió
// en parseAtom (case T_IDENTIFIER); esta función consume el resto.
static ExprPtr parsePathWidthCall(Lexer &lx) {
  auto e = std::make_unique<PathWidthExpr>();
  e->arg = parsePathExpr(lx);
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
  return e;
}

// path_x_min_at_y/path_x_max_at_y(&p, y [, expand]) — el '(' ya se consumió.
static ExprPtr parsePathXBoundCall(Lexer &lx, bool wantMax) {
  auto e = std::make_unique<PathXBoundExpr>();
  e->wantMax = wantMax;
  e->arg = parsePathExpr(lx);
  if (!lx.accept(T_COMMA)) parseError(lx, "',' y la altura y del corte");
  e->level = parseExpression(lx);
  if (lx.accept(T_COMMA)) e->expand = parseExpression(lx);
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
  return e;
}

// point_at(&p, t [, curve=b]) / angle_at(&p, t [, curve=b]) — el '(' ya se consumió.
static ExprPtr parsePathSampleCall(Lexer &lx, bool wantAngle) {
  auto e = std::make_unique<PathPointExpr>();
  e->wantAngle = wantAngle;
  e->arg = parsePathExpr(lx);
  if (!lx.accept(T_COMMA)) parseError(lx, "',' y el parámetro t∈[0,1]");
  e->param = parseExpression(lx);
  if (lx.accept(T_COMMA)) {                     // curve= nombrado, o posicional
    std::string k;
    if (attrNameHere(lx, k) && k == "curve") { lx.next(); lx.next(); }
    e->curve = parseExpression(lx);
  }
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
  return e;
}

static StmtPtr parseRepeat(Lexer &lx) {
  if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras repeat");
  auto st = std::make_unique<RepeatStmt>();
  if (lx.peek().type != T_IDENTIFIER) parseError(lx, "el nombre de la struct a repetir");
  st->structName = lx.next().str;               // 1er argumento: nombre (sin evaluar)
  while (lx.accept(T_COMMA)) {                   // resto: argumentos nombrados
    std::string k;
    if (!attrNameHere(lx, k)) parseError(lx, "un argumento nombrado (count=, scale=, transform=, …)");
    lx.next(); lx.next();                        // nombre y '='
    if (k == "transform") parseTransformValue(lx, st->xform);
    else                  st->named[k] = parseExpression(lx);
  }
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
  return st;
}

static StmtPtr parseFit(Lexer &lx) {
  if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras fit");
  auto st = std::make_unique<FitStmt>();
  if (startsPathExpr(lx)) {                      // fit(<PathExpr>) { rect }  (§9)
    st->pathArg = parsePathExpr(lx);
  } else {
    if (lx.peek().type != T_IDENTIFIER) parseError(lx, "el nombre de la struct o un path (&…)");
    st->structName = lx.next().str;
    if (lx.accept(T_LPAREN)) {                   // fit(Struct(args), …) { rect } (§10.2)
      parseMixedArgList(lx, st->invokePos, st->invokeNamed);
    }
  }
  while (lx.accept(T_COMMA)) {                   // argumentos nombrados (stretch=…)
    std::string k;
    if (!attrNameHere(lx, k)) parseError(lx, "un argumento nombrado (stretch=)");
    lx.next(); lx.next();
    ExprPtr v = parseExpression(lx);
    // `fit` es el único de la familia que descarta en PARSE-time: su `named` no
    // llega a exec(), así que la compuerta de checkKnownArgs no lo alcanzaría.
    if (k != "stretch")
      evalError(("fit: `" + k + "=` no existe (¿mal escrito?); el único "
                 "argumento nombrado de fit es `stretch=`").c_str());
    st->stretchE = std::move(v);
  }
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
  if (!lx.accept(T_LBRACE)) parseError(lx, "'{' del rectángulo de fit");
  while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
    if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
    st->coords.push_back(parseTerm(lx));
  }
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
  return st;
}

static StmtPtr parsePlace(Lexer &lx) {
  if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras place");
  auto st = std::make_unique<PlaceStmt>();
  if (lx.peek().type != T_IDENTIFIER) parseError(lx, "el nombre de la struct a colocar");
  st->structName = lx.next().str;
  while (lx.accept(T_COMMA)) {                   // scale/shift/count/gap/both_sides/r/from/to
    std::string k;
    if (!attrNameHere(lx, k)) parseError(lx, "un argumento nombrado (scale=, count=, r=, …)");
    lx.next(); lx.next();
    st->named[k] = parseExpression(lx);
  }
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
  if (!lx.accept(T_LBRACE)) parseError(lx, "'{' del locus de place");
  while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
    if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
    st->coords.push_back(parseTerm(lx));
  }
  checkCoordPairs(lx, "el locus de place", st->coords, {});
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
  return st;
}

// `(args nombrados) { base }` de sine — compartido por la sentencia (parseSine)
// y la expresión de path (parsePathExpr).
static void parseSineArgs(Lexer &lx, std::map<std::string, ExprPtr> &named,
                          std::vector<ExprPtr> &coords) {
  if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras sine");
  if (lx.peek().type != T_RPAREN) {             // args nombrados: half_cycles=, amplitude=, …
    do {
      std::string k;
      if (!attrNameHere(lx, k)) parseError(lx, "un argumento nombrado (half_cycles=, amplitude=, …)");
      lx.next(); lx.next();
      named[k] = parseExpression(lx);
    } while (lx.accept(T_COMMA));
  }
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
  if (!lx.accept(T_LBRACE)) parseError(lx, "'{' de la base de sine");
  while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
    if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
    coords.push_back(parseTerm(lx));
  }
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
}

static StmtPtr parseSine(Lexer &lx) {
  auto st = std::make_unique<SineStmt>();
  parseSineArgs(lx, st->named, st->coords);
  return st;
}

static StmtPtr parseInvoke(Lexer &lx, const std::string &name) {
  auto st = std::make_unique<InvokeStmt>();
  st->name = name;
  lx.next();                                   // consume '('
  parseMixedArgList(lx, st->pos, st->named);
  return st;
}

// Parsea un flujo ya tokenizado en una lista de sentencias (sin ejecutar).
static std::vector<StmtPtr> parseProgram(Lexer &lx) {
  std::vector<StmtPtr> prog;
  while (lx.peek().type != T_EOF) {
    if (lx.accept(T_NEWLINE)) continue;          // saltar líneas en blanco/separadores
    // `exit` (§18): corta el parseo DEL ARCHIVO aquí; lo que sigue no se convierte en
    // sentencias, así que un error de SINTAXIS posterior tampoco se reporta —llaves sin
    // cerrar, una primitiva mal escrita— que es justo lo que se quiere al construir la
    // figura por partes. ⚠️ Un error LÉXICO sí salta (un '@' suelto, una letra acentuada
    // fuera de una cadena): el Lexer tokeniza el archivo ENTERO antes de que este bucle
    // corra, y adelantar el corte al lexer costaría más de lo que vale.
    // Va en este bucle y no en parseStatement porque este ES el nivel de archivo:
    // parseProgram lo usan tanto el documento principal como cada `include`, de modo
    // que un `exit` en un incluido corta ESE archivo y el principal sigue, que es lo
    // que dice la spec («detiene el procesamiento del archivo»).
    if (lx.peek().type == T_IDENTIFIER && lx.peek().str == "exit") break;
    prog.push_back(parseStatement(lx));
  }
  return prog;
}

// Lee un archivo completo, lo tokeniza y lo parsea (para include). El Lexer es
// local: las sentencias copian sus lexemas, así que puede destruirse al salir.
static std::vector<StmtPtr> parseFile(const std::string &path) {
  std::ifstream in(path);
  // FATAL, no aviso: un include que no resuelve deja el documento INCOMPLETO por
  // definición. Antes devolvía {} y seguía, así que si el archivo perdido solo
  // aportaba cosas opcionales —colores, structs no usadas en todas las ramas— la
  // figura se generaba a medias con código de salida 0. Es el mismo criterio por el
  // que `evalError` es fatal: un documento inconsistente no debe producir salida.
  if (!in) {
    std::fprintf(stderr, "Error: include no pudo abrir %s\n", path.c_str());
    std::exit(1);
  }
  std::stringstream ss;
  ss << in.rdbuf();
  Lexer lx;
  lx.tokenize(ss.str().c_str());
  return parseProgram(lx);
}

static bool fileReadable(const std::string &p) {
  std::ifstream f(p.c_str());
  return f.good();
}

static StmtPtr parseInclude(Lexer &lx) {
  lx.next();                                     // 'include'
  if (lx.peek().type != T_STRING) parseError(lx, "el nombre de archivo (cadena) tras include");
  std::string fname = lx.next().str;
  // Búsqueda (§15): una ruta ABSOLUTA se toma tal cual; si no, LOCAL primero —junto
  // al archivo principal (g_baseDir)— y luego la BIBLIOTECA instalada (MG_LIBDIR, que
  // el Makefile hornea al compilar). Lo local PISA lo instalado: una copia junto a la
  // figura gana sobre la de la lib del sistema (útil para tantear una variante sin
  // reinstalar). Sin MG_LIBDIR (build sin la macro) solo se mira lo local.
  std::vector<std::string> cands;
  if (!fname.empty() && fname[0] == '/')
    cands.push_back(fname);
  else {
    cands.push_back(g_baseDir.empty() ? fname : g_baseDir + "/" + fname);
#ifdef MG_LIBDIR
    cands.push_back(std::string(MG_LIBDIR) + "/" + fname);
#endif
  }
  std::string path;
  for (const std::string &c : cands)
    if (fileReadable(c)) { path = c; break; }
  if (path.empty()) {
    // FATAL, como cualquier include que no resuelve: el documento queda incompleto.
    std::fprintf(stderr, "Error: include no encontró \"%s\" (busqué:", fname.c_str());
    for (const std::string &c : cands) std::fprintf(stderr, " %s", c.c_str());
    std::fprintf(stderr, ")\n");
    std::exit(1);
  }
  auto st = std::make_unique<IncludeStmt>();
  st->prog = parseFile(path);                    // parseo recursivo (permite includes anidados)
  return st;
}

// --- Driver: fuente V3 -> MetaGrafica ----------------------------------------
// Declarada en parserv3.h; main.cpp (entry point) es el único llamador.
MetaGrafica *buildFromSource(const std::string &src) {
  Lexer lx;
  lx.tokenize(src.c_str());
  std::vector<StmtPtr> prog = parseProgram(lx);
  auto *mg = new MetaGrafica();
  mg->setName("mgv3");
  mg->setDepth(64);
  Scope scope;
  GraphicsItemList items;
  for (auto &st : prog) st->exec(scope, *mg, items);
  mg->setGraphicsItems(std::move(items));
  return mg;
}
