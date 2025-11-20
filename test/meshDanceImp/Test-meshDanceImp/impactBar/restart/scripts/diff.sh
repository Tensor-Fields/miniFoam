#!/bin/bash

dir1=$1
dir2=$2

diff -ywEZBbr --suppress-common-lines $dir1 $dir2
