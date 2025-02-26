import os
import subprocess
import math

from PIL import Image
from PIL import ImageChops
import requests

import images

def convertBackground(source, target):
    try: os.mkdir(target)
    except: pass

    if os.path.exists(target+"/desk_tiles.cfg"):
        os.remove(target+"/desk_tiles.cfg")

    # convert court.png (full courtroom view) if available
    if os.path.exists(source+"/court.png") and os.path.exists(source+"/design.ini"):
        img = Image.open(source+"/court.png").convert("RGB").convert("P", colors=256, palette=Image.ADAPTIVE, dither=False)
        originalHeight = img.size[1]

        # resize if not 192p
        if img.size[1] != 192:
            ratio = img.size[0] / float(img.size[1])
            img = img.resize((int(192*ratio), 192), Image.BICUBIC)

        partWidth = 264.
        parts = int(math.ceil(img.size[0] / partWidth))
        for i in range(parts):
            x0 = partWidth*i
            x1 = partWidth*(i+1)
            if x1 > img.size[0]: x1 = img.size[0]
            part = img.crop((x0, 0, x1, 192))
            if (part.size[0] < 264):
                part = part.crop((0, 0, 264, 192))
            part.save("court%d.png" % i)

        for i in range(parts):
            subprocess.Popen( ("grit court%d.png -g -gB8 -gt -m! -p -ftb -fh!" % i).split(" ") ).wait()
            if os.path.exists(target+"/court%d.img.bin" % i):
                os.remove(target+"/court%d.img.bin" % i)
            if os.path.exists(target+"/court%d.pal.bin" % i):
                os.remove(target+"/court%d.pal.bin" % i)
            os.rename("court%d.img.bin" % i, target+"/court%d.img.bin" % i)
            os.rename("court%d.pal.bin" % i, target+"/court%d.pal.bin" % i)
            os.remove("court%d.png" % i)

        inifile = open(source+"/design.ini").read()
        inifile += "\n[nds]\n"
        inifile += "original_scale=%d\n" % (originalHeight / img.size[1])
        inifile += "total_parts=%d\n" % parts
        open(target+"/design.ini", "w").write(inifile)

    # convert background first
    for imgfile in ["defenseempty", "prosecutorempty", "witnessempty", "helperstand", "prohelperstand", "judgestand", "jurystand", "seancestand"]:
        for ext in [".png", ".webp", ".gif", ".apng"]:
            full_filename = source+"/"+imgfile+ext
            if not os.path.exists(full_filename):
                continue

            img = Image.open(full_filename).convert("RGB")

            # if aspect ratio is not 4:3, crop
            ratioW = 256/192.
            if math.floor(img.size[0] / img.size[1] * 1000) != 1333:
                w = ratioW * img.size[1]
                img = img.crop(((img.size[0]-w)/2, 0, (img.size[0]+w)/2, img.size[1]))

            if img.size[0] != 256 or img.size[1] != 192:
                img = img.resize((256, 192), Image.BICUBIC)

            img.save("temp.png")
            img.close()

            # 8-bit tiles, generate map file, enable palette, extended palette slot 0, export to .bin, don't generate .h file
            subprocess.Popen("grit temp.png -g -gB8 -gt -m -mp 0 -p -ftb -fh!".split(" ")).wait()

            if os.path.exists(target+"/"+imgfile+".img.bin"):
                os.remove(target+"/"+imgfile+".img.bin")
            if os.path.exists(target+"/"+imgfile+".map.bin"):
                os.remove(target+"/"+imgfile+".map.bin")
            if os.path.exists(target+"/"+imgfile+".pal.bin"):
                os.remove(target+"/"+imgfile+".pal.bin")
            os.rename("temp.img.bin", target+"/"+imgfile+".img.bin")
            os.rename("temp.map.bin", target+"/"+imgfile+".map.bin")
            os.rename("temp.pal.bin", target+"/"+imgfile+".pal.bin")

    # then convert desks. we won't bother with helper desks since these are mostly only used for special effects (source: GS4Night background)
    for imgfile, imgindex in [["defensedesk", 0], ["prosecutiondesk", 1], ["stand", 2], ["judgedesk", 3], ["jurydesk", 4]]:
        for ext in [".png", ".webp", ".gif", ".apng"]:
            full_filename = source+"/"+imgfile+ext
            if not os.path.exists(full_filename):
                continue

            img = Image.open(full_filename).convert("RGBA")

            # if aspect ratio is not 4:3, crop
            ratioW = 256/192.
            if math.floor(img.size[0] / img.size[1] * 1000) != 1333:
                w = ratioW * img.size[1]
                img = img.crop(((img.size[0]-w)/2, 0, (img.size[0]+w)/2, img.size[1]))

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

            # set transparency
            pix = img.load()
            for y in range(img.size[1]):
                for x in range(img.size[0]):
                    if pix[x, y][3] < 224:
                        pix[x, y] = (255, 0, 255, 255)
                    else:
                        pix[x, y] = (pix[x,y][0], pix[x,y][1], pix[x,y][2], 255)

            img.save("temp.png")
            img.close()
            
            with open(target+"/desk_tiles.cfg", "a") as f:
                f.write("%s: %d,%d\n" % (imgfile, horizontalTiles, verticalTiles))

            # 8-bit tiles, transparency, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
            subprocess.Popen("grit temp.png -gB8 -gt -gTFF00FF -ftb -fh! -m! -Mh4 -Mw8".split(" ")).wait()
            
            if os.path.exists(target+"/"+imgfile+".img.bin"):
                os.remove(target+"/"+imgfile+".img.bin")
            if os.path.exists(target+"/"+imgfile+".pal.bin"):
                os.remove(target+"/"+imgfile+".pal.bin")
            os.rename("temp.img.bin", target+"/"+imgfile+".img.bin")
            os.rename("temp.pal.bin", target+"/"+imgfile+".pal.bin")

