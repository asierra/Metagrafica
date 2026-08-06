#include "text.h"
#include "text_parser.h"   // sus propias declaraciones: sin esto podian divergir
#include <algorithm>
#include <memory>
#include <set>
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

// ---------------------------------------------------------------------------
// Espaciado matemático automático estilo TeX (plan_text_space, Parte B).
//
// En modo math los espacios del fuente se IGNORAN (se consumen sin imprimir): el
// espaciado lo pone la clase de cada átomo. Se clasifica cada átomo (Ord/Op/Bin/
// Rel/Open/Close/Punct) al descargarlo y se inserta, ANTES del run, el espacio que
// pide el par (clase previa, clase actual) según la tabla de TeX. Ese espacio viaja
// como pre_space (em) en el TextState del run; el motor lo mide y lo aplica.
enum MathClass { MC_NONE = 0, MC_ORD, MC_OP, MC_BIN, MC_REL, MC_OPEN, MC_CLOSE, MC_PUNCT, MC_INNER };

// Estado de la máquina, entre átomos de UNA fórmula. Se reinicia en cada '$' (y en
// cada '/n', que abre renglón nuevo). No es reentrante, como el resto del parser.
static int    math_prev_class    = MC_NONE;
static double math_pending_space = 0;   // overrides \, \; \! \quad, aditivos al auto

// `{` y `}` solo llegan aquí ESCAPADOS (`\{`, E1): sin escapar son sintaxis de
// agrupación y se atienden en sus propios `case` del bucle, que nunca caen al
// `default`. Escapados son delimitadores como `(` y `[`, y darles MC_ORD no
// borraría el espaciado sino que lo dejaría MAL POCO, que es lo difícil de ver.
static int mathClassOfByte(unsigned char c) {
  switch (c) {
  case '=': case '<': case '>':  return MC_REL;
  case '+': case '-':            return MC_BIN;   // unario se reclasifica en mathAtomSpace
  case ',': case ';':            return MC_PUNCT;
  case '(': case '[': case '{':  return MC_OPEN;
  case ')': case ']': case '}':  return MC_CLOSE;
  default:                       return MC_ORD;   // letras, dígitos, '.', '|', prima…
  }
}

// Clase de un símbolo \nombre. La mayoría (griego, \infty, \partial, \nabla, \prime…)
// son Ord; solo las relaciones, binarios, delimitadores y grandes operadores difieren.
static int mathClassOfName(const string &n) {
  static const std::set<string> rel = {
    "approx","neq","leq","geq","equiv","sim","in","ni","subset","supset","subseteq",
    "supseteq","propto","cong","mid","leftarrow","rightarrow","leftrightarrow",
    "Leftarrow","Rightarrow","Leftrightarrow","uparrow","downarrow","Uparrow","Downarrow" };
  static const std::set<string> bin = {
    "pm","cdot","times","div","cap","cup","vee","wedge","oplus","otimes","oslash",
    "diamond","bullet" };
  static const std::set<string> open  = { "langle","lceil","lfloor" };
  static const std::set<string> close = { "rangle","rceil","rfloor" };
  static const std::set<string> op    = { "sum","int","prod" };
  if (rel.count(n))   return MC_REL;
  if (bin.count(n))   return MC_BIN;
  if (open.count(n))  return MC_OPEN;
  if (close.count(n)) return MC_CLOSE;
  if (op.count(n))    return MC_OP;
  return MC_ORD;
}

