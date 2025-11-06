#!/bin/bash

## Run multiple impactBar tests
##
## Automatically creates a test-case, runs it until firstEndTime, re-starts from
## there until one timeStep only
##
## Serial Variables:
## -- stopAt: writeNow, endTime
## -- endTime:40e-6   , 40e-6
## Permutational variables: 
## -- OF:     fe41    , fe40,    bekaert
##
## \author: Maalik (ali@tensorfields.com), UCD, Blue Room
## \date:   05-10-2025

## First things
    this=$0

    # N of args
    nargs=5;
    
    # Print help
    if [[ "${@}" =~ "-h" ]]; then
        echo "
Usage:

run.sh <foam> <stopAt> <endTime> <template> <caseName>

Args:

<foam>:     s4f or bekaert
<stopAt>:   endTime or writeNow
<endTime>:  endTime value in seconds
<template>: Template directory which will be copied as a new case.
<caseName>: Name of the newly created case
"
        exit 1
    fi

    # Check nargs
    if [[ $# -ne $nargs ]]; then
        echo Error in $this
        echo $this needs $nargs args.
        exit 1
    fi

## Parse args
    foam=$1
    stopAt=$2
    endTime=$3
    template=$4
    caseName=$5

## Read input file
# ...

## Create case
if ! mkdir $caseName; then 
    echo
    echo Warning in $this
    echo $caseName exists.
    #read -p "Remove $caseName? [y/N] " answer
    #if [[ $answer =~ ^[Yy]$ ]]; then
    #    rm -r $caseName
    #    echo $caseName removed.
    #else
        exit 1
    #fi
else
    Case=$caseName
fi

if ! cp -r "$template"/* $Case/; then
    echo Error
    echo Could not copy $template to $Case
    exit 1
fi

## Set time control
    if [[ $stopAt == "writeNow" ]]; then
        sed -i "/stopAt/,/\;/s/stopAt.*/stopAt    writeNow\;/g" $Case/system/controlDict
    elif [[ $stopAt == "endTime" ]]; then
        if [ -z $endTime ]; then
            echo Error in $this
            echo endTime not found.
        else
            sed -i "/endTime/,/\;/s/endTime.*/endTime    $endTime\;/g" $Case/system/controlDict
        fi

        ## Important: Set stopAt after endTime, or sed messes up
        sed -i "/stopAt/,/\;/s/stopAt.*/stopAt    endTime\;/g" $Case/system/controlDict
    else
        echo ERROR in $this
        echo stopAt cannot be $stopAt.
        exit 1
    fi

## Run
    # Create the mesh
    m4 $Case/system/blockMeshDict.cylinder.m4 > $Case/constant/polyMesh/blockMeshDict
    blockMesh -case $Case > $Case/log.blockMesh
    
    # Set the initial conditions
    if ! cp $Case/0.orig $Case/0 -r; then
        echo
        echo Error in $this
        echo $Case/0.org not found
        exit 1;
    fi

    setFields -case $Case > $Case/log.setFields

    ## Run before re-start
    if [[ $foam == bekaert ]]; then
        plasticNonLinSolidFoam -case $Case &> $Case/log.b4_restart
    elif [[ $foam == s4f ]]; then
        solids4Foam -case $Case &> $Case/log.b4_restart
    else
        echo Error in $this
        echo $foam is not an option; use bekaert or s4f.
        exit 1;
    fi

    ## Run after re-start

    ## start from latestTime
    sed -i "/startFrom/,/\;/s/startFrom.*/startFrom    latestTime\;/g" $Case/system/controlDict

    ## Run for one time-step only to test the re-start
    sed -i "/stopAt/,/\;/s/stopAt.*/stopAt    writeNow\;/g" $Case/system/controlDict

    if [[ $foam == bekaert ]]; then
        plasticNonLinSolidFoam -case $Case &> $Case/log.after_restart
    elif [[ $foam == s4f ]]; then
        solids4Foam -case $Case &> $Case/log.after_restart
    else
        echo Error in $this
        echo $foam is not an option; use bekaert or s4f.
        exit 1;
    fi
