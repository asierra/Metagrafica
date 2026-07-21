% ═════════════════════════════════════════════════════════════════════════════
% Catálogo de los símbolos matemáticos que se escriben `\comando` (§14).
%
% Showcase, como `markers-demo` o `line_patterns`: su trabajo es que la tabla de
% símbolos no pueda romperse en silencio. Son los 69 nombres de
% `map_symbol` que salen por el font **Symbol**; los ~38 griegos de esa misma
% tabla NO están aquí porque `get_symbol_code` los desvía antes a `map_tex_cmmi`
% (ya en Latin Modern Math) y el corpus ya los ejercita —`turning_points`,
% `franck_condon`.
%
% Es la referencia de la migración P1 de `plan_lmmath.md` (Symbol → LM Math):
% el golden por bytes NO puede validar ese cambio —todos los símbolos cambian de
% fuente, así que los bytes se mueven por diseño— y lo único que distingue
% "arreglado" de "roto" es comparar estos 69 glifos antes y después.
% ═════════════════════════════════════════════════════════════════════════════

display_size 17 12
world_window 0 7 0 10.6
font "Times-Roman"

simb = [
       "$\aleph$", "$\wp$", "$\Re$", "$\Im$",
       "$\partial$", "$\infty$", "$\prime$", "$\nabla$",
       "$\bot$", "$\forall$", "$\exists$", "$\neg$",
       "$\sharp$", "$\clubsuit$", "$\diamondsuit$", "$\heartsuit$",
       "$\spadesuit$", "$\int$", "$\prod$", "$\sum$",
       "$\wedge$", "$\vee$", "$\cap$", "$\cup$",
       "$\diamond$", "$\bullet$", "$\div$", "$\oslash$",
       "$\otimes$", "$\oplus$", "$\pm$", "$\cdot$",
       "$\times$", "$\propto$", "$\mid$", "$\Leftrightarrow$",
       "$\Leftarrow$", "$\Rightarrow$", "$\leq$", "$\geq$",
       "$\approx$", "$\supset$", "$\subset$", "$\supseteq$",
       "$\subseteq$", "$\in$", "$\ni$", "$\leftrightarrow$",
       "$\leftarrow$", "$\rightarrow$", "$\sim$", "$\equiv$",
       "$\colon$", "$\uparrow$", "$\downarrow$", "$\Uparrow$",
       "$\Downarrow$", "$\rangle$", "$\langle$", "$\rceil$",
       "$\lceil$", "$\rfloor$", "$\lfloor$", "$\angle$",
       "$\therefore$", "$\neq$", "$\textdegree$", "$\cong$",
       "$\surd$"
]
nom = [
       "aleph", "wp", "Re", "Im",
       "partial", "infty", "prime", "nabla",
       "bot", "forall", "exists", "neg",
       "sharp", "clubsuit", "diamondsuit", "heartsuit",
       "spadesuit", "int", "prod", "sum",
       "wedge", "vee", "cap", "cup",
       "diamond", "bullet", "div", "oslash",
       "otimes", "oplus", "pm", "cdot",
       "times", "propto", "mid", "Leftrightarrow",
       "Leftarrow", "Rightarrow", "leq", "geq",
       "approx", "supset", "subset", "supseteq",
       "subseteq", "in", "ni", "leftrightarrow",
       "leftarrow", "rightarrow", "sim", "equiv",
       "colon", "uparrow", "downarrow", "Uparrow",
       "Downarrow", "rangle", "langle", "rceil",
       "lceil", "rfloor", "lfloor", "angle",
       "therefore", "neq", "textdegree", "cong",
       "surd"
]

for i = 0 to 68 {
    col = mod(i, 7)
    fil = (i - col)/7
    x = col + 0.5
    y = 10 - fil
    font_size 13
    align "center"
    text(simb[i]) { (x) (y) }
    font_size 6
    text(nom[i]) { (x) (y-0.42) }
}
