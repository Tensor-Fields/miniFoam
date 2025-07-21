#!/bin/bash

# minifoam library
wmake libso src

echo
echo See above for errors, if any.
echo libs compiled.
echo

# minifoam applications
#ls -1v test | while read -r app; do
#    wmake test/$app
#    done

wmake test/cyclicRandomDALE

echo
echo See above for errors, if any.
echo apps compiled.
echo
echo End.
echo