// Espacio (em) entre un átomo de clase prev y uno de clase cur, tabla de TeX:
// thin=3/18, med=4/18, thick=5/18. Las combinaciones "imposibles" en fórmula válida
// (marcadas * en el TeXbook) devuelven 0.
static double mathGlue(int prev, int cur) {
  const double THIN = 3.0/18, MED = 4.0/18, THICK = 5.0/18;
  if (prev == MC_NONE || cur == MC_NONE) return 0;
  // Inner (una fracción, `\frac`) se comporta como Ord salvo que Ord/Close/Punct/Inner
  // le ponen un thin ANTES (Ord-Inner = «1» del TeXbook), y él pone thin antes de
  // Ord/Op/Open/Punct/Inner — por eso `G\frac…` lleva un fino y no queda pegado.
  switch (prev) {
  case MC_ORD:   return cur==MC_OP?THIN : cur==MC_BIN?MED : cur==MC_REL?THICK : cur==MC_INNER?THIN : 0;
  case MC_OP:    return cur==MC_REL?THICK : (cur==MC_ORD||cur==MC_OP||cur==MC_INNER)?THIN : 0;
  case MC_BIN:   return (cur==MC_ORD||cur==MC_OP||cur==MC_OPEN||cur==MC_INNER)?MED : 0;
  case MC_REL:   return (cur==MC_ORD||cur==MC_OP||cur==MC_OPEN||cur==MC_INNER)?THICK : 0;
  case MC_OPEN:  return 0;
  case MC_CLOSE: return cur==MC_OP?THIN : cur==MC_BIN?MED : cur==MC_REL?THICK : cur==MC_INNER?THIN : 0;
  case MC_PUNCT: return THIN;
  case MC_INNER: return cur==MC_BIN?MED : cur==MC_REL?THICK : cur==MC_CLOSE?0 :
                        (cur==MC_OP||cur==MC_ORD||cur==MC_OPEN||cur==MC_PUNCT||cur==MC_INNER)?THIN : 0;
  default:       return 0;
  }
}

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
// §14.1: solo se crea si aparece un `/n`. Mientras sea null, parse_text devuelve
// exactamente lo que devolvía antes (Text o TextLine) → cero churn.
std::unique_ptr<TextBlock> text_block;


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

// Descarga un átomo de clase cur: devuelve el espacio (em) que va ANTES y avanza el
// estado de la máquina de espaciado (plan_text_space, Parte B). Reglas:
//  - Dentro de sub/superíndices (text_state.script != 0) NO hay espaciado inter-átomo
//    y NO se toca el estado: la base y su índice forman UN átomo (como TeX en script
//    style). Así `x^{a=b}` queda pegado y el `=` interno no abre thick.
//  - Un Bin al inicio o tras Bin/Op/Rel/Open/Punct es unario → se reclasifica a Ord
//    (regla de TeX): `-U` pegado, `a-b` con med a ambos lados.
//  - Los overrides (\, \; \! \quad) se acumulan en math_pending_space y se suman aquí.
static double mathAtomSpace(int cur) {
  if (text_state.script != 0) { math_pending_space = 0; return 0; }
  if (cur == MC_BIN &&
      (math_prev_class == MC_NONE  || math_prev_class == MC_BIN ||
       math_prev_class == MC_OP    || math_prev_class == MC_REL ||
       math_prev_class == MC_OPEN  || math_prev_class == MC_PUNCT))
    cur = MC_ORD;
  double sp = mathGlue(math_prev_class, cur) + math_pending_space;
  math_pending_space = 0;
  math_prev_class = cur;
  return sp;
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
  // El pre_space es la ENTRADILLA de UN run: una vez horneado en su Text (arriba), no
  // debe quedar colgado en text_state, o el siguiente run lo heredaría por error
  // (dos espacios colapsados, o un espacio espurio tras un símbolo). Se pone a 0 aquí;
  // el próximo átomo con espacio lo vuelve a fijar (plan_text_space, Parte B).
  text_state.pre_space = 0;
}

// Cierra el run en curso SELLÁNDOLO en el renglón: mueve el `text` pendiente a
// text_line para que el siguiente átomo NO se fusione con él (plan_text_space,
// Parte B). Sin esto, dos runs consecutivos con el MISMO estado y pre_space —p.ej.
// `=` y la `y` que le sigue, ambos con thick— se colapsarían en uno solo (`=y`) y se
// perdería el segundo espacio. Se usa en los límites de espaciado del modo math.
static void mathSeal()
{
  textflush();
  if (text && text->length() > 0) {
    if (!text_line)
      text_line = std::make_unique<TextLine>();
    text_line->addText(std::move(text));
    text = nullptr;
  }
}

void add_symbol(unsigned char symbol_code, FontFace font_face, double pre_space = 0)
{
  if (symbol_code > 0) {
    textflush();                       // cierra el run pendiente: no mezcla su cara con la del símbolo
    tspush();
    text_state.font_face = font_face;
    text_state.pre_space = pre_space;  // espaciado math del símbolo (Parte B)
    accum.push_back(symbol_code);
    textflush();
    tspop();
  } else {
    fprintf(stderr, "Error: Unrecognized code <%x>.\n", symbol_code);
  }
}

