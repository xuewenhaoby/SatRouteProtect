#!/bin/bash

gnome-terminal -x bash -c "sudo ip netns exec attack$1 ./attackSocket $1 $2;exec bash"

