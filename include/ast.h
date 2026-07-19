#ifndef AST_H
#define AST_H

// AST y evaluador del parser V3. Slice 1: expresiones (§5).
// Estilo ortodoxo: sin RTTI/excepciones; unique_ptr para hijos, virtual eval.

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <cmath>
#include <cstdio>
#include <cstdlib>   // std::exit (evalError es fatal)

#include "tokens.h"   // códigos de operador (T_PLUS, T_MINUS, …)

// --- Valor V3: número, cadena o lista (una tupla es una lista de 2) ---------
struct Value {
  enum Type { NUMBER, STRING, LIST } type = NUMBER;
  double num = 0.0;
  std::string str;
  std::vector<Value> items;

  Value() {}
  Value(double n) : type(NUMBER), num(n) {}
  Value(const std::string &s) : type(STRING), str(s) {}
};

// Representación textual de un Value, para la concatenación con '+' (§6.1) y el
// estampado de text(). Un número se formatea con %g (entero sin punto, decimales
// mínimos), igual que TextStmt cuando el contenido evalúa a número.
inline std::string valueToStr(const Value &v) {
  if (v.type == Value::STRING) return v.str;
  char buf[64];
  std::snprintf(buf, sizeof buf, "%g", v.num);
  return buf;
}

// Error de evaluación: FATAL (sin excepciones → aborta con código 1).
//
// Antes solo imprimía y seguía con 0, así que el documento roto llegaba a la
// salida —p.ej. `-nan -nan moveto` en el EPS— y `mg` terminaba con código 0: el
// usuario recibía un archivo inválido junto con un "todo bien". Abortar aquí es
// seguro y no deja archivo a medias: main corre buildFromSource() entero (parse
// + exec) ANTES de construir el Display y abrir la salida.
//
// Para condiciones con respaldo sensato (color desconocido → negro) está `warn`,
// que sí continúa; los sitios que llaman a evalError son errores duros.
inline Value evalError(const char *msg, const std::string &extra = "") {
  std::fprintf(stderr, "Error de evaluación: %s%s\n", msg, extra.c_str());
  std::exit(1);
  return Value(0.0);                  // inalcanzable; conserva el tipo de retorno
}

// Aviso NO fatal: imprime y sigue, sin alterar la semántica (a diferencia de
// evalError). Para casos con respaldo sensato (p.ej. color desconocido → negro);
// la generación de salida continúa normalmente.
inline void warn(const char *msg, const std::string &extra = "") {
  std::fprintf(stderr, "Aviso: %s%s\n", msg, extra.c_str());
}

// Adelantadas: el álgebra de paths (§9, PathExpr) vive en parserv3.cpp, fuera
// de esta jerarquía (un Path no es un Value, ver el comentario allá). Scope
// solo necesita ligar {expr, ámbito del llamador} sin conocer su AST completo.
struct PathExpr;
struct Scope;

// Ligadura de un parámetro path de struct (§8.x: `struct Nivel(&onda, w)`).
// `scope` es el ámbito del LLAMADOR, no el de la struct que declara el
// parámetro: PathRef::evalPath reenvía el ámbito VIGENTE, así que sin guardar
// el del llamador aparte, un path pasado como argumento resolvería sus propias
// referencias (p.ej. un `sine(amplitude=a)` con `a` local del llamador) en el
// ámbito equivocado. El PathExpr no muta ni se posee aquí: vive en g_paths o
// en el AST de quien invocó (InvokeStmt/FitStmt), y ese nodo sobrevive a la
// ejecución del cuerpo (dueño del marco de exec del llamador).
struct PathBinding {
  const PathExpr *expr = nullptr;
  Scope *scope = nullptr;
};

// --- Ámbito léxico: variables encadenadas al padre --------------------------
struct Scope {
  std::map<std::string, Value> vars;
  std::map<std::string, PathBinding> pathBindings;   // parámetros &onda (§8.x)
  Scope *parent = nullptr;
  explicit Scope(Scope *p = nullptr) : parent(p) {}

  Value *find(const std::string &name) {
    for (Scope *s = this; s; s = s->parent) {
      auto it = s->vars.find(name);
      if (it != s->vars.end()) return &it->second;
    }
    return nullptr;
  }

