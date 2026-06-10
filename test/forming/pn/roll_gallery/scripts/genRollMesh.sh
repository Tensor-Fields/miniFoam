#!/bin/bash

##
## Generate roll mesh 
##
if ! generate3DRollerMesh -region rolls &> log.generate3DRollerMesh; then #&> log.generate3DRollerMesh.rollers
    echo Error in roll mesh generation; exit 1; fi

mergeMeshesAndFields . . . . -addRegion rolls -overwrite -noFields #&> log.mergeMeshesAndFields.rollers

##
## Position rolls (see bekaertFunctions.sh)
##
positionCellZone topRoll    topRollrollerContact1    wireContact "(0 -1 0)" \
    topTrans.txt 1e-3 100

positionCellZone bottomRoll bottomRollrollerContact1 wireContact "(0  1 0)" \
    botTrans.txt 1e-3 100

positionCellZone rightRoll  rightRollrollerContact1  wireContact "(0 0 -1)" \
    righTrans.txt 1e-3 100
