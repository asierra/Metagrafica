%{
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>

/* * En tu parser de C++ tendrás un header similar a este que defina los tokens
 * y la estructura yylval para pasar datos desde Flex al Parser.
 */
#include "tokens.h"

/* Definiciones de los globales que declara tokens.h (extern). El parser leerá
 * yylval / yytokline / yytokcol. */
YYSTYPE yylval;
int yytokline = 1, yytokcol = 1;

/* Profundidad de () y [] SOLAMENTE (no {}): dentro de args o listas el salto de
 * línea se suprime. {} NO cuenta porque tiene doble papel: bloque de sentencias
 * (el newline separa) vs lista de puntos (el parser salta el newline). */
int bracketDepth = 0;

/* Seguimiento de columna para errores archivo:línea:columna (§19).
 * YY_USER_ACTION corre antes de cada acción: guarda el inicio del token
 * (yytokline/yytokcol) y avanza la columna. La regla de '\n' la reinicia. */
int yycolumn = 1;
#define YY_USER_ACTION  { yytokline = yylineno; yytokcol = yycolumn; yycolumn += yyleng; }

%}

/* * Opciones modernas de Flex:
 * noyywrap: No procesar múltiples archivos automáticamente
 * nounput/noinput: Evitar warnings del compilador por funciones no usadas
 * yylineno: Flex mantendrá el número de línea automáticamente
 */
%option noyywrap nounput noinput yylineno

/* Expresiones regulares base */
DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*

/* Enteros (5), decimales (5.5), sin cero inicial (.5) y notación científica (1e-3).
 * SIN signo inicial: el '-' unario lo maneja el parser (Unary ::= "-" Unary, §2);
 * el signo del EXPONENTE sí es parte del número. */
NUMBER      {DIGIT}*\.?{DIGIT}+([eE][-+]?{DIGIT}+)?

/* Cadenas entre comillas dobles (ej. "blue", "$\alpha$", "/bTítulo") */
STRING      \"[^\"]*\"

%%

 /* --- ESPACIOS, SALTOS Y COMENTARIOS --- */
\n                      { yycolumn = 1; if (bracketDepth == 0) return T_NEWLINE; }
[ \t\r]+                { /* Ignorar espacios y tabulaciones */ }
"%".*                   { /* Comentario estilo TeX hasta fin de línea. Por eso el
                             módulo es mod(a,b) y NO el operador '%'. */ }

 /* --- PUNTUACIÓN Y AGRUPADORES --- */
"{"                     { return T_LBRACE; }   /* {} no suprime newlines (ver arriba) */
"}"                     { return T_RBRACE; }
"("                     { bracketDepth++; return T_LPAREN; }
")"                     { if (bracketDepth > 0) bracketDepth--; return T_RPAREN; }
"["                     { bracketDepth++; return T_LBRACKET; }   /* listas: [10, 2]  e indexado: a[i] */
"]"                     { if (bracketDepth > 0) bracketDepth--; return T_RBRACKET; }
","                     { return T_COMMA; }
";"                     { return T_SEMICOLON; }  /* subtrayectos de paths */
"&"                     { return T_AMP; }        /* referencia a path: &nombre */

 /* --- OPERADORES (los de dos caracteres antes que los de uno) --- */
"=="                    { return T_EQ; }
"!="                    { return T_NE; }
"<="                    { return T_LE; }
">="                    { return T_GE; }
"="                     { return T_ASSIGN; }
"<"                     { return T_LT; }
">"                     { return T_GT; }
"+"                     { return T_PLUS; }
"-"                     { return T_MINUS; }
"*"                     { return T_STAR; }
"/"                     { return T_SLASH; }
"^"                     { return T_CARET; }      /* potencia (§5.2); el '^' de
                                                    superíndices vive dentro de "..." */

 /* --- PALABRAS CLAVE (antes que {ID}: en empate de longitud gana la 1ª regla) --- */
"struct"                { return T_STRUCT; }
"for"                   { return T_FOR; }
"to"                    { return T_TO; }
"step"                  { return T_STEP; }
"if"                    { return T_IF; }
"else"                  { return T_ELSE; }
"and"                   { return T_AND; }
"or"                    { return T_OR; }
"include"               { return T_INCLUDE; }

 /* --- IDENTIFICADORES, NÚMEROS Y CADENAS --- */
{ID}                    {
                          /* Primitivas (polyline), variables (x), atributos (color),
                           * funciones (sin, mod)… todo cae aquí; el parser resuelve. */
                          yylval.str = strdup(yytext);
                          return T_IDENTIFIER;
                        }

{NUMBER}                {
                          yylval.num = std::atof(yytext);
                          return T_NUMBER;
                        }

{STRING}                {
                          /* Quitamos las comillas antes de mandar el texto al parser */
                          yytext[yyleng-1] = '\0';
                          yylval.str = strdup(yytext + 1);
                          return T_STRING;
                        }

 /* --- MANEJO DE ERRORES LÉXICOS --- */
.                       {
                          std::cerr << "Error léxico en " << yytokline << ":" << yytokcol
                                    << ": carácter inesperado '" << yytext << "'\n";
                          exit(1);
                        }

%%

/* * NOTA ARQUITECTÓNICA:
 * Los nombres de primitivas/atributos/funciones ("polyline", "width", "color",
 * "sin", "mod"…) NO son keywords: caen en T_IDENTIFIER y el parser los resuelve
 * por tabla. Así, añadir una primitiva nueva NO toca el lexer.
 * Sí son keywords las palabras de control (struct/for/to/step/if/else) y los
 * conectores lógicos (and/or), porque el parser los necesita como tokens.
 */
