#!/bin/bash

# Rezone + Remap, and
# plot L1 error vs mesh spacing.

# Rezone can be between given source and target meshes, or random.
# Random rezoning can be cyclic two.
# Remap is advective

# Roadmap:
# Include other remaps: Interpolative

# Test data
source test_table.dat

# Rezone mode: the argument, else 'mode' from test_table.dat
if [ -n "$1" ] && [ -n "$mode" ] && [ "$1" != "$mode" ]; then
    echo "Warning: argument mode '$1' overrides test_table.dat mode '$mode'"
fi
mode=${1:-$mode}

# Cyclic mode adds -cyclic to cyclicRandomDALE
cyclicFlag=""
if [ "$cyclic" = on ]; then
    cyclicFlag="-cyclic"
fi

# Get the number of tests
# (can be counted from nX, since M is optional)
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

    # Set up time controls
    # Cyclic: only M and nRemap take effect; deltaT = 1/nRemap, endTime = M,
    # a write every nRemap steps. Otherwise study.sh sets them per study.
    if [ "$cyclic" = on ]; then
    #    deltaT=$(awk -v n="${nRemap[i]}" 'BEGIN { printf "%.15g", 1/n }')
         sed -i "s/^\(\s*startFrom\s\+\)\S\+;/\1latestTime;/" "$Case/system/controlDict"
    fi
         # Per-case values (test_table.dat case* arrays and nRemap)
         sed -i "s/END_TIME/${caseEndTimes[i]}/" "$Case/system/controlDict"
         sed -i "s/DELTA_T/${caseTimeSteps[i]}/" "$Case/system/controlDict"
         sed -i "s/WRITE_INTERVAL/${caseWriteIntervals[i]}/" "$Case/system/controlDict"
         sed -i "s/NREMAP/${nRemap[i]}/g" $Case/constant/ALEDict
    #fi

    # Copy source mesh and field to a new case
    echo Copy source mesh and field to source/
    if [ -d $Case/source ]; then
        rm -r $Case/"source"
    fi
    mkdir $Case/"source"
    cp -r $Case/constant $Case/system $Case/"source"

    # Case grading mesh
    echo Case grading mesh
    sed -i "s/nX/${nX[i]}/g" $Case/constant/polyMesh/blockMeshDict
    sed -i "s/nY/${nY[i]}/g" $Case/constant/polyMesh/blockMeshDict
    sed -i "s/nGradX/${nGradX[i]}/g" $Case/constant/polyMesh/blockMeshDict
    sed -i "s/nGradY/${nGradY[i]}/g" $Case/constant/polyMesh/blockMeshDict

    # Source uniform mesh
    echo Source uniform mesh
    sed -i "s/nX/${nX[i]}/g" $Case/"source"/constant/polyMesh/blockMeshDict
    sed -i "s/nY/${nY[i]}/g" $Case/"source"/constant/polyMesh/blockMeshDict
    # Uniform grading
    sed -i "s/nGradX/1/g" $Case/"source"/constant/polyMesh/blockMeshDict
    sed -i "s/nGradY/1/g" $Case/"source"/constant/polyMesh/blockMeshDict

    blockMesh -case $Case > $Case/log.blockMesh
    blockMesh -case $Case/"source" > $Case/"source"/log.blockMesh

    # Update schemes
    echo Update schemes
    sed -i "s/DDT/${ddtScheme[i]}/g" $Case/system/fvSchemes
    sed -i "s/GRAD_1/${grad1Scheme[i]}/g" $Case/system/fvSchemes
    sed -i "s/GRAD_2/${grad2Scheme[i]}/g" $Case/system/fvSchemes
    sed -i "s/MESH_PHI/${meshPhiScheme[i]}/g" $Case/system/fvSchemes

    # Update ALEDict

    # Update field
    echo Update field
    cp -r $Case/0_orig $Case/0
    cp -r $Case/0 $Case/"source"

    setAlgebraicFields -case $Case/"source" -a 0 -b 1 -c 1 -d 1 -sinsin \
        > $Case/"source"/log.setAlgebraicFields

    setAlgebraicFields -case $Case -a 0 -b 1 -c 1 -d 1 -sinsin \
        > $Case/log.setAlgebraicFields

    # Rezone + Remap
    if [[ $mode = "sinOnce" ]]; then

	# Required by meshDancImp implementation
        #ln -rs $Case/source/constant $Case/constant/sourceMesh

	# Required by cyclicRandomDALE
        ln -rs $Case/"source" $Case/constant/

        #meshDanceImp "sourceMesh" -case $Case \
	#	                  -flipFluxSign \
	#			  > $Case/log.meshDanceImp
	
	. ./runDALEsinOnce.sh

	#
        # Reset source to be the the same final mesh with exact values
	#

	# Find latest time
        times=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$')

        latestTime=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$' | tail -n1)

	# Overwrite source mesh with the final mesh
        cp -r "$Case/$latestTime"/polyMesh/* "$Case/source/"constant/polyMesh

	# Evaluate source field at source cells
        setAlgebraicFields -case $Case/"source" -a 0 -b 1 -c 1 -d 1 -sinsin \
            > $Case/"source"/log.setAlgebraicFields

    # Rezone + Remap
    elif [[ $mode = "once" ]]; then

	# Required by meshDancImp implementation
        #ln -rs $Case/source/constant $Case/constant/sourceMesh

	# Required by cyclicRandomDALE
        ln -rs $Case/"source" $Case/constant/

        #meshDanceImp "sourceMesh" -case $Case \
	#	                  -flipFluxSign \
	#			  > $Case/log.meshDanceImp
	
        cyclicRandomDALE -nY ${daleNY[i]:-${nY[i]}} \
                         ${M[i]:+-M ${M[i]}} \
                         ${alpha[i]:+-alpha ${alpha[i]}} \
                         -randomDirs \( "1" "1" "0" \)  \
                         -notFixedPatchNames \( "front" \)  \
                         -writeIntermediateMeshes \
                         ${cyclicFlag} \
                         -case $Case \
	  	         -seed ${seed[i]:-1000} \
			 -targetMesh "source" \
			 -maxIter 1 \
                         -flipFluxSign \
                         > $Case/log.cyclicRandomDALE #\
	  	         #-fixedTimeStep # Unnecessary, time will not be adjusted
	  	         # when controlDict.adjustTimestep is commented.

	#
        # Reset source to be the the same final mesh with exact values
	#

	# Find latest time
        times=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$')

        latestTime=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$' | tail -n1)

	# Overwrite source mesh with the final mesh
        cp -r "$Case/$latestTime"/polyMesh/* "$Case/source/"constant/polyMesh

	# Evaluate source field at source cells
        setAlgebraicFields -case $Case/"source" -a 0 -b 1 -c 1 -d 1 -sinsin \
            > $Case/"source"/log.setAlgebraicFields

    elif [[ $mode = "cyclic" ]]; then

        # Cyclic rezone + remap
        echo Cyclic rezone + remap ...
        cyclicRandomDALE -nY ${daleNY[i]:-${nY[i]}} \
                         ${M[i]:+-M ${M[i]}} \
                         -alpha .125 \
                         -randomDirs \( "1" "1" "0" \)  \
                         -notFixedPatchNames \( "front" \)  \
                         -writeIntermediateMeshes \
			 -cyclic \
                         -case $Case \
	  	         -seed ${seed[i]:-1000} \
			 -maxIter 1 \
                         > $Case/log.cyclicRandomDALE #\
	  	         #-fixedTimeStep # Unnecessary, time will not be adjusted
	  	         # when controlDict.adjustTimestep is commented.
    elif [[ $mode = "random" ]]; then

        # Random rezone + remap
        echo Random rezone + remap ...
        cyclicRandomDALE -nY ${daleNY[i]:-${nY[i]}} \
                         ${M[i]:+-M ${M[i]}} \
                         ${alpha[i]:+-alpha ${alpha[i]}} \
                         -motionDirs \( "1" "1" "0" \)  \
                         -notFixedPatchNames \( "front" \)  \
                         -writeIntermediateMeshes \
                         ${cyclicFlag} \
                         -case $Case \
	  	         -seed ${seed[i]:-1000} \
			 -maxIter 1 \
			 -flipFluxSign \
			 -random \
                         > $Case/log.cyclicRandomDALE #\
			 #-N_iter ${N_iter[i]} \
	  	         #-fixedTimeStep # Unnecessary, time will not be adjusted
	  	         # when controlDict.adjustTimestep is commented.

	#
        # Reset source to be the random mesh with exact values
	#

	# Find latest time
        times=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$')

        latestTime=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$' | tail -n1)

	# Overwrite source mesh with the final random mesh
        cp -r "$Case/$latestTime"/polyMesh/* "$Case/source/"constant/polyMesh

	# Evaluate source field at random cells
        setAlgebraicFields -case $Case/"source" -a 0 -b 1 -c 1 -d 1 -sinsin \
            > $Case/"source"/log.setAlgebraicFields

    elif [[ $mode = "interpolate" ]]; then
        miniMapAllFields > $Case/log.miniMapAllFields


    else
	 echo; echo "Error: unknown mode '$mode'; pass it as the argument or set"
	 echo "mode in test_table.dat: sinOnce once cyclic random interpolate"
	 echo
	 exit 1
    fi

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
        -sourceCase "source" > "$Case/log.meshToMeshError"

    # Move back times to Case
    mv "$Case/times/"* "$Case/"
    rm -r "$Case/times"
done

#
# Plot
#

echo

source "plot.sh"

echo End.
echo
