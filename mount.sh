#!/bin/bash

sudo chmod 777 -R $1
sudo chmod 777 -R $2
bin/nxt2fs $1 $2 -f -s
