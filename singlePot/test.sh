#!/bin/bash

for((i=1;i<=32;i++));
do
	sudo ip netns exec sat$i ./satSocket $i $1 $2 >out.file 2>&1 &
	##gnome-terminal -x bash -c "sudo ip netns exec sat$i ./socket $i $1 $2;exec bash"
done
