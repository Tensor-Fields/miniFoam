cyclicRandomDALE            \
                 ${M[i]:+-M ${M[i]}} \
                 ${alpha[i]:+-alpha ${alpha[i]}} \
                 -beta  0.5 \
                 -notFixedPatchNames \( "front" \)  \
                 -writeIntermediateMeshes           \
                 -maxIter 1    \
                 -flipFluxSign \
                 -sinCycle     \
                 ${cyclicFlag} \
                 -fixedPatchNames \( "movingWall" "fixedWalls" \) \
                 -motionDirs '("1" "1" "0")' \
		 -case $Case \
                 > $Case/log.cyclicRandomDALE #\

                 #-case $Case \

