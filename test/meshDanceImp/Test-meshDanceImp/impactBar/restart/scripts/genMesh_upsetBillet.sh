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

clear

if ! genBlockMeshDict \
    -RR       10 \
    -ZZ       15 \
    -nOL       2 \
    -nPeriph   3 \
    -nRT      10 \
    -nZT      10 \
    -theta     1 \
    -Rd       20 \
    -dieThickness 1 \
    -nRd      1 \
    -convert  1e-3
then
    echo Error
    echo genBlockMeshDict failed
    exit 1
fi

if ! cp blockMeshDict constant/polyMesh/ ; then
    echo Error
    echo Could not cp blockMeshDict
    exit 1
fi

if ! blockMesh ; then
    echo Error
    echo blockMesh failed
    exit 1
fi

if ! autoPatch 80 -overwrite; then
    echo Error
    echo autoPatch failed
    exit 1
fi

if ! createPatch -overwrite; then
    echo Error
    echo createPatch failed
    exit 1
fi

if ! transformPoints -rotate "((1 0 0) (0 1 0))"; then
    echo Error
    echo transformPoints failed
    exit 1
fi

if ! transformPoints -rotate "((0 0 1) (1 0 0))"; then
    echo Error
    echo transformPoints failed
    exit 1
fi
