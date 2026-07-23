% Catálogo de los símbolos matemáticos que se escriben \comando.
%
% Los 69 nombres de la tabla de símbolos, compuestos en Latin Modern Math. Las
% letras griegas no están aquí: viajan por otra tabla, y las ejercitan
% turning_points.mg y franck_condon.mg.
%
% NOTAS --------------------------------------------------------------------
% Su trabajo es que la tabla de símbolos no pueda romperse en silencio. El golden
% por bytes NO pudo validar la migración a Latin Modern Math —todos los símbolos
% cambian de fuente, así que los bytes se mueven por diseño— y lo único que
% distinguía «arreglado» de «roto» fue comparar estos 69 glifos antes y después.

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
