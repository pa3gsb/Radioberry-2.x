#!/bin/bash


JOBS=1
NICE=10

RECORDER=./recordings

DECODER=./ft8d

date
TIMESTAMP=`date --utc +'%y%m%d_%H%M'`

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