#!/bin/bash

##
## Description:
##    Plots a file with this format, column 2 vs 3:
##
##    (0.0 -29.999845 0.0002644956)
##    (0.0 -28.44431 0.0002644956)
##    (0.0 -26.888775 0.0002644956)
##    (... ...        ...         )
##
## Usage:
##    ./plotThis.sh <file>
##
## Author:
##    Maalik (Ali Shayegh, alishayegh@proton.me)
##
## Date:
##    Jun 06 2026
##

file=$1

cat $file | gnuplot -p -e "plot '$file' u 2:3 w l"
