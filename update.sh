#!/bin/bash

sudo losetup /dev/loop0 floppy.img
sudo mount /dev/loop0 /mnt/dmaos_floppy
sudo cp kernel /mnt/dmaos_floppy/kernel
sleep 3
sudo umount /mnt/dmaos_floppy
sudo losetup -d /dev/loop0
