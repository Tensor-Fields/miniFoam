#!/bin/bash

##
## Procedure
##

## - Solve wave Eq. using the given scheme
## 
## - Repeat above for multiple scheme-deltaT combinations,
## e.g., euler-deltaT0, euler-deltaT1,
## integrated-deltaT0, integrated-deltaT1, etc.
## 
## - At each timestep, write the error field compared to the exact solution.
## 
## - Compare how scheme-deltaT combination matches an exact solution.
## 
## - For one (or more) time, draw a plot, showing:
## --- Exact solution
## --- scheme-deltaT combination solution.

## Create multiple studies, each study has its own Test.sh
## Particularly, this is to run multiple studies with same

## We keep a copy of necessary scripts for each study, so that they can be run
## standalone if copied or moved, without link breakage. In other words,
## studies created from study_template are symlink-proof.
#
## Usage: ./study.sh <study_template_dir>

## -------------------------------Code Starts-------------------------------- ##

##
## First Things
##

    ## Parse
    studyTemplate=$1;
    inputConfigFile=$2;

    if [ -z "$studyTemplate" ]; then
        echo ERROR
        echo First arg is the template study dir
        exit 1
    elif [ -z "$inputConfigFile" ]; then
        echo ERROR
        echo Input config file is the second arg
        exit 1
    fi

    ## Initialize
    inputConfig="$inputConfigFile";
    scriptsDir="$1/scripts"
    templateCase="templateCase"

#if [ -f study.dat ]; then
#    rm -f study.dat
#fi
#
#if ! cp $input study.dat; then
#    echo 
#    echo Error:
#    echo Cannot find $2
#    echo
#    exit 1
#fi

if [ -z "$scriptsDir/input_parser.sh" ]; then
    echo ERROR
    echo $scriptsDir/input_parser.sh does not exist.
    exit 1
fi

source $scriptsDir/input_parser.sh $inputConfig

# Number of studies
N=${#StudyNames[@]}

# Parse input
study_tmp=$(readlink -f $studyTemplate)

# Create studies
for((i=0; i<N; i++))
do
    echo

    study="${StudyNames[i]}"

    if [ -d $study ]; then
	echo Remove $study
        rm -r $study
    fi

    cp -r $study_tmp $study
    echo $study created

    #- ReadMe; Copy it anyway, and add descriptions from inputConfigFile
    ReadMe=$(readlink -f "$study_tmp/ReadMe.md")
    #if [ -L ReadMe.md ]; then
    #    rm ReadMe.md
    if [ -z $ReadMe ]; then
        echo ERROR
        echo $study_tmp/ReadMe.md must exist.
        exit 1;
    else
    #    cp $ReadMe $study/ReadMe.md
    #
    #    echo >> $study/ReadMe.md
        echo "Description:" >> $study/ReadMe.md
        echo ${Description[@]} >> $study/ReadMe.md
    fi

    #
    # Link-proof copying scripts
    #

    #- Test script
    TestScript=$(readlink -f "$scriptsDir/Test.sh")
    #caseNameScript=$(readlink -f "$study_tmp/caseName.sh")

    #exit 1;

    #if [ -L "$scriptsDir/Test.sh" ]; then
    #    rm "$scriptsDir/Test.sh"
    #    cp $TestScript $study
    #    echo Test.sh copied to $study
    #fi

    #if [ -L "$study/caseName.sh" ]; then
    #    rm "$study/caseName.sh"
    #    cp $caseNameScript $study
    #    echo caseName.sh copied to $study
    #fi

    #- plot scripts
    plotScript=$(readlink -f "$scriptsDir/plot.sh")

    #if [ -L "$study/plot.sh" ]; then
    #    rm "$study/plot.sh"
    #    cp $plotScript $study
    #    echo plot.sh copied to $study
    #fi

    gpltScript=$(readlink -f "$scriptsDir/gplt.sh")

    #if [ -L "$study/gplt.sh" ]; then
    #    rm "$study/gplt.sh"
    #    cp $gpltScript $study
    #    echo gplt.sh copied to $study
    #fi

    #- Batch run script
    runScript=$(readlink -f "$scriptsDir/run.slurm")

    #if [ -L "$study/run.slurm" ]; then
    #    rm "$study/run.slurm"
    #    cp $runScript $study
    #    echo run.slurm copied to $study
    #fi

    #- Test table
    #testTable=$(readlink -f "$study_tmp/inputConfigFile")
    testTable=$(readlink -f $inputConfig)

    #if [ -L "$study/inputConfigFile" ]; then
    if [ -L "$study/$inputConfig" ]; then
        rm "$study/$inputConfige"
    fi
    cp $testTable $study
    echo input config copied to $study
                                                       
    #- template
    templateCase=$(readlink -f "$study_tmp/templateCase")
    if [ -L $study/templateCase ]; then
        rm $study/templateCase
        cp -r $templateCase $study/templateCase
	echo templateCase/ copied to $study
    fi

    #- study plot
    splot=$(readlink -f "$scriptsDir/study.gplt.sh")
    #if [ -L "study.gplt.sh" ]; then
    #    rm "study.gplt.sh"
    #fi
    #cp -r $splot .
    #echo study.gplt.sh copied to .

    # Set up the study template
    #sed -i "s/END_TIME/${studyEndTimes[i]}/" "$study/template/system/controlDict"
    #sed -i "s/DELTA_T/${studyTimeSteps[i]}/" "$study/template/system/controlDict"
    #sed -i "s/WRITE_INTERVAL/${studyWriteIntervals[i]}/" "$study/template/system/controlDict"

    # Submit the study
    #sed -i "s/MODE/$2/g" $scriptsDir/run.slurm
    #declare -p CaseNames

    cd $study;
        #sbatch run.slurm
        scripts=$(echo $scriptsDir | xargs basename)

        if [ "${Permutational[0]}" = "true" ]; then
            source $scripts/permutationalTest.sh
        else
            source $scripts/Test.sh $templateCase $scripts $inputConfig \
                "${CaseNames[@]}"
        fi

        source "$scriptsDir/plot.sh"

    cd ..;

    echo
    echo "Study $study created and submitted"
    echo
done
