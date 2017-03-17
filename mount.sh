#!/bin/bash

sudo chmod 777 $1
bin/nxt2fs $1 $2 -f -s
