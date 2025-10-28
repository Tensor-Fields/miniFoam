#!/bin/bash

#
# We do not use Test.sh, because we do not need most of it
# We copy the bit we need here.

# Usage
# ./rezoneAndRemap <case_dir>
#

# Parse args
Case=$1

#
# Create source dir
#

# Copy source mesh and field to a new case

echo Copy source mesh and field to source/
if [ -d $Case/source ]; then
    rm -r $Case/source
fi

mkdir $Case/source
cp -r $Case/0 $Case/constant $Case/system $Case/source

#
# Create the final (smooth) mesh
#

# Smooth the mesh
echo LaplacianSmoothBilletMesh ...

# Add these options when the geometry is not wedge
# -noWedge \
# -emptyDir 2 \

LaplacianSmoothBilletMesh -freePatches \( "front" "back" \) \
                          -slideCrossingPoints \
                          -latestTime \
                          -maxIter 100 \
                          -crossingPatches \( "front" "back" \) \
                          -wedgeDir 2 \
                          -radialDir 1 \
                          -wedgeAperture 1 \
                          -case $Case \
                          -patchWithSlidingPoints "abd" \
                          -noFunctionObjects \
                          > log.LaplacianSmoothBilletMesh
