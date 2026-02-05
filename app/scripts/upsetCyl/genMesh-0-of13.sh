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
