#!/bin/sh

#
# logging of ws2000 values
# to RRD and logfile
#
WSLOGGER="/usr/local/bin/wstst"
RRDTOOL="/usr/bin/rrdtool"
SENSORS="0 1 17 18 19 20 21"
TMPLOG="/tmp/wslog.$$"
WSLOG="/proj/wetter/logs/wslog"
RRD="/proj/wetter/logs/sens"

# reading data of ws2000
$WSLOGGER 12 >$TMPLOG

# append data to logfile
cat $TMPLOG >>$WSLOG

#
# loop over sensors
#
for s in $SENSORS 
do
  DATA="`cat $TMPLOG |\
    gawk '/^'"$s"'\t/ { 
                       if ($2 == "temp" && $6 == 1) 
                          printf("%d:%.1f ", $5, -$7)
                       else 
                          printf("%d:%.1f ", $5, $7)
                     }'`"
$RRDTOOL update $RRD$s.rrd $DATA
done

#
# cleanup
#
if [ -w $TMPLOG ]; then 
  rm $TMPLOG
fi


