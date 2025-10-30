#!/bin/bash

## Compare two directories and show only the differences

dir2=$(awk -F'/' '{print$1}')

find \
    $(readlink -f $1)
	-type d -name modifier\* | \
	while read -r dir; do

	    dir1=$(dirname $dir)
	    midDir=$(awk '/' '{for (i=2; i<=NF; i++) print$i}')
	    file1=$(basename $dir)
        diff -rwybBZE -W170 --suppress-common-lines \
        $(readlink -f $1) $(readlink -f $2) \
	    | grep -v "Only in"

#    diff -rwybBZE -W170 --suppress-common-lines \
#    $(readlink -f $1) $(readlink -f $2) \
