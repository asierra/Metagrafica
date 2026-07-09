// Parser V3 (descenso recursivo). Compila la gramática V3 (§1-§18) a un árbol
// MetaGrafica. Post-cutover (§22.6): es el ÚNICO front-end de bin/mg (Parser.cpp
// V1 sigue en el árbol solo como referencia/insumo de mg1to2.py, fuera del
// build). El entry point (main()) vive en src/main.cpp; este archivo expone
// buildFromSource()/g_baseDir/g_flags vía include/parserv3.h.

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

#include "ast.h"
#include "tokens.h"

// Motor reutilizado (árbol dibujable + backends)
#include "structures.h"    // MetaGrafica, GraphicsItemList, point, Path, primitivas
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

static void parseError(const Lexer &lx, const char *what) {
  const Tok &t = lx.peek();
  std::fprintf(stderr, "Error de sintaxis en %d:%d: se esperaba %s\n", t.line, t.col, what);
  std::exit(1);
}

// --- Gramática de expresiones (§2) ------------------------------------------
static ExprPtr parseExpression(Lexer &);
static ExprPtr parseUnary(Lexer &);   // adelantada: parsePower la necesita

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
static ExprPtr parseAnd(Lexer &lx) {          // Comparison ("and" Comparison)*
  ExprPtr e = parseComparison(lx);
  while (lx.peek().type == T_AND) {
    lx.next();
    e = ExprPtr(new BinExpr(T_AND, std::move(e), parseComparison(lx)));
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
static const std::map<std::string, int> map_color = {
  {"black", 0}, {"blue", 0xff}, {"brown", 0xa52a2a}, {"cyan", 0xffff},
  {"gray", 0x808080}, {"green", 0xff00}, {"lightgray", 0xd3d3d3},
  {"magenta", 0xff00ff}, {"orange", 0xcc3232}, {"red", 0xff0000},
  {"white", 0xffffff}, {"yellow", 0xffff00},
};
static int get_color(const std::string &c) {
  if (!c.empty() && c[0] == '#') return (int)std::strtol(c.c_str() + 1, nullptr, 16);
  auto it = map_color.find(c);
  return it == map_color.end() ? 0 : it->second;
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
static FillPattern buildHatchPattern(const Value &v, double gap) {
  if (v.type == Value::STRING) {
    if (v.str == "hatch")           return { HatchLine{45.0, gap} };
    if (v.str == "hatchback")       return { HatchLine{135.0, gap} };
    if (v.str == "crosshatch")      return { HatchLine{45.0, gap}, HatchLine{135.0, gap} };
    return {};
  }
  return { HatchLine{v.num, gap} };
}

// §4.11: emite el HatchAttr (FillPattern completo) + activa el relleno.
// Compartido por el atributo por-primitiva (emitPrimStyle) y la sentencia de
// estado (StateStmt), para que ambos registros se comporten idéntico.
static void emitHatch(const Value &v, double gap, GraphicsItemList &out) {
  FillPattern fp = buildHatchPattern(v, gap);
  if (fp.empty()) return;
  g_flags.using_hatcher = true;
  out.push_back(std::make_unique<HatchAttr>(fp));
  out.push_back(std::make_unique<GraphicsState>(GS_FILL));
}

// Estilo por-primitiva (§7.5) compartido por PrimStmt y compound (§9.4):
// color/fill/line_width + tramado (hatch/hatch_gap, §4.11) + contorno (color con
// relleno). Emite en `attrs` los GraphicsItem que el llamador acota con push/pop.
static void emitPrimStyle(const std::map<std::string, ExprPtr> &named, Scope &s,
                          GraphicsItemList &attrs) {
  for (const char *k : {"color", "fill", "line_width", "dash"}) {
    auto it = named.find(k);
    if (it != named.end()) emitStyleAttr(k, it->second->eval(s), attrs);
  }
  if (named.count("hatch")) {   // §4.11: hatch=ángulo|"estilo" [+ hatch_gap]
    Value hv = named.at("hatch")->eval(s);
    double gap = named.count("hatch_gap") ? named.at("hatch_gap")->eval(s).num : 4.0;
    emitHatch(hv, gap, attrs);
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
  if (name == "dash") {                                             // §4.10: patrón de línea
    int idx = 0;                                                    // solid/none/continuous → 0
    if (v.type == Value::STRING) {
      if (v.str == "longdashed") idx = 1;                               // dashArrayForIndex: {4,2}
      else if (v.str == "dashed") idx = 2;
      else if (v.str == "dotted") idx = 3;                          // {2,1.6}
      else if (v.str == "dashdot" || v.str == "dash-dot") idx = 4;  // {4,2,1,2}
      else if (v.str == "dashdotdot") idx = 5;                      // {4,2,2,2,2,2}
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
    emitStyleAttr(name, v, out);
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
    s.vars[name] = val->eval(s);
  }
};

struct ConfigStmt : Stmt {
  std::string which;
  std::vector<ExprPtr> args;
  ExprPtr stretchE;                        // world_window ... stretch= (null = meet)
  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &) override {
    auto n = [&](size_t i) { return args[i]->eval(s).num; };
    if (which == "display_size")      mg.setDimension(n(0), n(1));
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
    Scope child(&parent);                  // reads suben al padre; writes son locales
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
// Registro global (§15: nombres globales). Posee las definiciones.
static std::map<std::string, std::unique_ptr<StructDef>> g_structs;

struct StructDefStmt : Stmt {
  std::unique_ptr<StructDef> def;
  void exec(Scope &, MetaGrafica &, GraphicsItemList &) override {
    g_structs[def->name] = std::move(def);   // registra; el registro pasa a poseerla
  }
};

struct InvokeStmt : Stmt {
  std::string name;
  std::vector<ExprPtr> pos;
  std::map<std::string, ExprPtr> named;      // incluye at/rotate/scale (aún ignorados)
  void exec(Scope &caller, MetaGrafica &mg, GraphicsItemList &out) override {
    auto it = g_structs.find(name);
    if (it == g_structs.end()) { evalError("struct no definida: ", name); return; }
    StructDef *def = it->second.get();
    // Ámbito de la struct: hijo del GLOBAL (léxico), con los parámetros ligados.
    // Los args se evalúan en el ámbito del LLAMADOR; los defaults, en el de la struct.
    Scope local(caller.root());
    for (size_t i = 0; i < def->params.size(); i++) {
      const std::string &pn = def->params[i];
      auto nit = named.find(pn);
      if (i < pos.size())          local.vars[pn] = pos[i]->eval(caller);
      else if (nit != named.end()) local.vars[pn] = nit->second->eval(caller);
      else if (def->defaults[i])   local.vars[pn] = def->defaults[i]->eval(local);
      else                         local.vars[pn] = Value(0);
    }
    // Modificadores de colocación (§8): at/rotate/scale → marco canónico fijo
    // T·R·S (post-multiplicación: escala/gira en el marco local, luego traslada;
    // el orden de escritura es irrelevante). Se evalúan en el ámbito del llamador.
    // Nombres reservados: no son parámetros de la struct.
    Matrix frame;
    bool hasFrame = false;
    auto ait = named.find("at"), rit = named.find("rotate"), sit = named.find("scale");
    if (ait != named.end()) {
      Value v = ait->second->eval(caller);
      if (v.type == Value::LIST && v.items.size() >= 2) { frame.translate(v.items[0].num, v.items[1].num); hasFrame = true; }
    }
    if (rit != named.end()) { frame.rotate(rit->second->eval(caller).num); hasFrame = true; }
    if (sit != named.end()) {
      Value v = sit->second->eval(caller);
      if (v.type == Value::LIST && v.items.size() >= 2) frame.scale(v.items[0].num, v.items[1].num);
      else                                              frame.scale(v.num, v.num);
      hasFrame = true;
    }

    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));  // aísla estado (como structure())
    if (hasFrame) {
      auto t = std::make_unique<Transform>();
      t->setOperation(OPMPUSH); t->setMatrix(frame);
      out.push_back(std::move(t));
    }
    for (auto &st : def->body) st->exec(local, mg, out);
    popTransforms(countTransforms(def->body), out);   // transforms locales del cuerpo (§11.1)
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
      for (auto &st : def->body) st->exec(local, mg, out);
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
  ExprPtr stretchE;                       // stretch= (null = proporcional/meet)
  std::vector<ExprPtr> coords;            // 4 Terms = 2 esquinas del rectángulo
  void exec(Scope &caller, MetaGrafica &mg, GraphicsItemList &out) override {
    auto it = g_structs.find(structName);
    if (it == g_structs.end()) { evalError("struct no definida (fit): ", structName); return; }
    StructDef *def = it->second.get();
    if (coords.size() < 4) { evalError("fit requiere un rectángulo (2 puntos)"); return; }
    double x1 = coords[0]->eval(caller).num, y1 = coords[1]->eval(caller).num;
    double x2 = coords[2]->eval(caller).num, y2 = coords[3]->eval(caller).num;
    Box w = structBox(caller, def);
    bool stretch = stretchE && stretchE->eval(caller).num != 0.0;
    double tdx = x2 - x1, tdy = y2 - y1;   // firmados: el orden de esquinas refleja

    Matrix M;
    if (stretch) {
      M.translate(x1, y1);
      M.scale(tdx / w.dx, tdy / w.dy);     // window → rectángulo exacto
      M.translate(-w.x, -w.y);
    } else {
      double sx = tdx / w.dx, sy = tdy / w.dy;
      double s = (std::fabs(sx) < std::fabs(sy)) ? std::fabs(sx) : std::fabs(sy);
      double ssx = (sx < 0 ? -s : s), ssy = (sy < 0 ? -s : s);   // uniforme, con signo
      double ox = x1 + (tdx - ssx * w.dx) / 2, oy = y1 + (tdy - ssy * w.dy) / 2;  // centrado
      M.translate(ox, oy);
      M.scale(ssx, ssy);
      M.translate(-w.x, -w.y);
    }
    Scope local(caller.root());
    for (size_t i = 0; i < def->params.size(); i++)
      local.vars[def->params[i]] = def->defaults[i] ? def->defaults[i]->eval(local) : Value(0);
    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
    { auto t = std::make_unique<Transform>(); t->setOperation(OPMPUSH); t->setMatrix(M); out.push_back(std::move(t)); }
    for (auto &st : def->body) st->exec(local, mg, out);
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
  Box w = structBox(caller, def);
  bool needN = (w.x != 0.0 || w.y != 0.0 || w.dx != 1.0 || w.dy != 1.0);
  if (needN) {
    Matrix N; N.scale(1.0 / w.dx, 1.0 / w.dy); N.translate(-w.x, -w.y);   // ventana→unidad
    auto t = std::make_unique<Transform>(); t->setOperation(OPMPUSH); t->setMatrix(N);
    prlist.push_back(std::move(t));
  }
  for (auto &st : def->body) st->exec(local, mg, prlist);
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
// oscilando perpendicular. Se expande a un Polyline(GI_BEZIER) (sin clase de
// motor). Puntos de control = los de V1 (bzsinepaths.mg) → coincide con el oráculo.
// Núcleo: phase=0 (jorobas &sinpi con signo alterno) + squared (bumps sin²,
// &cos2pi invertido). phase!=0 y half_cycles fraccionario: diferidos (plan_sine.md).
struct SineStmt : Stmt {
  std::map<std::string, ExprPtr> named;   // half_cycles, amplitude, phase, squared
  std::vector<ExprPtr> coords;            // 2 puntos = base
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    double n = namedNum(s, named, "half_cycles", 1);
    double A = namedNum(s, named, "amplitude", 1);
    double phase = namedNum(s, named, "phase", 0);
    bool squared = named.count("squared") && named.at("squared")->eval(s).num != 0.0;
    if (coords.size() < 4) { evalError("sine requiere una base (2 puntos)"); return; }
    if (n <= 0) return;
    if (phase != 0.0 && squared)
      std::fprintf(stderr, "sine: phase!=0 con squared aún no implementado; se dibuja phase=0\n");

    point p1(coords[0]->eval(s).num, coords[1]->eval(s).num);
    point p2(coords[2]->eval(s).num, coords[3]->eval(s).num);
    double dx = p2.x - p1.x, dy = p2.y - p1.y, L = std::sqrt(dx * dx + dy * dy);
    if (L == 0.0) return;
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
      Path qp;
      qp.push_back(W(0.0, Q[startQ][6] * A));              // moveto en la y inicial del 1er cuarto
      for (int i = 0; i < 2 * ni; i++) {
        const double *q = Q[(startQ + i) % 4];
        double base = i * wq;
        qp.push_back(W(base + q[0] * wq, q[1] * A));
        qp.push_back(W(base + q[2] * wq, q[3] * A));
        qp.push_back(W(base + q[4] * wq, q[5] * A));
      }
      auto pq = std::make_unique<Polyline>(GI_BEZIER);
      pq->setPath(qp);
      out.push_back(std::move(pq));
      return;
    }

    Path path;
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
    auto p = std::make_unique<Polyline>(GI_BEZIER);
    p->setPath(path);
    out.push_back(std::move(p));
  }
};

struct PrimStmt : Stmt {
  std::string name;
  std::vector<ExprPtr> pos;               // args posicionales
  std::map<std::string, ExprPtr> named;   // args nombrados
  std::vector<ExprPtr> coords;            // coordenadas (Term), en pares x y
  std::vector<size_t> breaks;             // índices en coords donde ';' abre subtrayecto

  double namedOr(Scope &s, const char *k, double def) const {
    auto it = named.find(k);
    return it == named.end() ? def : it->second->eval(s).num;
  }
  double posOr(Scope &s, size_t i, double def) const {
    return i < pos.size() ? pos[i]->eval(s).num : def;
  }

  // Evalúa las coords [from, to) a un Path (pares x y).
  Path evalPath(Scope &s, size_t from, size_t to) const {
    Path path;
    for (size_t i = from; i + 1 < to; i += 2)
      path.push_back(point(coords[i]->eval(s).num, coords[i + 1]->eval(s).num));
    return path;
  }

  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    // polyline/polygon/bezier admiten subtrayectos disjuntos separados por ';'
    // (§4): un item por subtrayecto, mismo estilo (distinto de compound, §9.4,
    // que combina en un solo relleno par-impar). El resto de primitivas ignora
    // los ';' (no aplican: rectangle=pares, dot=posiciones, circle/arc=centros).
    std::vector<std::unique_ptr<GraphicsItem>> items;
    GraphicsItemType poly = GI_POLYLINE;
    bool isPoly = true;
    if      (name == "polyline") poly = GI_POLYLINE;
    else if (name == "polygon")  poly = GI_POLYGON;
    else if (name == "bezier")   poly = GI_BEZIER;   // path = p0 c1 c2 p1 [c1 c2 p2…]; Polyline::draw agrupa de 3 en 3 → curveto
    else isPoly = false;

    if (isPoly) {
      // §4.1: closed=true cierra cada subtrayecto (closepath) sin repetir el
      // punto inicial. Default false = polilínea abierta. En polygon es redundante
      // (el relleno ya cierra); solo cambia algo si se pide explícitamente.
      bool closed = named.count("closed") && named.at("closed")->eval(s).num != 0.0;
      size_t start = 0;
      auto flush = [&](size_t end) {
        Path path = evalPath(s, start, end);
        start = end;
        if (path.empty()) return;
        auto p = std::make_unique<Polyline>(poly);
        p->setPath(std::move(path));
        p->setClosed(closed);
        items.push_back(std::move(p));
      };
      for (size_t b : breaks) flush(b);
      flush(coords.size());
    } else {
      Path path = evalPath(s, 0, coords.size());
      std::unique_ptr<GraphicsItem> item;
      if      (name == "rectangle") { auto p = std::make_unique<Rectangle>(); p->setPath(path); item = std::move(p); }
      else if (name == "dot") { auto p = std::make_unique<Dot>(); p->setRadius(posOr(s, 0, 1)); p->setPath(path); item = std::move(p); }
      else if (name == "circle") { auto p = std::make_unique<Arc>(); double r = posOr(s, 0, 1); p->setRadius(r, r); p->setAngles(0, 360); p->setPath(path); item = std::move(p); }
      else if (name == "ellipse") { auto p = std::make_unique<Arc>(); double rx = posOr(s, 0, 1), ry = posOr(s, 1, rx); if (rx != ry) g_flags.using_ellipse = true; p->setRadius(rx, ry); p->setAngles(0, 360); p->setPath(path); item = std::move(p); }
      else if (name == "arc") { auto p = std::make_unique<Arc>(); double r = posOr(s, 0, 1); p->setRadius(r, r); p->setAngles(namedOr(s, "from", 0), namedOr(s, "to", 360)); p->setPath(path); item = std::move(p); }
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
    if (name == "dot") {
      if (named.count("color") && !named.count("fill")) {
        attrs.push_back(std::make_unique<GraphicsState>(GS_NOFILL));
      } else if (!named.count("fill")) {
        auto a = std::make_unique<Attribute>(); a->set(AT_FCOLOR, 0); attrs.push_back(std::move(a));   // negro
        attrs.push_back(std::make_unique<GraphicsState>(GS_FILL));
      }
    }
    if (attrs.empty()) {
      for (auto &it : items) out.push_back(std::move(it));
      return;
    }
    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
    for (auto &a : attrs) out.push_back(std::move(a));
    for (auto &it : items) out.push_back(std::move(it));
    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

// Texto (§4.8): text("s") { p1 p2 … } estampa la cadena en cada coordenada.
// El contenido pasa por parse_text (mismo markup que V1); una llamada por punto
// porque el GraphicsItem resultante es no-copiable (siembra múltiple, §4).
struct TextStmt : Stmt {
  ExprPtr content;
  std::map<std::string, ExprPtr> named;   // §7.5: estilo por-primitiva (font_size/size, font, align, valign, color)
  std::vector<ExprPtr> coords;            // coordenadas (Term), en pares x y
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
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
struct NumbersStmt : Stmt {
  std::map<std::string, ExprPtr> named;
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
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
      std::snprintf(num, sizeof num, fmt, from + i * by);
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
struct AxisStmt : Stmt {
  std::map<std::string, ExprPtr> named;
  std::vector<ExprPtr> coords;              // 2 puntos: p1 p2
  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &out) override {
    if (coords.size() < 4) return;
    point p1(coords[0]->eval(s).num, coords[1]->eval(s).num);
    point p2(coords[2]->eval(s).num, coords[3]->eval(s).num);
    double from = namedNum(s, named, "from", 0), to = namedNum(s, named, "to", 1);
    double step = namedNum(s, named, "step", 1);
    double startv = named.count("start") ? namedNum(s, named, "start", from) : from;
    std::string tdir = namedStr(s, named, "ticks"); if (tdir.empty()) tdir = "out";
    bool labels = named.count("labels") ? namedNum(s, named, "labels", 1) != 0.0 : true;
    int decimals = (int)namedNum(s, named, "decimals", 0);
    std::string title = namedStr(s, named, "title");
    double titleSize = namedNum(s, named, "title_size", 0);
    double tickSize = namedNum(s, named, "tick_size", 3.0);   // pt
    double labelGap = namedNum(s, named, "label_gap", 8.0);   // pt

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
      double t = (to == from) ? 0 : (v - from) / (to - from);
      return point(p1.x + t * dx, p1.y + t * dy);
    };
    const double eps = 1e-9;

    // 1. línea del eje
    { auto pl = std::make_unique<Polyline>(GI_POLYLINE); Path pp;
      pp.push_back(p1); pp.push_back(p2); pl->setPath(pp); out.push_back(std::move(pl)); }

    // 2. marcas
    if (tdir == "grid") {                     // barren el campo perpendicular (ventana o field=)
      double field = named.count("field") ? namedNum(s, named, "field", 0) : (horiz ? wdy : wdx);
      double base  = horiz ? wy : wx;
      Path pp; pp.push_back(horiz ? point(0, field) : point(field, 0));
      for (double v = startv; v <= to + eps; v += step) {
        point q = posOf(v); pp.push_back(horiz ? point(q.x, base) : point(base, q.y));
      }
      auto pl = std::make_unique<Polyline>(GI_TICKS); pl->setPath(pp); out.push_back(std::move(pl));
    } else if (tdir != "none") {
      point o = physOut(tickSize), mark = o, off(0, 0);
      if (tdir == "in")        mark = point(-o.x, -o.y);
      else if (tdir == "both") { off = point(-o.x, -o.y); mark = point(2 * o.x, 2 * o.y); }
      Path pp; pp.push_back(mark);
      for (double v = startv; v <= to + eps; v += step) {
        point q = posOf(v); pp.push_back(point(q.x + off.x, q.y + off.y));
      }
      auto pl = std::make_unique<Polyline>(GI_TICKS); pl->setPath(pp); out.push_back(std::move(pl));
    }

    // 3. etiquetas numéricas (heredan el estado de texto vigente, §14)
    if (labels) {
      point g = physOut(labelGap);
      char fmt[8]; std::snprintf(fmt, sizeof fmt, "%%.%df", decimals < 0 ? 0 : decimals);
      for (double v = startv; v <= to + eps; v += step) {
        point q = posOf(v); char num[64]; std::snprintf(num, sizeof num, fmt, v);
        auto gs = std::make_unique<GraphicsState>(); gs->setPosition(point(q.x + g.x, q.y + g.y));
        out.push_back(std::move(gs));
        out.push_back(parse_text(num, FN_NOFACE, g_flags.using_reencode, g_flags.using_fontcmmi));  // hereda font ambiente
      }
    }

    // 4. título. Eje vertical → texto girado con el ángulo de la línea (MTLC),
    //    acotado con save/restore del dispositivo para cerrar el grupo de rotación.
    if (!title.empty()) {
      point m(p1.x + 0.5 * dx, p1.y + 0.5 * dy);
      point t = physOut(labelGap * 3.0);
      point tp(m.x + t.x, m.y + t.y);
      double ang = horiz ? 0.0 : std::atan2(uy, ux) * 57.29577951308232;   // rad→grados
      std::string titleFont = namedStr(s, named, "title_font");
      // Cara del título: title_font la hornea en el Text (override propio, §13.5);
      // sin él, FN_NOFACE → hereda la ambiente. title_size se aísla con push/pop.
      FontFace tff = titleFont.empty() ? FN_NOFACE
                     : get_font_face_from_string(titleFont, g_flags.using_fontcmmi);
      bool wrap = titleSize > 0;
      if (wrap) {
        out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));
        auto a = std::make_unique<Attribute>(); a->set(AT_THEIGHT, (int)titleSize); out.push_back(std::move(a));
      }
      if (ang != 0.0) out.push_back(std::make_unique<GraphicsState>(GS_DEVSAVE));
      auto gs = std::make_unique<GraphicsState>(); gs->setPosition(tp); out.push_back(std::move(gs));
      if (ang != 0.0) {
        auto tr = std::make_unique<Transform>();
        tr->setOperation(OPMRT); tr->setRotation(ang); tr->setPredefinedMatrix(MTLC);
        out.push_back(std::move(tr));
      }
      out.push_back(parse_text(title, tff, g_flags.using_reencode, g_flags.using_fontcmmi));
      if (ang != 0.0) out.push_back(std::make_unique<GraphicsState>(GS_DEVRESTORE));
      if (wrap) out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
    }
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

static bool isConfig(const std::string &n) { return n == "display_size" || n == "world_window"; }
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
         n == "dot" || n == "arc" || n == "ellipse" || n == "bezier";
}

static StmtPtr parseBlock(Lexer &);        // adelantadas (mutuamente recursivas)
static StmtPtr parseStructDef(Lexer &);
static StmtPtr parseFor(Lexer &);
static StmtPtr parseIf(Lexer &);
static StmtPtr parseRepeat(Lexer &);
static StmtPtr parseFit(Lexer &);
static StmtPtr parsePlace(Lexer &);
static StmtPtr parseSine(Lexer &);
static StmtPtr parseInclude(Lexer &);
static StmtPtr parseInvoke(Lexer &, const std::string &);

// Un nombre de atributo puede coincidir con una palabra de control: `to`/`step`
// son keywords (for i = a to b) pero también nombres nombrados (arc(from=, to=)).
// Dentro de (…) se desambigua por contexto: keyword seguida de '=' = nombre.
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

static StmtPtr parseStatement(Lexer &lx) {
  const Tok &t = lx.peek();
  if (t.type == T_LBRACE) return parseBlock(lx);      // bloque anónimo (§7.1)
  if (t.type == T_STRUCT) return parseStructDef(lx);  // struct Nombre(...) { … }
  if (t.type == T_FOR)    return parseFor(lx);        // for v = a to b [step s] { … }
  if (t.type == T_IF)     return parseIf(lx);         // if cond { … } [else { … }]
  if (t.type == T_INCLUDE) return parseInclude(lx);   // include "archivo.mg" (§15)
  if (t.type != T_IDENTIFIER) parseError(lx, "un comando, asignación, struct o '{'");
  std::string name = t.str;

  if (lx.peek(1).type == T_ASSIGN) {          // asignación: ID = Expr
    lx.next(); lx.next();
    auto st = std::make_unique<AssignStmt>();
    st->name = name;
    st->val = parseExpression(lx);
    return st;
  }
  lx.next();                                   // consume el nombre del comando

  if (isConfig(name)) {
    auto st = std::make_unique<ConfigStmt>();
    st->which = name;
    int n = (name == "display_size") ? 2 : 4;   // world_window: 4 valores
    for (int i = 0; i < n; i++) st->args.push_back(parseTerm(lx));   // valores por espacio -> Term
    std::string k;                              // world_window ... stretch=true (§13.7)
    if (attrNameHere(lx, k) && k == "stretch") { lx.next(); lx.next(); st->stretchE = parseExpression(lx); }
    return st;
  }
  if (isPrim(name)) {
    auto st = std::make_unique<PrimStmt>();
    st->name = name;
    if (lx.accept(T_LPAREN)) parseArgList(lx, st->pos, st->named);   // args (Expression)
    if (lx.accept(T_LBRACE)) {                  // { coords }  (Term; ';' abre subtrayecto §4; newline se salta)
      while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
        if (lx.accept(T_SEMICOLON)) { st->breaks.push_back(st->coords.size()); continue; }
        if (lx.accept(T_NEWLINE)) continue;
        st->coords.push_back(parseTerm(lx));
      }
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
      st->args.push_back(parseTerm(lx));
      int t = lx.peek().type;
      if (t == T_NUMBER || t == T_MINUS) st->args.push_back(parseTerm(lx));
    } else {
      int arity = (top == OPMRT) ? 1 : 2;      // rotate=1; translate/shear=2
      for (int i = 0; i < arity; i++) st->args.push_back(parseTerm(lx));
    }
    return st;
  }

  if (lx.peek().type == T_LPAREN) return parseInvoke(lx, name);   // invocación: Nombre( … )

  // Resto: sentencia de estado (color, line_width, …), argumentos hasta el
  // fin de línea (T_NEWLINE). '}' también la corta (estado dentro de bloque).
  auto st = std::make_unique<StateStmt>();
  st->name = name;
  while (lx.peek().type != T_NEWLINE && lx.peek().type != T_EOF && lx.peek().type != T_RBRACE) {
    SArg a;
    if (lx.peek().type == T_STRING) { a.isStr = true; a.str = lx.next().str; }
    else { a.isStr = false; a.num = parseTerm(lx); }
    st->args.push_back(std::move(a));
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

static StmtPtr parseStructDef(Lexer &lx) {
  lx.next();                                   // 'struct'
  if (lx.peek().type != T_IDENTIFIER) parseError(lx, "el nombre de la struct");
  auto def = std::make_unique<StructDef>();
  def->name = lx.next().str;
  if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras el nombre de la struct");
  if (lx.peek().type != T_RPAREN) {            // parámetros: ID [= default]
    do {
      if (lx.peek().type != T_IDENTIFIER) parseError(lx, "un nombre de parámetro");
      def->params.push_back(lx.next().str);
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
  if (lx.peek().type != T_IDENTIFIER) parseError(lx, "el nombre de la struct a ajustar");
  st->structName = lx.next().str;
  while (lx.accept(T_COMMA)) {                   // argumentos nombrados (stretch=…)
    std::string k;
    if (!attrNameHere(lx, k)) parseError(lx, "un argumento nombrado (stretch=)");
    lx.next(); lx.next();
    ExprPtr v = parseExpression(lx);
    if (k == "stretch") st->stretchE = std::move(v);
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
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
  return st;
}

static StmtPtr parseSine(Lexer &lx) {
  if (!lx.accept(T_LPAREN)) parseError(lx, "'(' tras sine");
  auto st = std::make_unique<SineStmt>();
  if (lx.peek().type != T_RPAREN) {             // args nombrados: half_cycles=, amplitude=, …
    do {
      std::string k;
      if (!attrNameHere(lx, k)) parseError(lx, "un argumento nombrado (half_cycles=, amplitude=, …)");
      lx.next(); lx.next();
      st->named[k] = parseExpression(lx);
    } while (lx.accept(T_COMMA));
  }
  if (!lx.accept(T_RPAREN)) parseError(lx, "')'");
  if (!lx.accept(T_LBRACE)) parseError(lx, "'{' de la base de sine");
  while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
    if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
    st->coords.push_back(parseTerm(lx));
  }
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
  return st;
}

static StmtPtr parseInvoke(Lexer &lx, const std::string &name) {
  auto st = std::make_unique<InvokeStmt>();
  st->name = name;
  lx.next();                                   // consume '('
  parseArgList(lx, st->pos, st->named);
  return st;
}

// Parsea un flujo ya tokenizado en una lista de sentencias (sin ejecutar).
static std::vector<StmtPtr> parseProgram(Lexer &lx) {
  std::vector<StmtPtr> prog;
  while (lx.peek().type != T_EOF) {
    if (lx.accept(T_NEWLINE)) continue;          // saltar líneas en blanco/separadores
    prog.push_back(parseStatement(lx));
  }
  return prog;
}

// Lee un archivo completo, lo tokeniza y lo parsea (para include). El Lexer es
// local: las sentencias copian sus lexemas, así que puede destruirse al salir.
static std::vector<StmtPtr> parseFile(const std::string &path) {
  std::ifstream in(path);
  if (!in) { std::fprintf(stderr, "include: no se pudo abrir %s\n", path.c_str()); return {}; }
  std::stringstream ss;
  ss << in.rdbuf();
  Lexer lx;
  lx.tokenize(ss.str().c_str());
  return parseProgram(lx);
}

static StmtPtr parseInclude(Lexer &lx) {
  lx.next();                                     // 'include'
  if (lx.peek().type != T_STRING) parseError(lx, "el nombre de archivo (cadena) tras include");
  std::string fname = lx.next().str;
  std::string path = g_baseDir.empty() ? fname : g_baseDir + "/" + fname;
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
