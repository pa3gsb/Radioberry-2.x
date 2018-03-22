#!/bin/bash
echo "===================================================================="
echo "===================================================================="
echo " Radioberry V2.0 beta 2 will be lauched using pihpsdr."
echo ""
echo ""
echo " 			Have fune Johan PA3GSB"
echo ""
echo ""
echo "===================================================================="
echo "===================================================================="
sleep 2
echo ""
echo ""
echo "Radioberry clock will be set. "
echo ""
echo ""
sudo /home/pi/radioberry/rbclock/rbclock
echo ""
echo "" 
echo "Radioberry firmware will be loaded."
echo ""
echo ""
sleep 2
cd /home/pi/radioberry/firmwareloader
sudo ./loadFPGA CL016
echo ""
echo ""
echo "pihpsdr will be started."
echo ""
sleep 2 
cd /home/pi/radioberry/pihpsdr
sudo ./pihpsdr
