#!/bin/bash

#
# Usage
# See ./remap.sh -h
#

## First things

    this=$0

    ## Print help
    if [[ "${@}" =~ "-h" ]]; then
        echo "
Usage:
./remap <Case> <maxIter> <writeInterval> <mode>

<mode>: rezone or remap"
        exit 1
    fi

    ## Mandatory number of args
    nargs=4

    if [[ "$nargs" -ne $# ]]; then
        echo
        echo Error
        echo Number of args must be 4
        echo Run ./remap.sh -h for help.
        exit 1
    fi

## Parse args
    Case=$1
    maxIter=$2
    newWriteInterval=$3
    mode=$4

## Definitions

    ## latestTime
    latestTime()
    {
        ls -1 $1 | \
            grep -E \
            '^[-]?[0-9]+[.]?[0-9]*$|[-]?[0-9]+[.]?[0-9]*[eE]+[+-]?[0-9]+$' | \
            sort -g | tail -1
    }

    ## Get keyword values, filtering comment lines, empty lines, and semicolon
    get_keywords()
    {
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
    }
    
    ## Replace keyword values
    replace_key_values()
    {
        echo >> $Case/system/controlDict
        echo "// Replaced key-value pairs" >> $Case/system/controlDict

        ## Replace stopAt value by 'endTime'
        #sed -i "/stopAt/,/\;/s/stopAt.*/stopAt    endTime\;/g" $Case/system/controlDict
        echo "stopAt    endTime;" >> $Case/system/controlDict

        ## Replace endTime value by (maxIter+latestTime)

            ## Get the latestTime
            #times=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9].*+)?$')
            
            #latestTime=$(ls -1v $Case | grep -E \
            #'^[0-9]+([eE][-+]?[0-9]+)?(\.[0-9]+)?$' | tail -n1)

            latestTime=$(latestTime "$Case")

        newEndTime=$(($latesTime + $maxIter))
        #sed -i \
        #    "/endTime/,/;/s/endTime\(\ *[1-9\.].*\)\;/endTime    $newEndTime\;/gp" \
        echo "endTime    $newEndTime;" >> \
            $Case/system/controlDict

        ## Replace deltaT
        #sed -i \
        #    "/deltaT/,/;/s/deltaT\(\ *[1-9\.].*\)\;/deltaT    1\;/gp" \
        echo "deltaT    1;" >> \
            $Case/system/controlDict

        ## Replace writeInterval
        #sed -i \
        #    "/writeInterval/,/;/s/writeInterval\(\ *[1-9\.].*\)\;/writeInterval    $newWriteInterval\;/gp" \
        echo "writeInterval    $newWriteInterval;" >> \
            $Case/system/controlDict
    }

    ## Recover original values to their respective keys
    recover_key_values()
    {
        echo >> $Case/system/controlDict
        echo "// Recovered key-value pairs" >> $Case/system/controlDict

        #sed -i "/endTime/,/;/s/endTime\(\ *[1-9\.].*\)\;/endTime    $endTime\;/gp" \
        echo "endTime    $endTime;" >> \
            $Case/system/controlDict

        #sed -i "/deltaT/,/;/s/deltaT\(\ *[1-9\.].*\)\;/deltaT    $deltaT\;/gp" \
        echo "deltaT    $deltaT;" >> \
            $Case/system/controlDict

        #sed -i \
        #    "/writeInterval/,/;/s/writeInterval\(\ *[1-9\.].*\)\;/writeInterval    $writeInterval\;/gp" \
        echo "writeInterval $writeInterval;" >> \
            $Case/system/controlDict

        echo "stopAt    $stopAt;" >> \
            $Case/system/controlDict
    }

    ## Move remap-created times to remapSteps dir
    mv_remap_steps()
    {
        newTimes=$(ls -1 | grep -E '^[0-9.]+$')

        diff=$(comm -13 <(echo "$1" | sort) <(echo "$newTimes" | sort))

        if [ -d "remapSteps" ]; then
            rm -r "remapSteps"
        fi

        if ! mkdir "remapSteps"; then
            echo
            echo Error
            echo Could not create remapSteps
        fi

        echo Move remap-created times to remapSteps

        for dir in "$diff"; do
            mv "$dir" "remapSteps"
        done
    }

    ## Rename latest remapped time to latestTime before remap
    update_time_names()
    {
        echo "Update timeNames..."
        latestNewTime=$(ls -1v $Case/remapSteps | grep -E \
        '^[0-9]+([eE][-+]?[0-9]+)?(\.[0-9]+)?$' | tail -n1)

        echo Latest remap-generated time is $latestNewTime
        echo Move $latestNewTime to "$1"

        mv $Case/"$1" $Case/"$latestTime"_before_remap
        mv $Case/"remapSteps"/"$latesTimeNewTime" $Case/"$1"
    }

## Driver
if [[ $mode == "rezone" ]]; then
    get_keywords;
    replace_key_values;

    ## Actually Rezone!
    if ! LaplacianSmoothBilletMesh \
        -freePatches \( "front" "back" \) \
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
        > log.LaplacianSmoothBilletMesh; then
        ## Following go together
        #-noWedge
        #-emptyDir <dir>
        echo
        echo Error
        echo LaplacianSmoothBilletMesh failed.
        echo
        exit 1;
    fi

    #
    # Create source dir, necessary for mapping
    #
    
    # Copy source mesh and field to a new case
    
    echo Copy source mesh and field to source/
    if [ -d $Case/source ]; then
        rm -r $Case/source
    fi
    
    mkdir $Case/source

    touch $Case/source/case.foam

    ## reconstructParMesh creates 0/polyMesh. We must replace constant/polyMesh
    ## with this for the workflow to work.
    if [ -d $Case/0/polyMesh ]; then

        cp -r "$Case/0/polyMesh/"* $Case/constant/polyMesh/

        rm -r $Case/0/polyMesh
    fi

    cp -r $Case/0 $Case/constant $Case/system $Case/source
    
    ## Get the latest time
    #latestTime=$(ls -1v $Case | grep -E \
    #'^[0-9]+([eE][-+]?[0-9]+)?(\.[0-9]+)?$' | tail -n1)

    latestTime=$(latestTime "$Case")
    
    ## Overwrite source mesh with smooth mesh

        ## Now overwrite
        cp "$Case/$latestTime/polyMesh"/points $Case/source/constant/polyMesh/
    
    ## The last time is a smooth mesh without fields, remove it; source is aware of
    ## that
    echo Rename $latestTime to smoothed

    if [ -d "smoothed" ]; then
        rm -r smoothed;
    fi

    mv $Case/$latestTime $Case/smoothed
    
    # Required by cyclicRandomDALE
    ln -rsf $Case/source $Case/constant/

    recover_key_values;
    echo
    echo Rezone Done.
    echo
    echo Next steps:
    echo "Clean timeName dirs (manually for now):
mkdir rezone_steps
mv <rezone steps> rezone_steps, except latestTime"
    echo
    echo End.
    echo
    exit 1

elif [[ $mode == "remap" ]]; then
 
    get_keywords;
    replace_key_values;

    oldTimes=$(ls -1 | grep -E '^[0-9.]+$')

    # The current design of meshDance requires that only 'front' is input to create
    # meshWire objects, and 'back' automatically is included; obviously back does
    # not appear as a fixedPatchName. Therefore, all patches except 'back' are
    # named as options.
    if ! cyclicRandomDALE \
        -nY 1 \
        -M 1 \
        -alpha .125 \
        -randomDirs \( "1" "1" "0" \) \
        -fixedPatchNames \
        \( \
            "billetContact" \
            "dieContact" \
            "loading" \
            "symmPlane"  \
        \) \
        -notFixedPatchNames \
        \( \
            "front" \
        \) \
        -writeIntermediateMeshes \
        -case $Case \
        -seed $(echo $RANDOM) \
        -targetMesh "source" \
        -maxIter 1 \
        -flipFluxSign \
        > $Case/log.cyclicRandomDALE #\ 
        #-fixedTimeStep # Unnecessary, time will not be adjusted
        # when controlDict.adjustTimestep is commented.

        ## Impact bar
        #"billetTop" \
        #"billetContact" \
        #"dieContact" \
        #"billetSide" \
        #"dieBottom" \

        #-illegalToAdvect \
        #\( \
        #     "grad(D)"                      \
        #     "grad(DD)"                     \
        #     "pointVelocity"                \
        #     "grad(DU)"                     \
        #     "U"                            \
        #     "Velocity"                     \
        #     "F"                            \
        #     "sigmaCaucy"                   \
        #     "sigmaCauchy"                  \
        #     "rho"                          \
        #     "globalToLocalFaceZonePointMap" \
        #\) \
    then
        echo
        echo Error in $this
        echo cyclicRandomDALE failed.
        echo
        #latestTime=$(ls -1v $Case | grep -E \
        #'^[0-9]+([eE][-+]?[0-9]+)?(\.[0-9]+)?$' | tail -n1)
        latestTime=$(latestTime "$Case")

        echo latestTime is $latestTime
        echo Follow instructions below:
        echo - Remove timesteps created,
        echo - Rename smoothed to a number '>' latestTime,
        echo - Solve other issues,
        echo - Re-run.
        echo
        echo Exit with Error
        exit 1
    fi

    ## Manually, until debug
    #### mv_remap_steps "$oldTimes";
    #### update_time_names "$latestTime";
    recover_key_values;

    echo
    echo Remap done.
    echo
else
    echo
    echo Error
    echo "<mode> must be either rezone or remap"
    echo
    exit 1
fi

# Commented error calculation

: << 'END'
#
# Reset source to be the the same final mesh with exact values
#

# Find latest time
times=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$')

latestTime=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$' | tail -n1)

# Overwrite source mesh with the final mesh
cp -r "$Case/$latestTime"/polyMesh/* "$Case/source/"constant/polyMesh

# Evaluate source field at source cells
setAlgebraicFields -case $Case/source -a 0 -b 1 -c 1 -d 1 -sinsin \
    > $Case/source/log.setAlgebraicFields

#
# Arrange files for map error application
#
times=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$')

latestTime=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$' | tail -n1)

# Create times/
if [ -d "$Case/times" ]; then
    rm -r "$Case/times"
fi

mkdir "$Case/times"

# Only keep the latestTime in Case
echo $times | sed 's/\ /\n/g' | while read -r time; \
    do mv "$Case/$time" "$Case/times"; done

mv "$Case/times/0" "$Case/times/$latestTime" $Case

#
# Calculate remap error
#

# Remove unnecessary files from the latestTime
rm "$Case/$latestTime/V0" \
   "$Case/$latestTime/meshPhi"

echo Calculate the remap error ...
meshToMeshError \
    -case $Case \
    -sourceCase source > "$Case/log.meshToMeshError"

# Move back times to Case
mv "$Case/times/"* "$Case/"
rm -r "$Case/times"
END
