#!/bin/bash 
for i in `seq 1 9` ; do ifconfig eth0:$i 192.168.6.10$i netmask 255.255.255.0 up; done
for i in `seq 1 9` ; do ifconfig eth0:1$i 192.168.6.11$i netmask 255.255.255.0 up; done
