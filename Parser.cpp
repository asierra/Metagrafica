#include "Parser.h"
#include "mgpp_tab.h"
#include "text_parser.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>

extern bool is_using_dot;
extern bool is_using_ellipse;
extern bool is_using_hatcher;
extern bool is_using_textalign;

#define MAX_KEYS 46
YYSTYPE yylval;
YYSTYPE yylvalaux;

#define BEGIN yy_start = 1 + 2 *
#define cadena 4
extern void setBegin(int c);

extern FILE *yyin, *yyout;
extern int yylex();
extern int dblevel;

FILE *logout = 0;


void dbprintf(const char *f, ...) {
  va_list p;

  if (dblevel & DBPARSER)
    printf(f, p);
}

void logprintf(char const *f, ...) {
  va_list p;

  if (logout == NULL)
    logout = stderr;

  //   fprintf(logout, f, p);

  if (logout != stderr)
    fprintf(stderr, f, p);
}

struct {
  const char *k;
  int g, t;
} tabla[MAX_KEYS] = {{"ARCST", YARCST, GI_NULL},
                     {"BR", YOP, GI_RECTANGLE},
                     {"BZ", YOP, GI_BEZIER},
                     {"CLPT", YGSTATE, GS_CLOSEPATH},
                     {"CLST", YCLST, GI_NULL},
                     {"CR", YOP, GI_CIRCLE},
                     {"DOT", YOP, GI_DOT},
                     {"DPST", YDPST, GI_NULL},
                     {"DT", YDT, GI_TEXT},
                     {"EL", YOP, GI_ELLIPSE},
                     {"EXIT", YEXIT, GI_NULL},
                     {"FCOLOR", YATRIB, AT_FCOLOR},
                     {"FGRAY", YATRIB, AT_FGRAY},
                     {"FILL", YGSTATE, GS_FILL},
                     {"FPATRN", YATRIB, AT_FPATRN},
                     {"GNNUM", YRPNUM, GI_NULL},
                     {"GNPT", YLISTA, GI_NULL},
                     {"INPUT", YOPS, GI_NULL},
                     {"INTXT", YOBSOLETE, GI_NULL},
                     {"LCOLOR", YATRIB, AT_LCOLOR},
                     {"LGRAY", YATRIB, AT_LGRAY},
                     {"LNST", YLNST, GI_NULL},
                     {"LPATRN", YATRIB, AT_LSTYLE},
                     {"LSTYLE", YATRIB, AT_LWIDTH},
                     {"LWIDTH", YATRIB, AT_LWIDTH},
                     {"MAXDEEP", YDEF, GI_NULL},
                     {"MKMR", YOPS, GI_NULL},
                     {"MKST", YMKST, GI_NULL},
                     {"MR", YOP, GI_NULL},
                     {"NOFILL", YGSTATE, GS_NOFILL},
                     {"OPPT", YGSTATE, GS_OPENPATH},
                     {"OPST", YOPST, GI_NULL},
                     {"PG", YOP, GI_POLYGON},
                     {"PL", YOP, GI_POLYLINE},
                     {"PWST", YPVST, GI_NULL},
                     {"RPST", YOP, GI_NULL},
                     {"SP", YOP, GI_SPLINE},
                     {"TALIGN", YATRIB, AT_TALIGN},
                     {"THEIGHT", YATRIB, AT_THEIGHT},
                     {"TICKS", YTICKS, GI_NULL},
                     {"TSIZE", YATRIB, AT_THEIGHT},
                     {"TSTYLE", YATRIB, AT_TSTYLE},
                     {"WW", YWW, GI_NULL},
                     {"XYDT", YXYDT, GI_NULL},
                     {"XYPP", YGSTATE, GS_PLUMEPOSITION},
                     {"XYTXT", YOBSOLETE, GI_NULL}
};

const char *opmat[] = {
    "TL", /* Traslada */
    "RT", /* Rota */
    "SC", /* Escala */
    "MT", /* Define */
    "ID", /* Identica */
};

// Matrices definidas
const char *dfmat[] = {"LC", // Local
                       "ST", // Structure
                       "RS", // Repite Structure
                       "PP", // Columnas texto podria ser posición de pluma
                       "MR", // Marcador
                       "MK"};

