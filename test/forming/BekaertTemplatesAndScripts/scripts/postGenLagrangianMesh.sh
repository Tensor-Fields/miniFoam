#!/bin/bash

## Allrun generates the mesh and starts the run, BUT,
## a Lagrangian simulation is reasonable in terms of wire positioning if
## initially a long wire's middle is not right beneath the roll's center, but, it
## is mostly downstream, and only a tiny fraction of it is upstream.

## Maalik (Ali Shayegh, ali@tensorfields.com), Richmond Hill, Dublin
## 06 Feb 2026, a cloudy/rainy afternoon

#
## Algorithm
#

## When Allrun reaches to the solver, stop the solver
## Remove constant/polymesh
rm -r constant/polyMesh

## translate wire, eg if wire length is 0.3, translate -0.1
transformPoints -translate "(-0.1 0 0)" -region wire

## Merge
cp constant/wire/polyMesh constant -r
mergeMeshes . . . . -addRegion rollers -noFunctionObjects

## Overwrite polyMesh (mergeMeshes generates a time directory)
cp 0.001/polyMesh/* constant/polyMesh -r
rm -r 0.001

## Position topRoll to touch the wire
positionCellZone topRoll rollerContact wireContact (0 -1 0) topRoll.txt 1e-3 1