def convertBackgrounds(folder, target):
    for bg in os.listdir(folder+"background"):
        if not os.path.isdir(folder+"background/"+bg):
            continue

        print(bg)
        convertBackground(folder+"background/"+bg, target+"background/"+bg)

# frames: [[PIL.Image, ms_duration], [PIL.Image, ms_duration], [PIL.Image, ms_duration]...]
def convertEmoteFrames(frames, targetFile, ogTarget, core, extra):
    targetPath = os.path.dirname(targetFile)
    no_ext_file = os.path.splitext(targetFile)[0]
    no_dir_ext_file = os.path.basename(no_ext_file)

    # if aspect ratio is not 4:3, crop
    ratioW = 256/192.
    if math.floor(frames[0][0].size[0] / frames[0][0].size[1] * 1000) != 1333:
        w = ratioW * frames[0][0].size[1]
        for i in range(len(frames)):
            frame = frames[i][0]
            frames[i][0] = frame.crop(((frame.size[0]-w)/2, 0, (frame.size[0]+w)/2, frame.size[1]))
    
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
        rightCorner = 63
        leftCorner = 0
        top = 0
        bottom = 63
        for i in range(len(frames)):
            frame = frames[i][0]
            frames[i][0] = frame.crop((0, 0, 64, 64))

    # crop and transparency
    croppedWidth = 0
    croppedHeight = 0
    for i in range(len(frames)):
        frame = frames[i][0]
        frame = frame.crop((leftCorner, top, rightCorner+1, bottom+1))
        croppedWidth = frame.size[0]
        croppedHeight = frame.size[1]
        frame = frame.crop((0, 0, math.ceil(frame.size[0]/64.)*64, math.ceil(frame.size[1]/64.)*64))

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
    result.save("temp%d.png" % core)

    streamFile = (frames[0][0].size[0] * frames[0][0].size[1] * len(noDuplicates) >= 512*1024)

    # 8-bit tiles, #FF00FF transparency color, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
    subprocess.Popen( ("grit temp%d.png -gB8 -gt -gTFF00FF %s -ftb -fh! -m! -Mh8 -Mw8" % (core, "" if streamFile else "-gzl")).split(" ") ).wait()
    if not os.path.exists("temp%d.img.bin" % core):
        print("Failed to convert: %s" % (targetFile))
        open("log.txt", "a").write("Failed to convert: %s\n" % (targetFile))
        return

    # move files
    if os.path.exists(no_ext_file+".img.bin"):
        os.remove(no_ext_file+".img.bin")
    if os.path.exists(no_ext_file+".pal.bin"):
        os.remove(no_ext_file+".pal.bin")
    os.rename("temp%d.img.bin" % core, no_ext_file+".img.bin")
    os.rename("temp%d.pal.bin" % core, no_ext_file+".pal.bin")

    # save frame durations (in ms) and sizes into a cfg file.
    with open(ogTarget+"/nds.cfg", "a") as f:
        f.write(extra+no_dir_ext_file.lower()+"_size: %d,%d\n" % (croppedWidth, croppedHeight))
        f.write(extra+no_dir_ext_file.lower()+"_offset: %d,%d\n" % (leftCorner, top))
        f.write(extra+no_dir_ext_file.lower()+"_stream: %d\n" % (streamFile))
        f.write(extra+no_dir_ext_file.lower()+"_frameGfxCount: %d\n" % (len(noDuplicates)))
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

