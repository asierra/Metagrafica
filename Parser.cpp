#include "Parser.h"
#include "mgpp_tab.h"
#include "text_parser.h"
#include "splines.h"
#include <algorithm>
#include <signal.h>
#include <stdio.h>
#include <string.h>

extern bool is_using_dot;
extern bool is_using_ellipse;
extern bool is_using_hatcher;
extern bool is_using_textalign;

#define MAX_KEYS 52
YYSTYPE yylval;
YYSTYPE yylvalaux;

#define BEGIN yy_start = 1 + 2 *
#define cadena 4
extern void setBegin(int c);

extern FILE *yyin, *yyout;
extern int yylex();
extern int dblevel;

FILE *logout = 0;

struct {
  const char *k;
  int g, t;
} tabla[MAX_KEYS] = {{"ARCST", YARCST, GI_NULL},
                     {"BR", YOP, GI_RECTANGLE},
                     {"BZ", YOP, GI_BEZIER},
                     {"CLPT", YGSTATE, GS_CLOSEPATH},
                     {"CLST", YCLST, GI_NULL},
                     {"CR", YOP, GI_CIRCLE},
                     {"CTPT", YCONCATPATH, GI_NULL},
                     {"DOT", YOP, GI_DOT},
                     {"DPST", YDPST, GI_NULL},
                     {"DT", YDT, GI_TEXT},
                     {"EL", YOP, GI_ELLIPSE},
                     {"EXIT", YEXIT, GI_NULL},
                     {"FCOLOR", YATRIB, AT_FCOLOR},
                     {"FGRAY", YATRIB, AT_FGRAY},
                     {"FILL", YGSTATE, GS_FILL},
                     {"FPATRN", YATRIB, AT_FPATRN},
                     {"GNBZPATH", YGNBZ, GI_NULL},
                     {"GNNUM", YRPNUM, GI_NULL},
                     {"GNPATH", YLISTA, GI_NULL},
                     {"INPUT", YOPS, GI_NULL},
                     {"INTXT", YOBSOLETE, GI_NULL},
                     {"INVPT", YINVPT, GI_NULL},
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
                     {"NORMPT", YNORMPT, GI_NULL},
                     {"OPPT", YGSTATE, GS_OPENPATH},
                     {"OPST", YOPST, GI_NULL},
                     {"PG", YOP, GI_POLYGON},
                     {"PL", YOP, GI_POLYLINE},
                     {"PWPT", YPVPT, GI_NULL},
                     {"PWST", YPVST, GI_NULL},
                     {"RPPT", YRPPT, GI_NULL},
                     {"RPST", YRPST, GI_NULL},
                     {"SP", YOP, GI_SPLINE},
                     {"TALIGN", YATRIB, AT_TALIGN},
                     {"THEIGHT", YATRIB, AT_THEIGHT},
                     {"TICKS", YTICKS, GI_NULL},
                     {"TSIZE", YATRIB, AT_THEIGHT},
                     {"TSTYLE", YATRIB, AT_TSTYLE},
                     {"WW", YWW, GI_NULL},
                     {"XYDT", YXYDT, GI_NULL},
                     {"XYPP", YGSTATE, GS_PLUMEPOSITION},
                     {"XYTXT", YOBSOLETE, GI_NULL}};


//const char *opmat[] = {
map<string, MatrixOperation> map_opmat = {
    {"TL", OPMTL}, /* Traslate */
    {"RT", OPMRT}, /* Rotate */
    {"SC", OPMSC}, /* Scale */
    {"SH", OPMSH}, /* Shear */
    {"MT", OPMMT}, /* Define */
    {"ID", OPMID}, /* Identity */
    {"CP", OPMCP} /* Compose */
};

// Matrices definidas
//const char *dfmat[] = {
map<string, PredefinedMatrix> map_dfmat = {
  {"LC", MTLC}, // Local
  {"ST", MTST}, // Structure
  {"PP", MTPP}, // Columnas texto podria ser posición de pluma
  {"PT", MTPT}, // Path
  {"RS", MTRS}
}; // Repeat structure

