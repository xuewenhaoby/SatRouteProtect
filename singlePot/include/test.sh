#!/bin/bash

for((i=1;i<=16;i++));
do
gnome-terminal -x bash -c "sudo ip netns exec sat$i ./socket $i;exec bash"
done