unsigned search_mat(char *key, const char *tabmat[]) {
  int i;
  for (i = 0; i < 5; i++)
    if (!strncmp(tabmat[i], key, 2))
      break;
  return (i < 5) ? i + 1 : 0;
}

int search_key(char *key) {
  int m, k, l = 0, r = MAX_KEYS - 1;
  while (l < r) {
    m = (l + r) / 2;
    k = strcmp(tabla[m].k, key);
    if (k == 0)
      return m + 1;
    else if (k < 0)
      l = m + 1;
    else
      r = m;
  }
  return 0;
}

int busca_key(char *key) {
  if (int idx = search_key(key)) {
    yylval.i = tabla[idx - 1].t;
    return tabla[idx - 1].g;
  }
  // raise(SIGINT);
  // printf("no es reservada %s\n", key);
  if (strlen(key) == 4) {
    unsigned opm = search_mat(key, opmat);
    if (opm > 0) {
      unsigned dfm = search_mat(&key[2], dfmat);
      if (dfm > 0) {
        //	    opm <<= 8;
        //	    opm |= (dfm & 0xff);
        yylval.i = (int)(opm - 1);
        yylvalaux.i = (int)(dfm - 1);
        dbprintf("MATRIXX %d %d ", yylval.i, yylvalaux.i);
        return YOPMAT;
      }
    }
  }
  yylval.s = key;
  return YIDENTIFIER;
}

string Parser::parseString() {
  string s;
  int i = yylex();

  if (i == YIDENTIFIER)
    s = yylval.s;
  else
    logprintf("Error: Se esperaba una cadena %d\n", i);

  //printf("parseando String %s\n", s.c_str());
  return s;
}

float Parser::parseFloat() {
  lastyylex = yylex();

  if (lastyylex == NUM)
    return yylval.f;
  logprintf("Error: Se esperaba un número %d\n", lastyylex);
  return 0;
}

void Parser::parseDef(int def) {
  float x, y;

  switch (def) {
  case 'D':
    x = parseFloat();
    y = parseFloat();
    mg->setDimension(x, y);
    break;
  case 'P':
    x = parseFloat();
    mg->setFontSize(x);
    break;
  default:
    fprintf(stderr, "Undefined definition %d %c\n", def, def);
    break;
  }
}

void Parser::oldParseMatrix(int mo, Matrix &mt) {
  float x, y;

  switch (mo) {
  case OPMTL:
    x = parseFloat();
    y = parseFloat();
    x = x / wdx;
    y = y / wdy;
    mt.translate(x, y);
    break;
  case OPMRT:
    x = parseFloat();
    mt.rotate(x);
    break;
  case OPMSC:
    x = parseFloat();
    y = parseFloat();
    mt.scale(x, y);
    break;
  case OPMID:
    mt.initialize();
    break;
  }
}

Transform *Parser::parseMatrix(int mo) {
  point z;
  Transform *t = new Transform();
  t->setOperation((MatrixOperation)mo);

  switch (mo) {
  case OPMTL:
    z.x = parseFloat();
    z.y = parseFloat();
    z.x /= wdx;
    z.y /= wdy;
    t->setPoint(z);
    break;
  case OPMRT:
    z.x = parseFloat();
    t->setRotation(z.x);
    break;
  case OPMSC:
    z.x = parseFloat();
    z.y = parseFloat();
    t->setPoint(z);
    break;
  case OPMID:
    break;
  }
  dbprintf("OPMATRIX %g %g ", z.x, z.y);
  return t;
}

