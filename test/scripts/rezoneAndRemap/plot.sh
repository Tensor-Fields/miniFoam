#!/bin/bash

# I use here-document (EOF) rather than gnuplot -e "..." which requires escaping
# special characters, and therefore extra hassle in writing and reading, and
# also, gnuplot -e '...' which gets complex if you have single quote also inside
# the body of gnuplot.
#
# Important: EOF needs escaping bash '$', so does gnuplot -e '...'
# Important: 'EOF' does not need escaping bash '$'

source test_table.dat

# Create table of data
if [ -d plot_src ]; then
    rm -r plot_src
fi

mkdir plot_src

echo "# sqrt(nX * nY)    L1_err"
echo "# sqrt(nX * nY)    L1_err" > "plot_src/L1_err.dat"

# Get the number of tests
s=${#M[@]}

echo $s tests found ...

# Scan the log files
for((i=0; i<s; i++))
#for((i=0; i<1; i++))
do
    #Case=${nX[i]}x${nY[i]}_${M[i]}
    source caseName.sh
    #echo ${Case[i]}
    #echo ${studyEndTimes[i]}
    #echo "${nX[i]}x${nY[i]}"_"${caseNameSuffix[i]}"_"${studyEndTimes[i]}"

        #awk -F" " -v nX=${nX[i]} -v nY=${nY[i]} -v Case=${caseNameSuffix[i]}\

    row=$(grep "L1 error" "$Case/log.meshToMeshError"| \
        awk -F" " \
	-v nX=${nX[i]} -v nY=${nY[i]} -v testCase=$Case \
	-v et=${studyEndTimes[i]} \
	-v ts=${studyTimeSteps[i]} \
	-v alpha=${alpha[i]} \
        '{print sqrt(nX*nY)" "$NF" "testCase" "alpha}')

    echo $row
    echo $row >> "plot_src/L1_err.dat"
done

. gplt.sh
