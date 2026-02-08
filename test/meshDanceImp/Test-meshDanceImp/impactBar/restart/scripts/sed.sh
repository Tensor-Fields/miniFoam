#!/bin/bash

##
## After axis entry which is the last patch entry, add the billetRight entry
##

sed -i '/axis/,/}/{/}/a\
    billetSide\
    {\
        type            solidTraction;\
        traction        uniform (0 0 0);\
        pressure        uniform 0;\
        value           uniform (0 0 0);\
    }
        }' $@

##
## Rename old patch names
##

sed -i 's/loading/dieBottom/g' $@

sed -i 's/left/billetTop/g' $@

#sed -i 's/billetRight/billetSide/g' $@
