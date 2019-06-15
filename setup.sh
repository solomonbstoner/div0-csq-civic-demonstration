#!/bin/bash
# Made by Solomon for Div0 Car Security Quarter
# The purpose of this script is to set up a CAN network as slcan0 for the
# CANable. 
# 
# WARNING: 
# If only 1 USB device is connected, the script will assume that it is
# the CANable device! 

for i in $(ls /sys/class/net 2> /dev/null);
do
	if [ $i == "slcan0" ];
	then
		echo "slcan0 already exists. Quitting..."
		exit 1
	fi
done

if [ $EUID -ne 0 ];
then
	# root rights are required for the CAN network to be created
	echo "This script must be run as root"
	exit 1
fi

no_of_usb_devices=0
canable_usb_device=""

for i in $(ls /dev/ttyACM* 2> /dev/null);
do
	no_of_usb_devices=$((no_of_usb_devices+1));
	echo "Found device: $i"
	canable_usb_device=$i
done

if [ $no_of_usb_devices -gt 1 ];
then
	echo "Error: More than 1 USB devices detected."
	exit 1
	# TODO: Ask for ttyACM number that corresponds to the CANable

elif [ $no_of_usb_devices -eq 0 ];
then
	echo "Error: No USB devices found."
	exit 1
fi

slcand -o -c -s6 $canable_usb_device slcan0
if [ $? -eq 0 ];
then
	echo "Successfully setting up slcan0 CAN network."
else
	echo "slcand: Error setting up slcan0 CAN network."
	exit 1
fi

# Sleep for a while, then activate slcan0
sleep 1

ifconfig slcan0 up
if [ $? -eq 0 ];
then
	echo "Successfully activated slcan0 interface."
else
	echo "ifconfig: Error activating slcan0 interface."
	exit 1
fi

# Sleep for a while, then increase queue length for slcan0
sleep 1

ifconfig slcan0 txqueuelen 1000
if [ $? -eq 0 ];
then
	echo "Successfully increased slcan0 queue length."
else
	echo "ifconfig: Error increasing slcan0 queue length."
	exit 1
fi

echo "Your CAN network has been set up as slcan0. :)"
