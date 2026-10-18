cyclicRandomDALE            \
                 -M 32 \
                 -alpha 0.25 \
                 -beta  0.5 \
                 -notFixedPatchNames \( "front" \)  \
                 -writeIntermediateMeshes           \
                 -maxIter 1    \
                 -flipFluxSign \
                 -sinCycle     \
                 -cyclic \
                 -fixedPatchNames \( "movingWall" "fixedWalls" \) \
                 -motionDirs '("1" "1" "0")' \
		         -case . \
                 > ./log.cyclicRandomDALE #\
