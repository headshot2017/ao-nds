import os
import subprocess
import math

from PIL import Image
import requests

import images

if os.name == "nt" and not os.path.exists("ffmpeg.exe"):
    print("Downloading 7zr...")
    a = requests.get("https://www.7-zip.org/a/7zr.exe")
    open("7zr.exe", "wb").write(a.content)
    
    print("Downloading ffmpeg...")
    with requests.get("https://www.gyan.dev/ffmpeg/builds/ffmpeg-git-full.7z", stream=True) as r:
        r.raise_for_status()
        downloaded = 0
        with open("ffmpeg.7z", 'wb') as f:
            for chunk in r.iter_content(chunk_size=8192): 
                f.write(chunk)
                downloaded += len(chunk)
                print("    %.2f MB / %.2f MB" % (downloaded/1024./1024., int(r.headers["Content-length"])/1024./1024.), end="\r")

    print("Extracting ffmpeg...      ")
    subprocess.Popen("7zr e ffmpeg.7z -o. ffmpeg.exe -r").wait()

    print("Extracted\n\n")
    os.remove("ffmpeg.7z")
    os.remove("7zr.exe")

print("AO NDS converter tool")
print("This tool will convert your AO 'base' folder to formats that will work with the Nintendo DS.\n")

print("Drag the AO base folder to this console window then press ENTER:")
folder = input("> ")

try:
    os.makedirs("converted/data/ao-nds")
    os.makedirs("converted/data/ao-nds/background")
    os.makedirs("converted/data/ao-nds/characters")
    os.makedirs("converted/data/ao-nds/evidence")
    os.makedirs("converted/data/ao-nds/sounds/general")
    os.makedirs("converted/data/ao-nds/sounds/music")
    os.makedirs("converted/data/ao-nds/sounds/blips")
    os.makedirs("converted/data/ao-nds/misc")
except:
    pass

