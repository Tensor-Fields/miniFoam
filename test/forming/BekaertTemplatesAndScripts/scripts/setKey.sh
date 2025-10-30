#!/bin/bash

#
# Replace values for given keywords in all fileModifiers
#
# Usage
# ./setKey.sh
# 
# Maalik (ali@tensorfields.com), RH, Dublin
# Date: 20 Sep 2025
#

if [[ "$1" == "-h" ]]
then
    echo \
    "    nCPUs
    endTime (0.11 @ mesh level 0)
    wireMeshSize (6e-4 @ mesh level 0)
    wireContactCellSize (6e-4 @ mesh level 0)
    wireLength (0.2 at the baseline level 0 case)
    rollMeshSize (4e-3 @ mesh level 0)
    rollContactCellSize (8e-4 @ mesh level 0)
    contactLength (0.12 @ mesh level 0)
    deltaT (0.002 @ templateCase)
    frictionCoeff (0.25 @ templateCase)"
    exit 1
fi

file=$1

get_field()
{
	local pattern=$1
    local file=$2

    cat $file | grep $pattern | awk -F" " '{print$2}'
}

#nCPUs=$1
#endTime=$2
#wireMeshSize=$3
#wireContactCellSize=$4
#wireLength=$5
#rollMeshSize=$6
#rollContactCellSize=$7
#contactLength=$8
#deltaT=$9
#frictionCoeff="${10}"

nCPUs=$(get_field nCPU $file)
endTime=$(get_field endTime $file)
wireMeshSize=$(get_field wireMeshSize $file)
wireContactCellSize=$(get_field wireContactCellSize $file)
wireLength=$(get_field wireLength $file)
rollMeshSize=$(get_field rollMeshSize $file)
rollContactCellSize=$(get_field rollContactCellSize $file)
contactLength=$(get_field contactLength $file)
deltaT=$(get_field deltaT $file)
frictionCoeff=$(get_field frictionCoeff $file)

find . -name fileModifiers | while read -r file; do \
    echo Read $file ...

    sed -i "s/\(numberOfSubdomains\).*$/\1 $nCPUs\;/g" $file
    sed -i "s/\(system\/controlDict\/endTime\).*$/\1 $endTime\;/g" $file

	#
	# Keywords to be added
	#

	# This specific key does not exist beforehand, we remove it as a
	# precautionary action and add it afterwards

	## deltaT

	### # Remove
	### sed -i \
	### "s/system\/controlDict\/deltaT.*)\;/)\;/g" \
	### $file
	### # Add
	### sed -i \
	### "s/)\;/system\/controlDict\/deltaT $deltaT\;)\;/g" \
	### $file

	sed -i \
	"s/\(system\/controlDict\/deltaT\).*$/\1 $deltaT\;/g" \
	$file

	## contactLength

	### # Remove
	### sed -i \
	### "s/system\/rollers\/meshDict\/contactLength.*)\;/)\;/g" \
	### $file
	### # Add
	### sed -i \
	### "s/)\;/system\/rollers\/meshDict\/contactLength $contactLength\;)\;/g" \
	### $file

	sed -i \
	"s/\(system\/rollers\/meshDict\/contactLength\).*$/\1 $contactLength\;/g" \
	$file

	sed -i \
	"s/\(system\/wire\/meshDict\/wireMeshDict\/wireLength\).*$/\1 $wireLength\;/g" \
	$file
    sed -i "s/\(wireMeshDict\/maxCellSize\).*$/\1 $wireMeshSize\;/g" $file
    sed -i "s/\(wireMeshDict\/contactCellSize\).*$/\1 $wireContactCellSize\;/g" $file
    sed -i "s/\(twoHighRollsDict\/topRoll\/maxCellSize\).*/\1 $rollMeshSize\;/g" $file
    sed -i \
	"s/\(twoHighRollsDict\/topRoll\/contactCellSize\).*/\1 $rollContactCellSize\;/g" \
	$file
    sed -i "s/\(twoHighRollsDict\/bottomRoll\/maxCellSize\).*/\1 $rollMeshSize\;/g" $file
    sed -i \
	"s/\(twoHighRollsDict\/bottomRoll\/contactCellSize\).*/\1 $rollContactCellSize\;/g" \
	$file

    sed -i \
	"s/\(frictionCoeff\).*/\1 $frictionCoeff\;/g" \
	$file

	echo Substitutions done.
	echo
done

echo \
"
nCPUs                  $nCPUs
endTime                $endTime
wireMeshSize           $wireMeshSize
wireContactCellSize    $wireContactCellSize
wireLength             $wireLength
rollMeshSize           $rollMeshSize
rollContactCellSize    $rollContactCellSize
contactLength          $contactLength
deltaT                 $deltaT
frictionCoeff          $frictionCoeff" \
> log.setKey

echo New settings written to log.setKey
echo; echo End.
