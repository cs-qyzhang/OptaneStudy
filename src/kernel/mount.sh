#!/bin/bash
# Mount file systems

if [[ $EUID -ne 0 ]]; then
	echo "Please run as root" 
	exit 1
fi

DRAM_DEV=/dev/pmem0.1
PMEM_DEV=/dev/pmem0

repdev=`mount | grep ReportFS | awk {'print \$1'}`
testdev=`mount | grep LatencyFS | awk {'print \$1'}`


if [ ! -z $repdev ] || [ ! -z $testdev ]; then
	echo "Please run umount.sh first"
	exit 1
fi


echo 0 > /proc/sys/kernel/soft_watchdog
# Hard lock watchdog at nmi_watchdog

sudo mkdir -p /mnt/latency
sudo mkdir -p /mnt/report

REPFS=`lsmod | grep repfs`
LATFS=`lsmod | grep latfs`

if [ ! -z "$REPFS" ]; then
	echo Unmounting existing ReportFS
	sudo ./umount.sh
elif [ ! -z "$LATFS" ]; then
	echo Unmounting existing LatencyFS
	sudo ./umount.sh
fi

sudo insmod repfs.ko
sudo insmod latfs.ko

sudo mount -t ReportFS $DRAM_DEV /mnt/report
sudo mount -t LatencyFS $PMEM_DEV /mnt/latency

sudo wrmsr -a 0x1a4 0xf