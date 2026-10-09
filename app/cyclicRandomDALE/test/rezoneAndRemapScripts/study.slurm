#!/bin/bash
#SBATCH --job-name=elephant
#SBATCH --output=out
#SBATCH --error=err
#SBATCH --partition=main
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1
#SBATCH --time=03:30:00

# Good practice to navigate to the submission directory
#cd $SLURM_SUBMIT_DIR

# Load OpenFOAM
fe40
#source /usr/lib/openfoam/openfoam2412/etc/bashrc
#source /usr/lib/openfoam/openfoam2312/etc/bashrc
#source /usr/lib/openfoam/openfoam2212/etc/bashrc
#source /usr/lib/openfoam/openfoam2112/etc/bashrc

# Execute the run script
#./Allrun &> log.Allrun

# Random is not cyclic, see Test.sh
# random, once, or cyclic
#./study.sh "study_template" "random"
#./study.gplt.sh
./study.sh "study_template"
