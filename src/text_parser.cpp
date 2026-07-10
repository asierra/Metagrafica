#include "text.h"
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
   {"alpha", 97}, {"beta", 98}, {"gamma", 103}, {"delta", 100},
   {"zeta", 122}, {"eta", 104}, {"theta", 113}, {"iota", 105},
   {"kappa", 107},{"lambda", 108}, {"mu", 109}, {"nu", 110},
   {"xi", 120}, {"pi", 112}, {"rho", 114}, {"sigma", 115},
   {"tau", 116}, {"upsilon", 117}, {"phi", 102}, {"chi", 99},
   {"psi", 121}, {"omega", 119}, {"varepsilon", 101}, {"vartheta", 74},
   {"varpi", 118}, {"varsigma", 86}, {"varphi", 106}, {"Gamma", 71},
   {"Delta", 68},  {"Theta", 81}, {"Lambda", 76}, {"Xi", 88},
   {"Pi", 80}, {"Sigma", 83}, {"Upsilon", 161}, {"Phi", 70},
   {"Psi", 89}, {"Omega", 87}, {"aleph", 192}, {"wp", 195},
   {"Re", 194}, {"Im", 193}, {"partial", 182}, {"infty", 165}, {"prime", 162},
   {"nabla", 209}, {"bot", 94}, {"forall", 34}, {"exists", 36}, {"neg", 216},
   {"sharp", 35}, {"clubsuit", 167}, {"diamondsuit", 168}, {"heartsuit", 169},
   {"spadesuit", 170}, {"int", 242}, {"prod", 213}, {"sum", 229},
   {"wedge", 217},   {"vee", 218},   {"cap", 199},   {"cup", 200},
   {"diamond", 224},   {"bullet", 183},   {"div", 184},   {"oslash", 198},
   {"otimes", 196},   {"oplus", 197},   {"pm", 177},   {"cdot", 215},
   {"times", 180},   {"propto", 181},   {"mid", 124},   {"Leftrightarrow", 219},
   {"Leftarrow", 220},   {"Rightarrow", 222},   {"leq", 163},   {"geq", 179},
   {"approx", 187},   {"supset", 201},   {"subset", 204},   {"supseteq", 202},
   {"subseteq", 205},   {"in", 206},   {"ni", 39},   {"leftrightarrow", 171},
   {"leftarrow", 172},   {"rightarrow", 174},   {"sim", 126},   {"equiv", 186},
   {"colon", 58},   {"uparrow", 173},   {"alpha", 97},   {"beta", 98},
   {"gamma", 103},   {"delta", 100},   {"zeta", 122},   {"eta", 104},
   {"theta", 113},   {"iota", 105},   {"kappa", 107},   {"lambda", 108},
   {"mu", 109},   {"nu", 110},   {"xi", 120},   {"pi", 112},
   {"rho", 114},   {"sigma", 115},   {"tau", 116},   {"upsilon", 117},
   {"phi", 102},   {"chi", 99},   {"psi", 121},   {"omega", 119},
   {"varepsilon", 101},   {"vartheta", 74},   {"varpi", 118},   {"varsigma", 86},
   {"varphi", 106},   {"Gamma", 71},   {"Delta", 68},   {"Theta", 81},
   {"Lambda", 76},   {"Xi", 88},   {"Pi", 80},   {"Sigma", 83},
   {"Upsilon", 161},   {"Phi", 70},   {"Psi", 89},   {"Omega", 87},
   {"aleph", 192},   {"wp", 195},   {"Re", 194},   {"Im", 193},
   {"partial", 182},   {"infty", 165},   {"prime", 162},   {"nabla", 209},
   {"bot", 94},   {"forall", 34},   {"exists", 36},   {"neg", 216},
   {"sharp", 35},   {"clubsuit", 167},   {"diamondsuit", 168},   {"heartsuit", 169},
   {"spadesuit", 170},   {"int", 242},   {"prod", 213},   {"sum", 229},
   {"wedge", 217},   {"vee", 218},   {"cap", 199},   {"cup", 200},
   {"diamond", 224},   {"bullet", 183},   {"div", 184},   {"oslash", 198},
   {"otimes", 196},   {"oplus", 197},   {"pm", 177},   {"cdot", 215},
   {"times", 180},   {"propto", 181},   {"mid", 124},   {"Leftrightarrow", 219},
   {"Leftarrow", 220},   {"Rightarrow", 222},   {"leq", 163},   {"geq", 179},
   {"approx", 187},   {"supset", 201},   {"subset", 204},   {"supseteq", 202},
   {"subseteq", 205},   {"in", 206},   {"ni", 39},   {"leftrightarrow", 171},
   {"leftarrow", 172},   {"rightarrow", 174},   {"sim", 126},   {"equiv", 186},
   {"colon", 58},   {"uparrow", 173},   {"downarrow", 175},   {"Uparrow", 221},
   {"Downarrow", 223},   {"rangle", 241},   {"langle", 225},   {"rceil", 249},
   {"lceil", 233},   {"rfloor", 251},   {"lfloor", 235},   {"angle", 208},
   {"therefore", 92},   {"neq", 185},   {"textdegree", 176},   {"cong", 64},
   {"downarrow", 175},   {"Uparrow", 221},   {"Downarrow", 223},   {"rangle", 241},
   {"langle", 225},   {"rceil", 249},   {"lceil", 233},   {"rfloor", 251},
   {"lfloor", 235},   {"angle", 208},   {"therefore", 92},   {"neq", 185},
   {"textdegree", 176},   {"cong", 64},   {"surd", 214}
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
const std::map<unsigned char, unsigned int> &cmmiUnicode() {
    static const std::map<unsigned char, unsigned int> m = makeUnicodeMap(map_tex_cmmi);
    return m;
}

string math_functions[] = {
  "cos", "cot", "csc", "sec", "sin", "tan"
};

string math_structs[] = {
  "frac", "int", "prod", "sum"
};

string UTF8toISO8859_1(const char * in)
{
    std::string out;
    if (in == NULL)
        return out;

    unsigned int codepoint;
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
              fprintf(stderr, "Caracter fuera de límites %c\n", ch);
                // do whatever you want for out-of-bounds characters
            }
        }
    }
    return out;
}

string font_style_codes="beigrsct$";

FontFace change_font_face(unsigned char code_font_face, FontFace font_face, bool& using_fontcmmi) {
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
  case 'g':
    font_face = FN_SYMBOL;
    break;
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
  for (char & c : input) 
    if (c < 0) {
      using_reencode = true;
      break;
    }
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
    case '$':
      if (text_state.font_face!=FN_TEX_CMMI) {
        textflush();
        tspush();
        text_state.font_face = FN_TEX_CMMI;
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
