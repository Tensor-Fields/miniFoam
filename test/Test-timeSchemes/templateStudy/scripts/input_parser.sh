#!/bin/bash

input="$1"

# Arrays have the same name as their counterpart in input.dat, buty the former
# starts with Upper-case.
declare -a D2dt2Schemes DeltaTs StudyNames Permutational SourceCase Description
while read -r key values; do
    # Skip lines starting with optional spaces/tabs followed by #
    [[ $key =~ ^[[:space:]]*# ]] && continue
    case $key in
        d2dt2Schemes) D2dt2Schemes=($values) ;;
        #deltaTs) DeltaTs=($values) ;;
        timeControls) 
            # Convert [a,b,c] to space-separated and store
            while [[ $values =~ \[([^]]+)\] ]]; do
                DeltaTs+=("${BASH_REMATCH[1]}")
                values=${values#*]}  # Remove processed element
            done
            ;;
        studyNames) StudyNames=($values) ;;
        permutational) Permutational=($values) ;;
        sourceCase) SourceCase=($values) ;;
        description) Description=($values) ;;
    esac
done < $input

## Initialazed arrays must be non-empty
    [ ${#D2dt2Schemes[@]} -eq 0 ] \
 && { echo "input.dat/d2dt2Schemes is empty"; exit 1; }

    [ ${#DeltaTs[@]} -eq 0 ] \
 && { echo "input.dat/deltaTs is empty"; exit 1; }

nCases=$(( ${#DeltaTs[@]} * ${#D2dt2Schemes[@]} ))

echo "d2dt2Schemes: ${D2dt2Schemes[@]}"
echo "deltaTs: ${DeltaTs[@]}"
echo "Number of cases: $nCases"
echo "Study names: ${StudyNames[@]}"

## Case names
CaseNames=()

for d2dt2Scheme in "${D2dt2Schemes[@]}"; do
    for deltaT in "${DeltaTs[@]}"; do
        CaseNames+=("${d2dt2Scheme}_${deltaT}")
    done
done

echo Case names: "${CaseNames[@]}"