  PathBinding *findPath(const std::string &name) {
    for (Scope *s = this; s; s = s->parent) {
      auto it = s->pathBindings.find(name);
      if (it != s->pathBindings.end()) return &it->second;
    }
    return nullptr;
  }

  // Ámbito global (raíz de la cadena): el cuerpo de una struct es hijo de éste
  // (ámbito léxico: ve las variables del archivo, no las locales del llamador).
  Scope *root() {
    Scope *s = this;
    while (s->parent) s = s->parent;
    return s;
  }
};

// --- Nodos de expresión -----------------------------------------------------
struct Expr {
  int line = 0, col = 0;
  virtual ~Expr() {}
  virtual Value eval(Scope &) const = 0;
};
using ExprPtr = std::unique_ptr<Expr>;

struct NumExpr : Expr {
  double v;
  explicit NumExpr(double x) : v(x) {}
  Value eval(Scope &) const override { return Value(v); }
};

struct StrExpr : Expr {
  std::string v;
  explicit StrExpr(std::string s) : v(std::move(s)) {}
  Value eval(Scope &) const override { return Value(v); }
};

struct IdentExpr : Expr {
  std::string name;
  explicit IdentExpr(std::string n) : name(std::move(n)) {}
  Value eval(Scope &sc) const override {
    if (name == "pi") return Value(M_PI);
    if (name == "true")  return Value(1.0);      // booleanos como número (§6.1)
    if (name == "false") return Value(0.0);
    if (Value *v = sc.find(name)) return *v;
    return evalError("variable no definida: ", name);
  }
};

struct UnExpr : Expr {           // solo '-' unario (§2)
  ExprPtr e;
  explicit UnExpr(ExprPtr x) : e(std::move(x)) {}
  Value eval(Scope &sc) const override {
    Value a = e->eval(sc);
    return Value(-a.num);
  }
};

// `not` lógico (§6.1). Aparte de UnExpr, que es el '-' aritmético: la verdad aquí
// es la misma convención que usan T_AND/T_OR y la condición de `if` —cero es falso,
// cualquier otra cosa verdadero— y el resultado se normaliza a 1.0/0.0.
struct NotExpr : Expr {
  ExprPtr e;
  explicit NotExpr(ExprPtr x) : e(std::move(x)) {}
  Value eval(Scope &sc) const override {
    return Value(e->eval(sc).num == 0.0 ? 1.0 : 0.0);
  }
};

struct BinExpr : Expr {          // + - * / ^   y comparación/lógicos (§6.1)
  int op;
  ExprPtr l, r;
  BinExpr(int o, ExprPtr a, ExprPtr b) : op(o), l(std::move(a)), r(std::move(b)) {}
  Value eval(Scope &sc) const override {
    // and/or con cortocircuito: no evalúa la rama derecha si no hace falta
    // (permite guardas como `i < len(a) and a[i] > 0`). Booleano = número !=0.
    if (op == T_AND) return Value(l->eval(sc).num != 0.0 && r->eval(sc).num != 0.0 ? 1.0 : 0.0);
    if (op == T_OR)  return Value(l->eval(sc).num != 0.0 || r->eval(sc).num != 0.0 ? 1.0 : 0.0);
    Value lv = l->eval(sc), rv = r->eval(sc);
    // '+' con al menos un operando de cadena = concatenación (§6.1); el número se
    // formatea con %g. Los demás operadores son numéricos.
    if (op == T_PLUS && (lv.type == Value::STRING || rv.type == Value::STRING))
      return Value(valueToStr(lv) + valueToStr(rv));
    double a = lv.num, b = rv.num;
    switch (op) {
      case T_PLUS:  return Value(a + b);
      case T_MINUS: return Value(a - b);
      case T_STAR:  return Value(a * b);
      case T_SLASH: return Value(a / b);
      case T_CARET: return Value(std::pow(a, b));
      case T_EQ:    return Value(a == b ? 1.0 : 0.0);
      case T_NE:    return Value(a != b ? 1.0 : 0.0);
      case T_LT:    return Value(a <  b ? 1.0 : 0.0);
      case T_GT:    return Value(a >  b ? 1.0 : 0.0);
      case T_LE:    return Value(a <= b ? 1.0 : 0.0);
      case T_GE:    return Value(a >= b ? 1.0 : 0.0);
    }
    return evalError("operador binario desconocido");
  }
};

