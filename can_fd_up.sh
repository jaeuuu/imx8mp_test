#!/bin/sh

ip link set $1 down

sleep 1

ip link set $1 up type can bitrate $2 dbitrate $3 fd on

sleep 1