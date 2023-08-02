#!/bin/bash

set -e

# options
size=3 # is actually size + 1

x_min=$1
y_min=$2
zoom=$3
tile_type=$4

[ -z "$tile_type" ] && tile_type="texture"

if [ "$#" -eq 3 ] || [ "$#" -eq 4 ]; then
  echo "x_min=${x_min}, y_min=${y_min}, zoom=${zoom}, dir=${tile_type}"
else
  echo "Error: invalid number of arguments: ${#}"
  exit 1
fi

x_max=$(expr $x_min + $size)
y_max=$(expr $y_min + $size)

if [ $tile_type == "height" ] 
then
  API="https://tile.nextzen.org/tilezen/terrain/v1/256/terrarium"
elif [ $tile_type == "normal" ] 
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

    tile="${tile_type}/tile_${zoom}_${x}_${y}.${FILETYPE}"

    if [ ! -f $tile ] 
    then

      if [ $tile_type == "texture" ]
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
  
  convert $hor +append "${tile_type}/hor-${y}.${FILETYPE}"

  vert="${vert} ${tile_type}/hor-${y}.${FILETYPE}"
done


#outfile="merged/merged_${x_min}_${y_min}_s${size}_z${zoom}_${tile_type}.${FILETYPE}"

outdir="combined/${zoom}/${x_min}/${y_min}"
mkdir -p $outdir

outfile="${outdir}/${tile_type}.${FILETYPE}"

convert $vert -append $outfile

echo "write to $outfile"

rm -f $vert 

