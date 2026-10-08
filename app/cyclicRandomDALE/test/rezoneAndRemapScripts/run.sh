#!/bin/bash

#Case="."

cyclicRandomDALE            \
                 -alpha .25 \
                 -beta  0.5 \
                 -notFixedPatchNames \( "front" \)  \
                 -writeIntermediateMeshes           \
                 -maxIter 1    \
                 -flipFluxSign \
                 -sinCycle     \
                 -fixedPatchNames \( "movingWall" "fixedWalls" \) \
                 -motionDirs \( 1 1 0 \) \
                 > log.cyclicRandomDALE #\

                 #> $Case/log.cyclicRandomDALE #\
                 #-case $Case \

