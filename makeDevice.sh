#!/bin/bash
rm -rf device fs.ext2
touch fs.ext2
dd if=/dev/zero of=fs.ext2 bs=1024 count=102400
mkfs.ext2 fs.ext2
mkdir device