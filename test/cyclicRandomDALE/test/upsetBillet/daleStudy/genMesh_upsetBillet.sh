#!/bin/bash

##
## Create an upsetting billet mesh and patchify it
##
## Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin
## 21 Nov 2025, Midnight
##

    #-RR <scalar>: Max radii
    #-ZZ <scalar>: Max height
    #-nOL <label>: N of layers
    #-nPeriph <label>: N of corner segments
    #-nRT <label>: N of total radial segments
    #-nZT <label>: N of total longitudinal segments
    #-theta <scalar>: Apperture of the cylinder
    #-Rd <scalar>: Die radii
    #-nRd <label>: N of raial cells of die
    #-convert <scalar>: convetToMeters

clear

Case=$1

	# level -1
    #-nRT      5 \
    #-nZT      5 \

	# level 0
    #-nRT      10 \
    #-nZT      10 \
    #-nRd      20 \

	# level 1 --- Failed
    #-nRT      20 \
    #-nZT      20 \
    #-nRd      40 \

	# level 1.1 --- Failed
    #-nRT      21 \
    #-nZT      21 \

	# level 2
    #-nRT      40 \
    #-nZT      40 \
    #-nRd      80 \

	##
	## Prime number levels
	##

	# level 0
    #-nRT      7 \
    #-nZT      7 \

	# level 1 --- Failed
    #-nRT      14 \
    #-nZT      14 \

	# level 2 --- Successful
    #-nRT      28 \
    #-nZT      28 \

	##
	## Are we out of the dangerous zone
	##

	# level 0 --- Successful
    #-nRT      30 \
    #-nZT      30 \

	# level 1
    #-nRT      60 \
    #-nZT      60 \

	# level 2
    #-nRT      120 \
    #-nZT      120 \

if ! genBlockMeshDict \
    -nRT      40 \
    -nZT      40 \
    -nRd      1 \
    -RR       10 \
    -ZZ       15 \
    -nOL       1 \
    -nPeriph   1 \
    -theta     1 \
    -Rd       20 \
    -dieThickness 1 \
    -convert  1e-3 \
    -case $Case > $Case/log.genBlockMeshDict
then
    echo Error
    echo genBlockMeshDict failed
    exit 1
fi

if ! mv blockMeshDict $Case; then
    echo Error
    echo mv blockMeshDict to $Case failed
    exit 1
fi

mkdir -p $Case/constant/polyMesh

if ! rm -rf $Case/constant/polyMesh/blockMeshDict; then
    echo Error
    echo Could not rm polyMesh/blockMeshDict
    exit 1
fi

if ! mv $Case/blockMeshDict $Case/constant/polyMesh/; then
    echo Error
    echo Could not cp blockMeshDict
    exit 1
fi

if ! blockMesh -case $Case > $Case/log.blockMesh; then
    echo Error
    echo blockMesh failed
    exit 1
fi

#if ! autoPatch 70 -overwrite -case $Case > $Case/log.autoPatch; then
#    echo Error
#    echo autoPatch failed
#    exit 1
#fi

#if ! createPatch -overwrite -case $Case > $Case/log.createPatch; then
#    echo Error
#    echo createPatch failed
#    exit 1
#fi

if ! transformPoints -rotate "((1 0 0) (0 1 0))" -case $Case \
    > $Case/log.transformPoints.1; then
    echo Error
    echo transformPoints failed
    exit 1
fi

if ! transformPoints -rotate "((0 0 1) (1 0 0))" -case $Case \
    > $Case/log.transformPoints.2; then
    echo Error
    echo transformPoints failed
    exit 1
fi
