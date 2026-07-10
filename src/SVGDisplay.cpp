#include "SVGDisplay.h"
#include "text_parser.h"
#include "font_lmmath_ttf.h"   // Latin Modern Math subset (g_lmmath_ttf / _len)
#include <cmath>
#include <iomanip>

using std::string;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SVGDisplay::SVGDisplay(string fname) {
    filename = fname;
}

// -------------------------------------------------------------
// HELPER: Generación de estilos SVG a partir del DisplayState
// -------------------------------------------------------------
std::string SVGDisplay::getStyleAttributes() {
    std::ostringstream style;
    char colorBuf[10];

    // Relleno: solo si el estado de dibujo tiene fill activo (igual que
    // EPSDisplay/PDFDisplay, que consultan dspstate.fill antes de rellenar).
    if (dspstate.fill) {
        if (!dspstate.hatch.empty()) {
            // Relleno con patrón de tramado: se referencia un <pattern> nativo
            // (emitido perezosamente en <defs>) en vez de un color plano.
            std::string id = ensureHatchPattern(dspstate.hatch);
            style << "fill=\"url(#" << id << ")\" ";
        } else if (dspstate.fillcolor != 0) {
            sprintf(colorBuf, "#%06X", dspstate.fillcolor);
            style << "fill=\"" << colorBuf << "\" ";
        } else {
            int g = (int)(dspstate.fillgray * 255.0);
            sprintf(colorBuf, "#%02X%02X%02X", g, g, g);
            style << "fill=\"" << colorBuf << "\" ";
        }
    } else {
        style << "fill=\"none\" ";
    }

    // Trazo: se omite cuando la figura está rellena sin contorno
    // (misma condición que "!dspstate.fill || dspstate.outlinefill" en EPS).
    if (!dspstate.fill || dspstate.outlinefill) {
        style << "stroke-width=\"" << strokeWidth() << "\" ";
        sprintf(colorBuf, "#%06X", dspstate.linecolor);
        style << "stroke=\"" << colorBuf << "\" ";
        // Misma fuente de verdad que EPS/PDF: Display::dashArrayForIndex().
        if (!dspstate.dash_array.empty()) {
            style << "stroke-dasharray=\"";
            for (size_t i = 0; i < dspstate.dash_array.size(); i++) {
                if (i > 0) style << ",";
                style << dspstate.dash_array[i];
            }
            style << "\" ";
        }
        if (dspstate.line_cap != 0) {
            static const char *caps[] = {"butt", "round", "square"};
            style << "stroke-linecap=\"" << caps[dspstate.line_cap] << "\" ";
        }
        if (dspstate.line_join != 0) {
            static const char *joins[] = {"miter", "round", "bevel"};
            style << "stroke-linejoin=\"" << joins[dspstate.line_join] << "\" ";
        }
    } else {
        style << "stroke=\"none\" ";
    }

    return style.str();
}

// Color de relleno como 6 dígitos hex (sin '#'), desde fillcolor o fillgray.
// Misma conversión que la rama sólida de getStyleAttributes().
std::string SVGDisplay::fillColorHex() {
    char buf[8];
    if (dspstate.fillcolor != 0) {
        sprintf(buf, "%06X", dspstate.fillcolor);
    } else {
        int g = (int)(dspstate.fillgray * 255.0);
        sprintf(buf, "%02X%02X%02X", g, g, g);
    }
    return std::string(buf);
}

