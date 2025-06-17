import numpy as np

with open('simplesimplex.txt', 'r') as f:
    lines = f.readlines()

nums = []
advances = []
verts = []

for line in lines:
    parts = [p.strip() for p in line.split(',')]
    print(parts)
    nums.append(int(parts[0]))
    advances.append(4 * int(parts[1]))
    verts.append([int(v.replace('{ ','').replace(' }','')) for v in parts[2:]])
    
nums = np.array(nums)
advances = np.array(advances)
verts = verts

# print(nums)
print(advances)
# print(verts)

numchars = 94
cheight = 128
assert len(nums) == numchars
assert len(advances) == numchars
assert len(verts) == numchars

charlines = []
for c in range(numchars):
    
    clines = []
    
    i = 0
    while (i+1) < nums[c]:
        i0x = verts[c][(i+0)*2+0]
        i0y = verts[c][(i+0)*2+1]
        i1x = verts[c][(i+1)*2+0]
        i1y = verts[c][(i+1)*2+1]
        if i0x == -1 or i1x == -1:
            i += 1
            continue
        clines += [[[4 * i0x, 4 * (i0y + 8)], [4 * i1x, 4 * (i1y + 8)]]]
        i += 1
        # print(i, nums[c], i0x, i0y, i1x, i1y)
        
    charlines.append(np.array(clines))
    
# debug = True
debug = False
if debug:
    for ci, char in enumerate(charlines):
        print(chr(ci + ord('!')), len(char), advances[ci])

    import matplotlib.pyplot as plt
    for ci, char in enumerate(charlines):
        for line in char:
            plt.plot(line[:,0], line[:,1], color='red', linewidth=1.0)
        plt.xlim([0, advances[ci]])
        plt.ylim([0, cheight])
        plt.gca().set_aspect('equal')
        plt.show()

#############


sizes = np.array([len(char) for char in charlines])
offsets = []
data = []

for ci, char in enumerate(charlines):
    
    offsets.append(len(data))
    
    for li in range(len(char)):
        
        data.append(
            (char[li,0,0] <<  0) | 
            (char[li,0,1] <<  8) |
            (char[li,1,0] << 16) |
            (char[li,1,1] << 24))
        
output = "static const char simplesimplex_nums[94] = {\n    "

for ci in range(len(charlines)):

    output += ('%2i' % sizes[ci])
    
    if ci != len(charlines) - 1: output += ', '
    if ci != len(charlines) - 1 and ci > 0 and (ci + 1) % 18 == 0:
        output += '\n    '

output += "\n};\n\n"

output += "static const char simplesimplex_advs[94] = {\n    "

for ci in range(len(charlines)):

    output += ('%2i' % advances[ci])
    
    if ci != len(charlines) - 1: output += ', '
    if ci != len(charlines) - 1 and ci > 0 and (ci + 1) % 18 == 0:
        output += '\n    '

output += "\n};\n\n"

output += "static const int simplesimplex_offsets[94] = {\n    "

for ci in range(len(charlines)):

    output += ('%3i' % offsets[ci])
    
    if ci != len(charlines) - 1: output += ', '
    if ci != len(charlines) - 1 and ci > 0 and (ci + 1) % 14 == 0:
        output += '\n    '

output += "\n};\n\n"

output += "static const int simplesimplex_lines[%i] = {\n    " % len(data)

for di in range(len(data)):

    output += ('0x%08X' % data[di])
    
    if di != len(data) - 1: output += ', '
    if di != len(data) - 1 and di > 0 and (di + 1) % 6 == 0:
        output += '\n    '

output += "\n};\n\n"

print('\n\n')
print(output)

