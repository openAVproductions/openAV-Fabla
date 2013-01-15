#! /bin/sh

export LD_PRELOAD=/home/jack_interposer/jack_interposer.so

echo "r \"http://www.openavproductions.com/fabla\"" | gdb jalv.gtk


