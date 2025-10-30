#!/bin/bash
wireMeshSize=$1
rollMeshSize=$2
contactCellSize=$3

find . -name fileModifiers | while read -r file; do \
    sed -i "s/\(wireMeshDict\/maxCellSize\).*$/\1 $wireMeshSize\;/g" $file
    sed -i "s/\(twoHighRollsDict\/topRoll\/maxCellSize\).*/\1 $rollMeshSize\;/g" $file
    sed -i \
	"s/\(twoHighRollsDict\/topRoll\/contactCellSize\).*/\1 $contactCellSize\;/g" \
	$file
done
