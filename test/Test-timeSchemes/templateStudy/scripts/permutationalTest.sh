#!/bin/bash

# Specialize a template case into multiple cases and run them in a loop. 

# Roadmap:

## -------------------------------Code Starts-------------------------------- ##

#
# Traverse all Cases
#

caseCounter=0

for d2dt2Scheme in "${D2dt2Schemes[@]}"; do
    #loop1Counter=0
    for dT in "${DeltaTs[@]}"; do
        ## Parse dT
        IFS=, read -a arr <<< "$dT"
        deltaT="${arr[0]}"
        writeInterval="${arr[1]}"
        endTime="${arr[2]}"

        echo
        echo Run new case ...
        CaseNames+=("${d2dt2Scheme}_${deltaT}")

        Case="${CaseNames[$caseCounter]}"

        if [ -z $Case ]; then
            echo Case not created.
            exit 1
        fi

        ## Create the case
        if [ -d $Case ]; then
            echo Remove existing $Case
            rm -r $Case
        fi

        if ! cp -r $templateCase $Case; then
            echo Error
            echo $templateCase not found
            exit 1
        else
            echo $Case created.
        fi

        ##
        ## Update controlDict
        ##

        ## timeControls
            ## deltaT
            sed -i \
                "/deltaT/,/\;/s/deltaT\( *\)\(.*\)$/deltaT\1${deltaT}\;/g" \
                $Case/system/controlDict

            ## writeInterval
            sed -i \
                "/writeInterval/,/\;/s/writeInterval\( *\)\(.*\)$/writeInterval\1${writeInterval}\;/g" \
                $Case/system/controlDict

            ## endTime
            sed -i \
                "/endTime/,/\;/s/endTime\( *\)\(.*\)$/endTime\1${endTime}\;/g" \
                $Case/system/controlDict

        ##
        ## Update schemes
        ##
        echo Update schemes

        ## d2dt2
        sed -i \
            "/d2dt2Scheme/,/}/s/d2dt2\((.*)\ *\)\(.*\)$/d2dt2\1${d2dt2Scheme}\;/g" \
            $Case/system/fvSchemes

        # Update field
        echo Update field
        cp -r $Case/0_orig $Case/0

        #setAlgebraicFields -case $Case -a 0 -b 1 -c 1 -d 1 -sinsin \
        #    > $Case/log.setAlgebraicFields

        waveFoam \
            -case $Case > $Case/log.waveFoam

        ##
        ## Get latestTime if needed
        ##
        times=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$')

        latestTime=$(ls -1v $Case | grep -E '^[0-9]+(\.[0-9]+)?$' | tail -n1)

        ##
        ## Error Calculations
        ##

        # Remove unnecessary files from the latestTime
        if [ -f "$Case/$latestTime/V0" ]; then
            rm "$Case/$latestTime/V0"
        fi

        if [ -f "$Case/$latestTime/meshPhi" ]; then
            rm "$Case/$latestTime/meshPhi"
        fi

        echo Calculate error ...
        meshToMeshError \
            -case $Case \
            -sourceCase "${SourceCase[0]}" \
            -latestTargetTime \
            -latestSourceTime > "$Case/log.meshToMeshError"

        ((caseCounter++));

    done
done

#
# Plot
#

echo

#source "plot.sh"

echo End.
echo
