#!/bin/bash

# When L1_err.dat is ready, and you only need to plot it.
# The comprehensive plot script, which scans the logs to find L1 errors
# is plot.sh which includes this script as well.

#if [ -d plot_src ]; then
#    rm -r plot_src
#fi
#
#mkdir plot_src

#
# Gnuplot
#
echo Gnuplot ...

gnuplot << 'EOF'
#set term epslatex
#set term epslatex size 16cm,11cm
#set term epslatex standalone size 16cm,11cm
set term epslatex standalone
#set term epslatex monochrome size 16cm,11cm

set output "plot_src/L1_err.tex"

#set term qt

set palette gray

set datafile commentschars '#'

set logscale xy 10

unset key

#set key font ",20"
#set key maxrows 9
#set key left box opaque outside width -15
set key left box opaque outside
#set key right Right box opaque outside width -8

set format y "10^{%L}"

unset yrange
#set yrange [*:0.001]
#set yrange [3e-6:0.001]

unset grid

set xlabel "$N_{\\mathrm{cells}}$"
set ylabel "$E_{\\mathrm{L}_1}$"

system("./genPlotOptions.sh plot.list")

load 'plot_src/plot.options'

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
#echo Write latex file

#cat > plot_src/plot.tex << EOF
#\documentclass{standalone}
#
#\usepackage{xcolor, graphicx}
#\usepackage{amsmath}
#
#\begin{document}
#    \input{plot_src/L1_err}
#\end{document}
#EOF

# Compile latex
echo Compile latex
if ! latex -interaction nonstopmode --output-dir plot_src plot_src/L1_err.tex \
    > plot_src/log.latex
#if ! pdflatex -interaction nonstopmode --output-dir plot_src plot_src/L1_err.tex \
#    > plot_src/log.latex
    #then echo Error: latex Unsuccessful; exit 1; fi
    then echo Warning: latex Unsuccessful; fi

#dvips plot_src/plot.dvi > plot_src/log.dvips
#
#ps2pdf plot.ps > plot_src/log.ps2pdf

dvips plot_src/L1_err.dvi > plot_src/log.dvips

ps2pdf L1_err.ps > plot_src/log.ps2pdf

echo L1_err.pdf written.
#echo plot.pdf written.

mv L1_err.ps plot_src
#mv plot.ps plot_src
#mv plot_src/plot.pdf .
