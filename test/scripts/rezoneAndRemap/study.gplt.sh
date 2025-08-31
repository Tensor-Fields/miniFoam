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
set term epslatex size 16cm,11cm
#set term epslatex standalone size 16cm,11cm
#set term epslatex monochrome size 16cm,11cm

set output "plot_src/L1_err.tex"

#set term qt

set palette gray

set datafile commentschars '#'

set logscale xy 10

unset key

#set key font ",20"
#set key maxrows 9
set key left box opaque outside width -15
#set key right Right box opaque outside width -8

set format y "10^{%L}"

unset yrange
#set yrange [*:0.001]
#set yrange [3e-6:0.001]

unset grid

#plot \
#     "1/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 19 t "$N_\\mathrm{iter}=1$", \
#     "11/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 19 t "$N_\\mathrm{iter}=11$", \
#     "17/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 57 t "$N_\\mathrm{iter}=17$", \
#     "97/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 207 t "$N_\\mathrm{iter}=97$", \
#     "99/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 207 t "$N_\\mathrm{iter}=99$", \
#     "199/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 77 t "$N_\\mathrm{iter}=199$", \
#     "359/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 97 t "$N_\\mathrm{iter}=359$", \
#     "563/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 107 t "$N_\\mathrm{iter}=563$", \
#     "1000/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 17 t "$N_\\mathrm{iter}=1000$", \
#     "1009/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 17 t "$N_\\mathrm{iter}=1009$", \
#     "1151/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 15 t "$N_\\mathrm{iter}=1151$", \
#     "1249/plot_src/L1_err.dat" u ($1):2:xtic(($1)) w lp dt 2 pt 13 t "$N_\\mathrm{iter}=1249$", \
#     "1399/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 27 t "$N_\\mathrm{iter}=1399$", \
#     "1499/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 37 t "$N_\\mathrm{iter}=1499$", \
#     "1601/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 47 t "$N_\\mathrm{iter}=1601$", \
#     "1709/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 67 t "$N_\\mathrm{iter}=1709$", \
#     "2000/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 67 t "$N_\\mathrm{iter}=2000$", \
#     "4000/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 67 t "$N_\\mathrm{iter}=4000$", \
#     "8000/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 67 t "$N_\\mathrm{iter}=8000$", \

#    "200/plot_src/L1_err.dat"  u ($1 > 0?1:1/0):2 w lp dt 2 pt 67 t "200, linear"
#    "200/plot_src/L1_err.dat"  u ($1):2:xtic(($1)) w lp dt 2 pt 67 t "$N_\\mathrm{iter}=8000$"