// Emite (una sola vez por combinación patrón+color) un <pattern> de tramado y
// devuelve su id, para referenciarlo con fill="url(#id)".
//
// Una familia: el tile es una línea HORIZONTAL repetida verticalmente cada
// `gap`, orientada con patternTransform="rotate(...)" — rotar el tile completo
// (línea + lattice cuadrado) preserva la periodicidad para CUALQUIER ángulo
// (§4.11 fase 2), no solo los 4 de la tabla FPATRN. El <pattern> vive en el
// marco lógico Y-up del <g scale(1,-1)> global, igual que los paths: una línea
// base (1,0) rotada por R queda con dirección (cos R, sin R). Para igualar a
// EPS/PDF —cuya línea apunta a (cos(90-a), sin(90-a))— hace falta R = 90 - a.
// (El viejo a-90 reflejaba en Y: hatch 45° salía \ en vez de /.)
//
// Varias familias (crosshatch, fase 3): patternTransform no sirve para dos
// ángulos a la vez, así que el tile se arma sin rotación con las diagonales
// de un cuadrado de lado gap*sqrt(2) — geometría exacta para 45°+135°, que es
// como el parser construye "crosshatch" (buildHatchPattern).
std::string SVGDisplay::ensureHatchPattern(const FillPattern &fp) {
    if (fp.empty()) return "";

    std::string color = fillColorHex();
    std::ostringstream idss;
    idss << "mgpat";
    for (const HatchLine &h : fp)
        idss << "_" << (long)std::lround(h.angle * 1000) << "_" << (long)std::lround(h.gap * 1000);
    idss << "_" << color;
    std::string id = idss.str();
    if (emitted_patterns.count(id)) return id;
    emitted_patterns.insert(id);

    double sw = strokeWidth();

    if (fp.size() == 1) {
        const HatchLine &h = fp[0];
        fprintf(file,
                "<defs><pattern id=\"%s\" patternUnits=\"userSpaceOnUse\" "
                "width=\"%f\" height=\"%f\" patternTransform=\"rotate(%f)\">"
                "<line x1=\"0\" y1=\"0\" x2=\"%f\" y2=\"0\" stroke=\"#%s\" "
                "stroke-width=\"%f\"/></pattern></defs>\n",
                id.c_str(), h.gap, h.gap, 90.0 - h.angle, h.gap, color.c_str(), sw);
        return id;
    }

    double gap = fp[0].gap;
    double d = gap * sqrt(2.0);
    fprintf(file,
            "<defs><pattern id=\"%s\" patternUnits=\"userSpaceOnUse\" "
            "width=\"%f\" height=\"%f\">"
            "<line x1=\"0\" y1=\"0\" x2=\"%f\" y2=\"%f\" stroke=\"#%s\" stroke-width=\"%f\"/>"
            "<line x1=\"0\" y1=\"%f\" x2=\"%f\" y2=\"0\" stroke=\"#%s\" stroke-width=\"%f\"/>"
            "</pattern></defs>\n",
            id.c_str(), d, d, d, d, color.c_str(), sw, d, d, color.c_str(), sw);
    return id;
}

std::string SVGDisplay::escapeXML(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        unsigned char c = (unsigned char)data[pos];
        switch(c) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:
                if (c < 0x80) {
                    buffer.append(1, (char)c);
                } else {
                    // El lexer entrega los literales de texto en Latin-1 (para la
                    // tabla ISOLatin1Encoding de PostScript); un SVG declarado como
                    // UTF-8 necesita esos bytes re-codificados, no copiados tal cual.
                    buffer.append(1, (char)(0xC0 | (c >> 6)));
                    buffer.append(1, (char)(0x80 | (c & 0x3F)));
                }
                break;
        }
    }
    return buffer;
}

// -------------------------------------------------------------
// CICLO DE VIDA DEL DOCUMENTO Y ESTADO
// -------------------------------------------------------------
void SVGDisplay::start() {
    file = fopen(filename.c_str(), "w");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo SVG %s\n", filename.c_str());
        return;
    }

    // dvx/dvy llegan en cm (ver Display::setDimension); los convertimos a
    // puntos igual que EPSDisplay::start() para que el tamaño del documento
    // coincida entre los tres backends.
    // Escala exacta (§3.2), sin el +0.5 de V1 (que era solo para el
    // BoundingBox entero de EPS y aquí ni eso).
    dvx = dvx * cm_to_point;
    dvy = dvy * cm_to_point;

    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    fprintf(file, "<svg width=\"%fpt\" height=\"%fpt\" viewBox=\"0 %f %f %f\" xmlns=\"http://www.w3.org/2000/svg\">\n",
            dvx, dvy, -dvy, dvx, dvy);

    // MetaGráfica y PostScript crecen hacia arriba. SVG crece hacia abajo.
    // Aplicamos una transformación global para que nuestras matemáticas (y el
    // texto) funcionen igual que en EPS.
    fprintf(file, "<g transform=\"scale(1, -1)\">\n");

    // Igual que EPSDisplay::start(): la semilla mundo→puntos (§3.1) se
    // inyecta como una matriz más, para que se componga automáticamente
    // con las de las estructuras.
    pushWorldMatrix();
}