struct IndexExpr : Expr {        // lista[i]  (base 0)
  std::string name;
  ExprPtr idx;
  IndexExpr(std::string n, ExprPtr i) : name(std::move(n)), idx(std::move(i)) {}
  Value eval(Scope &sc) const override {
    Value *v = sc.find(name);
    if (!v || v->type != Value::LIST) return evalError("no es una lista: ", name);
    int i = (int)idx->eval(sc).num;
    if (i < 0 || i >= (int)v->items.size()) return evalError("índice fuera de rango en ", name);
    return v->items[i];
  }
};

struct ArrayExpr : Expr {        // [ a, b, c ]
  std::vector<ExprPtr> elems;
  Value eval(Scope &sc) const override {
    Value out;
    out.type = Value::LIST;
    for (auto &e : elems) out.items.push_back(e->eval(sc));
    return out;
  }
};

struct CallExpr : Expr {         // función builtin: sin(x), mod(a,b), len(l)…
  std::string fn;
  std::vector<ExprPtr> args;
  explicit CallExpr(std::string f) : fn(std::move(f)) {}
  Value eval(Scope &sc) const override {
    std::vector<Value> a;
    for (auto &e : args) a.push_back(e->eval(sc));
    auto need = [&](size_t n) { return a.size() == n; };
    if (fn == "sin"   && need(1)) return Value(std::sin(a[0].num));
    if (fn == "cos"   && need(1)) return Value(std::cos(a[0].num));
    if (fn == "tan"   && need(1)) return Value(std::tan(a[0].num));
    if (fn == "sqrt"  && need(1)) return Value(std::sqrt(a[0].num));
    if (fn == "abs"   && need(1)) return Value(std::fabs(a[0].num));
    if (fn == "atan2" && need(2)) return Value(std::atan2(a[0].num, a[1].num));
    // exp/ln (§5.2): entran porque el corpus las exige — un potencial de Morse,
    // D(1-exp(-a(r-re)))², no es escribible sin ellas, y con ellas sus puntos de
    // retorno salen en forma cerrada en vez de medirse sobre la curva dibujada.
    if (fn == "exp"   && need(1)) return Value(std::exp(a[0].num));
    if (fn == "ln"    && need(1)) {
      if (a[0].num <= 0)
        return evalError("ln: argumento no positivo: ", std::to_string(a[0].num));
      return Value(std::log(a[0].num));
    }
    if (fn == "mod"   && need(2)) return Value(std::fmod(a[0].num, a[1].num));
    if (fn == "len"   && need(1)) return Value((double)a[0].items.size());
    // str(x): número→cadena en formato mínimo (reusa valueToStr). str(x, dec):
    // con dec decimales fijos. Útil para etiquetas concatenadas con '+'; complementa
    // el `decimals` de numbers (§13.2) cuando se arma texto suelto.
    if (fn == "str" && need(1)) return Value(valueToStr(a[0]));
    if (fn == "str" && need(2)) {
      int dec = (int)a[1].num;
      char buf[64];
      std::snprintf(buf, sizeof buf, "%.*f", dec < 0 ? 0 : dec, a[0].num);
      return Value(std::string(buf));
    }
    // gray(g): color gris como entero RGB. g∈[0,1], 0=negro, 1=blanco (§4).
    // Sirve donde iría un color (fill=/color=): esos aceptan número (RGB directo)
    // o cadena (nombre/hex). Fuera de rango se satura.
    if (fn == "gray" && need(1)) {
      double g = a[0].num < 0 ? 0 : (a[0].num > 1 ? 1 : a[0].num);
      int c = (int)(g * 255 + 1e-9);   // trunca (como el motor); eps absorbe el error fp (0.2→51, no 50)
      return Value((double)((c << 16) | (c << 8) | c));
    }
    return evalError("llamada inválida a función ", fn);
  }
};

#endif