void add_word(string word, FontFace font_face, double pre_space = 0)
{
  textflush();
  tspush();
  text_state.font_face = font_face;
  text_state.pre_space = pre_space;
  for(char& c : word)
    accum.push_back(c);
  textflush();
  tspop();
}

// Cierra el renglón en curso y lo mete en el bloque (§14.1). El corte se hace
// DENTRO del bucle principal, no partiendo la cadena de entrada antes, para que el
// estado tipográfico —cara, tamaño, math abierto— siga vivo de un renglón al
// siguiente: `/bTítulo/nsigue en negrita` se comporta como uno espera.
static void flush_line()
{
  if (accum.length() > 0)
    textflush();
  if (!text_block)
    text_block = std::make_unique<TextBlock>();
  if (text_line) {
    if (text) text_line->addText(std::move(text));
    text_block->addLine(std::move(text_line));
  } else {
    text_block->addLine(std::move(text));   // un solo chunk, o null si el renglón va vacío
  }
  text = nullptr;
  text_line = nullptr;
}

// Extrae un grupo {..} balanceado de `s` (ya codificado) empezando en pos, saltando
// espacios/tabs iniciales. Devuelve el contenido en out y deja pos justo TRAS el `}`.
// false si no hay grupo o queda sin cerrar. Lo usa el \frac inline.
static bool extractGroup(const string &s, int n, int &pos, string &out)
{
  while (pos < n && (s[pos] == ' ' || s[pos] == '\t')) pos++;
  if (pos >= n || s[pos] != '{') return false;
  int depth = 0, start = pos + 1;
  for (; pos < n; pos++) {
    // E1: una llave ESCAPADA (`\{`) es un carácter, no un delimitador de grupo. Sin
    // esta guardia `\frac{a\{}{b}` contaría el `\{` como apertura y cortaría los dos
    // grupos en el sitio equivocado — sin una queja, porque las llaves *parecen*
    // balanceadas. Salta el par entero: lo que sigue a la barra no se inspecciona.
    if (s[pos] == '\\' && pos + 1 < n) { pos++; continue; }
    if (s[pos] == '{') depth++;
    else if (s[pos] == '}') {
      if (--depth == 0) { out = s.substr(start, pos - start); pos++; return true; }
    }
  }
  return false;   // grupo sin cierre
}

// Núcleo de parse_text: opera sobre input YA codificado (ISO-8859-1 + ranuras 1..31).
static std::unique_ptr<GraphicsItem>
parse_text_core(const string &input, FontFace ff, bool &using_reencode, bool &using_fontcmmi);

// Parsea un sub-fragmento math (numerador/denominador de \frac) SIN destruir el renglón
// en curso. parse_text_core usa estado global de archivo (accum, text, text_line, la
// máquina de espaciado…) y lo REINICIA al entrar: se salva antes y se restaura después.
// El cuerpo ya viene codificado (subcadena de `input`), así que se procesa como math
// envuelto en $…$ (ASCII) SIN volver a codificar — evita la doble codificación de
// acentos/ranuras que daría re-pasar UTF8toISO8859_1 sobre bytes ya codificados.
static std::unique_ptr<GraphicsItem>
parse_sub(const string &enc_body, FontFace ff, bool &using_reencode, bool &using_fontcmmi)
{
  string                s_accum = std::move(accum);
  TextState             s_state = text_state;
  std::stack<TextState> s_stack; s_stack.swap(tstack);
  std::unique_ptr<Text>      s_text  = std::move(text);
  std::unique_ptr<TextLine>  s_line  = std::move(text_line);
  std::unique_ptr<TextBlock> s_block = std::move(text_block);
  int    s_prev = math_prev_class;
  double s_pend = math_pending_space;

  auto item = parse_text_core("$" + enc_body + "$", ff, using_reencode, using_fontcmmi);

  accum       = std::move(s_accum);
  text_state  = s_state;
  tstack.swap(s_stack);
  text        = std::move(s_text);
  text_line   = std::move(s_line);
  text_block  = std::move(s_block);
  math_prev_class    = s_prev;
  math_pending_space = s_pend;
  return item;
}

