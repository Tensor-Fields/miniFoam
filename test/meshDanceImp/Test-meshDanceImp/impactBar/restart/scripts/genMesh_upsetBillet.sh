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

if ! genBlockMeshDict \
    -RR       10 \
    -ZZ       15 \
    -nOL       1 \
    -nPeriph   2 \
    -nRT      10 \
    -nZT      10 \
    -theta     1 \
    -Rd       20 \
    -dieThickness 1 \
    -nRd      1 \
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
    
if ! cp $Case/blockMeshDict $Case/constant/polyMesh/ ; then
    echo Error
    echo Could not cp blockMeshDict
    exit 1
fi

if ! blockMesh -case $Case > $Case/log.blockMesh; then
    echo Error
    echo blockMesh failed
    exit 1
fi

if ! autoPatch 70 -overwrite -case $Case > $Case/log.blockMesh; then
    echo Error
    echo autoPatch failed
    exit 1
fi

if ! createPatch -overwrite -case $Case > $Case/log.createPatch; then
    echo Error
    echo createPatch failed
    exit 1
fi

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
