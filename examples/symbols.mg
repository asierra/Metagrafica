% Catálogo de los símbolos matemáticos que se escriben \comando.
%
% Los 110 nombres, compuestos en Latin Modern Math: 69 de operadores, relaciones,
% flechas y delimitadores, y 41 de letras griegas —minúsculas, variantes,
% mayúsculas y la ħ de `\hbar`—.
%
% NOTAS --------------------------------------------------------------------
% Su trabajo es que la tabla de símbolos no pueda romperse en silencio. El golden
% por bytes NO pudo validar la migración a Latin Modern Math —todos los símbolos
% cambian de fuente, así que los bytes se mueven por diseño— y lo único que
% distinguía «arreglado» de «roto» fue comparar estos glifos antes y después.
%
% LAS GRIEGAS ENTRARON EL 2026-07-23, y hasta entonces este archivo decía que no
% hacían falta «porque el corpus ya las ejercita (turning_points, franck_condon)».
% Medido: el corpus usaba **12 de las 41**. Las ONCE mayúsculas no tenían ninguna
% cobertura, ni 17 minúsculas y variantes; y `epsilon`, `varrho` y `hbar` son los
% tres nombres que existen SOLO en la tabla griega, o sea los únicos que nada más
% podía vigilar.
%
% VAN EN ESTE ARCHIVO Y NO EN UNO NUEVO a propósito. Por dentro son dos tablas
% (una para los símbolos, otra para las griegas, que se consulta primero y por eso
% gana en los 38 nombres que están en ambas), pero esa división es un artefacto de
% la IMPLEMENTACIÓN: quien escribe `\alpha` no sabe de qué tabla sale ni le
% importa. La unidad que el lector reconoce es «lo que puedo escribir con
% \comando». De paso, un solo archivo ejercita las DOS rutas de codificación del
% EPS (/LMMathSym para los bytes de la tabla de símbolos, el vector cmmi para las
% griegas), así que el golden cubre más por archivo, no menos.
%
% ENCUADRE: 12 columnas. `display_size` guarda el aspecto de `world_window`
% (12 × 12.8); si se desajusta, el meet encoge el dibujo entero y se lee como
% «salió chico» en vez de como «se deformó».

display_size 17 18.13
world_window 0 12 -0.9 11.9
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

% Las 41 de la tabla griega: 23 minúsculas, 6 variantes, 11 mayúsculas y hbar.
% Las 12 que el corpus ya usaba van aquí igual: una lámina de referencia se lee
% entera, no por lo que falte en otro sitio.
grsimb = [
       "$\alpha$", "$\beta$", "$\gamma$", "$\delta$",
       "$\epsilon$", "$\zeta$", "$\eta$", "$\theta$",
       "$\iota$", "$\kappa$", "$\lambda$", "$\mu$",
       "$\nu$", "$\xi$", "$\pi$", "$\rho$",
       "$\sigma$", "$\tau$", "$\upsilon$", "$\phi$",
       "$\chi$", "$\psi$", "$\omega$",
       "$\varepsilon$", "$\vartheta$", "$\varpi$",
       "$\varrho$", "$\varsigma$", "$\varphi$",
       "$\Gamma$", "$\Delta$", "$\Theta$", "$\Lambda$",
       "$\Xi$", "$\Pi$", "$\Sigma$", "$\Upsilon$",
       "$\Phi$", "$\Psi$", "$\Omega$",
       "$\hbar$"
]
grnom = [
       "alpha", "beta", "gamma", "delta",
       "epsilon", "zeta", "eta", "theta",
       "iota", "kappa", "lambda", "mu",
       "nu", "xi", "pi", "rho",
       "sigma", "tau", "upsilon", "phi",
       "chi", "psi", "omega",
       "varepsilon", "vartheta", "varpi",
       "varrho", "varsigma", "varphi",
       "Gamma", "Delta", "Theta", "Lambda",
       "Xi", "Pi", "Sigma", "Upsilon",
       "Phi", "Psi", "Omega",
       "hbar"
]

align "center"

% --- Bloque 1: símbolos (69) -------------------------------------------------
font_size 9
text("Símbolos") { 6 11.3 }
for i = 0 to 68 {
    col = mod(i, 12)
    fil = (i - col)/12
    x = col + 0.5
    y = 10.2 - fil
    font_size 13
    text(simb[i]) { (x) (y) }
    font_size 5.5
    text(nom[i]) { (x) (y-0.42) }
}

% --- Bloque 2: letras griegas (41) -------------------------------------------
font_size 9
text("Letras griegas") { 6 4.1 }
for i = 0 to 40 {
    col = mod(i, 12)
    fil = (i - col)/12
    x = col + 0.5
    y = 3.0 - fil
    font_size 13
    text(grsimb[i]) { (x) (y) }
    font_size 5.5
    text(grnom[i]) { (x) (y-0.42) }
}
