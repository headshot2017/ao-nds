import os
import subprocess
import math
import configparser

from PIL import Image
from PIL import ImageChops
import requests

import images

def convertBackground(source, target):
    try: os.mkdir(target)
    except: pass

    if os.path.exists(target+"/desk_tiles.cfg"):
        os.remove(target+"/desk_tiles.cfg")

    # convert background first
    for imgfile in ["defenseempty.png", "prosecutorempty.png", "witnessempty.png", "helperstand.png", "prohelperstand.png", "judgestand.png"]:
        full_filename = source+"/"+imgfile
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

        if os.path.exists(target+"/"+no_ext_file+".img.bin"):
            os.remove(target+"/"+no_ext_file+".img.bin")
        if os.path.exists(target+"/"+no_ext_file+".map.bin"):
            os.remove(target+"/"+no_ext_file+".map.bin")
        if os.path.exists(target+"/"+no_ext_file+".pal.bin"):
            os.remove(target+"/"+no_ext_file+".pal.bin")
        os.rename("temp.img.bin", target+"/"+no_ext_file+".img.bin")
        os.rename("temp.map.bin", target+"/"+no_ext_file+".map.bin")
        os.rename("temp.pal.bin", target+"/"+no_ext_file+".pal.bin")

    # then convert desks. we won't bother with helper desks since these are mostly only used for special effects (source: GS4Night background)
    for imgfile, imgindex in [["defensedesk.png", 0], ["prosecutiondesk.png", 1], ["stand.png", 2], ["judgedesk.png", 3]]:
        full_filename = source+"/"+imgfile
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
        
        with open(target+"/desk_tiles.cfg", "a") as f:
            f.write("%s: %d,%d\n" % (no_ext_file, horizontalTiles, verticalTiles))

        # 8-bit tiles, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
        subprocess.Popen("grit temp.png -gB8 -gt -ftb -fh! -m! -Mh4 -Mw8").wait()
        
        if os.path.exists(target+"/"+no_ext_file+".img.bin"):
            os.remove(target+"/"+no_ext_file+".img.bin")
        if os.path.exists(target+"/"+no_ext_file+".pal.bin"):
            os.remove(target+"/"+no_ext_file+".pal.bin")
        os.rename("temp.img.bin", target+"/"+no_ext_file+".img.bin")
        os.rename("temp.pal.bin", target+"/"+no_ext_file+".pal.bin")

def convertBackgrounds(folder):
    for bg in os.listdir(folder+"/background"):
        if not os.path.isdir(folder+"/background/"+bg):
            continue

        print(bg)
        convertBackground(folder+"/background/"+bg, "converted/data/ao-nds/background/"+bg)