void SVGDisplay::end() {
    if (!file) return;
    // Vaciar paths pendientes
    stroke();

    // Cerrar cualquier <g> abierto
    while(current_open_groups > 0) {
        fprintf(file, "</g>\n");
        current_open_groups--;
    }

    fprintf(file, "</g>\n"); // Cerrar el scale(1, -1) global
    fprintf(file, "</svg>\n");
    fflush(file);
    fclose(file);
    file = nullptr;
}

void SVGDisplay::save() {
    group_stack.push(current_open_groups);
    current_open_groups = 0;
    fprintf(file, "<g>\n"); // Nuevo contexto
}

void SVGDisplay::restore() {
    // Cerramos el tag <g> del save y todos los generados por transformaciones intermedias
    while(current_open_groups > 0) {
        fprintf(file, "</g>\n");
        current_open_groups--;
    }
    fprintf(file, "</g>\n");
    if (!group_stack.empty()) {
        current_open_groups = group_stack.top();
        group_stack.pop();
    }
}

// -------------------------------------------------------------
// TRANSFORMACIONES — solo la rama MTLC (device*); abrimos grupos
// <g transform="..."> que se cierran en el próximo restore() (igual que
// gsave/grestore envuelve translate/rotate en EPSDisplay). La rama MTST
// es contabilidad común y vive en Display.
// -------------------------------------------------------------
void SVGDisplay::deviceTranslate(double x, double y) {
    // x,y llegan como vector en unidades de mundo; las coordenadas de los
    // paths ya están en puntos, así que el translate se escala con sx/sy
    // igual que EPSDisplay::deviceTranslate. Sin esto el desplazamiento era
    // ~0 y las figuras trasladadas (p.ej. TLLC) caían encima de las originales.
    fprintf(file, "<g transform=\"translate(%f, %f)\">\n", x * sx, y * sy);
    current_open_groups++;
}

void SVGDisplay::deviceRotate(double a) {
    // Se rota alrededor de la posición actual de la pluma (cur_x, cur_y), no del
    // origen. En PostScript/EPS `rotate` gira la CTM pero preserva el punto
    // actual fijado por el moveto previo (p.ej. XYPP antes de RTLC): el texto
    // queda anclado ahí y solo giran los glifos. Rotar sobre el origen mandaría
    // el ancla fuera del lienzo (bug de las etiquetas verticales de eje).
    fprintf(file, "<g transform=\"rotate(%f, %f, %f)\">\n", a, cur_x, cur_y);
    current_open_groups++;
}

void SVGDisplay::deviceScale(double x, double y) {
    fprintf(file, "<g transform=\"scale(%f, %f)\">\n", x, y);
    current_open_groups++;
}

void SVGDisplay::deviceShear(double x, double y) {
    // A diferencia de PostScript, SVG sí soporta shear vía matrix().
    fprintf(file, "<g transform=\"matrix(1, %f, %f, 1, 0, 0)\">\n", y, x);
    current_open_groups++;
}
// deviceInitMatrix: sin override — no hay equivalente sin desenrollar los <g>

void SVGDisplay::structureDefBegin(std::string name) {
    // TODO: emitir <defs><g id="name">...</g></defs> y usar <use> en
    // structure() cuando isDefinedInDevice() sea true. Hoy el pipeline
    // nunca invoca Structure::define_in_device(), así que este método no
    // se ejecuta en la práctica; se deja el stub para cuando se active.
}

void SVGDisplay::structureDefEnd() {}

// -------------------------------------------------------------
// ATRIBUTOS
// -------------------------------------------------------------
void SVGDisplay::setLineWidth(double lw) {
    dspstate.line_width_pt = lw;
    dspstate.line_width_set = true;
}

