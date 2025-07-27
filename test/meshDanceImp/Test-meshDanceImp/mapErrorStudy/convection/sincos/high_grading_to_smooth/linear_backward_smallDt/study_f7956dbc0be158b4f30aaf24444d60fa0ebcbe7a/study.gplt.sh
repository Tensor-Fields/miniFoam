#!/bin/bash

# When L1_err.dat is ready, and you only need to plot it.
# The comprehensive plot script, which scans the logs to find L1 errors
# is plot.sh which includes this script as well.

if [ -d plot_src ]; then
    rm -r plot_src
fi

mkdir plot_src

#
# Gnuplot
#
echo Gnuplot ...

gnuplot << 'EOF'
set term epslatex 
set output "plot_src/L1_err.tex"

set datafile commentschars '#'

set logscale xy 10

set key top box opaque horizontal outside

set format y "10^{%L}"

set yrange [*:0.01]

unset grid

plot \
     "1/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 19 t "$N_\\mathrm{iter}=1$", \
     "11/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 19 t "$N_\\mathrm{iter}=11$", \
     "17/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 57 t "$N_\\mathrm{iter}=17$", \
     "97/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 207 t "$N_\\mathrm{iter}=97$", \
     "199/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 77 t "$N_\\mathrm{iter}=199$", \
     "359/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 97 t "$N_\\mathrm{iter}=359$", \
     "563/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 107 t "$N_\\mathrm{iter}=563$", \
     "1009/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 17 t "$N_\\mathrm{iter}=1009$", \
     "1151/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 15 t "$N_\\mathrm{iter}=1151$", \
     "1249/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 13 t "$N_\\mathrm{iter}=1249$", \
     "1399/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 27 t "$N_\\mathrm{iter}=1399$", \
     "1499/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 37 t "$N_\\mathrm{iter}=1499$", \
     "1601/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 47 t "$N_\\mathrm{iter}=1601$", \
     "1709/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 67 t "$N_\\mathrm{iter}=1709$", \
     "-"                  u ($1):3 w l t "2nd Order", \
     "-"                  u ($1):3 w l t "1st Order", \
     # 2nd order
     25            0.04    0.000229544
     50            0.02    0.000057386
     100           0.01    0.0000143465
     200           0.005   0.000003586625
     e
     # 1st order
     25            0.04    0.000229544 
     50            0.02    0.000114772 
     100           0.01    0.000057386
     200           0.005    0.000028693
     e
EOF

# Error handling
if [ $? -ne 0 ]; then
    echo "Error: gnuplot failed."
    exit 1
fi

#
# Latex
#

# Write latex file
echo Write latex file

cat > plot_src/plot.tex << EOF
\documentclass{standalone}

\usepackage{xcolor, graphicx}

\begin{document}
    \input{plot_src/L1_err}
\end{document}
EOF

# Compile latex
echo Compile latex
latex -interaction nonstopmode --output-dir plot_src plot_src/plot.tex \
    > plot_src/log.latex

dvips plot_src/plot.dvi > plot_src/log.dvips

ps2pdf plot.ps > plot_src/log.ps2pdf

echo plot.pdf written.

mv plot.ps plot_src
