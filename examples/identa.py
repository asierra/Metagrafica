#!/usr/bin/env python3
# indent_blocks.py

import sys

def indent_file(input_path, output_path=None, indent_size=2):
    if output_path is None:
        output_path = input_path  # sobrescribe por defecto (cuidado)

    with open(input_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    indent_level = 0
    out_lines = []

    for line in lines:
        stripped = line.strip()
        if not stripped:
            out_lines.append('')
            continue

        # Contar llaves de cierre y apertura
        close_count = stripped.count('}')
        open_count = stripped.count('{')

        # Antes de imprimir la línea, reducimos indentación por cada '}'
        indent_level -= close_count
        if indent_level < 0:
            indent_level = 0

        # Añadir la línea con la indentación actual
        out_lines.append(' ' * (indent_level * indent_size) + stripped)

        # Después de imprimir, aumentamos indentación por cada '{'
        indent_level += open_count

    with open(output_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(out_lines))

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Uso: python indent_blocks.py archivo_entrada [archivo_salida]")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None
    indent_file(input_file, output_file)
    print(f"Formateado completado. Salida: {output_file if output_file else input_file}")
