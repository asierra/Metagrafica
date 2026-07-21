#include "text.h"
#include "text_parser.h"   // sus propias declaraciones: sin esto podian divergir
#include <algorithm>
#include <memory>
#include <stack>
using std::stack;
using std::string;
using std::map;



const string ALFABETIC = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
const string TOKENLIM = "{}^$_/\\ ";
const string DIGIT = "0123456789";


// Standar symbol font codes
map<string, unsigned char> map_symbol = {
   {"alpha", 97}, {"beta", 98}, {"gamma", 103}, {"delta", 100}, {"zeta", 122}, {"eta", 104},
   {"theta", 113}, {"iota", 105}, {"kappa", 107}, {"lambda", 108}, {"mu", 109}, {"nu", 110},
   {"xi", 120}, {"pi", 112}, {"rho", 114}, {"sigma", 115}, {"tau", 116}, {"upsilon", 117},
   {"phi", 102}, {"chi", 99}, {"psi", 121}, {"omega", 119}, {"varepsilon", 101}, {"vartheta", 74},
   {"varpi", 118}, {"varsigma", 86}, {"varphi", 106}, {"Gamma", 71}, {"Delta", 68}, {"Theta", 81},
   {"Lambda", 76}, {"Xi", 88}, {"Pi", 80}, {"Sigma", 83}, {"Upsilon", 161}, {"Phi", 70},
   {"Psi", 89}, {"Omega", 87}, {"aleph", 192}, {"wp", 195}, {"Re", 194}, {"Im", 193},
   {"partial", 182}, {"infty", 165}, {"prime", 162}, {"nabla", 209}, {"bot", 94}, {"forall", 34},
   {"exists", 36}, {"neg", 216}, {"sharp", 35}, {"clubsuit", 167}, {"diamondsuit", 168}, {"heartsuit", 169},
   {"spadesuit", 170}, {"int", 242}, {"prod", 213}, {"sum", 229}, {"wedge", 217}, {"vee", 218},
   {"cap", 199}, {"cup", 200}, {"diamond", 224}, {"bullet", 183}, {"div", 184}, {"oslash", 198},
   {"otimes", 196}, {"oplus", 197}, {"pm", 177}, {"cdot", 215}, {"times", 180}, {"propto", 181},
   {"mid", 124}, {"Leftrightarrow", 219}, {"Leftarrow", 220}, {"Rightarrow", 222}, {"leq", 163}, {"geq", 179},
   {"approx", 187}, {"supset", 201}, {"subset", 204}, {"supseteq", 202}, {"subseteq", 205}, {"in", 206},
   {"ni", 39}, {"leftrightarrow", 171}, {"leftarrow", 172}, {"rightarrow", 174}, {"sim", 126}, {"equiv", 186},
   {"colon", 58}, {"uparrow", 173}, {"downarrow", 175}, {"Uparrow", 221}, {"Downarrow", 223}, {"rangle", 241},
   {"langle", 225}, {"rceil", 249}, {"lceil", 233}, {"rfloor", 251}, {"lfloor", 235}, {"angle", 208},
   {"therefore", 92}, {"neq", 185}, {"textdegree", 176}, {"cong", 64}, {"surd", 214}
   
   };

// TeX computer modern math font with hbar
map<string, unsigned char> map_tex_cmmi = {
  {"alpha", 174},  {"beta", 175},  {"gamma", 176},
  {"delta", 177},  {"epsilon", 178},  {"zeta", 179},  {"eta", 180},
  {"theta", 181},  {"iota", 182},  {"kappa", 183},  {"lambda", 184},
  {"mu", 185},  {"nu", 186},  {"xi", 187},  {"pi", 188},
  {"rho", 189},  {"sigma", 190},  {"tau", 191},  {"upsilon", 192},
  {"phi", 193},  {"chi", 194},  {"psi", 195},  {"omega", 33},
  {"varepsilon", 34},  {"vartheta", 35},  {"varpi", 36},  {"varrho", 37},
  {"varsigma", 38},  {"varphi", 39},  {"Gamma", 161},  {"Delta", 162},
  {"Theta", 163},  {"Lambda", 164},  {"Xi", 165},  {"Pi", 166},
  {"Sigma", 167},  {"Upsilon", 168},  {"Phi", 169},  {"Psi", 170},
  {"Omega", 172}, {"hbar", 199}
};