GraphicsItem *Parser::parsePrimitive(int type) {
  GraphicsItem *p = nullptr;
  Polyline *pl = nullptr;

  switch (type) {
  case GI_SPLINE:
  case GI_BEZIER:
  case GI_POLYGON:
  case GI_POLYLINE:
    pl = new Polyline((GraphicsItemType)type);
    p = pl;
    break;
  case GI_RECTANGLE: {
    Rectangle *rr = new Rectangle();
    rr->setPath(parsePoints());
    p = rr;
  } break;
  case GI_ELLIPSE:
  case GI_CIRCLE: {
    int k = 0;
    float r[8]; // r d0 a0
    while (yylex() == NUM && k < 8) {
      r[k++] = yylval.f;
    }
    if (type == GI_CIRCLE && k > 0) {
      Arc *e = new Arc();
      e->setRadius(r[0] / wdy, r[0] / wdy);
      if (k > 1) {
        if (k == 2)
          e->setAngles(0.0, r[1]);
        else
          e->setAngles(r[2], r[1] + r[2]);
      }
      e->setPath(parsePoints());
      p = e;
    } else if (type == GI_ELLIPSE && k > 1) {
      is_using_ellipse = true;
      Arc *e = new Arc();
      e->setRadius(r[0] / wdx, r[1] / wdy);
      if (k > 2) {
        if (k == 3)
          r[3] = 0;
        e->setAngles(r[2], r[3]);
      }
      e->setPath(parsePoints());
      p = e;
    }
    break;
  }
  case GI_DOT:
    if (yylex() == NUM) {
      is_using_dot = true;
      float r = yylval.f/2.0;
      Dot *d = new Dot();
      d->setRadius(r);
      d->setPath(parsePoints());
      p = d;
    }
    break;
  }
  if (pl)
    pl->setPath(parsePoints());

  return p;
}

Parser::Parser(string f) {
  filename = f;
  int n = filename.rfind(".mg");
  if (n < 0)
    n = filename.rfind(".MG");

  if (n < 0) {
    n = filename.size();
    filename += ".mg";
  }
  canfilename = filename.substr(0, n);

  printf("Opening %s\n", filename.c_str());
  yyin = fopen(filename.c_str(), "r");
  if (yyin==NULL) {
    fprintf(stderr, "Error: File not found %s\n", filename.c_str());
    exit(1);
  }
  wmx = wmy = 0;
  wdx = wdy = 1;
}

Parser::~Parser() { fclose(yyin); }

void Parser::setLogFile(FILE *f) { logout = f; }

Path Parser::parseRectangle() {
  Path l;
  float x, y;
  point z;

  x = parseFloat();
  y = parseFloat();
  z.x = (x - wmx) / wdx;
  z.y = (y - wmy) / wdy;
  l.push_back(z);
  x = parseFloat();
  y = parseFloat();
  z.x = (x - wmx) / wdx;
  z.y = (y - wmy) / wdy;
  l.push_back(z);

  return l;
}

point Parser::parsePoint() {
  float x, y;
  point z;

  x = parseFloat();
  y = parseFloat();
  z.x = (x - wmx) / wdx;
  z.y = (y - wmy) / wdy;

  return z;
}

Path Parser::parsePoints() {
  Path l;
  float x, y;
  point z;
  int k = 0;

  while ((lastyylex = yylex()) == NUM) {
    x = yylval.f;
    y = parseFloat();
    z.x = (x - wmx) / wdx;
    z.y = (y - wmy) / wdy;
    l.push_back(z);
    k++;
  }
  if (k == 0 && lastyylex == IDLISTA) {
    if (listmap.find(&yylval.s[1]) != listmap.end()) {
      l = listmap[&yylval.s[1]];
    } else
      fprintf(stderr, "Error: identificador de lista inv�lido [%s]\n", yylval.s);
  }
  return l;
}

