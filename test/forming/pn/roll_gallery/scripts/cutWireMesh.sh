#!/bin/bash

## Cut the wire
if ! setSet -batch wireQuarter.setSet &> log.setSet
    then echo Error in setSet; exit 1; fi

if ! subsetMesh -overwrite -patch wireSymmetryY quarter &> log.subsetMesh
    then echo Error in subsetMesh; exit 1; fi

rm -r constant/polyMesh
cp -r constant/setSubset/polyMesh constant/
