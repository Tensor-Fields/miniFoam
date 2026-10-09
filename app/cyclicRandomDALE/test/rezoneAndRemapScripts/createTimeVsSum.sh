#!/bin/bash

# Print a list like
# t1    sum1
# t2    sum2
# t3    sum3
# ...

# Usage
# ./createTimeVsSum.sh <caseName>

# \author: Maalik (ali@tensorfields.com), Richmond Hill, Dublin
# \date: Aug 30, evening

Case=$1

#times=( 0 200 )
times=( 0 40 80 120 160 200 )

rm $Case/time_vs_sum.dat

for i in "${times[@]}"; do
    volIntegrate -time $i -p 20 -case $Case p | grep sum | sed s/sum\ *=/$i\ / >> $Case/time_vs_sum.dat
done
