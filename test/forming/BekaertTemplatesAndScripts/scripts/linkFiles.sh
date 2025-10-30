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
    ln -rsf $scripts/templateCase $target
    ln -rsf $scripts/scripts/createProcessLine.sh $target
    ln -rsf $scripts/scripts/createStudy.sh $target
    ln -rsf $scripts/scripts/ $target
    ln -rsf $templateProcessLine $target
elif $interactive; then
    ln -rsi $scripts/templateCase $target
    ln -rsi $scripts/scripts/createProcessLine.sh $target
    ln -rsi $scripts/scripts/createStudy.sh $target
    ln -rsi $scripts/scripts/ $target
    ln -rsi $templateProcessLine $target
else
    ln -rs $scripts/templateCase $target
    ln -rs $scripts/scripts/createProcessLine.sh $target
    ln -rs $scripts/scripts/createStudy.sh $target
    ln -rs $scripts/scripts/ $target
    ln -rs $templateProcessLine $target
fi
