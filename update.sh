#!/bin/bash

sudo /sbin/losetup /dev/loop0 floppy.img
sudo /bin/mount /dev/loop0 /mnt/dmanix_floppy
for f in kernel initrd; do
  sudo cp $f /mnt/dmanix_floppy/$f
done
sleep 2
sudo /bin/umount /mnt/dmanix_floppy
sudo /sbin/losetup -d /dev/loop0
