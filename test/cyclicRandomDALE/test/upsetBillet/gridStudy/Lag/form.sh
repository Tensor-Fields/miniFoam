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

#-----------------------------------------------------------------------------#
## Definitions
#-----------------------------------------------------------------------------#
this=$0

## Assign values
is()
{
    if [[ $# -ne 1 ]]; then
        echo Error
        echo At least one argument is missing.
        echo See $this -h
        exit 1
    else
        echo $1
    fi
}

#-----------------------------------------------------------------------------#
## Driver
#-----------------------------------------------------------------------------#
## First things

    # N of args
    nargs=5;
    
    # Print help
    if [[ "${@}" =~ "-h" ]]; then
        echo "
Usage:

run.sh <foam> <stopAt> <endTime> <template> <caseName> <restart> <meshGeneratorScript> <options>

Important:
Order matters; args first, options next.

Args:

<foam>:     s4f, bekaert, or form
<stopAt>:   endTime or writeNow
<endTime>:  endTime value in seconds
<template>: Template directory which will be copied as a new case.
<caseName>: Name of the newly created case
<restart>: "true" for restart, "false" for no restart
<meshGeneratorScript>: Self-explanatory!

Options:
-h: Print help
-np <nCores>, works with bekaert solver only
-f: Force case re-creation if it exists. 
"
        exit 1
    fi

    ### # Check nargs
    ### if [[ $# -ne $nargs ]]; then
    ###     echo Error in $this
    ###     echo $this needs $nargs args.
    ###     exit 1
    ### fi

## Parse args
    foam="$(is $1)"
    stopAt="$(is $2)"
    endTime="$(is $3)"
    template="$(is $4)"
    caseName="$(is $5)"
    Restart="$(is $6)"
    meshGeneratorScript="$(is $7)"
    nCores="$(is $9)"

## Read input file
# ...

## Create case
#if ! mkdir $caseName; then 
#    if [[ "${@}" =~ "-f" ]]; then
#	    rm -r $caseName
#		echo $caseName removed.
#		if ! mkdir $caseName; then
#		    echo Error
#			echo Could not create $caseName after removal
#			exit 1
#		fi
#	else
#        echo
#        echo Error in $this
#        echo $caseName exists.
#	    echo Pass -f to force re-creation.
#        #read -p "Remove $caseName? [y/N] " answer
#        #if [[ $answer =~ ^[Yy]$ ]]; then
#        #    rm -r $caseName
#        #    echo $caseName removed.
#        #else
#            exit 1
#        #fi
#	fi
#fi

Case=$caseName

#if ! cp -r "$template"/* $Case/; then
#    echo Error
#    echo Could not copy $template to $Case
#    exit 1
#fi

## Set time control
#    if [[ $stopAt == "writeNow" ]]; then
#        sed -i "/stopAt/,/\;/s/stopAt.*/stopAt    writeNow\;/g" $Case/system/controlDict
#    elif [[ $stopAt == "endTime" ]]; then
#        if [ -z $endTime ]; then
#            echo Error in $this
#            echo endTime not found.
#        else
#            sed -i "/endTime/,/\;/s/endTime.*/endTime    $endTime\;/g" $Case/system/controlDict
#        fi
#
#        ## Important: Set stopAt after endTime, or sed messes up
#        sed -i "/stopAt/,/\;/s/stopAt.*/stopAt    endTime\;/g" $Case/system/controlDict
#    else
#        echo ERROR in $this
#        echo stopAt cannot be $stopAt.
#        exit 1
#    fi

## Run
    ### # Clean the previous mesh
    ### if ! rm -f $Case/constant/polyMesh/*; then
    ###     echo Error
    ###     echo Could not remove previous mesh files
    ### fi

    ### if ! rm -rf $Case/constant/polyMesh/sets; then
    ###     echo Error
    ###     echo Could not remove previous mesh sets
    ### fi

    # Create the mesh

    ## Old mesh generator

    #m4 $Case/system/blockMeshDict.cylinder.m4 > $Case/constant/polyMesh/blockMeshDict
    #blockMesh -case $Case > $Case/log.blockMesh

    ## New mesh generator
    #. $meshGeneratorScript $Case
    
    # Set the initial conditions
    #if ! cp $Case/0.orig $Case/0 -r; then
    #    echo
    #    echo Error in $this
    #    echo $Case/0.orig not found
    #    exit 1;
    #fi

    #setFields -case $Case > $Case/log.setFields

    ## Run before re-start
    if [[ $foam == form ]]; then

        ## Define cell zones
        if ! setSet -batch $Case/setSetDict -case $Case > $Case/log.setSet; then
            echo
            echo Error
            echo setSet failed
            exit 1
        fi

        if ! setsToZones -case $Case  > $Case/log.setsToZones; then
            echo
            echo Error
            echo setsToZones failed
            exit 1
        fi

        ## Define materials
        if ! setMatFromCellZones -case $Case > $Case/log.setMatFromCellZones;
		then
            echo
            echo Error
            echo setMatFromCellZones failed
            exit 1
        fi

        if ! form -case $Case > $Case/log.solver; then
            echo Error
            echo form failed.
            exit 1
        fi

    elif [[ $foam == bekaert ]]; then

        ## Define cell zones ---- blockMesh already writes blocks to sets
        ### if ! setSet -batch $Case/setSetDict -case $Case > $Case/log.setSet; then
        ###     echo
        ###     echo Error
        ###     echo setSet failed
        ###     exit 1
        ### fi

        ### if ! setsToZones -case $Case  > $Case/log.setsToZones; then
        ###     echo
        ###     echo Error
        ###     echo setsToZones failed
        ###     exit 1
        ### fi

        ## Define materials
        setMatFromCellZones -case $Case > $Case/log.setMatFromCellZones

        if ! plasticNonLinSolidFoam -case $Case > $Case/log.solver.b4_restart; then
            echo Error
            echo plasticNonLinSolidFoam failed.
            exit 1
        fi
    elif [[ $foam == s4f ]]; then
        if ! solids4Foam -case $Case > $Case/log.b4_restart; then
            echo Error
            echo solids4Foam failed.
            exit 1
        fi
    else
        echo Error in $this
        echo $foam is not an option; use bekaert or s4f.
        exit 1;
    fi

    ## Run after re-start

    if [[ "$Restart" == "false" ]]; then
        echo Run finished. It will not restart.
        echo End.
        exit 0
    else
        ## start from latestTime
        sed -i "/startFrom/,/\;/s/startFrom.*/startFrom    latestTime\;/g" $Case/system/controlDict

        ## Run for one time-step only to test the re-start
        sed -i "/stopAt/,/\;/s/stopAt.*/stopAt    writeNow\;/g" $Case/system/controlDict

        if [[ $foam == form ]]; then

            if [[ "${@}" =~ "-np" ]]; then
                if ! decomposePar -force -case $Case > $Case/log.decomposePar;
                then
                    echo Error
                    echo decomposePar failed.
                    exit 1
                fi

                if ! mpirun -np $nCores form \
                    -case $Case -parallel \
                    > $Case/log.after_restart; then
                    echo Error
                    echo mpirun failed.
                    exit 1
                fi
            else
                if ! form -case $Case > $Case/log.after_restart; then
                    echo Error
                    echo plasticNonLinSolidFoam failed to restart.
                    exit 1
                fi
            fi
        elif [[ $foam == bekaert ]]; then

            if [[ "${@}" =~ "-np" ]]; then
                if ! decomposePar -force -case $Case > $Case/log.decomposePar;
                then
                    echo Error
                    echo decomposePar failed.
                    exit 1
                fi

                if ! mpirun -np $nCores plasticNonLinSolidFoam \
                    -case $Case -parallel \
                    > $Case/log.after_restart; then
                    echo Error
                    echo mpirun failed.
                    exit 1
                fi
            else
                if ! plasticNonLinSolidFoam -case $Case > $Case/log.after_restart; then
                    echo Error
                    echo plasticNonLinSolidFoam failed to restart.
                    exit 1
                fi
            fi
        elif [[ $foam == s4f ]]; then
            if ! solids4Foam -case $Case > $Case/log.after_restart; then
                echo Error
                echo solids4Foam failed to restart.
                exit 1
            fi
        else
            echo Error in $this
            echo $foam is not an option; use bekaert or s4f.
            exit 1;
        fi
    fi
