#!/bin/bash

set -e

# options
size=3 # is actually size + 1

#x_min=2060
#y_min=1562
#zoom=12

#x_min=4122
#y_min=3127
#zoom=13

# lukla
#x_min=6069
#y_min=3439
#zoom=13

# mt everest
#x_min=6074
#y_min=3432
#zoom=13

# vorarlberg
# all upper left corners
#x_min=268
#y_min=178
#zoom=9

#x_min=536
#y_min=356
#zoom=10

x_min=1072
y_min=712
zoom=11

#x_min=2144
#y_min=1424
#zoom=12

x_max=$(expr $x_min + $size)
y_max=$(expr $y_min + $size)

DIR=height

mkdir -p $DIR

if [ $DIR == "height" ] 
then
  API="https://tile.nextzen.org/tilezen/terrain/v1/256/terrarium"
elif [ $DIR == "normal" ] 
then
  API="https://tile.nextzen.org/tilezen/terrain/v1/256/normal"
else
  API="https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile"
fi


FILETYPE="png"

vert=""


for y in $(seq $y_min $y_max); do

  hor=""

  for x in $(seq $x_min $x_max); do

    tile="${DIR}/tile_${zoom}_${x}_${y}.${FILETYPE}"

    if [ ! -f $tile ] 
    then

      if [ $DIR == "texture" ]
      then
        echo "texture"
        curl --output $tile "${API}/${zoom}/${y}/${x}" 
      else
        echo "not texture"
        curl -H "Origin: https://tangrams.github.io" --output $tile "${API}/${zoom}/${x}/${y}.${FILETYPE}?api_key=dmlO1fVQRPKI-GrVIYJ1YA&" 
      fi

      #sleep 0.1 # common curtesy to the server
    fi

    hor="${hor} ${tile}"
  done
  
  convert $hor +append "${DIR}/hor-${y}.${FILETYPE}"

  vert="${vert} ${DIR}/hor-${y}.${FILETYPE}"
done


outfile="merged/merged_${x_min}_${y_min}_s${size}_z${zoom}_${DIR}.${FILETYPE}"
convert $vert -append $outfile
echo "write to $outfile"

rm -f $vert 

