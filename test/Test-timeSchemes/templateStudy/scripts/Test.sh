#!/bin/bash

# Specialize multiple template case and run it.

# Roadmap:
# Include other remaps: Interpolative

## -------------------------------Code Starts-------------------------------- ##

##
## Parse
##
templateCase=$1
scriptsDir=$2
input=$3

shift 3

CaseNames=("$@")

##
## Initialize
##
nCases=${#CaseNames[@]}

#
# Traverse all Cases
#
for((i=0; i<nCases; i++))
do
    #source caseName.sh
    Case="${CaseNames[i]}"

    if [ -z $Case ]; then
        echo Case not created.
        exit 1
    fi

    # Create the case
    if [ -d $Case ]; then
        echo Remove existing $Case
        rm -r $Case
    fi

    if ! cp -r $templateCase $Case; then
        echo Error
        echo $templateCase not found
        exit 1
    else
        echo $Case created.
    fi

    # Set up time controls
    sed -i "s/END_TIME/${studyEndTimes[i]}/" "$Case/system/controlDict"
    sed -i "s/DELTA_T/${studyTimeSteps[i]}/" "$Case/system/controlDict"
    sed -i "s/WRITE_INTERVAL/${studyWriteIntervals[i]}/" "$Case/system/controlDict"

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
    sed -i "s/nY/${nY[i]}/g" $Case/constant/polyMesh/blockMeshDict
    sed -i "s/nGradX/${nGradX[i]}/g" $Case/constant/polyMesh/blockMeshDict
    sed -i "s/nGradY/${nGradY[i]}/g" $Case/constant/polyMesh/blockMeshDict

    # Source uniform mesh
    echo Source uniform mesh
    sed -i "s/nX/${nX[i]}/g" $Case/source/constant/polyMesh/blockMeshDict
    sed -i "s/nY/${nY[i]}/g" $Case/source/constant/polyMesh/blockMeshDict

    # Uniform grading
    sed -i "s/nGradX/1/g" $Case/source/constant/polyMesh/blockMeshDict
    sed -i "s/nGradY/1/g" $Case/source/constant/polyMesh/blockMeshDict

    blockMesh -case $Case > $Case/log.blockMesh
    blockMesh -case $Case/source > $Case/source/log.blockMesh

    ##
    ## Update controlDict
    ##

    ## deltaT
    sed -i \
        "/deltaT/,/\;/s/deltaT\( *\)\(.*\)$/deltaT\1${DeltaTs[i]}\;/gp" \
        $Case/system/controlDict

    ##
    ## Update schemes
    ##
    echo Update schemes

    ## d2dt2
    sed -i \
        "/d2dt2Scheme/,/}/s/d2dt2\((.*)\*\)\(.*\)$/d2dt2\1${D2dt2Schemes[i]}\;/gp" \
        $Case/system/fvSchemes

    sed -i "s/DDT/${ddtScheme[i]}/g" $Case/system/fvSchemes
    sed -i "s/GRAD_1/${grad1Scheme[i]}/g" $Case/system/fvSchemes
    sed -i "s/GRAD_2/${grad2Scheme[i]}/g" $Case/system/fvSchemes
    sed -i "s/MESH_PHI/${meshPhiScheme[i]}/g" $Case/system/fvSchemes

    # Update field
    echo Update field
    cp -r $Case/0_orig $Case/0
    cp -r $Case/0 $Case/source

    setAlgebraicFields -case $Case/source -a 0 -b 1 -c 1 -d 1 -sinsin \
        > $Case/source/log.setAlgebraicFields

    setAlgebraicFields -case $Case -a 0 -b 1 -c 1 -d 1 -sinsin \
        > $Case/log.setAlgebraicFields

    # Rezone + Remap
    if [[ $1 = "once" ]]; then

	# Required by meshDancImp implementation
        #ln -rs $Case/source/constant $Case/constant/sourceMesh

	# Required by cyclicRandomDALE
        ln -rs $Case/source $Case/constant/

        #meshDanceImp "sourceMesh" -case $Case \
	#	                  -flipFluxSign \
	#			  > $Case/log.meshDanceImp
	
        cyclicRandomDALE -nY ${nY[i]} \
                         -M ${M[i]} \
                         -alpha ${alpha[i]} \
                         -randomDirs \( "1" "1" "0" \)  \
                         -notFixedPatchNames \( "front" \)  \
                         -writeIntermediateMeshes \
                         -case $Case \
	  	         -seed $((RANDOM)) \
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
        setAlgebraicFields -case $Case/source -a 0 -b 1 -c 1 -d 1 -sinsin \
            > $Case/source/log.setAlgebraicFields

    elif [[ $1 = "cyclic" ]]; then

        # Cyclic rezone + remap
        echo Cyclic rezone + remap ...
        cyclicRandomDALE -nY ${nY[i]} \
                         -M ${M[i]} \
                         -alpha .125 \
                         -randomDirs \( "1" "1" "0" \)  \
                         -notFixedPatchNames \( "front" \)  \
                         -writeIntermediateMeshes \
			 -cyclic \
                         -case $Case \
	  	         -seed $((RANDOM)) \
			 -maxIter 1 \
                         > $Case/log.cyclicRandomDALE #\
	  	         #-fixedTimeStep # Unnecessary, time will not be adjusted
	  	         # when controlDict.adjustTimestep is commented.
    elif [[ $1 = "random" ]]; then

        # Random rezone + remap
        echo Random rezone + remap ...
        cyclicRandomDALE -nY ${nY[i]} \
                         -M ${M[i]} \
                         -alpha ${alpha[i]} \
                         -randomDirs \( "1" "1" "0" \)  \
                         -notFixedPatchNames \( "front" \)  \
                         -writeIntermediateMeshes \
                         -case $Case \
	  	         -seed $((RANDOM)) \
			 -maxIter 10 \
			 -flipFluxSign \
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
        setAlgebraicFields -case $Case/source -a 0 -b 1 -c 1 -d 1 -sinsin \
            > $Case/source/log.setAlgebraicFields

    elif [[ $1 = "interpolate" ]]; then
        miniMapAllFields > $Case/log.miniMapAllFields


    else
	 echo; echo Error: Test.sh needs an argument:
	 echo cyclic or once
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
        -sourceCase source > "$Case/log.meshToMeshError"

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