# frames: [[PIL.Image, ms_duration], [PIL.Image, ms_duration], [PIL.Image, ms_duration]...]
def convertEmoteFrames(frames, targetFile, ogTarget, extra):
    targetPath = os.path.dirname(targetFile)
    no_ext_file = os.path.splitext(targetFile)[0]
    no_dir_ext_file = os.path.basename(no_ext_file)

    # resize if not 256x192
    for i in range(len(frames)):
        frame = frames[i][0]
        if frame.size[0] != 256 or frame.size[1] != 192:
            frames[i][0] = frame.resize((256, 192), Image.BICUBIC)

    leftCorner = frames[0][0].size[0]
    rightCorner = 0
    top = frames[0][0].size[1]
    bottom = 0

    # find corners first
    for frame, duration in frames:
        found = False
        pix = frame.load()

        for x in range(frame.size[0]):
            for y in range(frame.size[1]):
                if pix[x, y][3] != 0 and x < leftCorner:
                    leftCorner = x
                    found = True
                    break
            if found: break

        found = False
        for x in range(frame.size[0]-1, -1, -1):
            for y in range(frame.size[1]):
                if pix[x, y][3] != 0 and x > rightCorner:
                    rightCorner = x
                    found = True
                    break
            if found: break

        found = False
        for y in range(frame.size[1]):
            for x in range(frame.size[0]):
                if pix[x, y][3] != 0 and y < top:
                    top = y
                    found = True
                    break
            if found: break

        found = False
        for y in range(frame.size[1]-1, -1, -1):
            for x in range(frame.size[0]):
                if pix[x, y][3] != 0 and y > bottom:
                    bottom = y
                    found = True
                    break
            if found: break

    if leftCorner == frames[0][0].size[0] and rightCorner == 0 and top == frames[0][0].size[1] and bottom == 0:
        # empty image?
        rightCorner = 31
        leftCorner = 0
        top = 0
        bottom = 31
        for i in range(len(frames)):
            frame = frames[i][0]
            frames[i][0] = frame.crop((0, 0, 32, 32))

    # crop and transparency
    croppedWidth = 0
    croppedHeight = 0
    for i in range(len(frames)):
        frame = frames[i][0]
        frame = frame.crop((leftCorner, top, rightCorner+1, bottom+1))
        croppedWidth = frame.size[0]
        croppedHeight = frame.size[1]
        frame = frame.crop((0, 0, math.ceil(frame.size[0]/32.)*32, math.ceil(frame.size[1]/32.)*32))

        pix = frame.load()
        for x in range(frame.size[0]):
            for y in range(frame.size[1]):
                if pix[x, y][3] == 0:
                    pix[x, y] = (255, 0, 255, 255)

        frames[i][0] = frame.convert("RGB")

    # to find difference between frames: diff = ImageChops.difference(frame1, frame2)
    # if difference exists, diff.getbbox() will return a tuple. if not, returns None

    # get rid of duplicate frames to reduce size, and use frame indexes instead
    noDuplicates = []
    frameIndexes = []
    for i in range(len(frames)):
        frame, duration = frames[i]

        found = False
        for j in range(len(noDuplicates)):
            other = noDuplicates[j]
            if not ImageChops.difference(frame, other).getbbox():
                # frame == other, so use the existing index
                frameIndexes.append(j)
                found = True
                break

        if not found:
            noDuplicates.append(frame)
            frameIndexes.append(len(noDuplicates)-1)

    # combine all non-duplicate frames into one and save.
    result = Image.new("RGB", (frames[0][0].size[0], frames[0][0].size[1] * len(noDuplicates)), (255, 0, 255))
    for i in range(len(noDuplicates)):
        frame = noDuplicates[i]
        result.paste(frame, (0, i*frame.size[1]))
    result.save("temp.png")

    streamFile = (frames[0][0].size[0] * frames[0][0].size[1] * len(noDuplicates) >= 512*1024)

    # 8-bit tiles, #FF00FF transparency color, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
    subprocess.Popen("grit temp.png -gB8 -gt -gTFF00FF %s -ftb -fh! -m! -Mh4 -Mw4" % ("" if streamFile else "-gzl")).wait()
    if not os.path.exists("temp.img.bin"):
        print("Failed to convert: %s" % (no_dir_ext_file))
        return

    # move files
    if os.path.exists(no_ext_file+".img.bin"):
        os.remove(no_ext_file+".img.bin")
    if os.path.exists(no_ext_file+".pal.bin"):
        os.remove(no_ext_file+".pal.bin")
    os.rename("temp.img.bin", no_ext_file+".img.bin")
    os.rename("temp.pal.bin", no_ext_file+".pal.bin")

    # save frame durations (in ms) and sizes into a cfg file.
    with open(ogTarget+"/nds.cfg", "a") as f:
        f.write(extra+no_dir_ext_file.lower()+"_size: %d,%d\n" % (croppedWidth, croppedHeight))
        f.write(extra+no_dir_ext_file.lower()+"_offset: %d,%d\n" % (leftCorner, top))
        f.write(extra+no_dir_ext_file.lower()+"_stream: %d\n" % (streamFile))
        f.write(extra+no_dir_ext_file.lower()+"_durations: ")
        for i in range(len(frames)):
            duration = frames[i][1]
            if i == len(frames)-1:
                f.write("%d" % duration)
            else:
                f.write("%d," % duration)
        f.write("\n")
        f.write(extra+no_dir_ext_file.lower()+"_indexes: ")
        for i in range(len(frames)):
            ind = frameIndexes[i]
            if i == len(frames)-1:
                f.write("%d" % ind)
            else:
                f.write("%d," % ind)
        f.write("\n")

