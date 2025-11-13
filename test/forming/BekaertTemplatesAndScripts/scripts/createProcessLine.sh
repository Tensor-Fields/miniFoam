#!/bin/bash

## Usage ./createNewStudy.sh <name>

referenceCase=$1
studyName=$2

cp -r $(readlink -f $referenceCase) $studyName

## Init scripts
ln -sf scripts/setKey.sh $studyName

#ln -rsf drawing5 $studyName
