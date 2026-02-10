#!/bin/bash

## Calculate magnitude of vectorFields, and their errors

## Usage:
## ./<script>    <vectorFieldName>    <referenceCase>

## Maalik (Ali Shayegh, ali@tensorfields.com)
## Richmond Hill, Dublin, IE
## Feb 09 2026, Night

if [[ $# -ne 2 ]] 
then
    echo
	echo Error
	echo No. of args must be 2.
	exit 1
fi

## Parse args

## Vector field name
vf=$1

## Reference case
Ref=$2

sss=$RANDOM;
eee=$RANDOM;

## Override time controls for this case
if ! echo "
//{$sss
startFrom    startTime;

startTime       0.1;

stopAt    endTime;

endTime    1;

deltaT          0.1;

writeControl    timeStep;

writeInterval   1;
//$eee}
" >> system/controlDict;
then
    echo
	echo Error
	echo in echo
	exit 1
fi

## Override time controls for the reference case
if ! echo "
//{$sss
startFrom    startTime;

startTime       0.1;

stopAt    endTime;

endTime    1;

deltaT          0.1;

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
if ! mag -field $vf
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

## Calc error at the latest time step wrt the given case
# -atSourceCellCenters -unbounded
if ! meshToMeshError -sourceCase $Ref -latestTargetTime -latestSourceTime
then
    echo
	echo Error
	echo in meshToMeshError
	exit 1
fi

## Remove time controls for the target case
if ! sed -i "/{$sss/,/$eee}/d" system/controlDict
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
