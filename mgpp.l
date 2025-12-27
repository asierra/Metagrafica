%{
// Version en C++ del analizador lexico para MG.
// Autor: Alejandro Aguilar Sierra, 1995-2023

#include "MGLexer.h"
#include "mgpp_tab.h"
#include <string.h>
#include <fstream>

// Implementación de métodos de MGLexer

void MGLexer::init_tables() {
    // Inicializar mapa de palabras clave
    keyword_map = {
        {"ARCST", {YARCST, GI_NULL}},
        {"BR", {YOP, GI_RECTANGLE}},
        {"BZ", {YOP, GI_BEZIER}},
        {"CLPT", {YGSTATE, GS_CLOSEPATH}},
        {"CLST", {YCLST, GI_NULL}},
        {"CR", {YOP, GI_CIRCLE}},
        {"CTPT", {YCONCATPATH, GI_NULL}},
        {"DOT", {YOP, GI_DOT}},
        {"DPST", {YDPST, GI_NULL}},
        {"DT", {YDT, GI_TEXT}},
        {"EL", {YOP, GI_ELLIPSE}},
        {"EXIT", {YEXIT, GI_NULL}},
        {"FCOLOR", {YATRIB, AT_FCOLOR}},
        {"FGRAY", {YATRIB, AT_FGRAY}},
        {"FILL", {YGSTATE, GS_FILL}},
        {"FPATRN", {YATRIB, AT_FPATRN}},
        {"GNBZPATH", {YGNBZ, GI_NULL}},
        {"GNNUM", {YRPNUM, GI_NULL}},
        {"GNPATH", {YLISTA, GI_NULL}},
        {"INPUT", {YOPS, GI_NULL}},
        {"INTXT", {YOBSOLETE, GI_NULL}},
        {"INVPT", {YINVPT, GI_NULL}},
        {"LCOLOR", {YATRIB, AT_LCOLOR}},
        {"LGRAY", {YATRIB, AT_LGRAY}},
        {"LNST", {YLNST, GI_NULL}},
        {"LPATRN", {YATRIB, AT_LSTYLE}},
        {"LSTYLE", {YATRIB, AT_LWIDTH}},
        {"LWIDTH", {YATRIB, AT_LWIDTH}},
        {"MAXDEEP", {YDEF, GI_NULL}},
        {"MKMR", {YOPS, GI_NULL}},
        {"MKST", {YMKST, GI_NULL}},
        {"MR", {YOP, GI_NULL}},
        {"NOFILL", {YGSTATE, GS_NOFILL}},
        {"NORMPT", {YNORMPT, GI_NULL}},
        {"OPPT", {YGSTATE, GS_OPENPATH}},
        {"OPST", {YOPST, GI_NULL}},
        {"PG", {YOP, GI_POLYGON}},
        {"PL", {YOP, GI_POLYLINE}},
        {"PWPT", {YPVPT, GI_NULL}},
        {"PWST", {YPVST, GI_NULL}},
        {"RPPT", {YRPPT, GI_NULL}},
        {"RPST", {YRPST, GI_NULL}},
        {"SP", {YOP, GI_SPLINE}},
        {"TALIGN", {YATRIB, AT_TALIGN}},
        {"THEIGHT", {YATRIB, AT_THEIGHT}},
        {"TICKS", {YTICKS, GI_NULL}},
        {"TSIZE", {YATRIB, AT_THEIGHT}},
        {"TSTYLE", {YATRIB, AT_TSTYLE}},
        {"WW", {YWW, GI_NULL}},
        {"XYDT", {YXYDT, GI_NULL}},
        {"XYPP", {YGSTATE, GS_PLUMEPOSITION}},
        {"XYTXT", {YOBSOLETE, GI_NULL}}
    };

    map_opmat = {
        {"TL", OPMTL}, {"RT", OPMRT}, {"SC", OPMSC}, {"SH", OPMSH},
        {"MT", OPMMT}, {"ID", OPMID}, {"CP", OPMCP}
    };

    map_dfmat = {
        {"LC", MTLC}, {"ST", MTST}, {"PP", MTPP}, {"PT", MTPT}, {"RS", MTRS}
    };
}

bool MGLexer::search_mat(string key, int &opm, int &dfm) {
  if (map_opmat.find(key.substr(0,2)) != map_opmat.end() && 
      map_dfmat.find(key.substr(2,3)) != map_dfmat.end()) {
    opm = map_opmat[key.substr(0,2)];
    dfm = map_dfmat[key.substr(2,3)];
    return true;
  }
  return false;
}

int MGLexer::busca_key(const char *key) {
  if (keyword_map.find(key) != keyword_map.end()) {
    yylval.i = keyword_map[key].type;
    return keyword_map[key].token;
  }
  if (strlen(key) == 4) {
    int opm, dfm;
    if (search_mat(key, opm, dfm)) {
      yylval.i = (int)(opm);
      yylvalaux.i = (int)(dfm);
      return YOPMAT;
    }
  }
  yylval.s = strdup(key); // Ojo: strdup usa malloc, idealmente usar std::string
  return YIDENTIFIER;
}

void openFile(const char* name, MGLexer* lexer) {
     char incname[256];
     
     strcpy(incname, name);
     if (strchr(incname,'.')==NULL) 
	strcat(incname,".mg");	     
	
     auto stream = std::make_unique<std::ifstream>(incname);
     if ( !stream->is_open() ){                 
                 fprintf(stderr, "Error: File not found %s\n", incname);
                 exit( 1 );
     }
     
     // Push current buffer
     lexer->yypush_buffer_state(lexer->yy_create_buffer(stream.get(), YY_BUF_SIZE));
     lexer->push_stream(std::move(stream));
}


int dblevel=0;

#define dbprintf(x, y) if (dblevel & DBPARSER) printf(x, y)

%}
%option yylineno
%option c++
%option yyclass="MGLexer"
%option outfile="lexmg.cpp"
%option noyywrap
%option noyymore
%option noreject

