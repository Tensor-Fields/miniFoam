#!/bin/bash

## Clean target directory after error calculation

## Usage
##    ./<script>  <targetCase>  <path/to/mappedDir>

if [[ $# -ne 2 ]]
then
    echo Error
    echo No. of args must be 2.
    echo
    echo Usage:
    echo "    ./<script>  <targetCase>  <path/to/mappedDir>"
    exit 1
fi

Case=$1
mappedDir=$2

if ! find $Case -name \*err -o -name \*Ref | xargs rm
then
    echo Error
    echo "Removing *err *Ref files failed."
    exit 1
fi
    
#rm -rI $mappedDir
