##
## \details
##     Read a file, scale it, write the output
##     python scale.py 0.001 input.txt output.txt
##
## \author
##    ChatGPT
##
## \date
##    Jun 06 2026
##

import sys
import re

s = float(sys.argv[1])
inp = sys.argv[2]
out = sys.argv[3]

num = r"[+-]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?"

pattern = re.compile(rf"\(\s*({num})\s+({num})\s+({num})\s*\)")

with open(inp) as f, open(out, "w") as g:
    for line in f:
        m = pattern.search(line)
        if not m:
            continue
        x, y, z = map(float, m.groups())
        g.write(f"({x*s} {y*s} {z*s})\n")
