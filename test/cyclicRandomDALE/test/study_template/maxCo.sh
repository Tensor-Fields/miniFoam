#!/bin/bash

# Find maxCo in all logs, list it sorted in maxCo.txt.

#
# Only prints maxCo sorted
#

#find . -name \*out -print0 | \
#    xargs -0 -I_ grep "Mesh Courant" _ | \
#    awk -F":" '{print $NF}' | \
#    sort -g > \
#    maxCo.txt

#
# Prints two columns; study and maxCo
#

if [ -f "maxCo.txt" ]; then
    rm "maxCo.txt"
fi

if [ -f "tmp.txt" ]; then
    rm "tmp.txt"
fi

find . -mindepth 2 -name \*out | \
    while read -r file; do
        dir=$(echo $file); 
	grep "Mesh Courant" $file | \
        awk -F":" -v d=$dir '{print d" "$NF}' | \
	sort -t" " -k2 -t" " -g >> \
	tmp.txt;
    done;

sort -t" " -k2 -g tmp.txt > \
    maxCo.txt

rm tmp.txt
