#!/bin/bash

if [[ $# -ne 2 ]]; then
    echo Error: No. of args must be 2.
    echo Usage:
    echo "    ./cleanPLY.sh    <file>    <col>"
    exit 1
fi

file=$1

## The column based on which you sort results
N=$2

## Clean header
sed '/ply/,/end_header/d' $file > "tmp_$file"

## Sort points in the walk order
if ! read x y z < <(sort -k"$N","$N"n "tmp_$file" | tail -n 1)
then
    echo Error in sort.
    exit 1; fi

if ! python3 findNearest.py "tmp_$file" $x $y $z > sorted_$file
then
    echo Error in findNearest
    exit 1; fi

## First add parentheses, then zero x's.
sed -i 's/^/(/g' sorted_$file
sed -i 's/$/)/g' sorted_$file
sed -i 's/(-\?[0-9]*\(\.\)\?[0-9]*e\?\-\?[0-9]*/(0 /g' sorted_$file

## Clean tmp files
rm "tmp_$file"

## Plot profile
. plotThis.sh sorted_$file
