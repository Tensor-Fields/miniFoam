#!/bin/bash

#
# Usage
# ./rezoneAndRemap <case_dir>
#

## First things

    ## Print help
    if [[ "${@}" =~ "-h" ]]; then
        echo "
Usage:
./rezone <Case> <maxIter> <writeInterval>"
        exit 1
    fi

    ## Mandatory number of args
    nargs=3

    if [[ "$nargs" -ne $# ]]; then
        echo
        echo Error
        echo Number of args must be 3
        echo Run ./rezone -h for help.
        exit 1
    fi

# Parse args
Case=$1
maxIter=$2
newWriteInterval=$3

## Get keywork values, filtering comment lines, empty lines, and semicolon
    ## stopAt
    stopAt=$(sed -n '/stopAt/,/\;/p' $Case/system/controlDict|grep -v "//\|/\*" | \
        grep stopAt | awk -F' ' '{print$2}' | sed -e s/\;//g | grep -v '^[[:space:]]*$' | tail -n1)

    if [ -z $stopAt ]; then
        echo
        echo Error
        echo Could not get 'stopAt' from controlDict
        exit 1
    fi

    ## endTime
    endTime=$(sed -n '/endTime/,/\;/p' $Case/system/controlDict|grep -v "//\|/\*" | \
        grep endTime | awk -F' ' '{print$2}' | sed -e s/\;//g | grep -v '^[[:space:]]*$' | tail -n1)
    
    ## deltaT
    deltaT=$(sed -n '/deltaT/,/\;/p' $Case/system/controlDict|grep -v "//\|/\*" | \
        grep deltaT | awk -F' ' '{print$2}' | sed -e s/\;//g | grep -v '^[[:space:]]*$' | tail -n1)

    ## writeInterval
    writeInterval=$(sed -n '/writeInterval/,/\;/p' $Case/system/controlDict|grep -v "//\|/\*" | \
        grep writeInterval | awk -F' ' '{print$2}' | sed -e s/\;//g | grep -v '^[[:space:]]*$' | tail -n1)
    
## Replace keyword values
    ## Replace stopAt value by 'endTime'
    sed -i "/stopAt/,/\;/s/stopAt.*/stopAt    endTime\;/g" $Case/system/controlDict

    ## Replace endTime value by (maxIter+latestTime)

        ## Get the latestTime
        times=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9].*+)?$')
        
        latestTime=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9].*+)?$' | tail -n1)

    newEndTime=$(($latesTime + $maxIter))
    sed -i \
        "/endTime/,/;/s/endTime\(\ *[1-9\.].*\)\;/endTime    $newEndTime\;/gp" \
        $Case/system/controlDict

    ## Replace deltaT
    sed -i \
        "/deltaT/,/;/s/deltaT\(\ *[1-9\.].*\)\;/deltaT    1\;/gp" \
        $Case/system/controlDict

    ## Replace writeInterval
    sed -i \
        "/writeInterval/,/;/s/writeInterval\(\ *[1-9\.].*\)\;/writeInterval    $newWriteInterval\;/gp" \
        $Case/system/controlDict

## Actually Rezone!
LaplacianSmoothBilletMesh -freePatches \( "front" "back" \) \
                          -slideCrossingPoints \
                          -latestTime \
                          -maxIter $maxIter \
                          -crossingPatches \( "front" "back" \) \
                          -wedgeDir 2 \
                          -radialDir 1 \
                          -wedgeAperture 1 \
                          -case $Case \
                          -patchWithSlidingPoints "abd" \
                          -noFunctionObjects \
                          > log.LaplacianSmoothBilletMesh

## Recover original values to their respective keys
    sed -i "/endTime/,/;/s/endTime\(\ *[1-9\.].*\)\;/endTime    $endTime\;/gp" \
        $Case/system/controlDict
    sed -i "/deltaT/,/;/s/deltaT\(\ *[1-9\.].*\)\;/deltaT    $deltaT\;/gp" \
        $Case/system/controlDict
    sed -i \
        "/writeInterval/,/;/s/writeInterval\(\ *[1-9\.].*\)\;/writeInterval    $writeInterval\;/gp" \
        $Case/system/controlDict

## Manual for now: Clean timeName dirs
    ## mkdir rezone_steps
    ## mv <rezone steps> rezone_steps, except latestTime
    ## mv oldEndTime oldEndTime_b4Rezone
    ## mv latestTime oldEndTime
