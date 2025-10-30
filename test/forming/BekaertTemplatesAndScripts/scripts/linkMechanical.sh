#!/bin/bash

## Absolute link of modifier/ to the correct mechanicalProperties

find . -name mechanicalProp\* | \
    grep modifiers | \
        while read -r target; do
	    ln -sf $(readlink -f ../../ReplaceFiles/mechanicalProperties) $target;
	done
