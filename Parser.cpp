#include "Parser.h"
#include "mgpp_tab.h"
#include "text_parser.h"
#include "splines.h"
#include <algorithm>
#include <stdio.h>
#include <string.h>

extern bool is_using_dot;
extern bool is_using_ellipse;
extern bool is_using_hatcher;
extern bool is_using_textalign;

// Macros de compatibilidad para MGLexer
#define yylval lexer->yylval
#define yylvalaux lexer->yylvalaux
#define yylex lexer->yylex
#define setBegin lexer->setBegin

#define cadena 4

FILE *logout = 0;


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

  if (i == YIDENTIFIER) {
    s = yylval.s;
    free(yylval.s); // Liberar memoria asignada por strdup en el lexer
  } else {
    fprintf(stderr, "Error: A string was expected, not %d\n", i);
  }

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

std::unique_ptr<Transform> Parser::parseMatrix(int mo) {
  point z;
  auto t = std::make_unique<Transform>();
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

std::unique_ptr<GraphicsItem> Parser::parsePrimitive(int type) {
  std::unique_ptr<GraphicsItemWithPath> pl = nullptr;

  switch (type) {
  case GI_SPLINE: {
    Path controlpoints = parsePath();
    if (is_spline_to_bezier) {
      printf("Converting spline to Bezier.\n");
      pl = std::make_unique<Polyline>(GI_BEZIER);
      pl->setPath(splines_to_bezier(controlpoints));
    } else {
      pl = std::make_unique<Polyline>((GraphicsItemType)type);
      pl->setPath(splines(controlpoints, spline_nodes_per_segment));
    }
    return pl;
  }
  return pl;
  case GI_BEZIER:
  case GI_POLYGON:
  case GI_POLYLINE:
    pl = std::make_unique<Polyline>((GraphicsItemType)type);
    break;
  case GI_RECTANGLE: {
    pl = std::make_unique<Rectangle>();
  } break;
  case GI_ELLIPSE:
  case GI_CIRCLE: {
    int k = 0;
    float r[8]; // r d0 a0
    while (yylex() == NUM && k < 8) {
      r[k++] = yylval.f;
    }
    if (type == GI_CIRCLE && k > 0) {
      auto e = std::make_unique<Arc>();
      e->setRadius(r[0] / wdy, r[0] / wdy);
      if (k > 1) {
        if (k == 2)
          e->setAngles(0.0, r[1]);
        else
          e->setAngles(r[2], r[1] + r[2]);
      }
      pl = std::move(e);
    } else if (type == GI_ELLIPSE && k > 1) {
      is_using_ellipse = true;
      auto e = std::make_unique<Arc>();
      e->setRadius(r[0] / wdx, r[1] / wdy);
      if (k > 2) {
        if (k == 3)
          e->setAngles(0.0, r[2]);
        else
          e->setAngles(r[3], r[2] + r[3]);
      }
      pl = std::move(e);
    }
    break;
  }
  case GI_DOT:
    if (yylex() == NUM) {
      is_using_dot = true;
      float r = yylval.f / 2.0;
      auto d = std::make_unique<Dot>();
      d->setRadius(r);
      pl = std::move(d);
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
  pInfile = std::make_unique<std::ifstream>(filename);
  if (!pInfile->is_open()) {
    fprintf(stderr, "Error: File not found %s\n", filename.c_str());
    exit(1);
  }
  lexer = std::make_unique<MGLexer>(pInfile.get());
  
  wmx = wmy = 0;
  wdx = wdy = 1;
  currentDepth = 0;
  mg = nullptr;
}

Parser::~Parser() { }

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
    char* original_s = yylval.s; // Guardar puntero para liberar
    if (listmap.find(name) != listmap.end()) {
      l = process_path(mtpt, listmap[name]);
    } else if (name=="buffer") {
      l.insert(l.end(), bufferpt.begin(), bufferpt.end());
      printf("buffer %zu\n", l.size());
      bufferpt.clear();
    } else {
      fprintf(stderr, "Error: Invalid path identifier [%s] %lu\n",
              name.c_str(), listmap.size());
    }
    free(original_s);
  }
  return l;
}

void Parser::parseMatrixOp(int token, GraphicsItemList &prlist, Matrix &mtpp, Matrix &mtpt, Matrix &mtrs, bool &using_mtlc) {
  PredefinedMatrix pmat = (PredefinedMatrix)yylvalaux.i;
  if (pmat == MTPP)
    oldParseMatrix(yylval.i, mtpp);
  else if (pmat == MTPT)
    oldParseMatrix(yylval.i, mtpt);
  else if (pmat == MTRS)
    oldParseMatrix(yylval.i, mtrs);
  else {
    std::unique_ptr<Transform> t = parseMatrix(yylval.i);
    t->setPredefinedMatrix(pmat);
    prlist.push_back(std::move(t));
  }
  using_mtlc = (pmat == MTLC);
}

void Parser::parseStructureOp(int token, GraphicsItemList &prlist, Structure* &st, Matrix &mtpp, Matrix &mtrs, point &pp) {
  switch (token) {
    case YDPST: {
      string name = parseString();
      Structure *strct = Structure::getStructure(name);
      if (!strct) {
        fprintf(stderr, "Error: Structure no definida <%s>\n", name.c_str());
        break;
      } 
      auto sr = std::make_unique<StructurePath>();
      sr->setStructure(strct);
      Path pt;
      pt.push_back(pp);
      sr->setPath(pt);
      prlist.push_back(std::move(sr));
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
      st->setGraphicsItems(std::move(p));
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
          auto sr = std::make_unique<StructureLine>();
          sr->setScale(sc, sc);
          sr->setBothSides(bothsides);
          sr->setShift(shift);
          sr->setGap(gap);
          sr->setPoints(llp, rup);
          sr->setStructure(st);
          prlist.push_back(std::move(sr));
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
          auto sr = std::make_unique<StructureArc>();
          sr->setScale(sc, sc);
          sr->setBothSides(bothsides);
          sr->setShift(shift);
          sr->setRadius(r / wdy);
          sr->setAngles(ai, af);
          sr->setPoint(p);
          sr->setStructure(st);
          prlist.push_back(std::move(sr));
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
        auto sr = std::make_unique<StructureRectangle>();
        sr->setStructure(st);
        point llp, rup;
        llp = parsePoint();
        rup = parsePoint();
        sr->setPoints(llp, rup);
        prlist.push_back(std::move(sr));
      } else {
        fprintf(stderr, "Error: no marked structure.\n");
      }
      break;
    }
    case YRPST: {
      if (st) {
        int n = (int)parseFloat();
        auto sr = std::make_unique<StructurePath>();
        sr->setStructure(st);
        Path pt;
        for (int i = 1; i <= n; i++) {
          pt.push_back(pp);
          mtpp.transform(pp.x, pp.y);
          if (!mtrs.is_identity()) {
            if (i==1) {
              auto gs = std::make_unique<GraphicsState>(GS_SAVE);
              prlist.push_back(std::move(gs));
            }
            sr->setPath(pt);
            prlist.push_back(std::move(sr));
            auto t = std::make_unique<Transform>();
            t->setOperation(OPMCP);
            t->setMatrix(mtrs);
            t->setPredefinedMatrix(MTST);
            prlist.push_back(std::move(t));
            sr = std::make_unique<StructurePath>();
            sr->setStructure(st);
            pt.clear();
            if (i==n) {
              auto gs = std::make_unique<GraphicsState>(GS_RESTORE);
              prlist.push_back(std::move(gs));
            }
          } else if (i==n) {
            sr->setPath(pt);
            prlist.push_back(std::move(sr));
          }
        }
      } else {
        fprintf(stderr, "Error: no marked structure.\n");
      }
      break;
    }
    case YIDENTIFIER: {
      string name = yylval.s;
      free(yylval.s);
      Structure *strct = Structure::getStructure(name);
      if (!strct) {
        fprintf(stderr, "Error: invalid identifief [%s]\n", name.c_str());
      } else {
        auto sr = std::make_unique<StructurePath>();
        sr->setStructure(strct);
        sr->setPath(parsePath());
        prlist.push_back(std::move(sr));
      }
      break;
    }
  }
}

