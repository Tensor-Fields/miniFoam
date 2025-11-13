#!/bin/bash

## Links files and directories necessary to run a processline.
## 
## \author: Maalik, BlueRoom, UCD, Dublin, IE, Cool, Sunny, Fruitful day!
## \date: 22 Oct 2025

## Target
target=$1
#processName=$2
templateProcessLine=$2
force=false
interactive=false

if [[ " $@ " =~ " -f " ]]; then
    force=true
fi

if [[ " $@ " =~ " -i " ]]; then
    interactive=true
fi
## alias
scripts=/home/maalik/foam/maalik-4.0/run/miniFoam/test/forming/BekaertTemplatesAndScripts
#templateProcessLine=/home/maalik/foam/maalik-4.0/run/miniFoam/test/forming/$processName/templateProcessLine

## Actually link
if $force; then
    ln -sf $scripts/templateCase $target
    ln -sf $scripts/scripts/createProcessLine.sh $target
    ln -sf $scripts/scripts/createStudy.sh $target
    ln -sf $scripts/scripts/ $target
    ln -sf $templateProcessLine $target
elif $interactive; then
    ln -si $scripts/templateCase $target
    ln -si $scripts/scripts/createProcessLine.sh $target
    ln -si $scripts/scripts/createStudy.sh $target
    ln -si $scripts/scripts/ $target
    ln -si $templateProcessLine $target
else
    ln -s $scripts/templateCase $target
    ln -s $scripts/scripts/createProcessLine.sh $target
    ln -s $scripts/scripts/createStudy.sh $target
    ln -s $scripts/scripts/ $target
    ln -s $templateProcessLine $target
fi
