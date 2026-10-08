#!/bin/bash

# Parse input
list=$1
total=$(wc -l $list | awk -F" " '{print$1}')

echo $1 has $total lines
echo

i=0

if [ -f plot_src/plot.options ]; then
   rm plot_src/plot.options
   echo Removed: plot_src/plot.options
   echo; fi

#
# Overwrite plot options set by stuy.gplt.sh
#
echo "
set xlabel \"\$\\\\sqrt{N_\\\\mathrm{cells}}\$\"
#set xrange [*:*] reverse

# Skip comment lines in L1_err.dat
set datafile commentschar '#'

# Key
<<<<<<< HEAD
#set key left box opaque outside width -25
set key left box opaque outside
=======
set key left box opaque outside width -5
#set key left box opaque outside
>>>>>>> 8fcf7739a206e0ffcc8928a3489d888584816a84
" >> plot_src/plot.options

#
# Generate the plot command
#
echo 'plot \' >> plot_src/plot.options

cat $list | grep -v "^\ *#"| \
while read -r dir key col3; do \
    ((i++))
    ## For the last line, ignore printing ,\
    #if ((i==$total)); then
    #    echo \
    #    \"$dir/plot_src/L1_err.dat\" \
    #    u \(\$1\):\(stringcolumn\(3\) eq \"$divScheme\"?\$2:1/0\) \
    #    w lp dt $(echo $RANDOM) pt $(echo $RANDOM) t \"\\\\tiny{\'$(echo $dir | sed s/'_'/' '/g )\', $divScheme}\" \
    #    >> plot_src/plot.options;

    #    echo Plot options $i printed to plot_src/plot.options
    ## For all other lines, print ,\ at the end
    #else

    # Plot column x=4 vs y=2 of L1_err, where its column 3 is the screening keyword
<<<<<<< HEAD
        echo \
        \"$dir/plot_src/L1_err.dat\" \
	u \(\$1\):\(\$2\) \
	w p dt $(echo $RANDOM) pt $(echo $RANDOM) ps 1.5 \
	t \"\", \\\
        >> plot_src/plot.options;

=======
    echo \
    \"$dir/plot_src/L1_err.dat\" \
	u \(\$1\):\(strstrt\(stringcolumn\(3\), \"$key\"\) \> 0 ?\$2:1/0\) \
	w p dt $(echo $RANDOM) pt $(echo $RANDOM) ps 1.5 \
	t \"\\\\tiny{$col3}\", \\\
    >> plot_src/plot.options;

	#u \(\$1\):\(\$2\) \
>>>>>>> 8fcf7739a206e0ffcc8928a3489d888584816a84
	#u \(\$4\):\(\$2\) \
	#u \(\$1\):\(strstrt\(stringcolumn\(3\), \"$key\"\) \> 0 ?\$2:1/0\) \
	#Error: u \(\$4\):\(stringcolumn\(3\) =~ \/$key\/ ?\$2:1/0\) \
	#Error: u \(\$4\):\(substr\(stringcolumn\(3\),\"_\"\)\[1\] eq \"$key\"?\$2:1/0\) \
        #u \(\$4\):\(stringcolumn\(3\) eq \"$key\"?\$2:1/0\) \

	#t \"\", \\\
	#t \"\\\\tiny{$col3}\", \\\
	#t \"\\\\tiny{\\\$N_{\\\\mathrm{iter}}\\\$=$col3}\", \\\
	#t \"\\\\tiny{$(echo $dir | sed s/'_'/' '/g), \\\\ttfamily{$divScheme}}\", \\\
	#t \"\\\\tiny{$dir, \\\\ttfamily{$divScheme}}\", \\\ Error: `_' gives subscript
	#t \"\\\\tiny{\'$(echo $dir | sed s/'_'/' '/g)\', \\\\ttfamily{$divScheme}}\", \\\
	#t \"\\\\tiny{\'$(echo $dir | sed s/'_'/' '/g)\', $divScheme}\", \\\
	#w lp dt $(echo $RANDOM) pt $(echo $RANDOM) t \"\\\\tiny{$dir, $divScheme}\", \\\

	echo Plot options $i printed to plot_src/plot.options
    #fi
done

#echo \
#    '"-" u ($1):3 w l lc 1 t "\\tiny{2^\\mathrm{nd} \\mathrm{Order}}", \
#     "-" u ($1):2 w l lc 2 dt 2 t "\\tiny{1^\\mathrm{st} \\mathrm{Order}}", \
#     #
#    0.01 0.0001	        0.0001
#    0.08 0.0000125	0.00000625
#    0.64 0.0000015625	0.000000390625
#    5.12 0.0000001953125	0.0000000244140625
#    e
#    #
#    0.01 0.0001	        0.0001
#    0.08 0.0000125	0.00000625
#    0.64 0.0000015625	0.000000390625
#    5.12 0.0000001953125	0.0000000244140625
#    e' \
#    >> plot_src/plot.options

#echo \
#    '"-" u ($1):3 w l lc 1 t "\\tiny{2^\\mathrm{nd} \\mathrm{Order}}", \
#     "-" u ($1):2 w l lc 2 dt 2 t "\\tiny{1^\\mathrm{st} \\mathrm{Order}}", \
#    #
#    1	1.00E-04	1.00E-04
#    2	0.00005	0.000025
#    4	0.000025	0.00000625
#    8	0.0000125	0.0000015625
#    e
#    #
#    1	1.00E-04	1.00E-04
#    2	0.00005	0.000025
#    4	0.000025	0.00000625
#    8	0.0000125	0.0000015625
#    e' \
#    >> plot_src/plot.options
 

