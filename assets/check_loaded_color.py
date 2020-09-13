from PIL import Image
import numpy as np
import scipy.misc as smp

colors = open("background_color.txt")

with open("background_color.txt") as f:
    content = f.readlines()
content = [ tuple(map(int, x[7:-1].split(',')[:-1])) for x in content ]

print(set(content))

# data = [[] for i in range(128)]
# for i in range(127, -1, -1):
# 	tmp = []
# 	for j in range(128):
# 		tmp.append(content[(127-i)*128+j])
# 	print(i)
# 	data[i] = tmp

# # img = np.zeros( (128,128,3), dtype=np.uint8 )

# img = smp.toimage( data )       # Create a PIL image
# img.save("test.png")                      # View in default viewer

