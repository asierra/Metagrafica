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
#include "SVGDisplay.h"
#include "EPSDisplay.h"

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

static ExprPtr parseExpression(Lexer &lx) {   // Term (("+"|"-") Term)*
  ExprPtr e = parseTerm(lx);
  while (lx.peek().type == T_PLUS || lx.peek().type == T_MINUS) {
    int op = lx.next().type;
    e = ExprPtr(new BinExpr(op, std::move(e), parseTerm(lx)));
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
};
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
    out.push_back(std::make_unique<GraphicsState>(GS_PUSHSTATE));  // aísla estado (como structure())
    for (auto &st : def->body) st->exec(local, mg, out);
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
    else if (name == "rectangle") { auto p = std::make_unique<Rectangle>(); p->setPath(path); item = std::move(p); }
    else if (name == "dot") { auto p = std::make_unique<Dot>(); p->setRadius(posOr(s, 0, 1)); p->setPath(path); item = std::move(p); }
    else if (name == "circle") { auto p = std::make_unique<Arc>(); double r = posOr(s, 0, 1); p->setRadius(r, r); p->setAngles(0, 360); p->setPath(path); item = std::move(p); }
    else if (name == "ellipse") { auto p = std::make_unique<Arc>(); double rx = posOr(s, 0, 1), ry = posOr(s, 1, rx); p->setRadius(rx, ry); p->setAngles(0, 360); p->setPath(path); item = std::move(p); }
    else if (name == "arc") { auto p = std::make_unique<Arc>(); double r = posOr(s, 0, 1); p->setRadius(r, r); p->setAngles(namedOr(s, "from", 0), namedOr(s, "to", 360)); p->setPath(path); item = std::move(p); }
    if (item) out.push_back(std::move(item));
  }
};

static bool isConfig(const std::string &n) { return n == "display_size" || n == "world_window" || n == "font_size"; }
static bool isPrim(const std::string &n) {
  return n == "polyline" || n == "polygon" || n == "circle" || n == "rectangle" ||
         n == "dot" || n == "arc" || n == "ellipse";
}

static StmtPtr parseBlock(Lexer &);        // adelantadas (mutuamente recursivas)
static StmtPtr parseStructDef(Lexer &);
static StmtPtr parseInvoke(Lexer &, const std::string &);

// Lista de args ( … ): posicionales (Expression) y nombrados (ID = Expression).
// Asume que '(' ya se consumió; consume hasta ')'.
static void parseArgList(Lexer &lx, std::vector<ExprPtr> &pos,
                         std::map<std::string, ExprPtr> &named) {
  if (lx.peek().type != T_RPAREN) {
    do {
      if (lx.peek().type == T_IDENTIFIER && lx.peek(1).type == T_ASSIGN) {
        std::string k = lx.next().str; lx.next();
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
    def->body.push_back(parseStatement(lx));
  }
  if (!lx.accept(T_RBRACE)) parseError(lx, "'}'");
  auto stmt = std::make_unique<StructDefStmt>();
  stmt->def = std::move(def);
  return stmt;
}

static StmtPtr parseInvoke(Lexer &lx, const std::string &name) {
  auto st = std::make_unique<InvokeStmt>();
  st->name = name;
  lx.next();                                   // consume '('
  parseArgList(lx, st->pos, st->named);
  return st;
}

// --- Driver: fuente V3 -> MetaGrafica ---------------------------------------
static MetaGrafica *buildFromSource(const std::string &src) {
  Lexer lx;
  lx.tokenize(src.c_str());
  auto *mg = new MetaGrafica();
  mg->setName("mgv3");
  mg->setDepth(64);
  Scope scope;
  GraphicsItemList items;
  while (lx.peek().type != T_EOF) {
    if (lx.accept(T_NEWLINE)) continue;          // saltar líneas en blanco/separadores
    StmtPtr st = parseStatement(lx);
    st->exec(scope, *mg, items);
  }
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

  std::unique_ptr<MetaGrafica> mg(buildFromSource(ss.str()));

  std::string out = argv[2];
  bool svg = out.size() > 4 && out.compare(out.size() - 4, 4, ".svg") == 0;
  if (svg) { SVGDisplay g(out); mg->draw(g); }
  else     { EPSDisplay g(out); mg->draw(g); }
  std::printf("render -> %s\n", out.c_str());
  return 0;
}
