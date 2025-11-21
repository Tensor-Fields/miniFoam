#!/bin/bash

## Roadmap: Compare all modifier files, not input.setKey.

## Global variables
this="createStudyReadMe.sh"
ReadMe="ReadMe.md"
input="input.setKey"
compareMe=$1

## First things
    #if [ -z $compareMe ]; then
	#    echo Error
	#	echo Usage: ./$this "file(s)/to/be/compared"
	#	exit 1
    #fi

## Parse
    ## Force overwritting existing ReadMe
    force=false

    if [[ " $@ " =~ " -f " ]]; then
        force=true
    fi

## Decide to overwrite or not
    if [ -f $ReadMe ]; then
        if $force; then
            echo rm $ReadMe
            if ! rm $ReadMe; then
                echo ERROR
                echo In $this
                echo rm $ReadMe
            fi
        else
            echo ERROR
            echo In $this
            echo Appending to the existing $ReadMe is not defined. Pass \"-f\".
            exit 1
        fi
    fi

## Print to file

    ## Create ReadMe
    echo Base settings: > $ReadMe

    ## Append Base Settings
    if ! [ -f $input ]; then
        echo ERROR
        echo In $this
	echo ./$input Does not exist to compare with
	exit 1
    fi

    cat $input >> $ReadMe

    ## Append differences
    echo >> $ReadMe
    echo Differences: >> $ReadMe
    echo Column 1: Difference"    "Column 2: Base >> $ReadMe

    #find . -mindepth 2 -maxdepth 2 -name $compareMe | \
    find . -mindepth 2 -maxdepth 2 -name $input | \
        while read -r subStudy; do
	    echo >> $ReadMe
	    echo $subStudy | xargs dirname | xargs basename >> $ReadMe
	    diff $subStudy $input \
	        -wybBEZ --suppress-common-lines \
	        >> $ReadMe
	done
