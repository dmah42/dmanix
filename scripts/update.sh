#!/bin/bash

FLOPPY=/mnt/dmanix_floppy
LOOP0=/dev/loop0

sudo /sbin/losetup $LOOP0 floppy.img
sudo /bin/mount $LOOP0 $FLOPPY
for f in kernel initrd; do
  sudo cp $f $FLOPPY/$f
done
sleep 2
sudo /bin/umount $FLOPPY
sudo /sbin/losetup -d $LOOP0
