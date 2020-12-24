#!/bin/bash

# CALL and GRID should be specified to enable uploads
CALL=
GRID=


# optional antenna description
ANTENNA=""

UPLOADER=./upload-to-pskreporter

date
TIMESTAMP=`date --utc +'%y%m%d_%H%M'`

echo "Processing ..."

REPORT=./decodes/report_$TIMESTAMP.txt

for file in `find . -name decodes_\*.txt`
do
  cat $file >> $REPORT
  rm -f $file
done

test -f $REPORT || exit

# sort by highest SNR, then print unique band/call combinations,
# and then sort them by date/time/frequency
sort -nr -k 4,4 $REPORT | awk '!seen[int($6/1e6)"_"$7]{print} {++seen[int($6/1e6)"_"$7]}' | sort -n -k 1,1 -k 2,2 -k 6,6 -o $REPORT

test -n "$CALL" -a -n "$GRID" || exit

echo "Uploading ..."

$UPLOADER $CALL $GRID "$ANTENNA" $REPORT

sudo rm -rf ./decodes/*