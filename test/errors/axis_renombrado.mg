% EXPECT: se renombró a `label=`
% Los nombres viejos de §13 tienen que FALLAR, no ignorarse: el renombre
% title->label / labels->tick_labels no puede ser silencioso.
display_size 5 5
axis(from=0, to=10, step=1, title="x") { 0 0  1 0 }
