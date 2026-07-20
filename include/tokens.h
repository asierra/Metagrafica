#ifndef TOKENS_H
#define TOKENS_H

// Tokens del lexer V3 (src/lexer.l). Equivalente V3 de include/mgpp_tab.h.
// El lexer rellena `yylval` con el lexema (cadena, strdup) o el valor numérico,
// y deja la ubicación del último token en yytokline/yytokcol (errores línea:col).

typedef union {
  char  *str;   // T_IDENTIFIER, T_STRING  (lexema; el parser toma posesión / libera)
  double num;   // T_NUMBER
} YYSTYPE;

extern YYSTYPE yylval;
extern int yytokline, yytokcol;

// Valores > 255 (convención flex/bison): no chocan con eventuales tokens de un
// solo carácter. Aquí el lexer devuelve SIEMPRE tokens con nombre.
enum Token {
  T_EOF = 0,                       // flex devuelve 0 al fin de la entrada

  // Puntuación y agrupadores
  T_LBRACE = 256, T_RBRACE, T_LPAREN, T_RPAREN,
  T_LBRACKET, T_RBRACKET, T_COMMA, T_SEMICOLON, T_AMP,

  // Operadores (los de dos caracteres se emiten como un solo token)
  T_ASSIGN, T_PLUS, T_MINUS, T_STAR, T_SLASH, T_CARET,
  T_EQ, T_NE, T_LT, T_GT, T_LE, T_GE,
  T_PLUSASSIGN,   // '+=', solo para acumular paths: `path w += pieza` (§9)

  // Palabras clave de control y conectores lógicos
  T_STRUCT, T_FOR, T_TO, T_STEP, T_IF, T_ELSE, T_AND, T_OR, T_NOT, T_INCLUDE,

  // Terminales con valor
  T_IDENTIFIER, T_NUMBER, T_STRING,

  // Separador de sentencias: salto de línea a nivel superior (suprimido dentro
  // de ()/{}/[]). El parser lo usa para terminar sentencias de estado.
  T_NEWLINE
};

#endif
