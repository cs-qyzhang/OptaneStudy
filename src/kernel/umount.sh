#!/bin/bash
if [[ $EUID -ne 0 ]]; then
	echo "Please run as root" 
	exit 1
fi

# enable hardware prefetch
sudo wrmsr -a 0x1a4 0x0

sudo umount /mnt/latency
sudo umount /mnt/report

sudo rmmod latfs
sudo rmmod repfs

echo 1 > /proc/sys/kernel/soft_watchdog
