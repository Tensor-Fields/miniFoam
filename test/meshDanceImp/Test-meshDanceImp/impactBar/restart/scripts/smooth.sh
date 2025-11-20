#!/bin/bash

# Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin, IE
# May 03 2025

# Smooth the mesh
LaplacianSmoothBilletMesh -freePatches \( "front" "back" \) \
                          -slideCrossingPoints \
                          -latestTime \
                          -maxIter 100 \
                          -crossingPatches \( "front" "back" \) \
                          -patchWithSlidingPoints "abd" # place holder

# Map fields

# NOTE: The latest mesh needs to be copied to constant to a directory called
# "targetMesh"

#meshDanceImp    targetMesh

