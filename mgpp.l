%{
// Version en C++ del analizador lexico para MG.
// Autor: Alejandro Aguilar Sierra, 1995-2023

#include "mgpp_tab.h"
#include <string.h>

extern int busca_key(char *);

#define MAX_INCLUDE_DEPTH 10
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
int include_stack_ptr = 0;

void setBegin(int c) {
     BEGIN(c);
}

void openFile(const char* name) {
     char incname[256];
     
     strcpy(incname, name);
     if (strchr(incname,'.')==NULL) 
	strcat(incname,".mg");	     
	
     yyin = fopen( incname, "r" );		
     if ( ! yyin ){                 
                 fprintf(stderr, "Error: File not found %s\n", incname);
                 exit( 1 );
     }
     fflush(stderr);
}


int dblevel=0;

#define dbprintf(x, y) if (dblevel & DBPARSER) printf(x, y)

%}
%option yylineno
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
<lista>"%"[^\n]*       /* eat commentary *
<lista>[ \t\n]+        /* eat space */
<lista>.               BEGIN(INITIAL);

<linea>{NUMERO}        { yylval.f = atof(yytext); return NUM; }
<linea>[ \t]+        /* eat space */
<linea>[\r\n]+         BEGIN(INITIAL);
<linea>.               BEGIN(INITIAL);

<cadena>[^\r\n]*	      { BEGIN(INITIAL); yylval.s = yytext; dbprintf("Cadena %s\n", yytext); return YIDENTIFIER; }

<incl>[ \t]*      /* eat the whitespace */
<incl>[^ \t\n\r]+   { /* got the include file name */
	 	    printf("Include [%s]\n", yytext);
             char *incname = strdup(yytext);     
             if ( include_stack_ptr >= MAX_INCLUDE_DEPTH )
                 {
                 fprintf(stderr, "Inclusiones anidadas demasiado profundas\n" );
                 exit( 1 );
                 }
	     else {
                include_stack[include_stack_ptr++] =
                     YY_CURRENT_BUFFER;
		openFile(yytext);     
//       	        openFile(incname);
	     
/*	     
	     if (strchr(incname,'.')==NULL) 
	     	strcat(incname,".mg");
	     
             yyin = fopen( incname, "r" );
		
	     if ( ! yyin ){                 
                 fprintf("File not found %s\n", incname);
		 free(incname);
                 exit( 1 );
             }
     	     free(incname);*/
                 yy_switch_to_buffer(
                    yy_create_buffer( yyin, YY_BUF_SIZE ) );
     		}    
             BEGIN(INITIAL);
             }
     
<<EOF>> {
             if ( --include_stack_ptr < 0 )
                 {
                 yyterminate();
                 }     
             else
                 {
                 yy_delete_buffer( YY_CURRENT_BUFFER );
                 yy_switch_to_buffer(
                      include_stack[include_stack_ptr] );
                 }
             }
%%

