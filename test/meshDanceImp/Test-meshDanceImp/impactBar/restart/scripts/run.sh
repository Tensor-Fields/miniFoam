#!/bin/bash

mkdir -p constant/polyMesh
m4 system/blockMeshDict.m4 > constant/polyMesh/blockMeshDict
blockMesh > log.blockMesh

#solids4Foam > log.solver

#decomposePar -force > log.decomposePar
#mpirun -np 4 solids4Foam -parallel > log.solver

form > log.form