def convertCharIcon(sourceFile, targetFile, core):
    img = Image.open(sourceFile).convert("RGBA").resize((38, 38)).crop((0, 0, 64, 64))
    imgbig = Image.open(sourceFile).convert("RGBA").crop((0, 0, 64, 64))

    pix = img.load()
    for x in range(img.size[0]):
        for y in range(img.size[1]):
            if pix[x, y][3] == 0:
                pix[x, y] = (255, 0, 255, 255)

    pix = imgbig.load()
    for x in range(imgbig.size[0]):
        for y in range(imgbig.size[1]):
            if pix[x, y][3] == 0:
                pix[x, y] = (255, 0, 255, 255)

    no_ext_file = os.path.splitext(targetFile)[0]

    for imgObj, suffix in ((img, ""), (imgbig, "_big")):
        imgObj.save("temp%d.png" % core)
        imgObj.close()

        # 8-bit tiles, #FF00FF transparency color, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
        subprocess.Popen( ("grit temp%d.png -gB8 -gt -gTFF00FF -ftb -fh! -m! -Mh8 -Mw8" % core).split(" ") ).wait()

        if os.path.exists(no_ext_file+suffix+".img.bin"):
            os.remove(no_ext_file+suffix+".img.bin")
        if os.path.exists(no_ext_file+suffix+".pal.bin"):
            os.remove(no_ext_file+suffix+".pal.bin")
        os.rename("temp%d.img.bin" % core, no_ext_file+suffix+".img.bin")
        os.rename("temp%d.pal.bin" % core, no_ext_file+suffix+".pal.bin")

def convertEmoteButtons(source, target, core):
    if not os.path.exists(target):
        os.mkdir(target)

    for f in os.listdir(source):
        try:
            img = Image.open(source+"/"+f).convert("RGBA")
        except:
            continue

        if img.size[0] != 40 or img.size[1] != 40:
            img = img.resize((40, 40))
        img = img.crop((0, 0, 64, 64))
        
        pix = img.load()
        for x in range(img.size[0]):
            for y in range(img.size[1]):
                if pix[x, y][3] < 128:
                    pix[x, y] = (255, 0, 255, 255)
                else:
                    pix[x, y] = (pix[x,y][0], pix[x,y][1], pix[x,y][2], 255)

        img.save("temp%d.png" % core)
        img.close()

        no_ext_file = target + "/" + os.path.splitext(f)[0]

        # 8-bit tiles, #FF00FF transparency color, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
        subprocess.Popen( ("grit temp%d.png -gB8 -gt -gTFF00FF -ftb -fh! -m! -Mh8 -Mw8" % core).split(" ") ).wait()

        if os.path.exists(no_ext_file+".img.bin"):
            os.remove(no_ext_file+".img.bin")
        if os.path.exists(no_ext_file+".pal.bin"):
            os.remove(no_ext_file+".pal.bin")
        os.rename("temp%d.img.bin" % core, no_ext_file+".img.bin")
        os.rename("temp%d.pal.bin" % core, no_ext_file+".pal.bin")

def convertEmoteAPNG(sourceFile, targetFile, ogTarget, core, extra):
    frames = images.load_apng(sourceFile)
    convertEmoteFrames(frames, targetFile, ogTarget, core, extra)