def convertCharIcon(sourceFile, targetFile):
    img = Image.open(sourceFile).convert("RGBA").resize((38, 38)).crop((0, 0, 64, 64))
    pix = img.load()
    for x in range(img.size[0]):
        for y in range(img.size[1]):
            if pix[x, y][3] == 0:
                pix[x, y] = (255, 0, 255, 255)

    img.save("temp.png")
    img.close()

    no_ext_file = os.path.splitext(targetFile)[0]

    # 8-bit tiles, #FF00FF transparency color, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
    subprocess.Popen("grit temp.png -gB8 -gt -gTFF00FF -ftb -fh! -m! -Mh8 -Mw8").wait()

    if os.path.exists(no_ext_file+".img.bin"):
        os.remove(no_ext_file+".img.bin")
    if os.path.exists(no_ext_file+".pal.bin"):
        os.remove(no_ext_file+".pal.bin")
    os.rename("temp.img.bin", no_ext_file+".img.bin")
    os.rename("temp.pal.bin", no_ext_file+".pal.bin")

def convertEmoteButtons(source, target):
    if not os.path.exists(target):
        os.mkdir(target)

    for f in os.listdir(source):
        img = Image.open(source+"/"+f).convert("RGBA").crop((0, 0, 64, 64))
        pix = img.load()
        for x in range(img.size[0]):
            for y in range(img.size[1]):
                if pix[x, y][3] < 128:
                    pix[x, y] = (255, 0, 255, 255)
                else:
                    pix[x, y] = (pix[x,y][0], pix[x,y][1], pix[x,y][2], 255)

        img.save("temp.png")
        img.close()

        no_ext_file = target + "/" + os.path.splitext(f)[0]

        # 8-bit tiles, #FF00FF transparency color, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
        subprocess.Popen("grit temp.png -gB8 -gt -gTFF00FF -ftb -fh! -m! -Mh8 -Mw8").wait()

        if os.path.exists(no_ext_file+".img.bin"):
            os.remove(no_ext_file+".img.bin")
        if os.path.exists(no_ext_file+".pal.bin"):
            os.remove(no_ext_file+".pal.bin")
        os.rename("temp.img.bin", no_ext_file+".img.bin")
        os.rename("temp.pal.bin", no_ext_file+".pal.bin")

def convertEmoteAPNG(sourceFile, targetFile, ogTarget, extra):
    frames = images.load_apng(sourceFile)
    convertEmoteFrames(frames, targetFile, ogTarget, extra)

def convertEmoteWEBP(sourceFile, targetFile, ogTarget, extra):
    frames = images.load_webp(sourceFile)[0]
    convertEmoteFrames(frames, targetFile, ogTarget, extra)

def convertEmotePNG(sourceFile, targetFile, ogTarget, extra):
    frames = [[Image.open(sourceFile).convert("RGBA"), 0]]
    convertEmoteFrames(frames, targetFile, ogTarget, extra)

def convertEmoteGIF(sourceFile, targetFile, ogTarget, extra):
    frames = []
    img = Image.open(sourceFile)
    for f in range(img.n_frames):
        img.seek(f)
        img.load()
        frames.append([img.convert("RGBA"), img.info["duration"]])
    convertEmoteFrames(frames, targetFile, ogTarget, extra)

def recursiveCharacter(source, target, ogTarget, extra=""):
    if not os.path.exists(target):
        os.mkdir(target)

    if os.path.exists(target+"/nds.cfg"):
        os.remove(target+"/nds.cfg")

    # copy char.ini from source to target
    if not extra:
        open(target+"/char.ini", "w").write(open(source+"/char.ini").read())

    for emote in os.listdir(source):
        filename = source+"/"+emote
        if os.path.isdir(filename) and emote.lower() != "emotions":
            recursiveCharacter(source+"/"+emote, target+"/"+emote, ogTarget, extra+emote+"/")
        elif emote.lower() == "char_icon.png":
            convertCharIcon(filename, target+"/"+emote)
        elif emote.lower().endswith(".apng"):
            convertEmoteAPNG(filename, target+"/"+emote, ogTarget, extra)
        elif emote.lower().endswith(".webp"):
            convertEmoteWEBP(filename, target+"/"+emote, ogTarget, extra)
        elif emote.lower().endswith(".png"):
            convertEmotePNG(filename, target+"/"+emote, ogTarget, extra)
        elif emote.lower().endswith(".gif"):
            convertEmoteGIF(filename, target+"/"+emote, ogTarget, extra)

    if not extra:
        convertEmoteButtons(source+"/emotions", target+"/emotions")

