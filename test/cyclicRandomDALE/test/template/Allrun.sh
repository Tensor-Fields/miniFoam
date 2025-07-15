#!/bin/sh

## Initialize fields
echo "setAlgebraicFields ..."
setAlgebraicFields > log.setAlgebraicFields

## Move mesh and advect fields
echo "meshDanceImp meshTarget ..."
meshDanceImp meshTarget > log.meshDanceImp

## A trick for now; meshToMeshError should be corrected, so that this is not
## required
ln -rs 0/meshSource 0.5/ # Okay, meshSource does not change
#ln -rs 0.5/meshPhi 0.5/meshSource/
rm 0.5/meshPhi
rm 0.5/V0

## Calculate the Error caused by mapping
echo "meshToMeshError ..."
meshToMeshError > log.meshToMeshError

echo; echo "End."
cyclicRandomDALE -nY 16 -alpha 0.125 -M 8 -randomDirs \( "1" "1" "0" \) |   grep -v Courant
