WSPRBerry

Use the radioberry as a standalone multichannel WSPR receicer.

There are ideas to increase the number of channels...

The WSPRBerry implements the following:

-) recording max 3 WPSR channels target 10CL25 and 2 WSPR channels target 10CL016.
-) using FPGA firmware, implements a DDC to produce 375 samples per second
-) writing the samples per band to a .c2 files to process by the WSPR decoder
-) upload the decoded to wsprnet

The advantage of the .c2 file format is that it’s about 1/8 the size of a .wav file. 

Using 4 channels and 375 samples per sec results in a bit stream of:

4 channels * 375 samples/sec * 6 bytes (IQ-data)  * 8 = 72000 bits/sec.

Using a SPI speed of 244 Khz to get the data from the FPGA firmware to the raspberry pi is sufficient.


Important is that the timing is in sync, you can check your rpi timing at the following site:  https://time.is/

Decode.sh
Modify the CALL and the GRID locator.

Modify the crontab:

sudo crontab -e
1-59/2 * * * * /home/pi/radio/wsprberry/decode.sh

This will start at odd minutes the decode process.

After the updated cron tab the cron service must be restarted:

sudo service cron restart

Load the firmware and start the wsprberry program.

Look at the wsprnet.org for logged stations.