bool search_mat(string key, int &opm, int &dfm) {
  //printf("key %s %s %s ", key.c_str(), key.substr(0,2).c_str(), key.substr(2,3).c_str());
  if (map_opmat.find(key.substr(0,2)) != map_opmat.end() && 
      map_dfmat.find(key.substr(2,3)) != map_dfmat.end()) {
    opm = map_opmat[key.substr(0,2)];
    dfm = map_dfmat[key.substr(2,3)];

    return true;
  }
  return false;
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
    int opm, dfm;
    if (search_mat(key, opm, dfm)) {
      yylval.i = (int)(opm);
      yylvalaux.i = (int)(dfm);
      //printf("MATRIXX %d %d\n", yylval.i, yylvalaux.i);
      return YOPMAT;
    }
  }
  yylval.s = key;
  return YIDENTIFIER;
}

map<string, int> map_color = {
    {"black", 0},
    {"blue", 0xff},
    {"brown", 0xa52a2a},
    {"cyan", 0xffff},
    {"gray", 0x808080},
    {"green", 0xff00},
    {"lightgray", 0xd3d3d3},
    {"magenta", 0xff00ff},
    {"orange", 0xcc3232},
    {"red", 0xff0000},
    {"white", 0xffffff},
    {"yellow", 0xffff00},
};

int get_color_from_string(string colorstr) {
  const string HEXDIGIT = "0123456789abcdf";
  int col = 0;
  bool outline=false;

  if (colorstr[0]=='-') {
    colorstr.erase(0, 1);
    outline = true;
  }
  std::transform(colorstr.begin(), colorstr.end(), colorstr.begin(), ::tolower);
  if (map_color.find(colorstr) != map_color.end()) {
    col = map_color[colorstr];
  } else if (colorstr.find_first_not_of(HEXDIGIT)==string::npos) {
    col = stoi(colorstr, 0, 16);
  } else {
    fprintf(stderr, "Error: Unrecognized color <%s>\n", colorstr.c_str());
  }
  //printf("color %s %x\n", colorstr.c_str(), col);
  if (outline)
    col = -col;
  return col;
}

string Parser::parseString() {
  string s;
  int i = yylex();

  if (i == YIDENTIFIER)
    s = yylval.s;
  else
    fprintf(stderr, "Error: A string was expected, not %d\n", i);

  return s;
}

float Parser::parseFloat() {
  lastyylex = yylex();

  if (lastyylex==NUM)
    return yylval.f;
  fprintf(stderr, "Error: A number was expected %d\n", lastyylex);
  return 0;
}

void Parser::parseDef(int def) {
  int n;
  float x, y;

  switch (def) {
  case 'D':
    x = parseFloat();
    y = parseFloat(); // ojo a veces no detecta error de no numero
    mg->setDimension(x, y);
    break;
  case 'P':
    x = parseFloat();
    mg->setFontSize(x);
    break;
  case 'S':  // 0 convert to Bezier, 1 conic spline, > 1 number of new points per segment CR Sp
    n = (int)parseFloat();
    if (n==0)
      is_spline_to_bezier = true;
    else if (n > 1)
      spline_nodes_per_segment = n;
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
    //printf("rota %g\n", x);
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
  case OPMSH:
  case OPMSC:
    z.x = parseFloat();
    z.y = parseFloat();
    t->setPoint(z);
    break;
  case OPMID:
    //printf("nuevo opmid\n");
    break;
  }
  return t;
}

