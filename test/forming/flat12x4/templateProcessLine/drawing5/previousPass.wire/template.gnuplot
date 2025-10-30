# To run this script, set the flags below and execute:
# $> gnuplot plotAll.gnuplot -p

# Flags
WIRE_PROFILE = TO_BE_SET
OUTPUT = TO_BE_SET
Y_SYMM = 0
Z_SYMM = 0
ADD_MINI_MAXI = 0

# Save file as PDF
set term pdfcairo enhanced color; set output "tmp.pdf"

# Use same scale on x and y axes
set size ratio -1
set grid xtics ytics; set xtics 1; set ytics 1

set title "Wire Profiles [in mm]"
unset key

# Save temporary plot
plot \
    WIRE_PROFILE u ($3 < 1e-6 ? NaN : 1e3*$3):(1e3*$2) w l lw 0.5 lc "red" t "Virtual twin"

# Add mini-maxi
if (ADD_MINI_MAXI > 0){
    set key center bmargin
    replot \
        "mini.txt" u 3:2 w l lw 0.5 lc "green" t "Mini", \
        "maxi.txt" u 3:2 w l lw 0.5 lc "blue" t "Maxi"
}

# Save final plot along with any symmetries
set output OUTPUT
if (Y_SYMM == 0 && Z_SYMM == 0) replot
if (Y_SYMM > 0 && Z_SYMM == 0) replot  WIRE_PROFILE u ($3 < 1e-6 ? NaN : 1e3*$3):(-1e3*$2) w l lw 0.5 lc "red" notitle
if (Y_SYMM == 0 && Z_SYMM > 0) replot  WIRE_PROFILE u ($3 < 1e-6 ? NaN : -1e3*$3):(1e3*$2) w l lw 0.5 lc "red" notitle
if (Y_SYMM > 0 && Z_SYMM > 0){
    replot  \
        WIRE_PROFILE u ($3 < 1e-6 ? NaN : 1e3*$3):(-1e3*$2) w l lw 0.5 lc "red" notitle, \
        WIRE_PROFILE u ($3 < 1e-6 ? NaN : -1e3*$3):(1e3*$2) w l lw 0.5 lc "red" notitle
}
