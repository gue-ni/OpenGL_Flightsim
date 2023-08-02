#!/bin/bash

x_root=536
y_root=356
zoom=10

#x_root=1072
#y_root=712
#zoom=11


size=$(python -c "print(2**($zoom - 9))")

N=$(expr $size - 1)

echo "size=${size}, N=${N}"


for x in $(seq 0 $N); do 
  for y in $(seq 0 $N); do 

    dx=$(expr $x \* 4)
    dy=$(expr $y \* 4)

    x_tile=$(expr $x_root + $dx)
    y_tile=$(expr $y_root + $dy)

    echo "${x_tile} ${y_tile}"

    ./create_map.sh $x_tile $y_tile $zoom texture
    ./create_map.sh $x_tile $y_tile $zoom height
    ./create_map.sh $x_tile $y_tile $zoom normal
  done
done