// Ancho de trazo en puntos. Reproduce el default de EPS/PDF: cuando el .mg
// nunca ejecuta LWIDTH, PostScript (y libharu) usan 1.0 pt, no una línea fina;
// SVG debe hacer lo mismo para que los trazos —y en particular los círculos
// diminutos de datos— tengan el mismo grosor. Un LWIDTH 0 explícito sí es un
// hairline, pero como stroke-width="0" es invisible en SVG se usa un mínimo.
double SVGDisplay::strokeWidth() {
    if (!dspstate.line_width_set)
        return 1.0f;                       // default de PostScript/PDF
    double sw = dspstate.line_width_pt;
    return sw > 0 ? sw : 0.5f;              // LWIDTH 0 explícito -> hairline visible
}
void SVGDisplay::setLineColor(int lc) { dspstate.linecolor = lc; }
void SVGDisplay::setFontSize(double p) { dspstate.fontSize = p; }
void SVGDisplay::setFontFace(FontFace face) { dspstate.fontFace = face; }

void SVGDisplay::setOpenPath(bool op) {
    Display::setOpenPath(op);
    if (!dspstate.openpath)
        stroke();
}

// -------------------------------------------------------------
// PRIMITIVAS DE DIBUJO (PATH BUILDER)
// -------------------------------------------------------------
void SVGDisplay::moveto_nopath(double x, double y) {
    // Solo actualiza la posición (usado para setPlumePosition); no debe
    // arrancar ni ensuciar el path acumulado, igual que en PDFDisplay.
    mt.transform(x, y);
    cur_x = x; cur_y = y;
}

void SVGDisplay::moveto(double x, double y) {
    mt.transform(x, y);
    cur_x = x; cur_y = y;
    path_builder << "M " << x << " " << y << " ";
}

void SVGDisplay::rmoveto(double dx, double dy) {
    // rmoveto solo lo usa la maquetación de texto (TextLine: alineación y
    // sub/superíndices), nunca para construir paths. Debe mover únicamente el
    // cursor de texto, como moveto_nopath. Si escribiera en path_builder
    // ensuciaría el path de la figura siguiente: p.ej. arc() elige M o L según
    // si path_builder está vacío, y un "m" residual lo hacía dibujar una línea
    // espuria desde el punto del texto hasta el arco.
    cur_x += dx; cur_y += dy;
}

void SVGDisplay::lineto(double x, double y) {
    mt.transform(x, y);
    cur_x = x; cur_y = y;
    path_builder << "L " << x << " " << y << " ";
}

void SVGDisplay::rlineto(double dx, double dy) {
    cur_x += dx; cur_y += dy;
    path_builder << "l " << dx << " " << dy << " ";
}

void SVGDisplay::curveto(double x1, double y1, double x2, double y2, double x3, double y3) {
    mt.transform(x1, y1);
    mt.transform(x2, y2);
    mt.transform(x3, y3);
    path_builder << "C " << x1 << " " << y1 << ", "
                 << x2 << " " << y2 << ", "
                 << x3 << " " << y3 << " ";
}

