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
# Poly mesh creation                                                          #
#-----------------------------------------------------------------------------#

## Approach 2 --- foam-extend-4.0
## Note: `fms' preserves boundary names after pMesh, stl does not.
## Note: The produced `fms' needs to be referred to in meshDict.
    if ! surfaceFeatureEdges constant/billet.stl \
         constant/billet_with_edges.fms \
         -angle 30 > log.surfaceFeatureEdges
    then echo "Error in surfaceFeatureEdges"; echo; exit 1; fi

    if ! pMesh > log.pMesh
    then echo "Error in pMesh"; echo; exit 1; fi
