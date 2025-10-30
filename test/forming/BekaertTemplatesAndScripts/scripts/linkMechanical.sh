#!/bin/bash

## Absolute link of modifier/ to the correct mechanicalProperties
## Usage: linkMechanical.sh <originalMecanicalProp>

find . -name mechanicalProp\* | \
    grep modifiers | \
        while read -r target; do
	    ln -sf $(readlink -f "$1") $target;
		echo $target overwritten by $(readlink -f "$1")
	done
