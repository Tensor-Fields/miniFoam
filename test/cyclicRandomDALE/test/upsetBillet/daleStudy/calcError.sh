#!/bin/bash

## Calculate magnitude of vectorFields, and their errors

## Usage: Type
##    <script> -h

## Maalik (Ali Shayegh, ali@tensorfields.com)
## Richmond Hill, Dublin, IE
## Feb 09 2026, Night

printHelp()
{
    echo
    echo Important:
    echo "    Only execute from within <targetCase>"
    echo
    echo Usage:
    echo "    ./<script>    <vectorFieldName>    <targetCase>  <referenceCase>"
}

if [[ "${@}" =~ "-h" ]]
then
    printHelp
    exit 1
fi

if [[ $# -ne 3 ]]
then
    echo
    echo Error
    echo No. of args must be 3.
    printHelp
    exit 1
fi

## Parse args

## Vector field name
vf=$1

## Case for which error is going to be calculated, target case
Case=$2

## Reference case
Ref=$3

## Mapped reference case
mappedRef=mapped_"${Case}"

sss=$RANDOM;
eee=$RANDOM;

## Override time controls for this case
if ! echo "
//{$sss
startFrom    latestTime;

startTime    0.1;

stopAt       endTime;

endTime      1;

deltaT       1;

writeControl    timeStep;

writeInterval   1;
//$eee}
" >> $Case/system/controlDict;
then
    echo
    echo Error
    echo in echo
    exit 1
fi

## Override time controls for the reference case
if ! echo "
//{$sss
startFrom    latestTime;

startTime       0.1;

stopAt    endTime;

endTime      1;

deltaT       1;

writeControl    timeStep;

writeInterval   1;
//$eee}
" >> $Ref/system/controlDict;
then
    echo
    echo Error
    echo in echo
    exit 1
fi

## Create mag fields here and in the reference case
if ! mag -field $vf -case $Case
then
    echo
    echo Error
    echo in mag
    exit 1
fi

if ! mag -field $vf -case $Ref
then
    echo
    echo Error
    echo in mag -case $Ref
    exit 1
fi

## Create Mapped reference case
if ! mkdir $Case/$mappedRef; then
    echo
    echo Error
    echo mkdir $Case/$mappedRef failed
    echo Remove it if it exists.
    exit 1
fi

if ! cp -r $Case/{constant,system} $Case/$mappedRef; then
    echo
    echo Error
    echo "cp -r $Case/{constant,system} $Case/$mappedRef failed"
    exit 1
fi

## Overwrite the points of the mesh
latestTime=$(ls -1v $Case | grep -E '^[0-9](\.[0-9]*)?$' | tail -n1)

if ! cp -r $Case/$latestTime/polyMesh/* $Case/$mappedRef/constant/polyMesh
then
    echo
    echo Error
    echo "Overwritting the $mappedRef mesh failed"
    exit 1
fi

## Actually map
if ! miniMapAllFields map -sourceMesh $Ref -targetMesh $Case/$mappedRef
then
    echo
    echo Error
    echo "miniMapAllFields failed"
    exit 1
fi

## Calc error at the latest time step wrt the given case
# -atSourceCellCenters -unbounded
if ! meshToMeshError -case $Case -sourceCase $Case/$mappedRef -latestTargetTime \
    -latestSourceTime -consistent -writeErrAndRefFields \
    > $Case/log.meshToMeshError
then
    echo
    echo Error
    echo in meshToMeshError
    exit 1
fi

## Remove time controls for the target case
if ! sed -i "/{$sss/,/$eee}/d" $Case/system/controlDict
then
    echo
    echo Error
    echo in sed
    exit 1
fi

## Remove time controls for the Reference case
if ! sed -i "/{$sss/,/$eee}/d" $Ref/system/controlDict
then
    echo
    echo Error
    echo in sed
    exit 1
fi
