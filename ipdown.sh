#!/bin/bash 
for i in `seq 1 9` ; do ifconfig eth0:$i down; done
for i in `seq 1 9` ; do ifconfig eth0:1$i down; done