void SVGDisplay::arc(double x, double y, double w, double h, double startAng, double endAng) {
    double sa = startAng, ea = endAng;
    mt.transform(x, y);
    // Radios por norma de columna, igual que EPS/PDF: círculo sigue círculo
    // bajo isometría+rotación.
    mt.transform_radii(w, h);
    if (h == 0) h = w;

    // Misma corrección de signos que EPSDisplay/PDFDisplay cuando la matriz
    // acumulada invierte uno o ambos ejes (estructuras reflejadas).
    if (w < 0 && h >= 0) {
        ea = 180 - startAng;
        sa = ea - endAng;
    } else if (w >= 0 && h < 0) {
        ea = 360 - startAng;
        sa = ea - endAng;
    } else if (w < 0 && h < 0) {
        sa = startAng + 180;
        ea = sa + endAng;
    }

    double startRad = sa * M_PI / 180.0;
    double endRad = ea * M_PI / 180.0;
    double rx = fabs(w), ry = fabs(h);

    double startX = x + rx * cos(startRad);
    double startY = y + ry * sin(startRad);
    double endX = x + rx * cos(endRad);
    double endY = y + ry * sin(endRad);

    if (path_builder.tellp() == 0)
        path_builder << "M " << startX << " " << startY << " ";
    else
        path_builder << "L " << startX << " " << startY << " ";

    if (fabs(ea - sa) >= 360.0) {
        // SVG no permite un arco de 360°: se parte en dos mitades.
        double midX = x - rx * cos(startRad);
        double midY = y - ry * sin(startRad);
        path_builder << "A " << rx << " " << ry << " 0 1 1 " << midX << " " << midY << " "
                     << "A " << rx << " " << ry << " 0 1 1 " << startX << " " << startY << " ";
        cur_x = startX; cur_y = startY;
        stroke();
        return;
    }

    int largeArcFlag = fabs(ea - sa) <= 180.0 ? 0 : 1;
    int sweepFlag = (ea > sa) ? 1 : 0;

    path_builder << "A " << rx << " " << ry << " 0 "
                 << largeArcFlag << " " << sweepFlag << " "
                 << endX << " " << endY << " ";
    cur_x = endX; cur_y = endY;
    stroke();
}

void SVGDisplay::closepath() {
    if (dspstate.openpath) return;
    if (path_builder.tellp() == 0) return;
    path_builder << "Z ";
}

void SVGDisplay::stroke() {
    if (dspstate.openpath) return;
    if (path_builder.tellp() == 0) return; // No hay nada que dibujar

    // Emitir el path acumulado con los estilos actuales
    fprintf(file, "<path d=\"%s\" %s/>\n", path_builder.str().c_str(), getStyleAttributes().c_str());

    // Limpiar el búfer para el siguiente objeto
    path_builder.str("");
    path_builder.clear();
}

void SVGDisplay::line(double x1, double y1, double x2, double y2) {
    moveto(x1, y1);
    lineto(x2, y2);
    stroke();
}

void SVGDisplay::rect(double x1, double y1, double x2, double y2) {
    // (x1,y1)-(x2,y2) son las esquinas opuestas del rectángulo, igual que
    // en EPSDisplay/PDFDisplay (Rectangle::draw pasa llp/rup directamente).
    moveto(x1, y1);
    lineto(x2, y1);
    lineto(x2, y2);
    lineto(x1, y2);
    path_builder << "Z ";
    stroke();
}

void SVGDisplay::dot(double x, double y, double r) {
    // r = RADIO del marcador en unidades de dispositivo (§4.6). La posición la
    // transforma el marco; el tamaño NO (físico). Relleno (disco) o contorno
    // (círculo abierto) según el estado de relleno (§4.6): dot(r)=disco;
    // dot(r,color=c) sin fill=abierto (trazo en color de línea).
    mt.transform(x, y);
    char colorBuf[10];
    if (dspstate.fill) {
        sprintf(colorBuf, "#%06X", dspstate.fillcolor);
        fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"%s\" />\n", x, y, r, colorBuf);
    } else {
        sprintf(colorBuf, "#%06X", dspstate.linecolor);
        fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"none\" stroke=\"%s\" stroke-width=\"%f\" />\n",
                x, y, r, colorBuf, dspstate.line_width_pt);
    }
}

// -------------------------------------------------------------
// TRADUCCIÓN DE FUENTES SIMBÓLICAS A UNICODE
// El parser codifica letras griegas y símbolos matemáticos como bytes de las
// fuentes Symbol (Adobe) o CMMI (TeX). SVG no dispone de esas fuentes, así que
// se traducen a los puntos de código Unicode equivalentes, que cualquier fuente
// del sistema renderiza. Los mapas byte->Unicode (symbolUnicode/cmmiUnicode)
// viven ahora en text_parser.cpp y los comparten los tres backends.
// -------------------------------------------------------------

// Codifica un punto de código Unicode como UTF-8.
static void appendUTF8(std::string &out, unsigned int cp) {
    if (cp < 0x80) {
        out += (char)cp;
    } else if (cp < 0x800) {
        out += (char)(0xC0 | (cp >> 6));
        out += (char)(0x80 | (cp & 0x3F));
    } else {
        out += (char)(0xE0 | (cp >> 12));
        out += (char)(0x80 | ((cp >> 6) & 0x3F));
        out += (char)(0x80 | (cp & 0x3F));
    }
}

