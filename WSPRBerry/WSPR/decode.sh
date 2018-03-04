#!/bin/bash

echo "WSPR decoding of received data"
cd /home/pi/radio/wsprberry
for file in `ls -a *.c2`
do
 ./wsprd -w -d $file
 rm $file
done

ALLMEPT=ALL_WSPR.TXT
CALL=PA3GSB
GRID=JO22XE

echo "Uploading ..."

# sort by highest SNR, then print unique date/time/band/call combinations,
# and then sort them by date/time/frequency
sort -nr -k 4,4 $ALLMEPT | awk '!seen[$1"_"$2"_"int($6)"_"$7] {print} {++seen[$1"_"$2"_"int($6)"_"$7]}' | sort -n -k 1,1 -k 2,2 -k 6,6 -o $ALLMEPT

curl -sS -m 30 -F allmept=@$ALLMEPT -F call=$CALL -F grid=$GRID http://wsprnet.org/meptspots.php > /dev/null

rm -f $ALLMEPT
