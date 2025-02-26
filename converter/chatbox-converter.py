from __future__ import print_function
import os
import subprocess
import sys

from PIL import Image

try:
    input = raw_input
except NameError:
    pass

def convert(filename):
    basename = os.path.splitext(os.path.basename(filename))[0]
    height = Image.open(filename).size[1]

    # 8-bit tiles, #FF00FF transparency color, generate map file, enable palette, export to .bin, don't generate .h file
    command = "grit {0} -gB4 -gt -gTFF00FF -m -p -ftb -fh!".split(" ")
    command[1] = filename;
    subprocess.Popen(command).wait()

    if not os.path.exists("converted/data/ao-nds/misc/chatboxes/" + basename):
        os.makedirs("converted/data/ao-nds/misc/chatboxes/" + basename)

    if os.path.exists("converted/data/ao-nds/misc/chatboxes/" + basename + "/chatbox.img.bin"):
        os.remove("converted/data/ao-nds/misc/chatboxes/" + basename + "/chatbox.img.bin")
    if os.path.exists("converted/data/ao-nds/misc/chatboxes/" + basename + "/chatbox.map.bin"):
        os.remove("converted/data/ao-nds/misc/chatboxes/" + basename + "/chatbox.map.bin")
    if os.path.exists("converted/data/ao-nds/misc/chatboxes/" + basename + "/chatbox.pal.bin"):
        os.remove("converted/data/ao-nds/misc/chatboxes/" + basename + "/chatbox.pal.bin")
    os.rename(basename+".img.bin", "converted/data/ao-nds/misc/chatboxes/"+basename+"/chatbox.img.bin")
    os.rename(basename+".map.bin", "converted/data/ao-nds/misc/chatboxes/"+basename+"/chatbox.map.bin")
    os.rename(basename+".pal.bin", "converted/data/ao-nds/misc/chatboxes/"+basename+"/chatbox.pal.bin")

    with open("converted/data/ao-nds/misc/chatboxes/"+basename+"/chatbox.ini", "w") as f:
        f.write("[general]\n")
        f.write("height = %d\n" % (height))
        f.write("hiddenFromSettings = 0\n")
        f.write("\n")
        f.write("# Name info.\n")
        f.write("# Set X and Y positions to the 'top center' of the name box.\n")
        f.write("# 'textColor' indicates the default text color in R,G,B (white by default)\n")
        f.write("[name]\n")
        f.write("x = 37\n")
        f.write("y = 3\n")
        f.write("textColor = 255,255,255\n")
        f.write("\n")
        f.write("# Text body info.\n");
        f.write("# Set Y position to the top area of the chatbox.\n")
        f.write("# 'arrowY' indicates the Y position of the 'next' arrow when text fully appears on the chatbox.\n")
        f.write("# 'textColor' indicates the default text color in R,G,B (white by default)\n")
        f.write("[body]\n")
        f.write("y = 20\n")
        f.write("lineSeparation = 16\n")
        f.write("arrowY = 62\n")
        f.write("textColor = 255,255,255\n")

def main():
    args = sys.argv[1:]
    if not args:
        print("AO NDS chatbox converter")
        print("Specifications:")
        print("Size: 256 x (multiple of 8), example: 256x72, 256x80")
        print("Image background MUST be magenta color (#FF00FF or RGB 255, 0, 255)")
        print("No transparency/alpha channel on the chatbox image.")
        print()
        print("Drag a chatbox image file to this program to convert it for use with AO NDS.")

        file = input("> ").strip('"')
        args.append(file)

    for img in args:
        print("Converting: " + img)
        convert(img)

    print("Your chatbox image has been converted.")
    print("You must now edit 'chatbox.ini' in converted/data/ao-nds/misc/chatboxes/<chatbox name>/")
    print("and specify the X,Y positions for the name text and chat text.")
    print("After that, copy the 'data' folder to the root of your SD card.")
    input("Press ENTER to exit\n")

if __name__ == "__main__":
    main()