def convertEmoteWEBP(sourceFile, targetFile, ogTarget, core, extra):
    frames = images.load_webp(sourceFile)
    convertEmoteFrames(frames, targetFile, ogTarget, core, extra)

def convertEmotePNG(sourceFile, targetFile, ogTarget, core, extra):
    frames = [[Image.open(sourceFile).convert("RGBA"), 0]]
    convertEmoteFrames(frames, targetFile, ogTarget, core, extra)

def convertEmoteGIF(sourceFile, targetFile, ogTarget, core, extra):
    frames = []
    img = Image.open(sourceFile)

    n_frames = 0
    for i in range(5):
        try:
            n_frames = img.n_frames
            break
        except:
            continue

    if not n_frames: return

    for f in range(n_frames):
        img.seek(f)
        img.load()
        duration = img.info["duration"] if "duration" in img.info else 0
        frames.append([img.convert("RGBA"), duration])
    convertEmoteFrames(frames, targetFile, ogTarget, core, extra)

def recursiveCharacter(source, target, ogTarget, core, extra=""):
    if not os.path.exists(target):
        os.mkdir(target)

    if os.path.exists(target+"/nds.cfg"):
        os.remove(target+"/nds.cfg")

    if not extra and os.path.exists(source+"/char.ini"):
        open(target+"/char.ini", "w").write(open(source+"/char.ini").read())

    for emote in os.listdir(source):
        filename = source+"/"+emote.lower()
        #print(filename)
        
        if os.path.isdir(filename) and emote.lower() != "emotions" and emote.lower() != "custom_objections":
            recursiveCharacter(source+"/"+emote.lower(), target+"/"+emote.lower(), ogTarget, core, extra+emote.lower()+"/")
        elif emote.lower() == "char_icon.png":
            convertCharIcon(filename, target+"/"+emote, core)
        elif not os.path.isdir(filename):
            header = open(filename, "rb").read(16)
            if header[:3] == b"GIF":
                convertEmoteGIF(filename, target+"/"+emote, ogTarget, core, extra)
            elif emote.lower().endswith(".apng"):
                convertEmoteAPNG(filename, target+"/"+emote, ogTarget, core, extra)
            elif header[8:12] == b"WEBP":
                convertEmoteWEBP(filename, target+"/"+emote, ogTarget, core, extra)
            elif emote.lower().endswith(".png"):
                convertEmotePNG(filename, target+"/"+emote, ogTarget, core, extra)

    if not extra:
        if os.path.exists(source+"/emotions"):
            convertEmoteButtons(source+"/emotions", target+"/emotions", core)

        for snd in ["holdit.wav", "holdit.opus", "objection.wav", "objection.opus", "takethat.wav", "takethat.opus", "custom.wav", "custom.opus"]:
            if not os.path.exists(source+"/"+snd): continue
            convertSound(source+"/"+snd, target+"/"+snd)

        for custom in ["custom.apng", "custom.webp", "custom.gif", "custom.png"]:
            if not os.path.exists(source+"/"+custom): continue
            convertShout(source+"/"+custom, target+"/"+custom, core)

        if os.path.exists(source+"/custom_objections"):
            if not os.path.exists(target+"/custom_objections"):
                os.mkdir(target+"/custom_objections")

            for shout in os.listdir(source+"/custom_objections"):
                if shout.lower().endswith(".opus") or shout.lower().endswith(".wav"):
                    convertSound(source+"/custom_objections/"+shout, target+"/custom_objections/"+shout)
                else:
                    convertShout(source+"/custom_objections/"+shout, target+"/custom_objections/"+shout, core)


def convertCharacters(source, target, core):
    for char in os.listdir(source)[core:]:
        if not os.path.exists(source+"/"+char+"/char.ini") or os.path.exists(target+"/"+char):
            continue

        print("[%d] %s" % (core+1, char))

        recursiveCharacter(source+"/"+char, target+"/"+char, target+"/"+char, core)

    print("Core %d finished" % (core+1))

