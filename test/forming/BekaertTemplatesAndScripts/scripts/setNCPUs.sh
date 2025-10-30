#!/bin/bash
nCPUs=$1

find . -name fileModifiers | while read -r file; do \
    sed -i "s/\(numberOfSubdomains\).*$/\1 $nCPUs\;/g" $file
done