// -------------------------------------------------------------
// Tabla nombre->Unicode y mapas byte->Unicode, COMPARTIDOS por los backends
// (SVG, PDF y EPS). Se reutilizan las tablas nombre->byte de arriba
// (map_symbol / map_tex_cmmi) junto con esta tabla nombre->Unicode, para no
// transcribir bytes a mano. Las formas base y sus variantes (var*) se eligen
// para que el glifo Unicode coincida EN FORMA con el que dibuja cmmi/LM Math
// (convención TeX/unicode-math, que NO es la sugerida por los nombres Unicode):
//   \epsilon = lunate ϵ (U+03F5)   \varepsilon = script ε (U+03B5)
//   \phi     = recta  ϕ (U+03D5)   \varphi     = lazo   φ (U+03C6)
static const std::map<std::string, unsigned int> kNameToUnicode = {
    // Griego minúsculas
    {"alpha",0x3B1},{"beta",0x3B2},{"gamma",0x3B3},{"delta",0x3B4},
    {"epsilon",0x3F5},{"zeta",0x3B6},{"eta",0x3B7},{"theta",0x3B8},
    {"iota",0x3B9},{"kappa",0x3BA},{"lambda",0x3BB},{"mu",0x3BC},
    {"nu",0x3BD},{"xi",0x3BE},{"pi",0x3C0},{"rho",0x3C1},
    {"sigma",0x3C3},{"tau",0x3C4},{"upsilon",0x3C5},{"phi",0x3D5},
    {"chi",0x3C7},{"psi",0x3C8},{"omega",0x3C9},
    {"varepsilon",0x3B5},{"vartheta",0x3D1},{"varpi",0x3D6},{"varrho",0x3F1},
    {"varsigma",0x3C2},{"varphi",0x3C6},
    // Griego mayúsculas
    {"Gamma",0x393},{"Delta",0x394},{"Theta",0x398},{"Lambda",0x39B},
    {"Xi",0x39E},{"Pi",0x3A0},{"Sigma",0x3A3},{"Upsilon",0x3A5},
    {"Phi",0x3A6},{"Psi",0x3A8},{"Omega",0x3A9},
    // Símbolos matemáticos
    {"aleph",0x2135},{"wp",0x2118},{"Re",0x211C},{"Im",0x2111},
    {"partial",0x2202},{"infty",0x221E},{"prime",0x2032},{"nabla",0x2207},
    {"bot",0x22A5},{"forall",0x2200},{"exists",0x2203},{"neg",0xAC},
    {"sharp",0x266F},{"clubsuit",0x2663},{"diamondsuit",0x2666},
    {"heartsuit",0x2665},{"spadesuit",0x2660},{"int",0x222B},{"prod",0x220F},
    {"sum",0x2211},{"wedge",0x2227},{"vee",0x2228},{"cap",0x2229},{"cup",0x222A},
    {"diamond",0x22C4},{"bullet",0x2219},{"div",0xF7},{"oslash",0x2298},
    {"otimes",0x2297},{"oplus",0x2295},{"pm",0xB1},{"cdot",0x22C5},
    {"times",0xD7},{"propto",0x221D},{"mid",0x2223},{"Leftrightarrow",0x21D4},
    {"Leftarrow",0x21D0},{"Rightarrow",0x21D2},{"leq",0x2264},{"geq",0x2265},
    {"approx",0x2248},{"supset",0x2283},{"subset",0x2282},{"supseteq",0x2287},
    {"subseteq",0x2286},{"in",0x2208},{"ni",0x220B},{"leftrightarrow",0x2194},
    {"leftarrow",0x2190},{"rightarrow",0x2192},{"sim",0x223C},{"equiv",0x2261},
    {"colon",0x3A},{"uparrow",0x2191},{"downarrow",0x2193},{"Uparrow",0x21D1},
    {"Downarrow",0x21D3},{"rangle",0x232A},{"langle",0x2329},{"rceil",0x2309},
    {"lceil",0x2308},{"rfloor",0x230B},{"lfloor",0x230A},{"angle",0x2220},
    {"therefore",0x2234},{"neq",0x2260},{"textdegree",0xB0},{"cong",0x2245},
    {"surd",0x221A},{"hbar",0x210F}};


