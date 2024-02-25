typedef union {
  char *s;
  float f;
  int i;
} YYSTYPE;
#define YOP 258
#define YOP2 259
#define YGSTATE 260
#define YOPS 261
#define YDEF 262
#define YFILL 263
#define YOPMAT 264
#define YATRIB 265
#define YLISTA 266
#define NUM 267
#define YDT 268
#define YXYDT 269
#define YOPST 270
#define YCLST 271
#define YIDENTIFIER 272
#define YTEXTO 273
#define YINCLUYO 274
#define IDLISTA 275
#define YCLOSE 276
#define YWW 277
#define YDPST 278
#define YSEP 279
#define YPVST 280
#define YMKST 281
#define YARCST 282
#define YLNST 283
#define YXYPP 284
#define YRPNUM 285
#define YTICKS 286
#define YEXIT 287
#define YOBSOLETE 288

extern YYSTYPE yylval;

enum { DBBASIC = 1, DBPARSER = 2, DBMISC = 4 };
