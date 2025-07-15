#!/bin/bash

# When L1_err.dat is ready, and you only need to plot it.
# The comprehensive plot script, which scans the logs to find L1 errors
# is plot.sh which includes this script as well.

# Create dir to collect side files
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

plot "L1_err.dat" u 1:2:xtic(1) w p pt 'o' t "Cyclic, random", \
     "-" u ($1):3 w l t "2nd Order", \
     "-" u ($1):3 w l t "1st Order", \
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

#gnuplot << 'EOF'
#set term epslatex 
#set output "plot_src/L1_err.tex"
#
#set datafile commentschars '#'
#
#set logscale xy 10
#
##plot "-" u ($1):3 w l t "2nd Order", \
##     "-" u ($1):3 w l t "1st Order", \
#plot "L1_err.dat" u 1:2:xtic(1) w p pt 'o' t "Cyclic, random"
#    ## 2nd order
#    #25            0.04    0.000229544
#    #50            0.02    0.000057386
#    #100           0.01    0.0000143465
#    #200           0.005   0.000003586625
#    #e
#    ## 1st order
#    #25            0.04    0.000229544 
#    #50            0.02    0.000114772 
#    #100           0.01    0.000057386
#    #200           0.005    0.000028693
#    #e
#EOF

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
