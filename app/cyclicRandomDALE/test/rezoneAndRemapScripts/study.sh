#!/bin/bash

# Create multiple studies, each study has its own Test.sh
# Particularly, this is to run multiple studies with same
# time steps, but different initial seeds.

# We keep a copy of necessary scripts for each study, so that they can be run
# standalone if copied or moved, without link breakage. In other words,
# studies created from study_template are symlink-proof.

# Usage: ./study.sh <study_template_dir> [mode]   (mode defaults to test_table.dat)

if [ -f test_table.dat ]; then
    rm -f test_table.dat
fi

if [ -f test.conf ]; then
    ./expandTable.sh "test.conf" > "test.dat"
else
    echo Error
    echo test.conf not found.
    exit 1
fi

if ! cp "test.dat" test_table.dat; then
    echo 
    echo Error:
    #echo Cannot find test_table_READ_ONLY.dat
    echo Cannot find test.dat
    echo
    exit 1
fi

source test_table.dat

# Rezone mode: the 2nd argument, else 'mode' from test_table.dat
mode=${2:-$mode}

# Number of studies
N=${#studyNames[@]}

# Parse input
study_tmp=$(readlink -f $1)

# Create studies
for((i=0; i<N; i++))
do
    echo

    study="${studyNames[i]}"

    if [ -d $study ]; then
	echo Remove $study
        rm -rf $study
    fi

    tmp=$(readlink -f $study_tmp)
    cp -r $tmp $study
    echo $study created

    #- ReadMe; Copy it anyway, and add descriptions from test_table.dat
    ReadMe=$(readlink -f "$study_tmp/ReadMe.md")
    # Remove the copied symlink first, otherwise cp fails ("same file") and
    # the echo's below append to the shared ReadMe.md through the link
    rm -f $study/ReadMe.md
    cp $ReadMe $study/ReadMe.md
    
    echo >> $study/ReadMe.md
    echo "Description:" >> $study/ReadMe.md
    echo $description >> $study/ReadMe.md
    #fi

    #
    # Link-proof copying scripts
    #

    #- Test script
    TestScript=$(readlink -f "$study_tmp/Test.sh")
    caseNameScript=$(readlink -f "$study_tmp/caseName.sh")

    #exit 1;

    if [ -L "$study/Test.sh" ]; then
        rm "$study/Test.sh"
        cp $TestScript $study
	echo Test.sh copied to $study
    fi

    if [ -L "$study/caseName.sh" ]; then
        rm "$study/caseName.sh"
        cp $caseNameScript $study
	echo caseName.sh copied to $study
    fi

    runDALEsinOnceScript=$(readlink -f "$study_tmp/runDALEsinOnce.sh")

    if [ -L "$study/runDALEsinOnce.sh" ]; then
        rm "$study/runDALEsinOnce.sh"
        cp $runDALEsinOnceScript $study
	echo runDALEsinOnce.sh copied to $study
    fi

    #- plot scripts
    plotScript=$(readlink -f "$study_tmp/plot.sh")

    if [ -L "$study/plot.sh" ]; then
        rm "$study/plot.sh"
        cp $plotScript $study
	echo plot.sh copied to $study
    fi

    gpltScript=$(readlink -f "$study_tmp/gplt.sh")

    if [ -L "$study/gplt.sh" ]; then
        rm "$study/gplt.sh"
        cp $gpltScript $study
	echo gplt.sh copied to $study
    fi

    #- Batch run script
    runScript=$(readlink -f "$study_tmp/run.slurm")

    if [ -L "$study/run.slurm" ]; then
        rm "$study/run.slurm"
        cp $runScript $study
	echo run.slurm copied to $study
    fi

    #- Test table
    #testTable=$(readlink -f "$study_tmp/test_table.dat")
    testTable=$(readlink -f "test_table.dat")

    #if [ -L "$study/test_table.dat" ]; then
    if [ -L "test_table.dat" ]; then
        rm "test_table.dat"
    fi
    cp $testTable $study
    echo test_table copied to $study
                                                       
    #- template
    template=$(readlink -f "$study_tmp/template")
    if [ -L $study/template ]; then
        rm $study/template
        cp -r $template $study/template
	echo template/ copied to $study
    fi

    #- study plot
    splot=$(readlink -f "$study_tmp/study.gplt.sh")
    if [ -L "study.gplt.sh" ]; then
        rm "study.gplt.sh"
    fi
    cp -r $splot .
    echo study.gplt.sh copied to .

    # Set up the study template
    #sed -i "s/END_TIME/${studyEndTimes[i]}/" "$study/template/system/controlDict"
    #sed -i "s/DELTA_T/${studyTimeSteps[i]}/" "$study/template/system/controlDict"
    #sed -i "s/WRITE_INTERVAL/${studyWriteIntervals[i]}/" "$study/template/system/controlDict"

    # Cyclic: time controls are set per case by Test.sh from M and nRemap
    #if [ "$cyclic" != on ]; then
    #   # Skipped when given per case; Test.sh sets those
    #   if [ ${#caseEndTimes[@]} -eq 0 ]; then
    #       sed -i "s/END_TIME/${studyEndTimes[i]}/" "$study/template/system/controlDict"
    #   fi
    #   if [ ${#caseTimeSteps[@]} -eq 0 ]; then
    #       sed -i "s/DELTA_T/${studyTimeSteps[i]}/" "$study/template/system/controlDict"
    #   fi
    #   if [ ${#caseWriteIntervals[@]} -eq 0 ]; then
    #       sed -i "s/WRITE_INTERVAL/${studyWriteIntervals[i]}/" "$study/template/system/controlDict"
    #   fi
    #   sed -i "s/NREMAP/${studyNRemaps[i]}/g" "$study/template/constant/ALEDict"
    #fi

    # Submit the study
    sed -i "s/MODE/$mode/g" $study/run.slurm
    cd $study;
    #sbatch run.slurm
    ./Test.sh $mode
    cd ..;

    echo
    echo "Study $study created and submitted"
    echo
done
