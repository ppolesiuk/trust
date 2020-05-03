#!/bin/bash
#
# This script draws given Graphviz scripts to PNG images.
# To run this script, just pass Graphviz files to it, e.g.,
# 
# $ ./draw_automaton.sh a1.gv a2.gv a3.gv
#
for file in $*
do
	echo "$(dirname $file)/$(basename $file .gv)"
	dot -Tpng $file > "$(dirname $file)/$(basename $file .gv).png"
done
