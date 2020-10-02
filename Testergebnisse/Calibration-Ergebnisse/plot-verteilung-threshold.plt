set title 'Häufigkeitsverteilung aller gemessenen Thresholds'
set xlabel 'Thresholds (in CPU-Zykeln)'
set ylabel 'absolute Häufigkeit'
set xrange [150:220]
set grid
set xtics 5
set boxwidth 0.5
set style fill solid
set term pdfcairo enhanced font ",10"
set output 'verteilung-thresholds.pdf'
plot "gesamt-thresholds.dat" using 2: xtic(20) with boxes linecolor rgb '#07519A' title 'Anzahl

