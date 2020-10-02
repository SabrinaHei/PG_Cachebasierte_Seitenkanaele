set title 'Häufigkeitsverteilung aller gemessenen Bitfehlerraten bei einem Threshold von 173' 
set xlabel 'Bitfehlerrate (in Relation zu Stringlänge)' 
set ylabel 'absolute Häufigkeit'
set boxwidth 0.5
set grid
set ytics 25
set style fill solid
set term pdfcairo enhanced font ",10"
set output 'verteilung-bitfehlerraten.pdf'
plot "Verteilung-Bitfehlerraten.dat" every ::0::6 using 1:3: xtic(2) with boxes linecolor rgb '#07519A' title 'Anzahl
