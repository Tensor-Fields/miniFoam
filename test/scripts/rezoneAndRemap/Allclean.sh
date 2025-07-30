#!/bin/bash

# Remove logs
rm log*
rm maxCo.txt
rm plot.pdf
rm -r plot_src
rm -r [1-9]*
rm slurm*out

if [ -L ReadMe.md ]; then
    rm ReadMe.md
fi

