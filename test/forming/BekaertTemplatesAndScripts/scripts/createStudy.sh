#!/bin/bash

## Create study
##
## For example if I want to study ten same processline simulations, which only
## differ by the timestep, I run
##
## Usage
## See Usage below in the code, or run
##
## ./createStudy.sh -h
##
## \author: Maalik (Ali Shayegh, ali@tensorfields.com), Rainy, BlueRoom, UCD
## \date: 23 Oct 2025

this="createStudy.sh"
studyName=$1
nOfSubStudies=$2
processName=$3

##
## Check/parse args
##

    ##
    ## Very First Checks
    ##
    
    ## Print usage
    if [[ " $@ " =~ " -h " ]]; then
        echo Usage:
	echo "./createStudy <args> <options> "
	echo
	echo Important:
	echo Order matters, i.e., args first, options afterwards.
        echo
	echo args:
        echo "    "studyName
        echo "    "nOfSubStudies
        echo "    "templateProcesslineDirectory
	echo
	echo options:
        echo "    "-f: Force remove and/or link
        echo "    "-i: Interactive link
	echo
	exit 1
    fi

    ##
    ## ERRORS
    ##
    
    ## New study must have a name.
    if [ -z $studyName ]; then
        echo ERROR
        echo In $this
        echo Provide a name for the study.
        echo
        exit 1
    fi
    
    ## New study must study a particular processLine.
    if [ -z $processName ]; then
        echo ERROR
        echo In $this
        echo Provide processLine dir for the study.
        echo
        exit 1
    fi

    ##
    ## Parse
    ##

    ## Each study at least contains one processLine dir
    N=$nOfSubStudies
    if [ -z $N ]; then
        echo Warning
        echo No. of sub-studies defaults to 1.
        N=1
        echo
    fi
    
    ## Optionally, force removing/linking files when needed.
    force=false
    
    if [[ " $@ " =~ " -f " ]]; then
        force=true
    fi

    ## Optionally, interatively link files when needed.
    interactive=false
    
    if [[ " $@ " =~ " -f " ]]; then
        interactive=true
    fi

## Create the study dir
    if [ -d $studyName ]
    then
        if $force; then
            rm -r $studyName
        else
    	echo
            echo Hint:
    	echo From $this
            echo Pass -f to force remove.
            rm -ri $studyName
        fi
    fi
    
    if ! mkdir $studyName; then
        echo Error
        echo In $this
        echo Cannot create $studyName
        echo
        exit 1
    fi

## Create the required links in studyName
    if $force; then
        if ! ./linkFiles.sh $studyName $processName -f; then
            echo ERROR
            echo In $this
            echo Incomplete linking.
            echo
            exit 1
        fi
    elif $interactive; then 
        if ! ./linkFiles.sh $studyName $processName -i; then
            echo ERROR
            echo In $this
            echo Incomplete linking.
            echo
            exit 1
        fi
    else
        if ! ./linkFiles.sh $studyName $processName; then
            echo ERROR
            echo In $this
            echo Incomplete linking.
            echo
            exit 1
        fi
    fi

## Files to be copied, not linked

    ## input.setKey
    if ! cp scripts/run.slurm $studyName; then
        echo ERROR
        echo In $this
        echo Could not copy run.slurm to $studyName.
        echo
        exit 1
    fi    

    ## input.setKey
    if ! cp scripts/input.setKey $studyName; then
        echo ERROR
        echo In $this
        echo Could not copy input file to $studyName.
        echo
        exit 1
    fi    

## Create sub-studies
    N=$nOfSubStudies
    
    for((i=0; i<N; i++)); do
        subStudyName=$studyName/processLine_$i
    
        if ! \
            ./createProcessLine.sh \
    	    ./templateProcessLine \
    	        $subStudyName; then
            echo ERROR
            echo In $this
    	echo Creating all process lines failed.
    	echo
    	exit 1
        else
    	echo $subStudyName created without an input file.
        fi
    done
    
echo
echo Next steps:
echo
echo Edit $studyName/input.setKeys.Then, inside each sub-study,
echo - Copy from $studyName/input.setKeys.
echo - ./setKeys input.setKeys.
echo - Run the processLine
echo

echo
echo End.
echo
