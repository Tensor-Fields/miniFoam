#!/bin/bash

## Compare a <key value> pair in a (number of) file(s).
## 
## Usage
## ./<scriptName> fileName keyword
##
## \note
## In case arguments are not provided, `file` defaults to `controlDict`, and
## `keyword` defaults to `deltaT`.
##
## \author: Maalik (Ali Shayegh, ali@tensorfields.com), Clear, Richmond Hill
## \date: Oct 25 2025

#--------------------------------Code Starts-----------------------------------#

#--------------------------------Definitions-----------------------------------#
help()
{
    cat << 'EOF'
Usage
./<scriptName> fileName keyword

Note
In case arguments are not provided, `file` defaults to `controlDict`, and
`keyword` defaults to `deltaT`.

Note
For now, it works for one file only, not a number of files.
EOF
}

#---------------------------------Execution------------------------------------#
## Firt things

    ## Print help
    if [[ " $* " =~ " -h " ]]; then
        help
        exit 0
    fi

## Parse args
    file=$1
    keyword=$2

    ## Default file to controlDict
        if [ -z "$1" ]; then
            file="controlDict"
        fi
    
    ## Default keyword to deltaT
        if [ -z "$1" ]; then
            keyword="deltaT"
        fi

## Compare
    files=$(find . -name $file)
    occurances=""
    
    if [ -z "$files" ]; then
        echo No file called \"$file\" found.
        exit 0;
    else
        #echo ^^^^ This is just a warning\; ignore it! ^^^^
        echo
        occurances=$(echo "$files" | xargs grep $keyword)
    fi
    
## Print
    if [ -z "$occurances" ]; then
        echo There is no occurance of $keyword in the found file\(s\).
    else
        #echo ^^^^ This is just a warning\; ignore it! ^^^^
        echo
        echo "$occurances"
    fi

echo
echo End.
echo
