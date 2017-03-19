#!/bin/bash
rm -rf device fs.ext2
touch fs.ext2
dd if=/dev/zero of=fs.ext2 bs=1024 count=204800
mkfs.ext2 fs.ext2 -b 4096
mkdir device