#!/bin/bash

# Find maxCo in all logs, list it sorted in maxCo.txt.

find . -name \*out -print0 | \
    xargs -0 -I_ grep "Mesh Courant" _ | \
    awk -F":" '{print $NF}' | \
    sort -g > \
    maxCo.txt
