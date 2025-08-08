#!/bin/bash

# Remove logs
rm log*
rm maxCo.txt
rm plot.pdf
rm -r plot_src
rm -ir [1-9]*
rm slurm*out

if [ -L ReadMe.md ]; then
    echo ReadMe.md is a link,
    rm -i ReadMe.md
fi