GraphicsItemList Parser::parsePrimitives() {
  GraphicsItemList prlist;
  Structure *st = NULL;
  static int depth = 0;
  Matrix mtst, mtmk, mtpp;
  float pwmx, pwmy, pwdx, pwdy;
  FontFace ff = FN_DEFAULT;
  int n;

  depth++;

  // Push window
  pwmx = wmx;
  pwmy = wmy;
  pwdx = wdx;
  pwdy = wdy;
  wmx = wmy = 0;
  wdx = wdy = 1;

  dbprintf("Depth %d\n", depth);
  while (1) {
    lastyylex = yylex();

    if (lastyylex == 0 || lastyylex == YCLST) {
      dbprintf("Endwillon %d ", lastyylex);
      break;
    }

    switch (lastyylex) {
    case YWW:
      wmx = parseFloat();
      wdx = parseFloat() - wmx;
      wmy = parseFloat();
      wdy = parseFloat() - wmy;
      dbprintf("WW %g %g %g %g\n", wmx, wmy, wdx, wdy);
      break;
    case YOP: {
      int primtype = yylval.i;
      do {
        GraphicsItem *p = parsePrimitive(primtype);
        if (p != NULL) {
          prlist.push_back(p);
        }
      } while (lastyylex == YSEP);
      break;
    }
    case YOPMAT: {
      PredefinedMatrix pmat = (PredefinedMatrix)yylvalaux.i;
      if (pmat == MTPP)
        oldParseMatrix(yylval.i, mtpp);
      else {
        Transform *t = parseMatrix(yylval.i);
        t->setPredefinedMatrix(pmat);
        prlist.push_back(t);
      }
      break;
    }
    /* ojo
    case YDPST: {
      string name = parseString();
      Structure *strct = Structure::getStructure(name);
      if (!strct) {
        fprintf(stderr, "Error: Structure no definida <%s>\n", name.c_str());
        break;
      } // ojo
      StructureUser *sr = new StructureUser();
      sr->setStructure(strct);
      //sr->setArrow(true);
      //sr->setMatrix(mtst);
      prlist.push_back(sr);
      break;
    }*/
    case YOPST: {
      string name = parseString();
      Structure *strct = Structure::getStructure(name);
      if (strct) {
        fprintf(stderr, "Error: ya existe la Structure %s.\n", name.c_str());
      }
      GraphicsItemList p = parsePrimitives();
      st = new Structure();
      st->setName(name);
      st->setGraphicsItems(p);
      // prlist.push_back(st);
      break;
    }
    case YLNST: {
      if (st) {
        StructureLine *sr = new StructureLine();
        point llp, rup;
        float sc = parseFloat();
        if (sc < 0) {
          sc = -sc;
          sr->setBothSides(true);
        }
        sr->setScale(sc, sc);
        if (yylex()==NUM) {
          float shift = 1 - yylval.f;
          sr->setShift(shift);
          yylex(); // consume the separator
        } 
        llp = parsePoint();
        rup = parsePoint();
        sr->setPoints(llp, rup);
        sr->setStructure(st);
        prlist.push_back(sr);
      } else {
        fprintf(stderr, "Error: no hay Structure marcada.\n");
      }
    } break;
    case YARCST: {
      if (st) {
        StructureArc *sr = new StructureArc();
        float sc = parseFloat();
        float r = parseFloat();
        float da = parseFloat();
        float ai = parseFloat();
        float af = ai + da;
        if (yylex()==NUM) {
          float shift = 1 - yylval.f;
          sr->setShift(shift);
          yylex(); // consume the separator
        } 
        point p = parsePoint();
        if (sc < 0) {
          sc = -sc;
          sr->setBothSides(true);
        }
        sr->setScale(sc, sc);
        sr->setRadius(r / wdy);
        sr->setAngles(ai, af);
        sr->setPoint(p);
        sr->setStructure(st);
        prlist.push_back(sr);
      } else {
        fprintf(stderr, "Error: no hay Structure marcada.\n");
      }
    } break;
    case YMKST: {
      string name = parseString();
      st = Structure::getStructure(name);
    } break;
    case YPVST: {
      if (st) {
        StructureRectangle *sr = new StructureRectangle();
        sr->setStructure(st);
        point llp, rup;
        llp = parsePoint();
        rup = parsePoint();
        sr->setPoints(llp, rup);
        prlist.push_back(sr);
      } else {
        fprintf(stderr, "Error: no hay Structure marcada.\n");
      }
      break;
    }
    case YIDENTIFIER: {
      string name = yylval.s;
      Structure *strct = Structure::getStructure(name);
      if (!strct) {
        fprintf(stderr, "Error: identificador invalido [%s]\n", name.c_str());
        logprintf("Error: identificador invalido [%s]\n", name.c_str());
      } else {
        // fprintf(stderr, "creando Structurepath %s\n", name.c_str());
        StructurePath *sr = new StructurePath();
        sr->setStructure(strct);
        sr->setPath(parsePoints());
        // sr->setMatrix(mtst);
        // s->setMatrix(mtst);
        prlist.push_back(sr);
      }
      break;
    }
    case YDEF:
      parseDef(yylval.i);
      break;
    case YATRIB: {
      AttributeType type = (AttributeType)yylval.i;
      Attribute *at = new Attribute();
      if (type==AT_TALIGN)
        is_using_textalign = true;
      if (type==AT_FPATRN)
        is_using_hatcher = true;
      if (type==AT_TSTYLE) {
        setBegin(cadena);
        yylex();  
        ff = get_font_face_from_string(&yylval.s[1]);
        if (ff!=FN_NOFACE)
          at->set(type, ff);
        else {
          ff = FN_DEFAULT;
          delete at;
          break;
        }
      } else {
        float f = parseFloat();
        at->set(type, (int)f);
      }
      prlist.push_back(at);
      break;
    }
    case YGSTATE: {
      GraphicsState *gs = new GraphicsState();
      if (yylval.i == GS_PLUMEPOSITION) {
        pp.x = (parseFloat() - wmx) / wdx;
        pp.y = (parseFloat() - wmy) / wdy;
        gs->setPosition(pp);
      } else
        gs->setGSType((GraphicsStateType)yylval.i);
      prlist.push_back(gs);
    } break;
    case YXYDT: {
      pp = parsePoint();
      //pp.x = (parseFloat() - wmx) / wdx;
      //pp.y = (parseFloat() - wmy) / wdy;
      GraphicsState *gs = new GraphicsState();
      gs->setPosition(pp);
      prlist.push_back(gs); }
    case YDT: { 
      if (lastyylex==YDT) { // ojo, dt debe actualizar la posición de algún modo
        mtpp.transform(pp.x, pp.y);
      }
      setBegin(cadena);
      yylex();
      GraphicsItem *t = parse_text(&yylval.s[1], ff);
      prlist.push_back(t);
      break;
    }
    case YLISTA: {
      int n = (int)parseFloat();
      float x = parseFloat();
      float y = parseFloat();
      Path l;
      string name = parseString();
      if (listmap.find(name) != listmap.end())
        l = listmap[name];
      else
        listmap[name] = l;
      point z;
      x = (x - wmx) / wdx;
      y = (y - wmy) / wdy;
      for (int i = 0; i < n; i++) {
        z.x = x;
        z.y = y;
        l.push_back(z);
        // mtdr.transforma(x, y);
      }
      // printf("Lista %d %g %g {%s} %d\n", n, x, y, name.c_str(), l.size());
      break;
    }
    case YRPNUM: {
      float x0 = parseFloat();
      float xinc = parseFloat();
      n = (int)parseFloat();
      int ndec = (int)parseFloat();
      char fmt[8], str[80];
      sprintf(fmt, "%%.%df", ndec);
      for (int i = 0; i < n; i++) {
        float x = x0 + i * xinc;
        GraphicsState *gs = new GraphicsState();
        gs->setPosition(pp);
        prlist.push_back(gs);
        mtpp.transform(pp.x, pp.y);
        sprintf(str, fmt, x);
        Text *text = new Text();
        text->setText(str);
        text->setFontFace(ff);
        prlist.push_back(text);
      }
      break;
    }
    case YTICKS: {
      point z;
      n = (int)parseFloat();
      z.x = parseFloat() / wdx;
      z.y = parseFloat() / wdy;
      Polyline *pl = new Polyline(GI_TICKS);
      Path l;
      l.push_back(z);
      for (int i = 0; i < n; i++) {
        l.push_back(pp);
        mtpp.transform(pp.x, pp.y);
      }
      pl->setPath(l);
      prlist.push_back(pl);
      }
      break;
    case YOBSOLETE:
      printf("Aviso: Comando obsoleto <>, usar el comando actual.\n");
      break;
    case YEXIT:
      return prlist;
      break;
    }
  }

  depth--;

  // Pop window
  wmx = pwmx;
  wmy = pwmy;
  wdx = pwdx;
  wdy = pwdy;
  dbprintf("End Depth %d\n", depth);
  return prlist;
}

MetaGrafica *Parser::parse() {
  mg = new MetaGrafica();
  mg->setName("root");
  mg->setGraphicsItems(parsePrimitives());
  logprintf("Parseo terminado\n");
  return mg;
}
