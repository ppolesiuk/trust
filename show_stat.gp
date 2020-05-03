# This script automaticly redraw data every 5 seconds
# it can be run with the following command
#
# $ gnuplot -e "stat_file='<YOUR_STAT_FILE'>" show_stat.gp
#
# If stat_file option is not passed, the default path is stat.dat
#
if (!exists("stat_file")) stat_file='stat.dat'
plot stat_file using 1:2 with lines
pause 5
reread