#plot \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linear"?$2:1/0)       w lp dt 2 pt 60 t "\\tiny{200, linear}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "cubic"       ?$2:1/0) w lp dt 2 pt 61 t "\\tiny{200, cubic}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "Gamma_0.01"  ?$2:1/0) w lp dt 2 pt 62 t "\\tiny{200, Gamma$=$0.01}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "Gamma_0.1"   ?$2:1/0) w lp dt 2 pt 63 t "\\tiny{200, Gamma$=$0.1}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "Gamma_0.5"   ?$2:1/0) w lp dt 2 pt 64 t "\\tiny{200, Gamma$=$0.5}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "Gamma_0.9"   ?$2:1/0) w lp dt 2 pt 65 t "\\tiny{200, Gamma$=$0.9}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "Gamma01_0.01"?$2:1/0) w lp dt 2 pt 66 t "\\tiny{200, Gamma01, 0.01}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "Gamma01_0.1" ?$2:1/0) w lp dt 2 pt 67 t "\\tiny{200, Gamma01, 0.1}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "Gamma01_0.5" ?$2:1/0) w lp dt 2 pt 68 t "\\tiny{200, Gamma01, 0.5}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "Gamma01_0.9" ?$2:1/0) w lp dt 2 pt 69 t "\\tiny{200, Gamma01, 0.9}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "GammaDC_0.01"?$2:1/0) w lp dt 2 pt 70 t "\\tiny{200,GammaDC, 0.01}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "GammaDC_0.1" ?$2:1/0) w lp dt 2 pt 71 t "\\tiny{200, GammaDC, 0.1}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "GammaDC_0.5" ?$2:1/0) w lp dt 2 pt 72 t "\\tiny{200, GammaDC, 0.5}", \
#     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "GammaDC_0.9" ?$2:1/0) w lp dt 2 pt 73 t "\\tiny{200, GammaDC, 0.9}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "linear"      ?$2:1/0) w lp dt 2 pt 74 t "\\tiny{4000, linear}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "cubic"       ?$2:1/0) w lp dt 2 pt 75 t "\\tiny{4000, cubic}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "Gamma_0.01"?$2:1/0)   w lp dt 2 pt 76 t "\\tiny{4000, Gamma$=$0.01}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "Gamma_0.1"   ?$2:1/0) w lp dt 2 pt 77 t "\\tiny{4000, Gamma$=$0.1}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "Gamma_0.5"   ?$2:1/0) w lp dt 2 pt 78 t "\\tiny{4000, Gamma$=$0.5}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "Gamma_0.9"?$2:1/0)    w lp dt 2 pt 79 t "\\tiny{4000, Gamma$=$0.9}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "Gamma01_0.01"?$2:1/0) w lp dt 2 pt 80 t "\\tiny{4000, Gamma01, 0.01}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "Gamma01_0.1" ?$2:1/0) w lp dt 2 pt 81 t "\\tiny{4000, Gamma01, 0.1}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "Gamma01_0.5" ?$2:1/0) w lp dt 2 pt 82 t "\\tiny{4000, Gamma01, 0.5}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "Gamma01_0.9" ?$2:1/0) w lp dt 2 pt 83 t "\\tiny{4000, Gamma01, 0.9}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "GammaDC_0.01"?$2:1/0) w lp dt 2 pt 84 t "\\tiny{4000,GammaDC, 0.01}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "GammaDC_0.1" ?$2:1/0) w lp dt 2 pt 85 t "\\tiny{4000, GammaDC, 0.1}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "GammaDC_0.5" ?$2:1/0) w lp dt 2 pt 86 t "\\tiny{4000, GammaDC, 0.5}", \
#     "4000/plot_src/L1_err.dat" u ($1):(stringcolumn(3) eq "GammaDC_0.9" ?$2:1/0) w lp dt 2 pt 87 t "\\tiny{4000, GammaDC, 0.9}"

plot \
     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linear"?$2:1/0) w lp dt 2 pt 60 t "\\tiny{200, linear}", \
     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linearUpwind"?$2:1/0) w lp dt 2 pt 60 t "\\tiny{200, linearUpwind}", \
     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linearUpwind_Gauss_linear"?$2:1/0) w lp dt 2 pt 61 t "\\tiny{200, linearUpwind Gauss linear}", \
     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linearUpwind_leastSquares"  ?$2:1/0) w lp dt 2 pt 62 t "\\tiny{200, linearUpwind leastSquares}", \
     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linearUpwindV"   ?$2:1/0) w lp dt 2 pt 63 t "\\tiny{200, linearUpwindV}", \
     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linear_grad(p)"   ?$2:1/0) w lp dt 2 pt 64 t "\\tiny{200, linear grad(p)}", \
     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linearUpwind grad(p)"   ?$2:1/0) w lp dt 2 pt 65 t "\\tiny{200, linearUpwind grad(p)}", \
     "200/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "quadraticLinearUpwindFit_grad(p)"?$2:1/0) w lp dt 2 pt 66 t "\\tiny{200, quadraticLinearUpwindFit grad(p)}", \
     "4000/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linear"?$2:1/0)          w lp dt 2 pt 80 t "\\tiny{200, linear}", \
     "4000/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linearUpwind_Gauss_linear"?$2:1/0) w lp dt 2 pt 81 t "\\tiny{200, linearUpwind Gauss linear}", \
     "4000/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linearUpwind_leastSquares"  ?$2:1/0) w lp dt 2 pt 82 t "\\tiny{200, linearUpwind leastSquares}", \
     "4000/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linearUpwindV"   ?$2:1/0) w lp dt 2 pt 83 t "\\tiny{200, linearUpwindV}", \
     "4000/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linear_grad(p)"   ?$2:1/0) w lp dt 2 pt 84 t "\\tiny{200, linear grad(p)}", \
     "4000/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "linearUpwind grad(p)"   ?$2:1/0) w lp dt 2 pt 85 t "\\tiny{200, linearUpwind grad(p)}", \
     "4000/plot_src/L1_err.dat"  u ($1):(stringcolumn(3) eq "quadraticLinearUpwindFit_grad(p)"?$2:1/0) w lp dt 2 pt 86 t "\\tiny{200, quadraticLinearUpwindFit grad(p)}", \
     "-"                  u ($1):3 w l t "\\tiny{2nd Order}", \
     "-"                  u ($1):3 w l t "\\tiny{1st Order}", \
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

