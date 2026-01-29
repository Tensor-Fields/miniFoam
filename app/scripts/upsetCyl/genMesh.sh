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
# Clean
#-----------------------------------------------------------------------------#
source clean.sh

#-----------------------------------------------------------------------------#
# Pre-mesh
#-----------------------------------------------------------------------------#

## Generate a cube mesh --- OpenFOAM 13 or fe40
if ! blockMesh -dict system/blockMeshDict.billet > log.blockMesh
then echo "Error in blockMesh"; echo; exit 1; fi

## Cube to stl --- OpenFOAM 13
if ! foamToSurface constant/billet.stl > log.foamToSurface
then echo "Error in foamToSurface"; echo; exit 1; fi

#-----------------------------------------------------------------------------#
# Poly mesh creation                                                          #
#-----------------------------------------------------------------------------#

## Approach 1 --- gmsh + OF13
    # - Generate tet mesh in gmsh
    # - Export to constant/billet.unv
    # - Uncomment the following lines
    
    #ideasUnvToFoam constant/billet.unv
    #polyDualMesh 30

## Approach 2 --- foam-extend-4.0
## Note: `fms' preserves boundary names after pMesh, stl does not.
## Note: The produced `fms' needs to be referred to in meshDict.
    if ! surfaceFeatureEdges constant/billet.stl \
         constant/billet_with_edges.fms \
         -angle 30 > log.surfaceFeatureEdges
    then echo "Error in surfaceFeatureEdges"; echo; exit 1; fi

    if ! pMesh > log.pMesh
    then echo "Error in pMesh"; echo; exit 1; fi

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
# Patchify
#-----------------------------------------------------------------------------#