void Parser::parsePathOp(int token, bool &is_concat, string &ctname, Path &ctpath, Matrix &mtpt) {
  switch (token) {
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
      std::reverse(l.begin(), l.end());
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
      int n = (int)parseFloat();
      for (int i=0; i < n; i++) {
        if (is_concat)
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
        if (is_concat)
          concat_paths(ctpath, listmap[name], mtpr, false);
        else
          bufferpt = process_path(mtpr, listmap[name]);
        printf("PWPT %s\n", name.c_str());
      } else
        fprintf(stderr, "Error PWPT: the path %s was not found.\n", name.c_str());
      break;
    }
    case YCONCATPATH: {
      is_concat = true;
      ctname = parseString();
      ctpath.clear();
      mtpt.initialize();
      break;
    }
  }
}

void Parser::parseAttribute(int token, GraphicsItemList &prlist, FontFace &ff) {
  AttributeType type = (AttributeType)yylval.i;
  auto at = std::make_unique<Attribute>();
  if (type == AT_TALIGN)
    is_using_textalign = true;
  if (type == AT_FPATRN)
    is_using_hatcher = true;
  if (type == AT_LCOLOR || type == AT_FCOLOR) {
    setBegin(cadena);
    yylex();
    int col = get_color_from_string(&yylval.s[1]);
    free(yylval.s);
    at->set(type, col);
  } else if (type == AT_TSTYLE) {
    setBegin(cadena);
    yylex();
    ff = get_font_face_from_string(&yylval.s[1]);
    free(yylval.s);
    if (ff != FN_NOFACE)
      at->set(type, ff);
    else {
      ff = FN_DEFAULT;
      return; // Don't add attribute
    }
  } else {
    float f = parseFloat();
    at->set(type, (int)f);
  }
  prlist.push_back(std::move(at));
}

