#!/bin/bash

## Processline path is input

process=$1
profiles="$process/profiles"

if [ -d $profiles ]; then
    echo $profiles/ exists. Remove or rename.
	exit 1
else
    mkdir -p $profiles
fi

find $process -name \*dxf | \
while read -r file; do 
    echo $file found\; copy to $profiles
    cp $file $profiles;  

    parentName=$(echo $file|awk -F"/" '{print$(NF-1)}'); 
    baseName=$(echo $file|awk -F"/" '{print$(NF)}'); 

	echo Remane $baseName to $parentName.dxf
    mv $profiles/$baseName "$profiles/$parentName.dxf"; 
	echo
done