# start
print("Converting backgrounds...")
for bg in os.listdir(folder+"/background"):
    if not os.path.isdir(folder+"/background/"+bg):
        continue

    print(bg)
    try: os.mkdir("converted/data/ao-nds/background/"+bg)
    except: pass

    if os.path.exists("converted/data/ao-nds/background/"+bg+"/desk_tiles.cfg"):
        os.remove("converted/data/ao-nds/background/"+bg+"/desk_tiles.cfg")

    # convert background first
    for imgfile in ["defenseempty.png", "prosecutorempty.png", "witnessempty.png", "helperstand.png", "prohelperstand.png", "judgestand.png"]:
        full_filename = folder+"/background/"+bg+"/"+imgfile
        if not os.path.exists(full_filename):
            continue

        no_ext_file = os.path.splitext(imgfile)[0]

        img = Image.open(full_filename).convert("RGB")
        if img.size[0] != 256 or img.size[1] != 192:
            img = img.resize((256, 192), Image.BICUBIC)
        img.save("temp.png")
        img.close()

        # 8-bit tiles, generate map file, enable palette, extended palette slot 0, export to .bin, don't generate .h file
        subprocess.Popen("./grit temp.png -g -gB8 -gt -m -mp 0 -p -ftb -fh!").wait()

        if os.path.exists("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".img.bin"):
            os.remove("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".img.bin")
        if os.path.exists("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".map.bin"):
            os.remove("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".map.bin")
        if os.path.exists("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".pal.bin"):
            os.remove("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".pal.bin")
        os.rename("temp.img.bin", "converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".img.bin")
        os.rename("temp.map.bin", "converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".map.bin")
        os.rename("temp.pal.bin", "converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".pal.bin")

    # then convert desks. we won't bother with helper desks since these are mostly only used for special effects (source: GS4Night background)
    for imgfile, imgindex in [["defensedesk.png", 0], ["prosecutiondesk.png", 1], ["stand.png", 2], ["judgedesk.png", 3]]:
        full_filename = folder+"/background/"+bg+"/"+imgfile
        if not os.path.exists(full_filename):
            continue

        no_ext_file = os.path.splitext(imgfile)[0]

        img = Image.open(full_filename).convert("RGBA")
        if img.size[0] != 256 or img.size[1] != 192:
            img = img.resize((256, 192), Image.BICUBIC)

        pix = img.load()

        # find top image corner from top to bottom til we hit a visible pixel
        found = False
        top = 0
        for y in range(img.size[1]):
            for x in range(img.size[0]):
                if pix[x, y][3] != 0:
                    top = y
                    found = True
                    break
            if found: break

        # crop corners
        img = img.crop((0, top, img.size[0], img.size[1]))

        # in the AA games on DS, desks are loaded as sprites.
        # the image width will be set to a size divisible by 64,
        # and the height will be set to a size divisible by 32
        # so that it can be loaded as 64x32 tiles
        horizontalTiles = int(math.ceil(img.size[0]/64.))
        verticalTiles = int(math.ceil(img.size[1]/32.))
        img = img.crop((0, img.size[1]-(verticalTiles*32), horizontalTiles*64, img.size[1]))

        img.save("temp.png")
        img.close()
        
        with open("converted/data/ao-nds/background/"+bg+"/desk_tiles.cfg", "a") as f:
            f.write("%s: %d,%d\n" % (no_ext_file, horizontalTiles, verticalTiles))

        # 16-bit, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
        subprocess.Popen("grit temp.png -gB8 -gt -ftb -fh! -m! -Mh4 -Mw8").wait()
        
        if os.path.exists("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".img.bin"):
            os.remove("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".img.bin")
        if os.path.exists("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".pal.bin"):
            os.remove("converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".pal.bin")
        os.rename("temp.img.bin", "converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".img.bin")
        os.rename("temp.pal.bin", "converted/data/ao-nds/background/"+bg+"/"+no_ext_file+".pal.bin")


print("\nConverting characters...")
"""
a = images.load_apng(folder+"/characters/kristoph/(a)confident.apng")
pix = a[0][0].load()
middleWidth = 0
top = 0

found = False
for x in range(a[0][0].size[0]):
    for y in range(a[0][0].size[1]):
        if pix[x, y][3] != 0:
            middleWidth = x
            found = True
            break
    if found: break

found = False
for x in range(a[0][0].size[0]-1, -1, -1):
    for y in range(a[0][0].size[1]-1, -1, -1):
        if pix[x, y][3] != 0 and a[0][0].size[0]-1-x < middleWidth:
            middleWidth = a[0][0].size[0]-1-x
            found = True
            break
    if found: break

found = False
for y in range(a[0][0].size[1]):
    for x in range(a[0][0].size[0]):
        if pix[x, y][3] != 0:
            top = y
            found = True
            break
    if found: break

copy = None
copy = a[0][0].crop((middleWidth, top, a[0][0].size[0]-middleWidth, a[0][0].size[1]))
copy = copy.crop((0, 0, math.ceil(copy.size[0]/32.)*32, math.ceil(copy.size[1]/32.)*32))
#copy.show()
"""
# TO-DO


print("\nConverting evidence images...")
# TO-DO


print("\nConverting sounds...")
def recursiveSounds(source, target):
    if not os.path.exists(target):
        os.mkdir(target)

    for f in os.listdir(source):
        if os.path.isdir(source+"/"+f):
            recursiveSounds(source+"/"+f, target+"/"+f)
        else:
            print(source+"/"+f)
            targetFile = os.path.splitext(target+"/"+f)[0] + ".wav"

            subprocess.Popen("ffmpeg -hide_banner -loglevel error -i \"%s\" -acodec pcm_s16le -ar 32000 -ac 1 -b:a 96k -y \"%s\"" % (source+"/"+f, targetFile)).wait()

recursiveSounds(folder+"/sounds/blips", "converted/data/ao-nds/sounds/blips")
recursiveSounds(folder+"/sounds/general", "converted/data/ao-nds/sounds/general")


print("\nConverting music...")
def recursiveMusic(source, target):
    if not os.path.exists(target):
        os.mkdir(target)

    for f in os.listdir(source):
        if os.path.isdir(source+"/"+f):
            recursiveMusic(source+"/"+f, target+"/"+f)
        else:
            print(source+"/"+f)
            targetFile = os.path.splitext(target+"/"+f)[0] + ".mp3"

            # need to apply this metadata title so that the mp3 player used in the NDS app doesn't act funky when loading it
            subprocess.Popen("ffmpeg -hide_banner -loglevel error -i \"%s\" -ar 22050 -ac 2 -b:a 96k -metadata title=\"                        \" -y \"%s\"" % (source+"/"+f, targetFile)).wait()

recursiveMusic(folder+"/sounds/music", "converted/data/ao-nds/sounds/music")


print("\nConverting objection images and chatbox...")
def chatbox():
    print(folder+"/misc/default/chatbox.png")
    img = Image.open(folder+"/misc/default/chatbox.png")
    img = img.crop((0, -2, img.size[0], img.size[1]))

    pix = img.load()
    for y in range(img.size[1]):
        for x in range(img.size[0]):
            if pix[x, y][3] == 0:
                pix[x, y] = (255, 0, 255, 255)

    img.save("temp.png")
    img.close()

    # 8-bit tiles, #FF00FF transparency color, generate map file, enable palette, extended palette slot 4, export to .bin, don't generate .h file
    subprocess.Popen("./grit temp.png -gB8 -gt -gTFF00FF -m -p -mp 1 -ftb -fh!").wait()

    if os.path.exists("converted/data/ao-nds/misc/chatbox.img.bin"):
        os.remove("converted/data/ao-nds/misc/chatbox.img.bin")
    if os.path.exists("converted/data/ao-nds/misc/chatbox.map.bin"):
        os.remove("converted/data/ao-nds/misc/chatbox.map.bin")
    if os.path.exists("converted/data/ao-nds/misc/chatbox.pal.bin"):
        os.remove("converted/data/ao-nds/misc/chatbox.pal.bin")
    os.rename("temp.img.bin", "converted/data/ao-nds/misc/chatbox.img.bin")
    os.rename("temp.map.bin", "converted/data/ao-nds/misc/chatbox.map.bin")
    os.rename("temp.pal.bin", "converted/data/ao-nds/misc/chatbox.pal.bin")

def shout(filename):
    print(filename)

    img = Image.open(filename)
    img.seek(1)
    img.save("temp.png")
    img.close()

    baseName = os.path.basename(filename)
    newFile = "converted/data/ao-nds/misc/" + os.path.splitext(baseName)[0] + ".img.bin"

    # 16-bit bitmap, LZ77 compression, export to .img.bin, don't generate .h file
    subprocess.Popen("./grit temp.png -gB16 -gb -gzl -ftb -fh!").wait()

    if os.path.exists(newFile):
        os.remove(newFile)
    os.rename("temp.img.bin", newFile)
    
chatbox()
shout(folder+"/misc/default/objection_bubble.gif")
shout(folder+"/misc/default/holdit_bubble.gif")
shout(folder+"/misc/default/takethat_bubble.gif")


print("Cleaning up temporary files...")
os.remove("temp.png") # lmao

print("Done!")
print("Inside the folder named 'converted', you will see a folder named 'data'.")
print("Copy this 'data' folder to the root of your SD card.")
