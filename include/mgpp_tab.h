#ifndef MGPP_TAB_H
#define MGPP_TAB_H

typedef union {
  char *s;
  float f;
  int i;
} YYSTYPE;

enum Token {
  YOP = 258,
  YGSTATE = 260,
  YOPS = 261,
  YDEF = 262,
  YFILL = 263,
  YOPMAT = 264,
  YATRIB = 265,
  YLISTA = 266,
  NUM = 267,
  YDT = 268,
  YXYDT = 269,
  YOPST = 270,
  YCLST = 271,
  YIDENTIFIER = 272,
  YTEXTO = 273,
  YINCLUYO = 274,
  IDLISTA = 275,
  YCLOSE = 276,
  YWW = 277,
  YDPST = 278,
  YSEP = 279,
  YPVST = 280,
  YMKST = 281,
  YARCST = 282,
  YLNST = 283,
  YRPST = 284,
  YXYPP = 285,
  YRPNUM = 286,
  YTICKS = 287,
  YEXIT = 288,
  YCONCATPATH = 289,
  YINVPT = 290,
  YRPPT = 291,
  YNORMPT = 292,
  YPVPT = 293,
  YGNBZ = 294,
  YOBSOLETE = 295
};

enum { DBBASIC = 1, DBPARSER = 2, DBMISC = 4 };

#endif