void Parser::parseGraphicsState(int token, GraphicsItemList &prlist, bool &is_concat, string &ctname, Path &ctpath, point &pp) {
  auto gs = std::make_unique<GraphicsState>();
  if (yylval.i==GS_CLOSEPATH && is_concat) {
    is_concat = false;
    listmap[ctname] = ctpath;
    mtpt.initialize();
    printf("New path %s size %zu\n", ctname.c_str(), listmap[ctname].size());
    return; // Don't add GS
  } else if (yylval.i == GS_PLUMEPOSITION) {
    pp.x = (parseFloat() - wmx) / wdx;
    pp.y = (parseFloat() - wmy) / wdy;
    gs->setPosition(pp);
  } else
    gs->setGSType((GraphicsStateType)yylval.i);
  prlist.push_back(std::move(gs));
}

void Parser::parseTextOp(int token, GraphicsItemList &prlist, point &pp, Matrix &mtpp, FontFace &ff) {
  if (token == YRPNUM) {
    float x0 = parseFloat();
    float xinc = parseFloat();
    int n = (int)parseFloat();
    int ndec = (int)parseFloat();
    char fmt[8], str[80];
    sprintf(fmt, "%%.%df", ndec);
    for (int i = 0; i < n; i++) {
      float x = x0 + i * xinc;
      auto gs = std::make_unique<GraphicsState>();
      gs->setPosition(pp);
      prlist.push_back(std::move(gs));
      mtpp.transform(pp.x, pp.y);
      sprintf(str, fmt, x);
      auto text = std::make_unique<Text>();
      text->setText(str);
      text->setFontFace(ff);
      prlist.push_back(std::move(text));
    }
  } else if (token == YTICKS) {
    point z;
    int n = (int)parseFloat();
    z.x = parseFloat() / wdx;
    z.y = parseFloat() / wdy;
    auto pl = std::make_unique<Polyline>(GI_TICKS);
    Path l;
    l.push_back(z);
    for (int i = 0; i < n; i++) {
      l.push_back(pp);
      mtpp.transform(pp.x, pp.y);
    }
    pl->setPath(l);
    prlist.push_back(std::move(pl));
  }
}