std::unique_ptr<GraphicsItem> parse_text(string input_utf8, FontFace ff, bool& using_reencode, bool& using_fontcmmi)
{
  if (input_utf8.size()==0) {
    fprintf(stderr, "Error: void text.\n");
    return nullptr;
  }
  // La codificación propia (acentos byte>=0x80 vía ISOLatin1Encoding + RANURAS 1..31 de
  // kExtraTextGlyphs que solo existen en el /Encoding que emite EPS, §14.4) se hace UNA
  // vez aquí; el núcleo y las recursiones de \frac trabajan ya sobre lo codificado.
  string input = UTF8toISO8859_1(input_utf8.c_str());
  return parse_text_core(input, ff, using_reencode, using_fontcmmi);
}

static std::unique_ptr<GraphicsItem>
parse_text_core(const string &input, FontFace ff, bool &using_reencode, bool &using_fontcmmi)
{
  FontFace font_face = ff;
  text = nullptr;
  text_line = nullptr;
  text_block = nullptr;
  text_state = TextState();
  text_state.font_face = font_face;
  // Estos acumuladores son globales de archivo (la máquina de estados no es
  // reentrante): si una llamada previa quedó con la pila o el acumulador colgados
  // (entrada malformada), hay que limpiarlos para no arrastrar estado a esta.
  accum.clear();
  while (!tstack.empty()) tstack.pop();
  math_prev_class = MC_NONE;   // máquina de espaciado math (Parte B): sin arrastre
  math_pending_space = 0;
  // Detección de reencode sobre el input ya codificado: acentos (byte<0) o ranuras
  // 1..31 obligan a EPS a emitir su /Encoding propio (§14.4).
  for (char c : input)
    if (c < 0 || (c > 0 && c < 32)) {
      using_reencode = true;
      break;
    }
  // v_end es int a propósito: cuando find_* devuelve string::npos (size_t máx) se
  // asigna a int (→ -1) y luego `v_end == string::npos` vuelve a convertir -1 a
  // size_t máx, así que la comparación funciona. Se deja como está (cambiarlo a
  // size_t requeriría revisar `it = v_end - 1` y `v_end > it` con `it` int).
  // v_end es int a propósito: recoge el size_t que devuelve find_first_*, y cuando
  // eso es npos la truncación lo deja en -1, que es contra lo que se compara abajo
  // —con el cast explícito, si no g++ avisa de comparar con y sin signo—.
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
        if (v_end == (int)string::npos) // Last word in string
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
        if (v_end == (int)string::npos) // Last word in string
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
        add_symbol(0x3C, FN_TEX_CMMI, math_mode ? mathAtomSpace(MC_REL) : 0);
        break;
    case '>':
        add_symbol(0x3E, FN_TEX_CMMI, math_mode ? mathAtomSpace(MC_REL) : 0);
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
      if (math_mode) {
        double sp = mathAtomSpace(MC_ORD);   // la prima se pega a su base (Ord)
        textflush();
        add_symbol(map_symbol["prime"], FN_SYMBOL, sp);
      }
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
        math_prev_class = MC_NONE;   // fórmula nueva: el 1er átomo no lleva entradilla
        math_pending_space = 0;
      } else {
        textflush();
        math_mode = false;
        math_pending_space = 0;      // suelta un override colgante al salir del math
        tspop();
      }
      break;
    case '/': {
      // `/n` rompe el renglón (§14.1). Va ANTES de la tabla de estilos porque 'n'
      // no está en font_style_codes y caería al literal. Se eligió `/n` y no `\n`
      // porque `\` consume todo lo alfabético que sigue (así se leen `\alpha` y
      // `\nabla`), así que `\n` sería el símbolo `ndos` en "uno\ndos".
      if (input[it+1] == 'n') {
        it++;
        flush_line();
        math_prev_class = MC_NONE;   // renglón nuevo: el 1er átomo arranca sin entradilla
        break;
      }
      std::size_t found = font_style_codes.find(input[it+1]);
      if (found!=std::string::npos) {
        // E2(b) (2026-08-06): un cambio de cara al FINAL de la cadena no cambia
        // nada — no queda texto al que aplicarlo. Es la firma exacta del `m/s` mal
        // leído: la barra se quiso literal, `/s` se tomó por «sanserif» y se llevó
        // la `s` por delante, en silencio. `5 m` es lo que salía.
        //
        // Se avisa SOLO en este caso porque es el único sin falsos positivos
        // posibles: cambiar de cara y no escribir nada después siempre es inútil.
        // La heurística más amplia que se consideró —«barra pegada a una letra»—
        // SÍ los tiene, medido sobre el corpus: el `$\mu/rm$` de fig2-5 lleva la
        // `/r` pegada a la `u` de `\mu` y es perfectamente legítima. Queda fuera
        // del aviso, por tanto, el `m/s` en MEDIO de una cadena (`v (m/s)`); para
        // ése la defensa es el escape `\/`, que la referencia ahora documenta.
        if (it + 2 >= iend)
          fprintf(stderr,
                  "Aviso: «/%c» al final del texto cambia la cara tipográfica y no queda "
                  "nada que dibujar con ella; si querías una barra literal, escribe «\\/».\n",
                  input[it+1]);
        font_face = change_font_face(input[++it], text_state.font_face, using_fontcmmi);
        if (font_face != text_state.font_face) {
          textflush();
          text_state.font_face = font_face;
        } 
      } else 
        accum.push_back(c);
      }
      break;
    case '\\': {
      // Overrides de espaciado explícito (§ TeX): \, thin, \; thick, \! thin negativo,
      // \quad 1em. Se suman a lo automático (aditivos) y aplican al PRÓXIMO átomo. Van
      // antes del escaneo alfabético porque `,;!` no son letras y el escaneo los ignora.
      unsigned char nx = input[it+1];
      if (math_mode && (nx == ',' || nx == ';' || nx == '!')) {
        math_pending_space += (nx == ',') ? 3.0/18 : (nx == ';') ? 5.0/18 : -3.0/18;
        it++;                 // consume el carácter del override (el `\` ya está en it)
        break;
      }
      if (math_mode && input.compare(it+1, 4, "quad") == 0) {
        math_pending_space += 1.0;
        it += 4;
        break;
      }
      // E1 (2026-08-06): `\` seguido de un carácter NO ALFABÉTICO significa ESE
      // CARÁCTER, literal y sin función. Es el escape que le faltaba al lenguaje de
      // marcado: hasta hoy `{`, `}`, `$`, `\` y `/` seguido de un código de cara eran
      // INALCANZABLES en una cadena. Y no fallaban ruidosamente — el escaneo de abajo
      // consume lo alfabético que sigue a la barra, así que ante `\{` no leía ningún
      // nombre, no entraba al `if (v_end > it)` y se comía los DOS caracteres EN
      // SILENCIO. La misma carencia se llevaba por delante `5 m/s`, que salía `5 m`.
      //
      // Va DESPUÉS de los overrides de espaciado (\, \; \!) porque son el mismo patrón
      // —mirar el siguiente carácter antes del escaneo— y tienen esos tres signos
      // tomados en modo math. Fuera de math no hay conflicto y `\,` es una coma.
      if (it + 1 >= iend) {
        // Barra colgante. Antes se descartaba sin decir nada; el aviso existe porque
        // una cadena que termina en `\` casi siempre es un escape a medio escribir.
        fprintf(stderr, "Aviso: «\\» al final del texto, sin nada que escapar.\n");
        break;   // el it++ del bucle la consume
      }
      if (ALFABETIC.find(nx) == string::npos) {
        it++;                       // consume el carácter escapado
        // En math el literal SIGUE SIENDO UN ÁTOMO y toma la clase que le toca: `\{`
        // abre (MC_OPEN) y `\}` cierra. No se descarga con add_symbol —no es un
        // símbolo de LM Math sino un byte de la cara vigente— sino que se acumula en
        // el run, como cualquier carácter del `default`.
        if (math_mode) {
          double sp = mathAtomSpace(mathClassOfByte(nx));
          if (sp != 0) { mathSeal(); text_state.pre_space = sp; }
        }
        if (nx == '-') using_reencode = true;   // mismo trato que el `-` del `default`
        accum.push_back(nx);
        break;
      }
      textflush();
      v_end = input.find_first_not_of(ALFABETIC, ++it);
      if (v_end == (int)string::npos) // Last word in string
        v_end = iend;
      if (v_end > it && v_end <= iend) {
        string variable = input.substr(it, v_end - it);
        // \frac{A}{B} inline (plan_frac.md): átomo 2-D en medio de la fórmula. Se
        // extraen los dos grupos {..} balanceados y se recursan como math con parse_sub
        // (que preserva el renglón en curso). Una fracción es Inner ≈ Ord para el
        // espaciado: mathAtomSpace(MC_INNER) da su entradilla y avanza la máquina de clases.
        if (math_mode && variable == "frac") {
          double sp = mathAtomSpace(MC_INNER);
          int j = v_end;
          string A, B;
          bool ok = extractGroup(input, iend, j, A) &&
                    extractGroup(input, iend, j, B);
          if (ok) {
            auto frac = std::make_unique<Fraction>();
            frac->setNum(parse_sub(A, font_face, using_reencode, using_fontcmmi));
            frac->setDen(parse_sub(B, font_face, using_reencode, using_fontcmmi));
            frac->setPreSpace(sp);
            mathSeal();                    // sella el run de texto previo como item aparte
            if (!text_line)
              text_line = std::make_unique<TextLine>();
            text_line->addItem(std::move(frac));
            it = j - 1;                    // el it++ del bucle deja it en el char siguiente
          } else {
            fprintf(stderr, "Error: \\frac requiere dos grupos {..}{..}.\n");
            it = v_end - 1;
          }
          break;
        }
        // \hat{X} y \vec{X}: la marca se DIBUJA encima de la base (Accent, en text.h).
        // Un átomo acentuado sigue siendo Ord para el espaciado — acentuar no cambia
        // cómo se relaciona la letra con sus vecinas.
        if (math_mode && (variable == "hat" || variable == "vec")) {
          double sp = mathAtomSpace(MC_ORD);
          int j = v_end;
          string A;
          if (extractGroup(input, iend, j, A)) {
            auto acc = std::make_unique<Accent>();
            acc->setKind(variable == "vec" ? Accent::VEC : Accent::HAT);
            acc->setBase(parse_sub(A, font_face, using_reencode, using_fontcmmi));
            acc->setPreSpace(sp);
            mathSeal();
            if (!text_line)
              text_line = std::make_unique<TextLine>();
            text_line->addItem(std::move(acc));
            it = j - 1;
          } else {
            fprintf(stderr, "Error: \\%s requiere un grupo {..}.\n", variable.c_str());
            it = v_end - 1;
          }
          break;
        }
        unsigned char code = get_symbol_code(variable, font_face, using_fontcmmi);
        if (code > 0)
          add_symbol(code, font_face, math_mode ? mathAtomSpace(mathClassOfName(variable)) : 0);
        else {
          string *f = std::find(math_functions, math_functions+6, variable);
          if (f!=math_functions+6)
            add_word(variable, FN_SERIF, math_mode ? mathAtomSpace(MC_OP) : 0);
          else
            fprintf(stderr, "Warning: symbol name unknown %s\n", variable.c_str());
        }
        it = v_end-1;
      }
      break;
    }
    case ' ':
      // Modo math: los espacios del fuente se CONSUMEN (puro TeX) — el espaciado lo
      // pone la tabla de clases, no el que escribió el usuario. No tocan prev_class:
      // los átomos vecinos se espacian entre sí. Fuera de math, un espacio normal.
      if (math_mode) break;
      accum.push_back(c);
      break;
    case '-': // Best - in Latin1
      using_reencode = true;
    default:
      // Modo math: el carácter es un átomo. Se calcula el espacio que va ANTES según
      // su clase (Ord/Bin/Rel/…); si no es cero, se sella el run previo y se marca la
      // entradilla del nuevo. Ord–Ord (letras, dígitos) da 0 → sigue acumulando en el
      // mismo run (plan_text_space, Parte B).
      if (math_mode) {
        double sp = mathAtomSpace(mathClassOfByte(c));
        if (sp != 0) { mathSeal(); text_state.pre_space = sp; }
      }
      accum.push_back(c);
    }
    it++;
  }
  if (accum.length() > 0)
    textflush();

  // Hubo al menos un `/n`: cierra el último renglón y devuelve el bloque.
  if (text_block) {
    flush_line();
    return std::move(text_block);
  }

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
