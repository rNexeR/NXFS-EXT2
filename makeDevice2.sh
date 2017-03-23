#!/bin/bash
rm -rf device fs.ext2
touch fs2.ext2

dd if=/dev/zero of=fs2.ext2 bs=4096 count=51200
mkfs.ext2 fs2.ext2 -b 4096
mkdir device
