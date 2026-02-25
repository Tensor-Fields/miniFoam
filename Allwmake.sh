#!/bin/bash

# minifoam library
#wmakeLnInclude -f src
wmake libso src
wmake libso src/boundaryConditions

echo
echo See above for errors, if any.
echo libs compiled.
echo

# minifoam applications
#ls -1v test | while read -r app; do
#    wmake test/$app
#    done

wmake test/cyclicRandomDALE
wmake test/meshDanceImp
wmake test/LaplacianSmoothBilletMesh/
wmake test/ALEelasticFoam

wmake app/interpolateCellToPoint/
wmake app/mag/
wmake app/meshToMeshError/
wmake app/miniMapAllFields/
wmake app/scale/

echo
echo See above for errors, if any.
echo apps compiled.
echo
echo End.
echo