// Familias de fuente con métricas compatibles con las base-14 (Times,
// Helvetica, Courier) que usan las tablas de anchos de text.cpp. Se listan
// primero las fuentes con métricas idénticas (Liberation/Nimbus son
// métrica-compatibles) para que el avance de cur_x calculado coincida con el
// ancho realmente renderizado y el texto no se encabalgue ni se separe de más.
static const char *kSerifFamily =
    "'Times New Roman', Times, 'Liberation Serif', 'Nimbus Roman', serif";
static const char *kSansFamily =
    "Helvetica, Arial, 'Liberation Sans', 'Nimbus Sans', sans-serif";
static const char *kMonoFamily =
    "'Courier New', Courier, 'Liberation Mono', 'Nimbus Mono', monospace";

// Traduce la cara de fuente de MetaGráfica a atributos SVG.
static void svgFontAttrs(FontFace f, const char *&family,
                         const char *&style, const char *&weight) {
    family = kSerifFamily; style = "normal"; weight = "normal";
    switch (f) {
    case FN_SERIF_ITALIC:          style = "italic"; break;
    case FN_SERIF_BOLD:            weight = "bold"; break;
    case FN_SERIF_ITALIC_BOLD:     style = "italic"; weight = "bold"; break;
    case FN_SANSERIF:              family = kSansFamily; break;
    case FN_SANSERIF_ITALIC:       family = kSansFamily; style = "italic"; break;
    case FN_SANSERIF_BOLD:         family = kSansFamily; weight = "bold"; break;
    case FN_SANSERIF_ITALIC_BOLD:  family = kSansFamily; style = "italic"; weight = "bold"; break;
    case FN_COURIER:               family = kMonoFamily; break;
    case FN_COURIER_ITALIC:        family = kMonoFamily; style = "italic"; break;
    case FN_COURIER_BOLD:          family = kMonoFamily; weight = "bold"; break;
    case FN_COURIER_ITALIC_BOLD:   family = kMonoFamily; style = "italic"; weight = "bold"; break;
    case FN_SYMBOL:                break;                       // griego/símbolos, recto
    case FN_TEX_CMMI:              style = "italic"; break;     // matemática itálica
    default:                       break;                       // FN_SERIF / FN_DEFAULT
    }
}

// Codifica bytes en base64 estándar (para el data-URI del @font-face).
static void appendBase64(std::string &out, const unsigned char *data, unsigned int len) {
    static const char *T =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    unsigned int i = 0;
    for (; i + 3 <= len; i += 3) {
        unsigned int n = (data[i] << 16) | (data[i + 1] << 8) | data[i + 2];
        out += T[(n >> 18) & 63]; out += T[(n >> 12) & 63];
        out += T[(n >> 6) & 63];  out += T[n & 63];
    }
    if (len - i == 1) {
        unsigned int n = data[i] << 16;
        out += T[(n >> 18) & 63]; out += T[(n >> 12) & 63]; out += "==";
    } else if (len - i == 2) {
        unsigned int n = (data[i] << 16) | (data[i + 1] << 8);
        out += T[(n >> 18) & 63]; out += T[(n >> 12) & 63];
        out += T[(n >> 6) & 63];  out += "=";
    }
}

// ¿El run FN_TEX_CMMI es griego? (todos sus bytes están en cmmiUnicode, es decir,
// son glifos del subset LM Math). Los runs griegos son de un carácter; se
// generaliza por robustez. Cadena vacía = no.
static bool isCmmiGreekRun(const std::string &s) {
    if (s.empty()) return false;
    const std::map<unsigned char, unsigned int> &m = cmmiUnicode();
    for (unsigned char c : s)
        if (m.find(c) == m.end()) return false;
    return true;
}

