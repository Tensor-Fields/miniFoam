#!/bin/bash

# Do the cyclic random mesh motion test with advective remap.
# Plot L1 error vs mesh spacing.

# Test data
source test_table.dat

# Lipnikov and Shashkov
#declare -a  M=(8 16 32 64)
#declare -a nX=(15 30 60 120)
#declare -a nY=(16 32 64 128)

# Get the number of tests
s=${#M[@]}

# Traverse all tests
for((i=0; i<s; i++))
#for((i=0; i<1; i++))
do
    Case=${nX[i]}x${nY[i]}_${M[i]}

    # Create the case
    if [ -d $Case ]; then
        echo Remove existing $Case
        rm -r $Case
    fi

    cp -r template $Case
    echo $Case created.

    # Update mesh
    echo Update mesh
    sed -i "s/nX/${nX[i]}/g" $Case/constant/polyMesh/blockMeshDict
    sed -i "s/nY/${nY[i]}/g" $Case/constant/polyMesh/blockMeshDict

    blockMesh -case $Case > $Case/log.blockMesh

    # Update field
    echo Update field
    cp $Case/0_orig $Case/0 -r

    setAlgebraicFields -case $Case -a 0 -b 1 -c 1 -d 1 -sinsin \
        > $Case/log.setAlgebraicFields

    # Cyclic rezone + remap
    echo Cyclic rezone + remap ...
    cyclicRandomDALE -nY ${nY[i]} \
                     -M ${M[i]} \
                     -alpha .125 \
                     -randomDirs \( "1" "1" "0" \)  \
                     -notFixedPatchNames \( "front" \)  \
                     -writeIntermediateMeshes \
                     -case $Case > $Case/log.cyclicRandomDALE

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

    # Copy source mesh and field to a new case
    echo Copy source mesh and field to source/
    if [ -d $Case/source ]; then
        rm -r $Case/source
    fi
    mkdir $Case/source
    cp $Case/0 $Case/constant $Case/system $Case/source -r

    # Remove unnecessary files from the latestTime
    rm "$Case/$latestTime/V0" \
       "$Case/$latestTime/meshPhi"

    echo Calculate the remap error ...
    meshToMeshError \
        -case $Case \
        -sourceCase source > "$Case/log.meshToMeshError"

    # Move back times to Case
    mv "$Case/$time/"* "$Case/"
done

#
# Plot
#

#source "plot.sh"

echo End.
echo
