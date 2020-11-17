#!/bin/bash

JOBS=4
NICE=10

DIR=`readlink -f $0`
DIR=`dirname $DIR`

RECORDER=$DIR/recordings

DECODER=./ft8d

date
TIMESTAMP=`date --utc +'%y%m%d_%H%M'`

echo "Recording ..."

killall -q $RECORDER
$RECORDER $CONFIG

echo "Decoding ..."

for file in `ls -a ./recordings/*.c2`
do
  while [ `pgrep $DECODER | wc -l` -ge $JOBS ]
  do
    sleep 1
  done
  nice -n $NICE $DECODER $file
done > ./decodes/decodes_$TIMESTAMP.txt

wait

rm -r ./recordings/ft8_*.c2