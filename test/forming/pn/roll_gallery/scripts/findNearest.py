## \details
## Usage
##     python3 findNearest.py top.ply 100 100 100 > sorted_top.xyz

## \author ChatGPT

#!/usr/bin/env python3

import sys
import numpy as np

if len(sys.argv) != 5:
    print(f"Usage: {sys.argv[0]} points.xyz start_x start_y start_z")
    sys.exit(1)

fname = sys.argv[1]
start = np.array([float(sys.argv[2]),
                  float(sys.argv[3]),
                  float(sys.argv[4])])

pts = np.loadtxt(fname)

current = np.argmin(np.linalg.norm(pts - start, axis=1))

visited = np.zeros(len(pts), dtype=bool)
order = []

for _ in range(len(pts)):
    order.append(current)
    visited[current] = True

    d = np.linalg.norm(pts - pts[current], axis=1)
    d[visited] = np.inf

    if np.all(np.isinf(d)):
        break

    current = np.argmin(d)

ordered_pts = pts[order]

np.savetxt(sys.stdout, ordered_pts, fmt="%.10g")