GraphicsItem *Parser::parsePrimitive(int type) {
  GraphicsItemWithPath *pl = nullptr;

  switch (type) {
  case GI_SPLINE: {
    Path controlpoints = parsePath();
    if (is_spline_to_bezier) {
      printf("Converting spline to Bezier.\n");
      pl = new Polyline(GI_BEZIER);
      pl->setPath(splines_to_bezier(controlpoints));
    } else {
      pl = new Polyline((GraphicsItemType)type);
      pl->setPath(splines(controlpoints, spline_nodes_per_segment));
    }
    return pl;
  }
  return pl;
  case GI_BEZIER:
  case GI_POLYGON:
  case GI_POLYLINE:
    pl = new Polyline((GraphicsItemType)type);
    break;
  case GI_RECTANGLE: {
    pl = new Rectangle();
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
      pl = e;
    } else if (type == GI_ELLIPSE && k > 1) {
      is_using_ellipse = true;
      Arc *e = new Arc();
      e->setRadius(r[0] / wdx, r[1] / wdy);
      if (k > 2) {
        if (k == 3)
          e->setAngles(0.0, r[2]);
        else
          e->setAngles(r[3], r[2] + r[3]);
      }
      pl = e;
    }
    break;
  }
  case GI_DOT:
    if (yylex() == NUM) {
      is_using_dot = true;
      float r = yylval.f / 2.0;
      Dot *d = new Dot();
      d->setRadius(r);
      pl = d;
    }
    break;
  }
  if (pl) {
    pl->setPath(parsePath());
  }
  return pl;
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
  if (yyin == NULL) {
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

Path Parser::parsePath() {
  Path l;
  float x, y;
  point z;

  while ((lastyylex = yylex()) == NUM) {
    x = yylval.f;
    y = parseFloat();
    z.x = (x - wmx) / wdx;
    z.y = (y - wmy) / wdy;
    l.push_back(z);
  }
  if (l.size()==0 && lastyylex==IDLISTA) {
    string name = &yylval.s[1];
    if (listmap.find(name) != listmap.end()) {
      l = process_path(mtpt, listmap[&yylval.s[1]]);
    } else if (name=="buffer") {
      l.insert(l.end(), bufferpt.begin(), bufferpt.end());
      printf("buffer %zu\n", l.size());
      bufferpt.clear();
    } else {
      fprintf(stderr, "Error: Invalid path identifier [%s] %lu\n",
              &yylval.s[1], listmap.size());
    }
  }
  return l;
}

GraphicsItemList Parser::parsePrimitives() {
  GraphicsItemList prlist;
  Structure *st = NULL;
  static int depth = 0;
  Matrix mtst, mtpp, mtrs;
  float pwmx, pwmy, pwdx, pwdy;
  FontFace ff = FN_DEFAULT;
  int n;
  point pp;
  string ctpathname;
  Path ctpath;
  bool is_concatenatepath_active = false;
  bool using_mtlc = false;
  is_spline_to_bezier = false;
  spline_nodes_per_segment = 4;

  depth++;

  // Push window
  pwmx = wmx;
  pwmy = wmy;
  pwdx = wdx;
  pwdy = wdy;
  wmx = wmy = 0;
  wdx = wdy = 1;

  // dbprintf("Depth %d\n", depth);
  while (1) {
    lastyylex = yylex();

    if (lastyylex==IDLISTA) {
      string name = &yylval.s[1];
      if (is_concatenatepath_active) {
        if (listmap.find(name)!=listmap.end()) {
          concat_paths(ctpath, listmap[name], mtpt);
        }
      } else
        listmap[name] = parsePath();
      continue;
    }

    if (lastyylex == 0 || lastyylex == YCLST) {
      // dbprintf("Endwillon %d ", lastyylex);
      break;
    }

    switch (lastyylex) {
    case YWW:
      wmx = parseFloat();
      wdx = parseFloat() - wmx;
      wmy = parseFloat();
      wdy = parseFloat() - wmy;
      // dbprintf("WW %g %g %g %g\n", wmx, wmy, wdx, wdy);
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
      else if (pmat == MTPT)
        oldParseMatrix(yylval.i, mtpt);
      else if (pmat == MTRS)
        oldParseMatrix(yylval.i, mtrs);
      else {
        Transform *t = parseMatrix(yylval.i);
        t->setPredefinedMatrix(pmat);
        prlist.push_back(t);
      }
      using_mtlc = (pmat == MTLC);
      break;
    }
    case YDPST: {
      string name = parseString();
      Structure *strct = Structure::getStructure(name);
      if (!strct) {
        fprintf(stderr, "Error: Structure no definida <%s>\n", name.c_str());
        break;
      } 
      StructurePath *sr = new StructurePath();
      sr->setStructure(strct);
      Path pt;
      pt.push_back(pp);
      sr->setPath(pt);
      prlist.push_back(sr);
      break;
    }
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
        float gap = 0;
        float shift = 1;
        float sc = parseFloat();
        int n = 1;
        bool bothsides = (sc < 0);
        sc = fabs(sc);
        if (yylex() == NUM) {
          shift = 1 - yylval.f;
          if (yylex() == NUM) {
            n = (int)yylval.f;
            if (yylex() == NUM) {
              gap = yylval.f;
              yylex(); // consume the separator
            }
          }
        }
        point llp = parsePoint();
        point rup = parsePoint();
        for (int i = 1; i <= n; i++) {
          StructureLine *sr = new StructureLine();
          sr->setScale(sc, sc);
          sr->setBothSides(bothsides);
          sr->setShift(shift);
          sr->setGap(gap);
          sr->setPoints(llp, rup);
          sr->setStructure(st);
          prlist.push_back(sr);
          if (i < n) {
            mtpp.transform(llp.x, llp.y);
            mtpp.transform(rup.x, rup.y);
          }
        }
      } else {
        fprintf(stderr, "Error: there is no marked structure.\n");
      }
    } break;
    case YARCST: {
      if (st) {
        float sc = parseFloat();
        float r = parseFloat();
        float da = parseFloat();
        float ai = parseFloat();
        float shift = 1;
        int n = 1;
        float af = ai + da;
        bool bothsides = (sc < 0);
        sc = fabs(sc);
        if (yylex() == NUM) {
          shift = 1 - yylval.f;
          if (yylex() == NUM) {
            n = (int)yylval.f;
            yylex(); // consume the separator
          }
        }
        point p = parsePoint();
        for (int i = 1; i <= n; i++) {
          StructureArc *sr = new StructureArc();
          sr->setScale(sc, sc);
          sr->setBothSides(bothsides);
          sr->setShift(shift);
          sr->setRadius(r / wdy);
          sr->setAngles(ai, af);
          sr->setPoint(p);
          sr->setStructure(st);
          prlist.push_back(sr);
          if (i < n)
            mtpp.transform(p.x, p.y);
        }
      } else {
        fprintf(stderr, "Error: there is no marked structure.\n");
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
        fprintf(stderr, "Error: no marked structure.\n");
      }
      break;
    }
    case YRPST: {
      if (st) {
        int n = (int)parseFloat();
        StructurePath *sr = new StructurePath();
        sr->setStructure(st);
        Path pt;
        for (int i = 1; i <= n; i++) {
          pt.push_back(pp);
          mtpp.transform(pp.x, pp.y);
          if (!mtrs.is_identity()) {
            if (i==1) {
              GraphicsState *gs = new GraphicsState(GS_SAVE);
              prlist.push_back(gs);
            }
            sr->setPath(pt);
            prlist.push_back(sr);
            Transform *t = new Transform();
            t->setOperation(OPMCP);
            t->setMatrix(mtrs);
            t->setPredefinedMatrix(MTST);
            prlist.push_back(t);
            sr = new StructurePath();
            sr->setStructure(st);
            pt.clear();
            if (i==n) {
              GraphicsState *gs = new GraphicsState(GS_RESTORE);
              prlist.push_back(gs);
            }
          } else if (i==n) {
            sr->setPath(pt);
            prlist.push_back(sr);
          }
        }
      } else {
        fprintf(stderr, "Error: no marked structure.\n");
      }
      break;
    }
    case YIDENTIFIER: {
      string name = yylval.s;
      Structure *strct = Structure::getStructure(name);
      if (!strct) {
        fprintf(stderr, "Error: invalid identifief [%s]\n", name.c_str());
      } else {
        StructurePath *sr = new StructurePath();
        sr->setStructure(strct);
        sr->setPath(parsePath());
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
      if (type == AT_TALIGN)
        is_using_textalign = true;
      if (type == AT_FPATRN)
        is_using_hatcher = true;
      if (type == AT_LCOLOR || type == AT_FCOLOR) {
        setBegin(cadena);
        yylex();
        int col = get_color_from_string(&yylval.s[1]);
        at->set(type, col);
      } else if (type == AT_TSTYLE) {
        setBegin(cadena);
        yylex();
        ff = get_font_face_from_string(&yylval.s[1]);
        if (ff != FN_NOFACE)
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
      if (yylval.i==GS_CLOSEPATH && is_concatenatepath_active) {
        is_concatenatepath_active = false;
        listmap[ctpathname] = ctpath;
        mtpt.initialize();
        printf("New path %s size %zu\n", ctpathname.c_str(), listmap[ctpathname].size());
        break;
      } else if (yylval.i == GS_PLUMEPOSITION) {
        pp.x = (parseFloat() - wmx) / wdx;
        pp.y = (parseFloat() - wmy) / wdy;
        gs->setPosition(pp);
      } else
        gs->setGSType((GraphicsStateType)yylval.i);
      prlist.push_back(gs);
    } break;
    case YXYDT: {
      pp = parsePoint();
    }
    case YDT: {
      if (!using_mtlc) {
        GraphicsState *gs = new GraphicsState();
        gs->setPosition(pp);
        prlist.push_back(gs);
      }
      setBegin(cadena);
      yylex();
      GraphicsItem *t = parse_text(&yylval.s[1], ff);
      prlist.push_back(t);
      if (lastyylex==YDT) { // ojo, dt debe actualizar la posición de algún modo
        mtpp.transform(pp.x, pp.y);
      }
      break;
    }
    case YLISTA: {
      int n = (int)parseFloat();
      float xx = parseFloat();
      float yy = parseFloat();
      Path l;
      string name = parseString();
      if (listmap.find(name)!=listmap.end()) {
        fprintf(stderr, "Warning: the path %s already exists.\n", name.c_str());
      }
      point z;
      float x = (xx - wmx) / wdx;
      float y = (yy - wmy) / wdy;
      for (int i = 0; i < n; i++) {
        z.x = x;
        z.y = y;
        l.push_back(z);
        mtpt.transform(x, y);
      }
      listmap[name] = l;
      break;
    }
    case YINVPT: {
      string name = parseString();
      Path l = parsePath();
      l.reverse();
      listmap[name] = l;
      break;
    }
    case YGNBZ: {
      string name = parseString();
      Path l = parsePath();
      Path bzp = path_to_bezier(l);
      listmap[name] = bzp;
      break;
    }
    case YRPPT: {
      string name = parseString();
      n = (int)parseFloat();
      for (int i=0; i < n; i++) {
        if (is_concatenatepath_active)
          concat_paths(ctpath, listmap[name], mtpt);
        else {
          concat_paths(bufferpt, listmap[name], mtpt);
        }
      };
      break;
    }
    case YNORMPT: {
      string name = parseString();
      if (listmap.find(name)!=listmap.end()) {
        normalize_path(listmap[name]);
      } else
        fprintf(stderr, "Error NORMPT: the path %s was not found.\n", name.c_str());
      break;
    }
    case YPVPT: {
      string name = parseString();
      point llp, rup;
      llp = parsePoint();
      rup = parsePoint();
      if (listmap.find(name)!=listmap.end()) {
        Matrix mtpr;
        mtpr.to_rectangle(llp.x, llp.y, rup.x, rup.y);
        if (is_concatenatepath_active)
          concat_paths(ctpath, listmap[name], mtpr, false);
        else
          bufferpt = process_path(mtpr, listmap[name]);
        printf("PWPT %s\n", name.c_str());
      } else
        fprintf(stderr, "Error PWPT: the path %s was not found.\n", name.c_str());
      break;
    }
    case YCONCATPATH: {
      is_concatenatepath_active = true;
      ctpathname = parseString();
      ctpath.clear();
      mtpt.initialize();
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
    } break;
    case YOBSOLETE:
      printf("Aviso: Comando obsoleto <>, usar el comando actual.\n");
      break;
    case YEXIT:
      printf("Cerrando parsing\n");
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
  // dbprintf("End Depth %d\n", depth);
  return prlist;
}

MetaGrafica *Parser::parse() {
  mg = new MetaGrafica();
  mg->setName("root");
  mg->setGraphicsItems(parsePrimitives());
  // logprintf("Parseo terminado\n");
  return mg;
}
