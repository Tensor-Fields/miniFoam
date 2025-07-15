#!/bin/sh

blockMesh

setAlgebraicFields -a 0 -b 1 -c 1 -d 1 -sinsin

cyclicRandomDALE -nY 8 \
                 -alpha .125 \
                 -M 8 \
                 -randomDirs \( "1" "1" "0" \)  \
                 -notFixedPatchNames \( "front" \) -writeIntermediateMeshes
                 
#grep -v Courant

#echo; echo "End."
