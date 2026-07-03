#include "SVGDisplay.h"
#include "mg.h"
#include <cmath>
#include <iomanip>

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
        if (dspstate.fillpattern > 0) {
            // Relleno con patrón de tramado: se referencia un <pattern> nativo
            // (emitido perezosamente en <defs>) en vez de un color plano.
            std::string id = ensureHatchPattern(dspstate.fillpattern);
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
        // Misma escala que EPSDisplay/PDFDisplay ("l * 0.2"). A diferencia de
        // PostScript/PDF, donde 0 setlinewidth es una convención de "línea
        // más delgada posible", en SVG stroke-width="0" es literalmente
        // invisible, así que se usa un mínimo visible.
        float sw = dspstate.line_width * 0.2f;
        if (sw <= 0) sw = 0.5f;
        style << "stroke-width=\"" << sw << "\" ";
        sprintf(colorBuf, "#%06X", dspstate.linecolor);
        style << "stroke=\"" << colorBuf << "\" ";
        // Mismos patrones que EPSDisplay::setLineStyle, en las mismas unidades.
        switch (dspstate.line_style) {
            case 2: style << "stroke-dasharray=\"4,2\" "; break;
            case 3: style << "stroke-dasharray=\"2,1.6\" "; break;
            case 4: style << "stroke-dasharray=\"4,2,1,2\" "; break;
            case 5: style << "stroke-dasharray=\"4,2,2,2,2,2\" "; break;
            default: break; // 0/1: sólida
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

// Emite (una sola vez por combinación índice+color) un <pattern> de tramado y
// devuelve su id, para referenciarlo con fill="url(#id)".
//
// Estrategia: el patrón se construye siempre con una línea HORIZONTAL repetida
// verticalmente cada `gap`, y se orienta con patternTransform="rotate(...)".
// Así una sola construcción cubre los cuatro ángulos. El descriptor común
// (Display::patternFor) da ángulo y separación; el ángulo se ajusta con -90 y
// el signo del flip global scale(1,-1): hatch0=vertical, hatch90=horizontal,
// hatch45/135 diagonales, igual que el prólogo del EPSDisplay.
std::string SVGDisplay::ensureHatchPattern(int idx) {
    FillPattern fp = patternFor(idx);
    if (fp.empty()) return "";

    std::string color = fillColorHex();
    char idbuf[48];
    sprintf(idbuf, "mgpat_%d_%s", idx, color.c_str());
    std::string id(idbuf);
    if (emitted_patterns.count(id)) return id;
    emitted_patterns.insert(id);

    // Los 10 patrones actuales son una sola familia de líneas. El rayado
    // cruzado/punteado (varias familias o `dashes`) queda pendiente (Paso 4).
    const HatchLine &h = fp[0];
    float sw = dspstate.line_width * 0.2f;
    if (sw <= 0) sw = 0.5f;

    fprintf(file,
            "<defs><pattern id=\"%s\" patternUnits=\"userSpaceOnUse\" "
            "width=\"%f\" height=\"%f\" patternTransform=\"rotate(%f)\">"
            "<line x1=\"0\" y1=\"0\" x2=\"%f\" y2=\"0\" stroke=\"#%s\" "
            "stroke-width=\"%f\"/></pattern></defs>\n",
            id.c_str(), h.gap, h.gap, h.angle - 90.0f, h.gap, color.c_str(), sw);
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
    dvx = dvx * cm_to_point + 0.5;
    dvy = dvy * cm_to_point + 0.5;

    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    fprintf(file, "<svg width=\"%fpt\" height=\"%fpt\" viewBox=\"0 %f %f %f\" xmlns=\"http://www.w3.org/2000/svg\">\n",
            dvx, dvy, -dvy, dvx, dvy);

    // MetaGráfica y PostScript crecen hacia arriba. SVG crece hacia abajo.
    // Aplicamos una transformación global para que nuestras matemáticas (y el
    // texto) funcionen igual que en EPS.
    fprintf(file, "<g transform=\"scale(1, -1)\">\n");

    // Igual que EPSDisplay::start(): la escala de dvx/dvy que lleva del
    // espacio normalizado del documento a puntos se inyecta como una matriz
    // más, para que se componga automáticamente con las de las estructuras.
    Matrix mtmp;
    mtmp.scale(dvx, dvy);
    pushMatrix(mtmp);
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
void SVGDisplay::deviceTranslate(float x, float y) {
    // x,y llegan en coordenadas normalizadas del documento; las coordenadas de
    // los paths ya están en puntos, así que el translate debe escalarse a
    // puntos igual que EPSDisplay::deviceTranslate (x*dvx, y*dvy). Sin esto el
    // desplazamiento era ~0 y las figuras trasladadas (p.ej. TLLC) caían encima
    // de las originales.
    fprintf(file, "<g transform=\"translate(%f, %f)\">\n", x * dvx, y * dvy);
    current_open_groups++;
}

void SVGDisplay::deviceRotate(float a) {
    fprintf(file, "<g transform=\"rotate(%f)\">\n", a);
    current_open_groups++;
}

void SVGDisplay::deviceScale(float x, float y) {
    fprintf(file, "<g transform=\"scale(%f, %f)\">\n", x, y);
    current_open_groups++;
}

void SVGDisplay::deviceShear(float x, float y) {
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
void SVGDisplay::setLineStyle(int ls) { dspstate.line_style = ls; }
void SVGDisplay::setLineWidth(float lw) { dspstate.line_width = lw; }
void SVGDisplay::setLineColor(int lc) { dspstate.linecolor = lc; }
void SVGDisplay::setFontSize(float p) { dspstate.fontSize = p; }
void SVGDisplay::setFontFace(FontFace face) { dspstate.fontFace = face; }

void SVGDisplay::setOpenPath(bool op) {
    Display::setOpenPath(op);
    if (!dspstate.openpath)
        stroke();
}

// -------------------------------------------------------------
// PRIMITIVAS DE DIBUJO (PATH BUILDER)
// -------------------------------------------------------------
void SVGDisplay::moveto_nopath(float x, float y) {
    // Solo actualiza la posición (usado para setPlumePosition); no debe
    // arrancar ni ensuciar el path acumulado, igual que en PDFDisplay.
    mt.transform(x, y);
    cur_x = x; cur_y = y;
}

void SVGDisplay::moveto(float x, float y) {
    mt.transform(x, y);
    cur_x = x; cur_y = y;
    path_builder << "M " << x << " " << y << " ";
}

void SVGDisplay::rmoveto(float dx, float dy) {
    cur_x += dx; cur_y += dy;
    path_builder << "m " << dx << " " << dy << " ";
}

void SVGDisplay::lineto(float x, float y) {
    mt.transform(x, y);
    cur_x = x; cur_y = y;
    path_builder << "L " << x << " " << y << " ";
}

void SVGDisplay::rlineto(float dx, float dy) {
    cur_x += dx; cur_y += dy;
    path_builder << "l " << dx << " " << dy << " ";
}

void SVGDisplay::curveto(float x1, float y1, float x2, float y2, float x3, float y3) {
    mt.transform(x1, y1);
    mt.transform(x2, y2);
    mt.transform(x3, y3);
    path_builder << "C " << x1 << " " << y1 << ", "
                 << x2 << " " << y2 << ", "
                 << x3 << " " << y3 << " ";
}

void SVGDisplay::arc(float x, float y, float w, float h, float startAng, float endAng) {
    float sa = startAng, ea = endAng;
    mt.transform(x, y);
    mt.transf2d(w, h);
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

    float startRad = sa * M_PI / 180.0;
    float endRad = ea * M_PI / 180.0;
    float rx = fabs(w), ry = fabs(h);

    float startX = x + rx * cos(startRad);
    float startY = y + ry * sin(startRad);
    float endX = x + rx * cos(endRad);
    float endY = y + ry * sin(endRad);

    if (path_builder.tellp() == 0)
        path_builder << "M " << startX << " " << startY << " ";
    else
        path_builder << "L " << startX << " " << startY << " ";

    if (fabs(ea - sa) >= 360.0) {
        // SVG no permite un arco de 360°: se parte en dos mitades.
        float midX = x - rx * cos(startRad);
        float midY = y - ry * sin(startRad);
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

void SVGDisplay::stroke() {
    if (dspstate.openpath) return;
    if (path_builder.tellp() == 0) return; // No hay nada que dibujar

    // Emitir el path acumulado con los estilos actuales
    fprintf(file, "<path d=\"%s\" %s/>\n", path_builder.str().c_str(), getStyleAttributes().c_str());

    // Limpiar el búfer para el siguiente objeto
    path_builder.str("");
    path_builder.clear();
}

void SVGDisplay::line(float x1, float y1, float x2, float y2) {
    moveto(x1, y1);
    lineto(x2, y2);
    stroke();
}

void SVGDisplay::rect(float x1, float y1, float x2, float y2) {
    // (x1,y1)-(x2,y2) son las esquinas opuestas del rectángulo, igual que
    // en EPSDisplay/PDFDisplay (Rectangle::draw pasa llp/rup directamente).
    moveto(x1, y1);
    lineto(x2, y1);
    lineto(x2, y2);
    lineto(x1, y2);
    path_builder << "Z ";
    stroke();
}

void SVGDisplay::dot(float x, float y, float r) {
    // "dot" en EPS/PDF usa r como diámetro (radio = r/2) y toma el color de
    // línea actual, no un negro fijo.
    mt.transform(x, y);
    char colorBuf[10];
    sprintf(colorBuf, "#%06X", dspstate.linecolor);
    fprintf(file, "<circle cx=\"%f\" cy=\"%f\" r=\"%f\" fill=\"%s\" />\n", x, y, r / 2.0f, colorBuf);
}

void SVGDisplay::text(string s) {
    // El texto se dibuja en (cur_x, cur_y), la última posición alcanzada por
    // moveto/moveto_nopath (igual que PS usa el "current point" para show).
    // Para que las letras no salgan reflejadas bajo el scale(1,-1) global,
    // el <text> lleva su propio scale(1,-1) local; por eso "y" se da en
    // negativo, de forma que ese segundo flip lo deje en cur_y.
    std::string safeText = escapeXML(s);

    char colorBuf[10];
    sprintf(colorBuf, "#%06X", dspstate.linecolor);

    const char *anchor = "start";
    if (dspstate.text_align == 1) anchor = "middle";
    else if (dspstate.text_align == 2) anchor = "end";

    fprintf(file,
            "<text x=\"%f\" y=\"%f\" transform=\"scale(1, -1)\" text-anchor=\"%s\" "
            "fill=\"%s\" font-size=\"%f\" font-family=\"sans-serif\">%s</text>\n",
            cur_x, -cur_y, anchor, colorBuf, dspstate.fontSize * relfontsize, safeText.c_str());
}
