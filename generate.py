from xml.dom import minidom
from svgpathtools import parse_path
import numpy as np

""" Parse SVG XML and extract lines """

doc = minidom.parse('Consolines.svg')
path_strings = [path.getAttribute('d') for path in doc.getElementsByTagName('path')]
doc.unlink()

lines = []

for path_string in path_strings:
    
    elem = parse_path(path_string)
    
    c0 = np.array([elem[0].start.real, 128 - elem[0].start.imag])
    c1 = np.array([elem[1].start.real, 128 - elem[1].start.imag])
    c2 = np.array([elem[2].start.real, 128 - elem[2].start.imag])
    c3 = np.array([elem[3].start.real, 128 - elem[3].start.imag])
    
    lines.append([(c0 + c1) / 2, (c2 + c3) / 2])
    
lines = np.round((np.array(lines) - 0.5)).astype(np.int32)

""" Extract lines for each individual character """

numchars = 94
cwidth = 64
cheight = 128

charlines = []

for i in range(numchars):
    
    minwidth = (i + 0) * cwidth
    maxwidth = (i + 1) * cwidth
    
    charlines.append([])
    for line in lines:
        if np.all((line[:,0] > minwidth) & (line[:,0] <= maxwidth)):
            charlines[i].append(line - np.array([minwidth, 0]))
    
    charlines[i] = np.clip(np.array(charlines[i]), 0, 127)

""" Plot each Character """

debug = False
# debug = True
if debug:
    for ci, char in enumerate(charlines):
        print(chr(ci + ord('!')), len(char))

    import matplotlib.pyplot as plt
    for char in charlines:
        for line in char:
            plt.plot(line[:,0], line[:,1], color='red', linewidth=1.0)
        plt.xlim([0, cwidth])
        plt.ylim([0, cheight])
        plt.gca().set_aspect('equal')
        plt.show()

""" Generate lookup table """

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
        
output = "static const char consolines_nums[94] = {\n    "

for ci in range(len(charlines)):

    output += ('%2i' % sizes[ci])
    
    if ci != len(charlines) - 1: output += ', '
    if ci != len(charlines) - 1 and ci > 0 and (ci + 1) % 18 == 0:
        output += '\n    '

output += "\n};\n\n"


output += "static const int consolines_offsets[94] = {\n    "

for ci in range(len(charlines)):

    output += ('%3i' % offsets[ci])
    
    if ci != len(charlines) - 1: output += ', '
    if ci != len(charlines) - 1 and ci > 0 and (ci + 1) % 14 == 0:
        output += '\n    '

output += "\n};\n\n"

output += "static const int consolines_lines[%i] = {\n    " % len(data)

for di in range(len(data)):

    output += ('0x%08X' % data[di])
    
    if di != len(data) - 1: output += ', '
    if di != len(data) - 1 and di > 0 and (di + 1) % 6 == 0:
        output += '\n    '

output += "\n};\n\n"

print('\n\n')
print(output)

