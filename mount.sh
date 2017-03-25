#!/bin/bash

sudo chmod 777 -R $1
sudo chmod 777 -R $2
bin/fuse_x $1 $2 -f -s
