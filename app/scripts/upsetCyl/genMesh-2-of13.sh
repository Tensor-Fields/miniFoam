#!/bin/bash

#-----------------------------------------------------------------------------#
# Generate a polygonal wedge 
#
# Key
#    OF13: OpenFOAM 13
#    fe40: foam-extend-4.0
#
# Author: Maalik (ali@tensorfields.com), Richmond Hill, Dublin, IE
# Date: 29 Jan 2026, Night
#-----------------------------------------------------------------------------#

#-----------------------------------------------------------------------------#
# Extrude to a wedge --- OF13
#-----------------------------------------------------------------------------#
if ! extrudeMesh > log.extrudeMesh
then echo "Error in extrudeMesh"; echo; exit 1; fi

#-----------------------------------------------------------------------------#
# Die --- OF13
#-----------------------------------------------------------------------------#
if ! blockMesh -region die -dict system/die/blockMeshDict.die > \
    log.blockMesh.die
then echo "Error in blockMeshDict die"; echo; exit 1; fi

if ! mergeMeshes -addRegions \("die"\) > log.mergeMeshes
then echo "Error in mergeMeshes"; echo; exit 1; fi

#-----------------------------------------------------------------------------#
# Make ready for solver
#-----------------------------------------------------------------------------#
sed -i "/FoamFile/,/}/s/}/    version     2.0\;\n}/" constant/polyMesh/*

rm constant/polyMesh/cellZones
