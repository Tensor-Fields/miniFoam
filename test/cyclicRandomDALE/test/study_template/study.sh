#!/bin/bash

# Create multiple studies, each study has its own Test.sh
# Particularly, this is to run multiple studies with same
# time steps, but different initial seeds.

# We keep a copy of necessary scripts for each study, so that they can be run
# standalone if copied or moved, without link breakage. In other words,
# studies created from study_template are symlink-proof.

declare -a studyTimeSteps=(1  1   1    1    1    1    1    1    1    1    1    1    1)
declare -a studyEndTimes=(11  17  97   199  359  563  1009 1151 1249 1399 1499 1601 1709)
declare -a studyWriteIntervals=(11  17  97   199  359  563  1009 1151 1249 1399 1499 1601 1709)
declare -a studyNames=("${studyEndTimes[@]}") # Right
#declare -a studyNames=("1000_1" "1000_2" "1000_3")

# Number of studies
N=${#studyNames[@]}

# Create studies
for((i=0; i<N; i++))
do
    echo

    study=${studyNames[i]}

    if [ -d $study ]; then
	echo Remove $study
        rm -r $study
    fi

    cp -r study_template $study
    echo $study created

    #mkdir $study 

    #cp -r $template $study

    #
    # Link-proof copying scripts
    #

    #- Test script
    TestScript=$(readlink -f "$study/Test.sh")

    if [ -L "$study/Test.sh" ]; then
        rm "$study/Test.sh"
        cp $TestScript $study
    fi

    #- plot scripts
    plotScript=$(readlink -f "$study/plot.sh")

    if [ -L "$study/plot.sh" ]; then
        rm "$study/plot.sh"
        cp $plotScript $study
    fi

    gpltScript=$(readlink -f "$study/gplt.sh")

    if [ -L "$study/gplt.sh" ]; then
        rm "$study/gplt.sh"
        cp $gpltScript $study
    fi

    #- Batch run script
    runScript=$(readlink -f "$study/run.slurm")

    if [ -L "$study/run.slurm" ]; then
        rm "$study/run.slurm"
        cp $runScript $study
    fi

    #- Test table
    testTable=$(readlink -f "$study/test_table.dat")

    if [ -L "$study/test_table.dat" ]; then
        rm "$study/test_table.dat"
        cp $testTable $study
    fi
                                                       
    #- template
    template=$(readlink -f "$study/template")
    if [ -L $study/template ]; then
        rm $study/template
        cp -r $template $study
    fi

    # Set up the study template
    sed -i "s/END_TIME/${studyEndTimes[i]}/" "$study/template/system/controlDict"
    sed -i "s/DELTA_T/${studyTimeSteps[i]}/" "$study/template/system/controlDict"
    sed -i "s/WRITE_INTERVAL/${studyWriteIntervals[i]}/" "$study/template/system/controlDict"

    # Submit the study
    cd $study;
    sbatch run.slurm
    cd ..;

    echo
    echo "Study $study created and submitted"
    echo
done
