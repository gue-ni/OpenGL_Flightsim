import requests
import math
import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--lat", type=float, required=False)
parser.add_argument("--lon", type=float, required=False)
parser.add_argument("--zoom", type=int, required=True)
parser.add_argument("--xtile", type=int, required=False)
parser.add_argument("--ytile", type=int, required=False)
parser.add_argument("--download", required=False, action="store_true")

API_URL = "https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile"

# width of a tile in meters
def tile_width(latitude, zoom):
  # equatorial cirumference of reference geoid used by OpenStreetMap
  C = 40075016.686   
  return abs(C * math.cos(latitude) / (2 ** zoom))

# width of a pixel in meters, default is 256 pixels per tile
def pixel_width(latitude, zoom, pixels = 256.0):
   return tile_width(latitude, zoom) / pixels

# returns lat_lon of upper left corner of tile
def tile_to_lat_lon(x, y, z):
  n = 2 ** z
  lon_deg = x / n * 360.0 - 180.0
  lat_rad = math.atan(math.sinh(math.pi * (1 - 2 * y / n)))
  lat_deg = math.degrees(lat_rad)
  return (lat_deg, lon_deg)

# returns the tile containing the coord
def lat_lon_to_tile(coord, z):
  (lat_deg, lon_deg) = coord
  n = 2 ** z
  lat_rad = math.radians(lat_deg)
  lon_rad = math.radians(lon_deg)

  xtile = n * ((lon_deg + 180.0) / 360.0)
  ytile = n * (1 - (math.log(math.tan(lat_rad) + (1.0 / math.cos(lat_rad))) / math.pi)) / 2
  return (int(xtile), int(ytile), z)

# get coordinates of bounding box
def bounding_box(x,y,z, n_tiles = 1):
  min = tile_to_lat_lon(x + 0, y + 0, z)
  max = tile_to_lat_lon(x + n_tiles, y + n_tiles, z)
  return (min, max)

# download a tile and save to img
def download_tile(x, y, z, url_format = "zyx"):

  if url_format == "zyx":
    param = f"{z}/{y}/{x}"
  else:
    param = f"{z}/{x}/{y}"

  image_path = f"tiles/tile_{z}_{x}_{y}.jpg"

  url = f"{API_URL}/{param}"
  print("url =", url)
  
  if not os.path.isfile(image_path):

    res = requests.get(url, stream=True)

    if not res.ok:
      print("error during download")
      return

    with open(image_path, 'wb') as file:
      for chunk in res:
        file.write(chunk)
      print(f"wrote to {image_path}")

  else:
    print(f"{image_path} already exists")

if __name__ == "__main__":
  bregenz = (47.5008, 9.7423)
  vienna = (48.2082, 16.3738)
  ludesch = (47.1958, 9.7793)
  equator = (0.0, 20.0) 
  majorca = (39.6953, 3.0176)

  args = parser.parse_args()

  zoom = args.zoom
  print("zoom =", zoom)

  if args.lat and args.lon:
    coord = (args.lat, args.lon)
    tile = lat_lon_to_tile(coord, zoom)
    print("coord =", coord)

  elif args.xtile and args.ytile:
    tile = (args.xtile, args.ytile, zoom)
  
  print("tile =", tile)

  (x, y, z) = tile

  lat_lon = tile_to_lat_lon(x, y, z)
  print("tile upper left lat_lon =", lat_lon)

  bb = bounding_box(x, y, z)
  print("bb =", bb)

  print("tile_width (m) =", tile_width(lat_lon[0], z))
  print("pixel_width (m) =", pixel_width(lat_lon[0], z))


  if args.download:
    download_tile(x, y, z)