// ---------------------------------------------------------------------------
// Caracteres de texto FUERA de Latin-1 que las fuentes base-14 SI tienen (§14.4).
//
// El techo del texto corrido nunca fue Latin-1: es el repertorio de la fuente.
// Una base-14 de PostScript trae ~315 glifos (Times-Roman.afm), entre ellos las
// comillas tipograficas, las rayas, los puntos suspensivos y el resto de abajo.
// Lo unico que faltaba era DARLES POSICION: ISOLatin1Encoding no las tiene, asi
// que se descartaban aunque el glifo estuviera ahi.
//
// Se les asignan las ranuras 1..27, que en texto corrido no aparecen nunca (son
// controles C0). La ranura 0 queda libre a proposito: es el terminador de las
// cadenas C y el motor usa .c_str() en varios sitios.
//
// Cada backend traduce la ranura a lo suyo, que es lo que pide §14.4 ("el texto
// debe viajar y que cada uno resuelva"):
//   EPS -> nombre de glifo, en un /Encoding propio derivado de ISOLatin1
//   PDF -> su byte en WinAnsiEncoding (CP1252 tiene casi todos)
//   SVG -> el codepoint, en UTF-8
//
// Lo que sigue sin caber es lo que la fuente NO tiene (griego en texto corrido,
// cirilico, CJK): eso exige EMBEBER una fuente de texto Unicode en EPS, como ya
// se hace con LM Math para el math, y es una decision aparte.
const ExtraGlyph kExtraTextGlyphs[] = {
    { 1, 0x2018, "quoteleft",      0x91 },
    { 2, 0x2019, "quoteright",     0x92 },
    { 3, 0x201C, "quotedblleft",   0x93 },
    { 4, 0x201D, "quotedblright",  0x94 },
    { 5, 0x2013, "endash",         0x96 },
    { 6, 0x2014, "emdash",         0x97 },
    { 7, 0x2026, "ellipsis",       0x85 },
    { 8, 0x2022, "bullet",         0x95 },
    { 9, 0x2020, "dagger",         0x86 },
    {10, 0x2021, "daggerdbl",      0x87 },
    {11, 0x2030, "perthousand",    0x89 },
    {12, 0x2039, "guilsinglleft",  0x8B },
    {13, 0x203A, "guilsinglright", 0x9B },
    {14, 0x2044, "fraction",       0    },   // 0 = no esta en CP1252
    {15, 0x2122, "trademark",      0x99 },
    {16, 0x20AC, "Euro",           0x80 },
    {17, 0x0192, "florin",         0x83 },
    {18, 0x0152, "OE",             0x8C },
    {19, 0x0153, "oe",             0x9C },
    {20, 0x0160, "Scaron",         0x8A },
    {21, 0x0161, "scaron",         0x9A },
    {22, 0x0178, "Ydieresis",      0x9F },
    {23, 0x017D, "Zcaron",         0x8E },
    {24, 0x017E, "zcaron",         0x9E },
    {25, 0x0131, "dotlessi",       0    },
    {26, 0x0141, "Lslash",         0    },
    {27, 0x0142, "lslash",         0    },
};
const int kNumExtraTextGlyphs = (int)(sizeof(kExtraTextGlyphs)/sizeof(kExtraTextGlyphs[0]));

// codepoint -> ranura, o 0 si el caracter no esta en la tabla.
unsigned char extraSlotFor(unsigned int cp) {
    for (int i = 0; i < kNumExtraTextGlyphs; i++)
        if (kExtraTextGlyphs[i].codepoint == cp) return kExtraTextGlyphs[i].slot;
    return 0;
}
// ranura -> entrada, o nullptr si la ranura no es de esta tabla.
const ExtraGlyph *extraGlyphForSlot(unsigned char slot) {
    for (int i = 0; i < kNumExtraTextGlyphs; i++)
        if (kExtraTextGlyphs[i].slot == slot) return &kExtraTextGlyphs[i];
    return nullptr;
}

// Construye byte->Unicode para una fuente a partir de su tabla nombre->byte.
static std::map<unsigned char, unsigned int>
makeUnicodeMap(const std::map<std::string, unsigned char> &name2byte) {
    std::map<unsigned char, unsigned int> m;
    for (const auto &kv : name2byte) {
        auto it = kNameToUnicode.find(kv.first);
        if (it != kNameToUnicode.end())
            m[kv.second] = it->second;
    }
    return m;
}

