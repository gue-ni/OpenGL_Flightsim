#!/bin/bash

# Upper Left  (  108875.000,  586555.000) (  9d20'37.63"E, 49d 6'39.62"N)
# Lower Left  (  108875.000,  268625.000) (  9d33'19.58"E, 46d15'14.74"N)
# Upper Right (  689485.000,  586555.000) ( 17d18' 1.60"E, 49d 6'42.33"N)
# Lower Right (  689485.000,  268625.000) ( 17d 5'23.92"E, 46d15'17.32"N)
# Center      (  399180.000,  427590.000) ( 13d19'20.62"E, 47d44'53.74"N)


d=1024 # output image size

size=50000
shift_x=10000
shift_y=190000

# upper left corner
ox=108875
oy=586555

ulx=$(expr $ox + $shift_x)
uly=$(expr $oy - $shift_y)
lrx=$(expr $ulx + $size)
lry=$(expr $uly - $size)

echo $ulx $uly $lrx $lry

heightmap="hm-${ulx}-${uly}-${lrx}-${lry}-${d}x${d}.png"

format=Uint16

gdal_translate \
  -norat \
  -of PNG \
  -ot $format \
  -projwin $ulx $uly $lrx $lry \
  -scale 111.984 3795.406 1 65535 \
  -outsize $d $d \
  data/dhm_at_lamb_10m_2018.tif out/$heightmap