DIGIT   [0-9]
LETRA   [A-Za-z]
DIGILET [A-Za-z0-9]
ID      {LETRA}{DIGILET}+
IDA     {LETRA}+
ENTERO  {DIGIT}+
FLOTANTE  ({ENTERO}"."{DIGIT}*)|("."{DIGIT}+)
NUMERO	"-"?({ENTERO}|{FLOTANTE})

%x lista
%x linea
%x incl
%x cadena

%%

INPUT		  BEGIN(incl);

{ID}+                   { 
                          dbprintf("Identifier: %s\n", yytext);
                          return busca_key(yytext);
			}

"$"{LETRA}              {
			  yylval.i = yytext[1];
			  //printf("Control %s\n", yytext);
			  return YDEF;
			}


"&"{ID}                 { yylval.s = strdup(yytext); return IDLISTA; }



"%"[^\n]*        	/* eat commentary */ dbprintf("Com %s\n", yytext);
[ \r\t\n]+        /* eat space */
{NUMERO}        { yylval.f = atof(yytext);
                  dbprintf("Num [%s]\n", yytext);
		  return NUM; }

[;:]		  { return YSEP; }

[}]		  { return YCLOSE; }

.               /* Do nothing */ dbprintf("Nada [%s]\n", yytext);

<lista>{NUMERO}        { yylval.f = atof(yytext);
                         dbprintf("Lista Num [%s]\n", yytext);
                         return NUM; }
<lista>";"               return ';';
<lista>":"               return ':';
<lista>"%"[^\n]*       /* eat commentary */
<lista>[ \t\n]+        /* eat space */
<lista>.               BEGIN(INITIAL);

<linea>{NUMERO}        { yylval.f = atof(yytext); return NUM; }
<linea>[ \t]+        /* eat space */
<linea>[\r\n]+         BEGIN(INITIAL);
<linea>.               BEGIN(INITIAL);

<cadena>[^\r\n]*	      { BEGIN(INITIAL); yylval.s = strdup(yytext); dbprintf("Cadena %s\n", yytext); return YIDENTIFIER; }

<incl>[ \t]*      /* eat the whitespace */
<incl>[^ \t\n\r]+   { /* got the include file name */
	 	    printf("Include [%s]\n", yytext);
            openFile(yytext, this);
             BEGIN(INITIAL);
             }
     
<<EOF>> {
             yypop_buffer_state();
             if ( !YY_CURRENT_BUFFER )
                 {
                 yyterminate();
                 }     
             }
%%