// Accesores con inicialización perezosa. Devuelven byte->Unicode para cada
// fuente math; los backends los usan para traducir el byte del run a Unicode.
const std::map<unsigned char, unsigned int> &symbolUnicode() {
    static const std::map<unsigned char, unsigned int> m = makeUnicodeMap(map_symbol);
    return m;
}
// P2 (2026-07-20): en modo math las LETRAS LATINAS tambien salen de LM Math, en
// el plano de italica matematica (U+1D434 A..Z, U+1D44E a..z). Antes caian a
// Times-Italic, asi que una misma formula mezclaba dos tipografias: la Delta en
// Computer Modern y la V al lado en Times. Se anaden a cmmiUnicode() en vez de
// crear otra tabla porque los tres backends ya consultan ESTA para decidir "este
// byte va a LM Math": al entrar aqui, las letras toman la ruta sola.
//
// !! Hueco de Unicode: la h italica NO esta en U+1D455 (posicion sin asignar);
// vive en U+210E, el signo de la constante de Planck. Es el unico caso.
static void addMathItalicLatin(std::map<unsigned char, unsigned int> &m) {
    for (int i = 0; i < 26; i++) {
        m[(unsigned char)('A' + i)] = 0x1D434 + i;
        m[(unsigned char)('a' + i)] = 0x1D44E + i;
    }
    m[(unsigned char)'h'] = 0x210E;

    // Digitos, operadores, agrupadores y puntuacion del modo math (2026-07-20).
    // En TeX los digitos de una formula van RECTOS y del font matematico, no en
    // italica ni de la fuente de texto: sin esto, `E = mc^2` quedaba con las
    // letras en Computer Modern y el '=' y el '2' en Times, que es la misma
    // mezcla que P2 vino a quitar, solo que corrida a los otros caracteres.
    for (int i = 0; i <= 9; i++) m[(unsigned char)('0' + i)] = 0x0030 + i;
    const struct { unsigned char b; unsigned int cp; } ops[] = {
        { '+', 0x002B }, { '=', 0x003D }, { '<', 0x003C }, { '>', 0x003E },
        { '/', 0x002F }, { '(', 0x0028 }, { ')', 0x0029 }, { '[', 0x005B },
        { ']', 0x005D }, { ',', 0x002C }, { '.', 0x002E }, { ';', 0x003B },
        { ':', 0x003A }, { '!', 0x0021 }, { ' ', 0x0020 },
        // El '-' de una formula NO es el guion U+002D: es el SIGNO MENOS U+2212,
        // mas largo y a la altura de la barra del '+'. Es la diferencia que hace
        // que `x-1` parezca partido en dos y `x−1` se lea como una resta. Solo
        // aplica en modo math, que es donde vive esta tabla.
        { '-', 0x2212 },
    };
    for (const auto &o : ops) m[o.b] = o.cp;
}

const std::map<unsigned char, unsigned int> &cmmiUnicode() {
    static const std::map<unsigned char, unsigned int> m = [] {
        auto t = makeUnicodeMap(map_tex_cmmi);
        addMathItalicLatin(t);
        return t;
    }();
    return m;
}

string math_functions[] = {
  "cos", "cot", "csc", "sec", "sin", "tan"
};

// Reservado para futuros generadores de composición matemática (frac/int/prod/
// sum) que armarían una fórmula a partir de varios trozos de texto — requeriría
// una estructura de composición de texto. NO prioritario; y probablemente se
// descarte si a futuro se embeben fragmentos rendereados por TeX. Se deja como
// nota de intención, no como código muerto compilado.
// string math_structs[] = {
//   "frac", "int", "prod", "sum"
// };

