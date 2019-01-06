#!/bin/sh
fusermount -u mnt
make
./objfs mnt -o use_ino             
