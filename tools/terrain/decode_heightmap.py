from PIL import Image
import numpy as np
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("--input", type=str, required=True)
parser.add_argument("--output", type=str, required=True)

args = parser.parse_args()

image = Image.open(args.input)

print(image.format)
print(image.size)
print(image.mode)

data = np.array(image)
print(data.shape)

(width, height, channels) = data.shape

out_data = np.zeros(data.shape)

max_elevation = 3000.0

def decode_pixel(pixel):
  (r,g,b,a) = pixel
  elevation = (r * 256.0 + g + b / 256.0) - 32768.0
  return elevation


for x in range(width):
  for y in range(height):
    pixel = data[x, y]

    #(r,g,b,a) = pixel
    #elevation = (r * 256.0 + g + b / 256.0) - 32768.0

    elevation = decode_pixel(pixel)

    #print(pixel)
    #print(elevation)

    value = (elevation / max_elevation) * 255.0
    value = np.clip(value, 0, 255)

    # print(value)

    out_data[x,y] = [value, value, value, 255]
    
    #tmp = pixel * (1.0 / 255.0)
    #print(decode_pixel(tmp))


out_img = Image.fromarray(np.uint8(out_data))
out_img.save(args.output)