string UTF8toISO8859_1(const char * in)
{
    std::string out;
    if (in == NULL)
        return out;
    const char *start = in;   // solo para el aviso: la cadena entera como contexto

    unsigned int codepoint = 0;   // inicializado: ante UTF-8 malformado (byte de
                                  // continuación sin líder) no se usa sin definir
    while (*in != 0)
    {
        unsigned char ch = static_cast<unsigned char>(*in);
        if (ch <= 0x7f)
            codepoint = ch;
        else if (ch <= 0xbf)
            codepoint = (codepoint << 6) | (ch & 0x3f);
        else if (ch <= 0xdf)
            codepoint = ch & 0x1f;
        else if (ch <= 0xef)
            codepoint = ch & 0x0f;
        else
            codepoint = ch & 0x07;
        ++in;
        if (((*in & 0xc0) != 0x80) && (codepoint <= 0x10ffff))
        {
            if (codepoint <= 255)
            {
                out.append(1, static_cast<char>(codepoint));
            }
            else
            {
              // Fuera de Latin-1. Si la fuente base-14 SI tiene el glifo
              // (comillas tipograficas, rayas, puntos suspensivos...), se le da
              // una RANURA de la tabla kExtraTextGlyphs y sigue viaje; cada
              // backend la traduce a lo suyo (§14.4). Solo se descarta lo que la
              // fuente no tiene — griego en texto corrido, cirilico, CJK—, que
              // exigiria embeber una fuente de texto Unicode en EPS.
              //
              // El aviso dice el CODEPOINT y la cadena: antes imprimia `ch`, que a
              // esta altura es el ultimo byte de continuacion del caracter (un
              // 0x9C suelto, ilegible), y no decia donde.
              unsigned char slot = extraSlotFor(codepoint);
              if (slot) {
                out.append(1, static_cast<char>(slot));
              } else {
                fprintf(stderr,
                      "Aviso: carácter U+%04X sin glifo en las fuentes de texto, se descarta — texto: \"%s\"\n",
                      codepoint, start);
              }
            }
        }
    }
    return out;
}

string font_style_codes="beigrsct$";

FontFace change_font_face(unsigned char code_font_face, FontFace font_face, bool& using_fontcmmi) {
  // FN_NOFACE (= -1, "hereda la cara ambiente") NO puede recibir un bit: -1 ya los tiene
  // TODOS prendidos, así que `-1 | FN_ITALIC` sigue siendo -1 y el /i se traga EN SILENCIO.
  // Solo afecta a los modificadores de BIT (/b /e /i); /r /g /s /c /t /$ ASIGNAN una cara
  // y no necesitan base. Se resuelve contra la cara por default, que es exactamente lo que
  // hace un `text()` normal (hornea FN_DEFAULT = FN_SERIF = 0): así `axis(label="/ix")` sale
  // en itálica igual que `text("/ix")`, en vez de caer mudo a la romana ambiente.
  //
  // Solo llegan aquí con FN_NOFACE los textos que heredan la cara: rótulos de axis/numbers/
  // grid (§13). Los que ya fijan cara —`label_font=`, o el modo math, que asigna
  // FN_TIMES_ITALIC antes— no pasan por esta rama, de ahí que el bug se escondiera: fig6-4
  // usa `/i` DENTRO de `$…$` y funciona; fig4-5 lo usa pelado y no.
  if (font_face == FN_NOFACE &&
      (code_font_face == 'b' || code_font_face == 'e' || code_font_face == 'i'))
    font_face = FN_DEFAULT;
  switch (code_font_face) {
  case 'b':
    if (font_face < FN_SYMBOL)                      // There is no bold for symbol
      font_face = (FontFace)(font_face|FN_BOLD);
    else
      font_face = FN_SERIF_BOLD;
    break;
  case 'e':
  case 'i':
    if (font_face < FN_SYMBOL)                      // There is no oblique for symbol
      font_face = (FontFace)(font_face|FN_ITALIC);
    else
      font_face = FN_SERIF_ITALIC;
    break;
  // `/g` (griego en las posiciones del font Symbol) RETIRADO en P1 (2026-07-20).
  // Era de cuando Symbol era la única fuente disponible: mandaba bytes crudos y
  // esperaba la codificación completa de Symbol (griego en las letras, y de paso
  // sus dígitos y signos). Con LM Math la vía canónica del griego es TeX,
  // `\lambda`, que además nombra el glifo en vez de codificarlo en una posición
  // de byte. Al quitarlo, FN_SYMBOL solo se alcanza por `\comando` y basta con
  // codificar los 69 símbolos: el font Symbol desaparece del lenguaje.
  // Cae al `default`, que ya avisa "font face style unknown".

  case 'r': 
    font_face = FN_TIMES_ROMAN;
    break;
  case 's': 
    font_face = FN_SANSERIF;    
    break;
  case 'c':
  case 't':
    font_face = FN_COURIER;
    break;
  case '$':
    font_face = FN_TEX_CMMI;
    using_fontcmmi = true;
    break;
  default:
    fprintf(stderr, "Warning: font face style unknown <%c>.\n", code_font_face);
    font_face = FN_NOFACE;
    break;
  }
  return font_face;
}


