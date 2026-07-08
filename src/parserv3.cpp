// Parser V3 (descenso recursivo). Slice 1: expresiones (§5).
// Nombre distinto de Parser.cpp (V1) para convivir en cualquier filesystem.
// El V1 sigue siendo el motor activo (bin/mg + red golden) hasta el cutover;
// entonces se retira Parser.cpp y, si se quiere, este archivo pasa a Parser.cpp.

#include <cstdio>
#include <cstdlib>
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

// Banderas de uso recogidas durante la evaluación (parse_text las activa); se
// vuelcan al Display antes de dibujar, igual que main.cpp V1 (g.flags = parser.flags).
static MGFlags g_flags;

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

struct Stmt {
  virtual ~Stmt() {}
  virtual void exec(Scope &, MetaGrafica &, GraphicsItemList &) = 0;
};
using StmtPtr = std::unique_ptr<Stmt>;

// Argumento de sentencia de estado: número (Term) o cadena (color).
struct SArg { bool isStr = false; ExprPtr num; std::string str; };

struct StateStmt : Stmt {
  std::string name;
  std::vector<SArg> args;
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    if (name == "color" && !args.empty() && args[0].isStr) {
      if (args[0].str == "none") return;                 // sin trazo: pendiente
      auto a = std::make_unique<Attribute>();
      a->set(AT_LCOLOR, get_color(args[0].str));
      out.push_back(std::move(a));
    } else if (name == "fill" && !args.empty() && args[0].isStr) {
      if (args[0].str == "none") { out.push_back(std::make_unique<GraphicsState>(GS_NOFILL)); return; }
      auto a = std::make_unique<Attribute>();
      a->set(AT_FCOLOR, get_color(args[0].str));
      out.push_back(std::move(a));
      out.push_back(std::make_unique<GraphicsState>(GS_FILL));   // activar relleno (§4)
    } else if (name == "line_width" && !args.empty() && !args[0].isStr) {
      auto a = std::make_unique<Attribute>();
      a->setF(AT_LWIDTH, args[0].num->eval(s).num);      // pt directo (§4.10)
      out.push_back(std::move(a));
    }
    // dash/hatch/font/transform… → slice posterior (se ignoran por ahora)
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
  void exec(Scope &s, MetaGrafica &mg, GraphicsItemList &) override {
    auto n = [&](size_t i) { return args[i]->eval(s).num; };
    if (which == "display_size")      mg.setDimension(n(0), n(1));
    else if (which == "world_window") mg.setWindow(n(0), n(2), n(1) - n(0), n(3) - n(2));
    else if (which == "font_size")    mg.setFontSize(n(0));
  }
};

// Directorio del archivo principal (para resolver includes relativos).
static std::string g_baseDir;

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
    { auto t = std::make_unique<Transform>(); t->setOperation(OPMPOP); out.push_back(std::move(t)); }
    out.push_back(std::make_unique<GraphicsState>(GS_POPSTATE));
  }
};

struct PrimStmt : Stmt {
  std::string name;
  std::vector<ExprPtr> pos;               // args posicionales
  std::map<std::string, ExprPtr> named;   // args nombrados
  std::vector<ExprPtr> coords;            // coordenadas (Term), en pares x y

  double namedOr(Scope &s, const char *k, double def) const {
    auto it = named.find(k);
    return it == named.end() ? def : it->second->eval(s).num;
  }
  double posOr(Scope &s, size_t i, double def) const {
    return i < pos.size() ? pos[i]->eval(s).num : def;
  }

  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    Path path;
    for (size_t i = 0; i + 1 < coords.size(); i += 2)
      path.push_back(point(coords[i]->eval(s).num, coords[i + 1]->eval(s).num));

    std::unique_ptr<GraphicsItem> item;
    if (name == "polyline") { auto p = std::make_unique<Polyline>(GI_POLYLINE); p->setPath(path); item = std::move(p); }
    else if (name == "polygon") { auto p = std::make_unique<Polyline>(GI_POLYGON); p->setPath(path); item = std::move(p); }
    else if (name == "bezier") { auto p = std::make_unique<Polyline>(GI_BEZIER); p->setPath(path); item = std::move(p); }   // path = p0 c1 c2 p1 [c1 c2 p2…]; Polyline::draw agrupa de 3 en 3 → curveto
    else if (name == "rectangle") { auto p = std::make_unique<Rectangle>(); p->setPath(path); item = std::move(p); }
    else if (name == "dot") { auto p = std::make_unique<Dot>(); p->setRadius(posOr(s, 0, 1)); p->setPath(path); item = std::move(p); }
    else if (name == "circle") { auto p = std::make_unique<Arc>(); double r = posOr(s, 0, 1); p->setRadius(r, r); p->setAngles(0, 360); p->setPath(path); item = std::move(p); }
    else if (name == "ellipse") { auto p = std::make_unique<Arc>(); double rx = posOr(s, 0, 1), ry = posOr(s, 1, rx); p->setRadius(rx, ry); p->setAngles(0, 360); p->setPath(path); item = std::move(p); }
    else if (name == "arc") { auto p = std::make_unique<Arc>(); double r = posOr(s, 0, 1); p->setRadius(r, r); p->setAngles(namedOr(s, "from", 0), namedOr(s, "to", 360)); p->setPath(path); item = std::move(p); }
    if (item) out.push_back(std::move(item));
  }
};

