#!/bin/bash

# Do the cyclic random mesh motion test with advective remap.
# Plot L1 error vs mesh spacing.

# Test data
source test_table.dat

# Get the number of tests
s=${#M[@]}

# Traverse all tests
for((i=0; i<s; i++))
#for((i=0; i<1; i++))
do
    source caseName.sh

    # Create the case
    if [ -d $Case ]; then
        echo Remove existing $Case
        rm -r $Case
    fi

    cp -r template $Case
    echo $Case created.

    # Copy source mesh and field to a new case
    echo Copy source mesh and field to source/
    if [ -d $Case/source ]; then
        rm -r $Case/source
    fi
    mkdir $Case/source
    cp -r $Case/constant $Case/system $Case/source

    # Case grading mesh
    echo Case grading mesh
    sed -i "s/nX/${nX[i]}/g" $Case/constant/polyMesh/blockMeshDict
    sed -i "s/nY/${nX[i]}/g" $Case/constant/polyMesh/blockMeshDict
    sed -i "s/nGradX/${nGradX[i]}/g" $Case/constant/polyMesh/blockMeshDict
    sed -i "s/nGradY/${nGradY[i]}/g" $Case/constant/polyMesh/blockMeshDict

    # Source uniform mesh
    echo Source uniform mesh
    sed -i "s/nX/${nX[i]}/g" $Case/source/constant/polyMesh/blockMeshDict
    sed -i "s/nY/${nX[i]}/g" $Case/source/constant/polyMesh/blockMeshDict
    sed -i "s/nGradX/1/g" $Case/source/constant/polyMesh/blockMeshDict
    sed -i "s/nGradY/1/g" $Case/source/constant/polyMesh/blockMeshDict

    blockMesh -case $Case > $Case/log.blockMesh
    blockMesh -case $Case/source > $Case/source/log.blockMesh

    # Update schemes
    echo Update schemes
    sed -i "s/MESH_PHI/${meshPhiScheme[i]}/g" $Case/system/fvSchemes
    sed -i "s/DDT/${ddtScheme[i]}/g" $Case/system/fvSchemes

    # Update field
    echo Update field
    cp -r $Case/0_orig $Case/0
    cp -r $Case/0 $Case/source

    setAlgebraicFields -case $Case -a 0 -b 1 -c 1 -d 1 -sinsin \
        > $Case/log.setAlgebraicFields

    setAlgebraicFields -case $Case/source -a 0 -b 1 -c 1 -d 1 -sinsin \
        > $Case/log.setAlgebraicFields

    # rezone + remap
    ln -rs $Case/source/constant $Case/constant/sourceMesh

    meshDanceImp "sourceMesh" -case $Case > $Case/log.meshDanceImp

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
done

#
# Plot
#

source "plot.sh"

echo End.
echo