def convertEvidenceSubdir(source, target, subdir):
    if not os.path.exists(target+"/small/"+subdir): os.mkdir(target+"/small/"+subdir)
    if not os.path.exists(target+"/large/"+subdir): os.mkdir(target+"/large/"+subdir)

    for f in os.listdir(source+"/"+subdir):
        if subdir and os.path.isdir(source+"/"+subdir+"/"+f):
            convertEvidenceSubdir(source, target, subdir+"/"+f)
            continue

        try:
            imgOriginal = Image.open(source+"/"+subdir+"/"+f).convert("RGBA")
        except:
            continue

        no_ext_file = os.path.splitext(f)[0]
        print(subdir+"/"+no_ext_file)
        imgs = [[imgOriginal.resize((38, 38)).crop((0, 0, 64, 64)), "small"], [imgOriginal.crop((3, 3, 68, 68)), "large"]]

        for img, sizeStr in imgs:
            pix = img.load()
            for x in range(img.size[0]):
                for y in range(img.size[1]):
                    if pix[x, y][3] < 128:
                        pix[x, y] = (255, 0, 255, 255)
                    else:
                        pix[x, y] = (pix[x,y][0], pix[x,y][1], pix[x,y][2], 255)

            img.save("temp.png")
            img.close()

            # 8-bit tiles, #FF00FF transparency color, export to .img.bin, don't generate .h file, exclude map data, metatile height and width
            subprocess.Popen("grit temp.png -gB8 -gt -gTFF00FF -ftb -fh! -m! -Mh8 -Mw8".split(" ")).wait()

            targetFile = target + "/" + sizeStr + "/" + subdir + "/" + no_ext_file
            if os.path.exists(targetFile + ".img.bin"): os.remove(targetFile + ".img.bin")
            if os.path.exists(targetFile + ".pal.bin"): os.remove(targetFile + ".pal.bin")
            os.rename("temp.img.bin", targetFile + ".img.bin")
            os.rename("temp.pal.bin", targetFile + ".pal.bin")

def convertEvidenceImages(source, target):
    if not os.path.exists(target+"/small"): os.mkdir(target+"/small")
    if not os.path.exists(target+"/large"): os.mkdir(target+"/large")

    for f in os.listdir(source):
        if not os.path.isdir(source+"/"+f): continue
        convertEvidenceSubdir(source, target, f)

    convertEvidenceSubdir(source, target, "")

def convertSound(source, target):
    targetFile = os.path.splitext(target)[0] + ".wav"
    subprocess.Popen("ffmpeg -hide_banner -loglevel error -i \"%s\" -acodec pcm_s16le -ar 22050 -ac 1 -y \"%s\"" % (source, targetFile), shell=True).wait()

def convertSounds(source, target):
    if not os.path.exists(target):
        os.mkdir(target)

    for f in os.listdir(source):
        if os.path.isdir(source+"/"+f):
            convertSounds(source+"/"+f, target+"/"+f)
        else:
            print(source+"/"+f)
            convertSound(source+"/"+f, target+"/"+f)

def convertMusic(source, target):
    if not os.path.exists(target):
        os.mkdir(target)

    for f in os.listdir(source):
        if os.path.isdir(source+"/"+f):
            convertMusic(source+"/"+f, target+"/"+f)
        elif os.path.splitext(target+"/"+f)[1] in (".mp3", ".ogg", ".opus"):
            print(source+"/"+f)
            targetFile = os.path.splitext(target+"/"+f)[0] + ".adx"
            if os.path.splitext(f)[0].lower() == "~stop": # don't convert ~stop.mp3
                continue
            subprocess.Popen("ffmpeg -hide_banner -loglevel error -i \"%s\" -ar 32000 -y \"%s\"" % (source+"/"+f, targetFile), shell=True).wait()

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

    # 8-bit tiles, #FF00FF transparency color, generate map file, enable palette, export to .bin, don't generate .h file
    subprocess.Popen("grit temp.png -gB4 -gt -gTFF00FF -m -p -ftb -fh!".split(" ")).wait()

    if os.path.exists("converted/data/ao-nds/misc/chatboxes/default/chatbox.img.bin"):
        os.remove("converted/data/ao-nds/misc/chatboxes/default/chatbox.img.bin")
    if os.path.exists("converted/data/ao-nds/misc/chatboxes/default/chatbox.map.bin"):
        os.remove("converted/data/ao-nds/misc/chatboxes/default/chatbox.map.bin")
    if os.path.exists("converted/data/ao-nds/misc/chatboxes/default/chatbox.pal.bin"):
        os.remove("converted/data/ao-nds/misc/chatboxes/default/chatbox.pal.bin")
    os.rename("temp.img.bin", "converted/data/ao-nds/misc/chatboxes/default/chatbox.img.bin")
    os.rename("temp.map.bin", "converted/data/ao-nds/misc/chatboxes/default/chatbox.map.bin")
    os.rename("temp.pal.bin", "converted/data/ao-nds/misc/chatboxes/default/chatbox.pal.bin")

    with open("converted/data/ao-nds/misc/chatboxes/default/chatbox.ini", "w") as f:
        f.write("[general]\n")
        f.write("height = %d\n" % (img.size[1]))
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