// Emite el @font-face con LM Math embebida (base64 data-URI), una sola vez, para
// que el griego salga en Computer Modern sin recursos externos.
void SVGDisplay::ensureMathFont() {
    if (math_font_emitted) return;
    math_font_emitted = true;
    std::string b64;
    b64.reserve((g_lmmath_ttf_len + 2) / 3 * 4);
    appendBase64(b64, g_lmmath_ttf, g_lmmath_ttf_len);
    fprintf(file,
            "<defs><style type=\"text/css\">@font-face{font-family:'MGMath';"
            "src:url(data:font/ttf;base64,%s) format('truetype');}</style></defs>\n",
            b64.c_str());
}

std::string SVGDisplay::renderText(const std::string &s) {
    bool symbolic = (dspstate.fontFace == FN_SYMBOL ||
                     dspstate.fontFace == FN_TEX_CMMI);
    const std::map<unsigned char, unsigned int> *umap = nullptr;
    if (dspstate.fontFace == FN_SYMBOL)        umap = &symbolUnicode();
    else if (dspstate.fontFace == FN_TEX_CMMI) umap = &cmmiUnicode();

    std::string out;
    for (unsigned char c : s) {
        unsigned int cp = c;  // por defecto: byte como punto de código Latin-1
        if (symbolic && umap) {
            auto it = umap->find(c);
            if (it != umap->end()) cp = it->second;  // griego/símbolo -> Unicode
            // si no está en la tabla (p.ej. dígito latino en un run CMMI) se
            // deja el byte tal cual, que se renderiza en la cara itálica.
        }
        switch (cp) {
        case '&':  out += "&amp;";  break;
        case '<':  out += "&lt;";   break;
        case '>':  out += "&gt;";   break;
        case '"':  out += "&quot;"; break;
        case '\'': out += "&apos;"; break;
        default:   appendUTF8(out, cp);
        }
    }
    return out;
}

void SVGDisplay::text(string s) {
    // El texto se dibuja en (cur_x, cur_y), la última posición alcanzada por
    // moveto/moveto_nopath (igual que PS usa el "current point" para show).
    // Para que las letras no salgan reflejadas bajo el scale(1,-1) global,
    // el <text> lleva su propio scale(1,-1) local; por eso "y" se da en
    // negativo, de forma que ese segundo flip lo deje en cur_y.
    std::string body = renderText(s);

    const char *family, *style, *weight;
    svgFontAttrs(dspstate.fontFace, family, style, weight);

    // Griego matemático: si el run FN_TEX_CMMI son glifos del subset (están en
    // cmmiUnicode), se dibuja con la LM Math embebida en forma RECTA (los glifos
    // griegos de CM ya son la forma matemática; no aplicar italic evita
    // faux-italic). El latino de math (bytes ASCII, no en el mapa) sigue en el
    // serif del sistema itálico; FN_SYMBOL sin cambios (pendiente P1).
    if (dspstate.fontFace == FN_TEX_CMMI && isCmmiGreekRun(s)) {
        ensureMathFont();
        family = "'MGMath'";
        style = "normal";
    }

    char colorBuf[10];
    sprintf(colorBuf, "#%06X", dspstate.linecolor);

    const char *anchor = "start";
    if (dspstate.text_align == 1) anchor = "middle";
    else if (dspstate.text_align == 2) anchor = "end";

    double size = dspstate.fontSize * relfontsize;
    fprintf(file,
            "<text x=\"%f\" y=\"%f\" transform=\"scale(1, -1)\" text-anchor=\"%s\" "
            "fill=\"%s\" font-size=\"%f\" font-family=\"%s\" font-style=\"%s\" "
            "font-weight=\"%s\">%s</text>\n",
            cur_x, -cur_y, anchor, colorBuf, size, family, style, weight, body.c_str());

    // Avanzar cur_x igual que el "current point" de PostScript tras show, para
    // que los runs siguientes de la misma línea (p.ej. "Cosine ", pi, "/2") no
    // se encABALguen. El ancho se obtiene de las métricas de fuente.
    TextState ts;
    ts.font_face = dspstate.fontFace;
    ts.font_size = relfontsize;
    ts.script = 0;
    cur_x += text_width(ts, s) * dspstate.fontSize;
}
