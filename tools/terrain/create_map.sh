#!/bin/bash

set -e

# options
size=16

#x_min=2060
#y_min=1562
#zoom=12

x_min=4122
y_min=3127
zoom=13


x_max=$(expr $x_min + $size)
y_max=$(expr $y_min + $size)

DIR=normal

mkdir -p $DIR


# texture
#API="https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile"

# heightmap
#API="https://tile.nextzen.org/tilezen/terrain/v1/256/terrarium"

#normalmap
API="https://tile.nextzen.org/tilezen/terrain/v1/256/normal"

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