GraphicsItemList Parser::parsePrimitives() {
  GraphicsItemList prlist;
  Structure *st = NULL;
  Matrix mtst, mtpp, mtrs;
  float pwmx, pwmy, pwdx, pwdy;
  FontFace ff = FN_DEFAULT;
  point pp;
  string ctpathname;
  Path ctpath;
  bool is_concatenatepath_active = false;
  bool using_mtlc = false;
  is_spline_to_bezier = false;
  spline_nodes_per_segment = 4;

  currentDepth++;

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
      free(yylval.s);
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
        std::unique_ptr<GraphicsItem> p = parsePrimitive(primtype);
        if (p != NULL) {
          prlist.push_back(std::move(p));
        }
      } while (lastyylex == YSEP);
      break;
    }
    case YOPMAT: {
      parseMatrixOp(lastyylex, prlist, mtpp, mtpt, mtrs, using_mtlc);
      break;
    }
    case YDPST:
    case YOPST:
    case YLNST:
    case YARCST:
    case YMKST:
    case YPVST:
    case YRPST:
    case YIDENTIFIER: {
      parseStructureOp(lastyylex, prlist, st, mtpp, mtrs, pp);
      break;
    }
    case YDEF:
      parseDef(yylval.i);
      break;
    case YATRIB: {
      parseAttribute(lastyylex, prlist, ff);
      break;
    }
    case YGSTATE: {
      parseGraphicsState(lastyylex, prlist, is_concatenatepath_active, ctpathname, ctpath, pp);
    } break;
    case YXYDT: {
      pp = parsePoint();
    }
    case YDT: {
      if (!using_mtlc) {
        auto gs = std::make_unique<GraphicsState>();
        gs->setPosition(pp);
        prlist.push_back(std::move(gs));
      }
      setBegin(cadena);
      yylex();
      std::unique_ptr<GraphicsItem> t = parse_text(&yylval.s[1], ff);
      free(yylval.s);
      prlist.push_back(std::move(t));
      if (lastyylex==YDT) { // ojo, dt debe actualizar la posición de algún modo
        mtpp.transform(pp.x, pp.y);
      }
      break;
    }
    case YLISTA:
    case YINVPT:
    case YGNBZ:
    case YRPPT:
    case YNORMPT:
    case YPVPT:
    case YCONCATPATH: {
      parsePathOp(lastyylex, is_concatenatepath_active, ctpathname, ctpath, mtpt);
      break;
    }
    case YRPNUM:
    case YTICKS:
      parseTextOp(lastyylex, prlist, pp, mtpp, ff);
      break;
    case YOBSOLETE:
      printf("Aviso: Comando obsoleto <>, usar el comando actual.\n");
      break;
    case YEXIT:
      printf("Cerrando parsing\n");
      return prlist;
      break;
    }
  }

  currentDepth--;

  // Pop window
  wmx = pwmx;
  wmy = pwmy;
  wdx = pwdx;
  wdy = pwdy;
  // dbprintf("End Depth %d\n", depth);
  return prlist;
}

std::unique_ptr<MetaGrafica> Parser::parse() {
  auto mg_ptr = std::make_unique<MetaGrafica>();
  mg = mg_ptr.get();
  mg->setName("root");
  mg->setGraphicsItems(parsePrimitives());
  mg = nullptr;
  // logprintf("Parseo terminado\n");
  return mg_ptr;
}