def convertShout(source, target, core=0):
    frames = []

    if source.lower().endswith(".apng"):
        frames = images.load_apng(source)
    elif source.lower().endswith(".webp"):
        frames = images.load_webp(source)
    elif source.lower().endswith(".png"):
        frames = [[Image.open(source).convert("RGBA"), 0]]
    elif source.lower().endswith(".gif"):
        img = Image.open(source)
        for f in range(img.n_frames):
            img.seek(f)
            img.load()
            frames.append([img.convert("RGBA"), img.info["duration"]])
    else:
        return

    img = None
    for i in range(len(frames)-1, -1, -1):
        frame = frames[i][0]
        ex = frame.convert("L").getextrema()
        if ex[0] != ex[1]: # this frame is not blank
            img = frame
            break

    if not img:
        print("Couldn't convert shout %s" % source)
        return

    if img.size[0]//32 != 8 and img.size[1]//32 != 6:
        img = img.resize((256, 192), Image.BICUBIC)
    center = [(img.size[0]-256)/2, (img.size[1]-192)/2]
    img = img.crop((center[0], center[1], img.size[0]-center[0], img.size[1]-center[1]))

    pix = img.load()
    for y in range(img.size[1]):
        for x in range(img.size[0]):
            if pix[x, y][3] < 128:
                pix[x, y] = (255, 0, 255, 255)
            elif pix[x, y][3] < 255:
                pix[x, y] = (pix[x,y][0], pix[x,y][1], pix[x,y][2], 255)

    img = img.convert("P", dither=None)
    img.save("temp%d.png" % core)
    img.close()

    newFile = os.path.splitext(target)[0]

    # 8-bit tiles, #FF00FF transparency color, generate map file, enable palette, export to .bin, don't generate .h file
    subprocess.Popen( ("grit temp%d.png -gB4 -gt -gTFF00FF -m -p -ftb -fh!" % core).split(" ") ).wait()

    if os.path.exists(newFile+".img.bin"):
        os.remove(newFile+".img.bin")
    if os.path.exists(newFile+".map.bin"):
        os.remove(newFile+".map.bin")
    if os.path.exists(newFile+".pal.bin"):
        os.remove(newFile+".pal.bin")
    os.rename("temp%d.img.bin" % core, newFile+".img.bin")
    os.rename("temp%d.map.bin" % core, newFile+".map.bin")
    os.rename("temp%d.pal.bin" % core, newFile+".pal.bin")

def convertSpeedlines(source, target):
    img = Image.open(source).convert("RGBA")

    img.save("temp.png")
    img.close()

    # 8-bit tiles, generate map file, enable palette, extended palette slot 0, export to .bin, don't generate .h file
    subprocess.Popen("grit temp.png -g -gB8 -gt -m -mp 0 -p -ftb -fh!".split(" ")).wait()

    if os.path.exists(target+".img.bin"):
        os.remove(target+".img.bin")
    if os.path.exists(target+".map.bin"):
        os.remove(target+".map.bin")
    if os.path.exists(target+".pal.bin"):
        os.remove(target+".pal.bin")
    os.rename("temp.img.bin", target+".img.bin")
    os.rename("temp.map.bin", target+".map.bin")
    os.rename("temp.pal.bin", target+".pal.bin")