echo \
    '"-"                  u ($1):2 w l lc 1 dt 2 t "\\tiny{1^\\mathrm{st} \\mathrm{Order}}", \
     "-"                  u ($1):3 w l lc 2      t "\\tiny{2^\\mathrm{nd} \\mathrm{Order}}", \
     "-"                  u ($1):4 w l lc 3 dt 3 t "\\tiny{3^\\mathrm{rd} \\mathrm{Order}}", \
#
# Table 3, quadratic unlimited , Lipnikov and Shashkov, 2023
#
#"-"                  u ($1):2 w p      pt 4 t "\\tiny{Swept-face remap}", \
#15.4919    5.128e-4
#30.9839    6.759e-5
#61.9677    8.555e-6
#123.935    1.073e-6
#     e
<<<<<<< HEAD
     # 3rd order
=======
# 3rd order
>>>>>>> 8fcf7739a206e0ffcc8928a3489d888584816a84
#32	1.00E-04	1.00E-04	1.00E-05
#64	0.00005	0.000025	0.00000125
#128	0.000025	0.00000625	0.00000015625
#256	0.0000125	0.0000015625	0.00000001953125
#512	0.00000625	0.000000390625	2.44140625E-09
<<<<<<< HEAD
#	1st	2nd	3rd
16	1.00E-05	1.00E-05	1.00E-06
32	0.000005	0.0000025	0.000000125
64	0.0000025	0.000000625	0.000000015625
128	0.00000125	0.00000015625	0.000000001953125
     e
     # 2nd order
#32	1.00E-04	1.00E-04	1.00E-05
#64	0.00005	0.000025	0.00000125
#128	0.000025	0.00000625	0.00000015625
#256	0.0000125	0.0000015625	0.00000001953125
#512	0.00000625	0.000000390625	2.44140625E-09
#	1st	2nd	3rd
16	1.00E-05	1.00E-05	1.00E-06
32	0.000005	0.0000025	0.000000125
64	0.0000025	0.000000625	0.000000015625
128	0.00000125	0.00000015625	0.000000001953125
     e
     # 1st order
#32	1.00E-04	1.00E-04	1.00E-05
#64	0.00005	0.000025	0.00000125
#128	0.000025	0.00000625	0.00000015625
#256	0.0000125	0.0000015625	0.00000001953125
#512	0.00000625	0.000000390625	2.44140625E-09
#	1st	2nd	3rd
16	1.00E-05	1.00E-05	1.00E-06
32	0.000005	0.0000025	0.000000125
64	0.0000025	0.000000625	0.000000015625
128	0.00000125	0.00000015625	0.000000001953125
e' \
=======
	# 1	2	3
15	4.00E-04	4.00E-04	4.00E-04
30	0.0002	0.0001	0.00005
60	0.0001	0.000025	0.00000625
120	0.00005	0.00000625	0.00000078125
e			
	# 1	2	3
15	4.00E-04	4.00E-04	4.00E-04
30	0.0002	0.0001	0.00005
60	0.0001	0.000025	0.00000625
120	0.00005	0.00000625	0.00000078125
e			
	# 1	2	3
15	4.00E-04	4.00E-04	4.00E-04
30	0.0002	0.0001	0.00005
60	0.0001	0.000025	0.00000625
120	0.00005	0.00000625	0.00000078125
e			
' \
>>>>>>> 8fcf7739a206e0ffcc8928a3489d888584816a84
     >> plot_src/plot.options

#echo \
#    '"-"                  u ($1):3 w l lc 1 t "\\tiny{2^{nd} \\mathrm{Order}}", \
#     "-"                  u ($1):3 w l lc 2 dt 2 t "\\tiny{1^{st} \\mathrm{Order}}", \
#     # 2nd order
#     25            0.04    0.000229544
#     50            0.02    0.000057386
#     100           0.01    0.0000143465
#     200           0.005   0.000003586625
#     800           0.00125 0.000000224
#     e
#     # 1st order
#     25            0.04    0.000229544 
#     50            0.02    0.000114772 
#     100           0.01    0.000057386
#     200           0.005   0.000028693
#     800           0.00125 0.000007173
#     e' \
#     >> plot_src/plot.options

#
# First and second order error slopes for sinusoidal case
#

#echo \
#    '"-" u ($1):3 w l lc 1 t "\\tiny{2^\\mathrm{nd} \\mathrm{Order}}", \
#     "-" u ($1):2 w l lc 2 dt 2 t "\\tiny{1^\\mathrm{st} \\mathrm{Order}}", \
#     #
#     32	1.00E-02	1.00E-02
#     64	0.005	0.0025
#     128	0.0025	0.000625
#     256	0.00125	0.00015625
#     512	0.000625	0.0000390625
#     e
#     #
#     32	1.00E-02	1.00E-02
#     64	0.005	0.0025
#     128	0.0025	0.000625
#     256	0.00125	0.00015625
#     512	0.000625	0.0000390625
#     e' \
#    >> plot_src/plot.options

#echo \
#    '"-" u ($1):3 w l lc 1 t "\\tiny{2^\\mathrm{nd} \\mathrm{Order}}", \
#     "-" u ($1):2 w l lc 2 dt 2 t "\\tiny{1^\\mathrm{st} \\mathrm{Order}}", \
#     #
#     32	1.00E-06	1.00E-06
#     64	0.0000005	0.00000025
#     128	0.00000025	0.0000000625
#     256	0.000000125	0.000000015625
#     512	0.0000000625	0.00000000390625
#     e
#     #
#     32	1.00E-06	1.00E-06
#     64	0.0000005	0.00000025
#     128	0.00000025	0.0000000625
#     256	0.000000125	0.000000015625
#     512	0.0000000625	0.00000000390625
#     e' \
#    >> plot_src/plot.options


echo Written: plot_src/plot.options
echo End.