FontFace get_font_face_from_string(string s, bool& using_fontcmmi) {
  string out;
  FontFace font_face = FN_DEFAULT;
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  if (s.find("roman") != std::string::npos)
    out += 'r';
  if (s.find("sanserif")!=std::string::npos || s.find("helvetic")!=std::string::npos)
    out += 's';
  if (s.find("courier") != std::string::npos)
    out += 't';
  if (s.find("bold") != std::string::npos)
    out += 'b';
  if (s.find("italic") != std::string::npos)
    out += 'i';
  if (out.empty()) {
    fprintf(stderr, "Warning: font face style unknown <%s>.\n", s.c_str());
    return FN_NOFACE;
  }
  for (auto &cff : out) {
    font_face = change_font_face(cff, font_face, using_fontcmmi);
  }
  return font_face;
}


unsigned char get_symbol_code(string symbol_name, FontFace &font_face, bool& using_fontcmmi)
{
  unsigned char symbol_code;

  if (map_tex_cmmi.find(symbol_name) != map_tex_cmmi.end()) {
    symbol_code = map_tex_cmmi[symbol_name];
    font_face = FN_TEX_CMMI;
    using_fontcmmi = true;
  } else if (map_symbol.find(symbol_name) != map_symbol.end()) {  
    symbol_code = map_symbol[symbol_name];
    font_face = FN_SYMBOL;
    // Desde P1 (2026-07-20) los símbolos también salen de LM Math, así que piden
    // la misma fuente embebida que el griego. La bandera pasó a significar "este
    // documento necesita LM Math", no "usa cmmi" (su nombre se quedó atrás).
    using_fontcmmi = true;
  } else {
    //fprintf(stderr, "Warning: symbol name unknown %s\n", symbol_name.c_str());
    symbol_code = 0;
    font_face = FN_DEFAULT;
  }
  //printf("symbol %s %d\n", symbol_name.c_str(), symbol_code);
  return symbol_code;
}


// Auxiliary text accumulator to create text chunks
string accum;
TextState text_state;
stack<TextState> tstack;
std::unique_ptr<Text> text;
std::unique_ptr<TextLine> text_line;


void tspush() 
{
  //printf("pushing %s\n", text_state.str().c_str());
  tstack.push(text_state);
}

void tspop()
{
  // Guardia contra entrada malformada: un `}` o `$` de más (sin su push) NO debe
  // hacer top()/pop() sobre pila vacía (UB → segfault). Se ignora el pop huérfano.
  if (tstack.empty())
    return;
  text_state = tstack.top();
  tstack.pop();
  //printf("poping %s\n", text_state.str().c_str());
}

// text flush creates new Text with full font state and add it to list
void textflush()
{
  if (accum.size() > 0) {
    if (text==nullptr) {
      text = std::make_unique<Text>();
    } 
    if (text_line==nullptr)
      text_line = std::make_unique<TextLine>();

    if ((text->length() > 0) && (text->getState()==text_state)) {
      text->addText(accum);
      //printf("Cadena add <%s> text state %s\n", accum.c_str(), 
      //  text_state.str().c_str());
    }
    else {
      if (text->length() > 0) {
        text_line->addText(std::move(text));
        text = std::make_unique<Text>();
      }
      text->setState(text_state);
      text->setText(accum);
      //printf("Cadena set <%s> text state %s\n", accum.c_str(), 
      //  text_state.str().c_str());
    } 
    accum = "";
  } else {
    if (text && text->length() > 0) {
      text_line->addText(std::move(text));
      text = nullptr;
    }
  }
}

void add_symbol(unsigned char symbol_code, FontFace font_face)
{
  if (symbol_code > 0) {
    tspush();
    text_state.font_face = font_face;
    accum.push_back(symbol_code);
    textflush();
    tspop();
  } else {
    fprintf(stderr, "Error: Unrecognized code <%x>.\n", symbol_code);
  }
}

void add_word(string word, FontFace font_face)
{
  tspush();
  text_state.font_face = font_face;
  for(char& c : word) 
    accum.push_back(c);
  textflush();
  tspop();
}