// Texto (§4.8): text("s") { p1 p2 … } estampa la cadena en cada coordenada.
// El contenido pasa por parse_text (mismo markup que V1); una llamada por punto
// porque el GraphicsItem resultante es no-copiable (siembra múltiple, §4).
struct TextStmt : Stmt {
  ExprPtr content;
  std::map<std::string, ExprPtr> named;   // align/font… → slice posterior (ignorados)
  std::vector<ExprPtr> coords;            // coordenadas (Term), en pares x y
  void exec(Scope &s, MetaGrafica &, GraphicsItemList &out) override {
    Value c = content->eval(s);
    std::string str;
    if (c.type == Value::STRING) str = c.str;
    else { char buf[64]; std::snprintf(buf, sizeof buf, "%g", c.num); str = buf; }
    for (size_t i = 0; i + 1 < coords.size(); i += 2) {
      point p(coords[i]->eval(s).num, coords[i + 1]->eval(s).num);
      auto gs = std::make_unique<GraphicsState>();
      gs->setPosition(p);                                  // GS_PLUMEPOSITION
      out.push_back(std::move(gs));
      out.push_back(parse_text(str, FN_DEFAULT, g_flags.using_reencode, g_flags.using_fontcmmi));
    }
  }
};

static bool isConfig(const std::string &n) { return n == "display_size" || n == "world_window" || n == "font_size"; }
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
    int n = (name == "display_size") ? 2 : (name == "world_window") ? 4 : 1;
    for (int i = 0; i < n; i++) st->args.push_back(parseTerm(lx));   // valores por espacio -> Term
    return st;
  }
  if (isPrim(name)) {
    auto st = std::make_unique<PrimStmt>();
    st->name = name;
    if (lx.accept(T_LPAREN)) parseArgList(lx, st->pos, st->named);   // args (Expression)
    if (lx.accept(T_LBRACE)) {                  // { coords }  (Term; ';' subtrayecto ign. en slice 2; newline se salta)
      while (lx.peek().type != T_RBRACE && lx.peek().type != T_EOF) {
        if (lx.accept(T_SEMICOLON) || lx.accept(T_NEWLINE)) continue;
        st->coords.push_back(parseTerm(lx));
      }
      if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
    }
    return st;
  }
  if (name == "repeat") return parseRepeat(lx);   // repeat(Struct, count=…, …) (§17)
  if (name == "fit")    return parseFit(lx);      // fit(Struct[, stretch=]) { rect } (§10.2)
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

// --- Driver: fuente V3 -> MetaGrafica ---------------------------------------
static MetaGrafica *buildFromSource(const std::string &src) {
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

int main(int argc, char **argv) {
  if (argc < 3) {
    std::fprintf(stderr, "uso: %s entrada.mg salida.(svg|eps)\n", argv[0]);
    return 1;
  }
  std::ifstream in(argv[1]);
  if (!in) { std::fprintf(stderr, "no se pudo abrir %s\n", argv[1]); return 1; }
  std::stringstream ss;
  ss << in.rdbuf();

  // Directorio del archivo principal: resuelve los include relativos (§15).
  std::string inpath = argv[1];
  size_t slash = inpath.find_last_of('/');
  if (slash != std::string::npos) g_baseDir = inpath.substr(0, slash);

  std::unique_ptr<MetaGrafica> mg(buildFromSource(ss.str()));

  std::string out = argv[2];
  bool svg = out.size() > 4 && out.compare(out.size() - 4, 4, ".svg") == 0;
  if (svg) { SVGDisplay g(out); g.flags = g_flags; mg->draw(g); }
  else     { EPSDisplay g(out); g.flags = g_flags; mg->draw(g); }
  std::printf("render -> %s\n", out.c_str());
  return 0;
}