def convertCharacters(source, target):
    for char in os.listdir(source):
        if not os.path.exists(source+"/"+char+"/char.ini"):
            continue # invalid char folder

        print(char)

        recursiveCharacter(source+"/"+char, target+"/"+char, target+"/"+char)

def convertEvidenceImages(folder):
    pass

def convertSounds(source, target):
    if not os.path.exists(target):
        os.mkdir(target)

    for f in os.listdir(source):
        if os.path.isdir(source+"/"+f):
            convertSounds(source+"/"+f, target+"/"+f)
        else:
            print(source+"/"+f)
            targetFile = os.path.splitext(target+"/"+f)[0] + ".wav"

            subprocess.Popen("ffmpeg -hide_banner -loglevel error -i \"%s\" -acodec pcm_s16le -ar 32000 -ac 1 -b:a 96k -y \"%s\"" % (source+"/"+f, targetFile)).wait()

def convertMusic(source, target):
    if not os.path.exists(target):
        os.mkdir(target)

    for f in os.listdir(source):
        if os.path.isdir(source+"/"+f):
            convertMusic(source+"/"+f, target+"/"+f)
        else:
            print(source+"/"+f)
            targetFile = os.path.splitext(target+"/"+f)[0] + ".mp3"

            # need to apply this metadata title so that the mp3 player used in the NDS app doesn't act funky when loading it
            subprocess.Popen("ffmpeg -hide_banner -loglevel error -i \"%s\" -map 0:a -ar 22050 -ac 2 -b:a 96k -metadata title=\"000000000000000000000000000000000000000\" -y \"%s\"" % (source+"/"+f, targetFile)).wait()

def convertChatbox(folder):
    print(folder+"/misc/default/chatbox.png")
    img = Image.open(folder+"/misc/default/chatbox.png").convert("RGBA")
    img = img.crop((0, -2, img.size[0], img.size[1]))

    pix = img.load()
    for y in range(img.size[1]):
        for x in range(img.size[0]):
            if pix[x, y][3] == 0:
                pix[x, y] = (255, 0, 255, 255)

    img.save("temp.png")
    img.close()

    # 8-bit tiles, #FF00FF transparency color, generate map file, enable palette, extended palette slot 1, export to .bin, don't generate .h file
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

def convertShout(filename):
    print(filename)

    img = Image.open(filename)
    img.seek(1)
    img = img.convert("RGBA")

    pix = img.load()
    for y in range(img.size[1]):
        for x in range(img.size[0]):
            if pix[x, y][3] == 0:
                pix[x, y] = (255, 0, 255, 255)

    img.save("temp.png")
    img.close()

    baseName = os.path.basename(filename)
    newFile = "converted/data/ao-nds/misc/" + os.path.splitext(baseName)[0]

    # 8-bit tiles, #FF00FF transparency color, generate map file, enable palette, extended palette slot 2, export to .bin, don't generate .h file
    subprocess.Popen("./grit temp.png -gB8 -gt -gTFF00FF -m -p -mp 2 -ftb -fh!").wait()

    if os.path.exists(newFile+".img.bin"):
        os.remove(newFile+".img.bin")
    if os.path.exists(newFile+".map.bin"):
        os.remove(newFile+".map.bin")
    if os.path.exists(newFile+".pal.bin"):
        os.remove(newFile+".pal.bin")
    os.rename("temp.img.bin", newFile+".img.bin")
    os.rename("temp.map.bin", newFile+".map.bin")
    os.rename("temp.pal.bin", newFile+".pal.bin")
