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

// Reporte de error de evaluación (sin excepciones): imprime y sigue con 0.
inline Value evalError(const char *msg, const std::string &extra = "") {
  std::fprintf(stderr, "Error de evaluación: %s%s\n", msg, extra.c_str());
  return Value(0.0);
}

// --- Ámbito léxico: variables encadenadas al padre --------------------------
struct Scope {
  std::map<std::string, Value> vars;
  Scope *parent = nullptr;
  explicit Scope(Scope *p = nullptr) : parent(p) {}

  Value *find(const std::string &name) {
    for (Scope *s = this; s; s = s->parent) {
      auto it = s->vars.find(name);
      if (it != s->vars.end()) return &it->second;
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

struct BinExpr : Expr {          // + - * / ^   y comparación/lógicos (§6.1)
  int op;
  ExprPtr l, r;
  BinExpr(int o, ExprPtr a, ExprPtr b) : op(o), l(std::move(a)), r(std::move(b)) {}
  Value eval(Scope &sc) const override {
    // and/or con cortocircuito: no evalúa la rama derecha si no hace falta
    // (permite guardas como `i < len(a) and a[i] > 0`). Booleano = número !=0.
    if (op == T_AND) return Value(l->eval(sc).num != 0.0 && r->eval(sc).num != 0.0 ? 1.0 : 0.0);
    if (op == T_OR)  return Value(l->eval(sc).num != 0.0 || r->eval(sc).num != 0.0 ? 1.0 : 0.0);
    double a = l->eval(sc).num, b = r->eval(sc).num;
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
    if (fn == "mod"   && need(2)) return Value(std::fmod(a[0].num, a[1].num));
    if (fn == "len"   && need(1)) return Value((double)a[0].items.size());
    return evalError("llamada inválida a función ", fn);
  }
};

#endif