std::unique_ptr<GraphicsItem> parse_text(string input_utf8, FontFace ff, bool& using_reencode, bool& using_fontcmmi)
{
  if (input_utf8.size()==0) {
    fprintf(stderr, "Error: void text.\n");
    return nullptr;
  }
  FontFace font_face = ff;
  text = nullptr;
  text_line = nullptr;
  text_state = TextState();
  text_state.font_face = font_face;
  // Estos acumuladores son globales de archivo (la máquina de estados no es
  // reentrante): si una llamada previa quedó con la pila o el acumulador colgados
  // (entrada malformada), hay que limpiarlos para no arrastrar estado a esta.
  accum.clear();
  while (!tstack.empty()) tstack.pop();
  string input = UTF8toISO8859_1(input_utf8.c_str());
  // La codificacion propia hace falta tanto por los acentos (byte >= 0x80, que
  // ISOLatin1Encoding resuelve) como por las RANURAS 1..31 de kExtraTextGlyphs,
  // que solo existen en el /Encoding que emite EPS (§14.4).
  for (char & c : input)
    if (c < 0 || (c > 0 && c < 32)) {
      using_reencode = true;
      break;
    }
  // v_end es int a propósito: cuando find_* devuelve string::npos (size_t máx) se
  // asigna a int (→ -1) y luego `v_end == string::npos` vuelve a convertir -1 a
  // size_t máx, así que la comparación funciona. Se deja como está (cambiarlo a
  // size_t requeriría revisar `it = v_end - 1` y `v_end > it` con `it` int).
  int v_end, it=0, iend = input.size();
  // Modo matemático: '_'/'^' (sub/superíndice) solo operan entre '$…$'; fuera son
  // caracteres literales (así prosa como line_width no se parte en subíndice).
  bool math_mode = false;
  while (it < iend) {
    unsigned char c = input[it];
    switch(c) {
    case '_':
    case '^':
      if (!math_mode) {           // fuera de $…$: carácter literal
        accum.push_back(c);
        break;
      }
      textflush();
      tspush();
      it++;
      text_state.font_size *= fn_relsz_script;
      if (c=='_')
        text_state.script--;
      else
        text_state.script++;
      if (input[it]=='{') {
        break;
      } else if (input[it]=='\\') {
        v_end = input.find_first_not_of(ALFABETIC, ++it);
        if (v_end == string::npos) // Last word in string
          v_end = iend;
        if (v_end > it && v_end <= iend) {
          string variable = input.substr(it, v_end - it);
          unsigned char code = get_symbol_code(variable, font_face, using_fontcmmi);
          if (code > 0) {
            text_state.font_face = font_face;
            accum.push_back(code);
          } else {
            fprintf(stderr, "Error: Unrecognized variable <%s>.\n", variable.c_str());
            tspop();   // balancea el tspush del sub/superíndice antes de salir
            break;
          }
        }
        //it = v_end-1;
      } else {
        v_end = input.find_first_of(TOKENLIM, it);
        if (v_end == string::npos) // Last word in string
          v_end = iend;
        if (it >= v_end){
          //printf("punto %d %d %c\n", it, v_end, input[it]);
                  tspop();   // sub/superíndice vacío o al final: balancea el tspush
                  break;}
        accum = input.substr(it, v_end - it);
      }
      textflush();
      tspop();
      it = v_end-1;
      break;
    case '{':
        textflush();
        tspush();
        //printf("abriendo\n");
        break;
    case '}':
        textflush();
        tspop();
        //printf("cerrando\n");
        break;
    case '<':
        add_symbol(0x3C, FN_TEX_CMMI);
        break;
    case '>':
        add_symbol(0x3E, FN_TEX_CMMI);
        break;
    case '\'':
      // En modo math `'` es una PRIMA (v′, y'' ), como en TeX — no un apostrofo.
      // Hay que decirlo explicitamente por una COLISION DE BYTES: el apostrofo es
      // el byte 39 y en map_tex_cmmi el byte 39 es `varphi`. Mientras la cara por
      // defecto del math fue Times-Italic no se notaba; al volver a FN_TEX_CMMI
      // (P2, 2026-07-20) el apostrofo empezo a salir por esa tabla y `v'=0` se
      // renderizaba como `vφ=0`. Fuera de math sigue siendo un apostrofo normal.
      // El textflush() previo NO sobra: add_symbol acumula sobre el MISMO buffer y
      // lo vuelca con la cara del SIMBOLO, asi que sin vaciar antes, la `v` de
      // `v'` salia por LMMathSym —donde su byte no esta codificado— y desaparecia.
      if (math_mode) { textflush(); add_symbol(map_symbol["prime"], FN_SYMBOL); }
      else           accum.push_back(c);
      break;
    case '$':
      // Abrir/cerrar según la bandera math_mode que el propio `$` mantiene, NO
      // según font_face: un cambio de fuente dentro de $…$ (p.ej. `$\alpha /r x$`)
      // deja font_face != cmmi y el `$` de cierre se tomaría por apertura.
      if (!math_mode) {
        textflush();
        tspush();
        // Cara por DEFECTO del math = FN_TEX_CMMI, o sea LM Math (P2, 2026-07-20).
        //
        // Historia: era FN_TEX_CMMI, pasó a FN_TIMES_ITALIC el 2026-07-11 y vuelve
        // ahora. El motivo de aquel cambio era que el LATINO no estaba en LM Math:
        // un run mixto "\Delta V" mandaba el byte griego a Times-Italic (= ¢) en
        // EPS/PDF, y en SVG el ancho cmmi descuadraba el subíndice. Con P2 el
        // latino SÍ está —cmmiUnicode() cubre griego y letras—, así que un run
        // mixto ya es homogéneo: todo cae en LM Math y no hay nada que partir.
        // Es lo que hace que una fórmula deje de mezclar dos tipografías.
        text_state.font_face = FN_TEX_CMMI;
        // La bandera pide que el backend EMBEBA LM Math. Antes solo la levantaba
        // get_symbol_code al ver un `\comando`, lo que bastaba porque el math sin
        // comandos iba a Times-Italic. Desde P2 el math por defecto YA es LM Math,
        // asi que una formula de solo letras y digitos —`$x'$`, `$y = x^2$`— la
        // necesita igual: sin ella el EPS no define /LMMath y Ghostscript sustituye
        // por una fuente cualquiera (el byte 162 salia como ¢).
        using_fontcmmi = true;
        math_mode = true;
      } else {
        textflush();
        math_mode = false;
        tspop();
      }
      break;
    case '/': {
      std::size_t found = font_style_codes.find(input[it+1]);
      if (found!=std::string::npos) {
        font_face = change_font_face(input[++it], text_state.font_face, using_fontcmmi);
        if (font_face != text_state.font_face) {
          textflush();
          text_state.font_face = font_face;
        } 
      } else 
        accum.push_back(c);
      }
      break;
    case '\\': 
      textflush();
      v_end = input.find_first_not_of(ALFABETIC, ++it);
      if (v_end == string::npos) // Last word in string
        v_end = iend;
      if (v_end > it && v_end <= iend) {
        string variable = input.substr(it, v_end - it);
        unsigned char code = get_symbol_code(variable, font_face, using_fontcmmi);
        if (code > 0)
          add_symbol(code, font_face);
        else {
          string *f = std::find(math_functions, math_functions+6, variable);
          if (f!=math_functions+6)
            add_word(variable,  FN_SERIF);
          else
            fprintf(stderr, "Warning: symbol name unknown %s\n", variable.c_str());
        }
        it = v_end-1;
      }
      break;
    case '-': // Best - in Latin1
      using_reencode = true;
    default:
      accum.push_back(c);
    }
    it++;
  }
  if (accum.length() > 0)
    textflush();

  // Si nunca se descargó un chunk, text_line es nullptr (p.ej. contenido vacío
  // como `$` o `$$`, solo delimitadores): no hay texto que devolver. Sin esta
  // guardia, `text_line->length()` deshace un puntero nulo → segfault.
  if (!text_line)
    return std::move(text);   // text también es null aquí → texto vacío

  if (text_line->length() > 1) {
    textflush();
    return std::move(text_line);
  } else if (text_line->length()==1) {
    // El único chunk pudo quedar ya en text_line (p.ej. `$\phi$`: add_symbol lo
    // descargó y el `$` de cierre lo movió con text=null). Devolvemos el
    // text_line con lo que haya; si además quedó un `text` pendiente, lo unimos.
    if (text) text_line->addText(std::move(text));
    return std::move(text_line);
  } else {
    //printf("simple texto %s\n", text->getText().c_str());
    return std::move(text);
  }
}
