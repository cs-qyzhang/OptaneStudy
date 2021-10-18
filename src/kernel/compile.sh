#!/bin/bash
sudo ./umount.sh
make clean && make
sudo ./mount.sh