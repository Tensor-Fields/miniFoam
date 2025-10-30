#!/bin/bash
rm -rf constant/polyMesh
\cp -r "previousPass.wire/0/wire"/polyMesh constant/ &> log.cp.existingCaseMesh.wire
\cp -r "previousPass.wire/0/wire"/polyMesh constant/wire/ &> log.cp.existingCaseMesh.wire.region
generateDieMesh -region die &> log.generateDieMesh.die
mergeMeshesAndFields . . . . -addRegion die -overwrite -noFields &> log.mergeMeshesAndFields.die
