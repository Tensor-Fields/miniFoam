#!/bin/bash

# minifoam library
wmake libso src

# minifoam applications
ls -1v test | while read -r app; do
    wmake test/$app
    done

echo
echo See above for errors, if any.
echo End.
echo
