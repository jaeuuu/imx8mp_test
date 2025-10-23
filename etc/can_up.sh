#!/bin/sh

ip link set $1 down

sleep 1

ip link set $1 up type can bitrate $2

sleep 1