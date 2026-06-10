#!/bin/bash

##
## Generate wire mesh 
##
#generateWireMesh -region wire #&> log.generateWireMesh.wire
#if ( ! blockMesh -region wire > log.generateWireMesh.wire )
if ! blockMesh -region wire &> log.blockMesh; then 
    exit; fi

rm -rf constant/polyMesh
cp -r constant/wire/polyMesh constant/ #&> log.cp.mesh.wire
