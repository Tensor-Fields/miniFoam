#!/bin/bash

## Copy gif's from supplementary/symlinks
ls -1 ../supplementary/ | \
    while read -r file; do echo $file; cp $(readlink -f ../supplementary/$file) $file ; done
