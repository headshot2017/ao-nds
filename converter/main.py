import os
import subprocess
import shutil
from multiprocessing import Pool, cpu_count, freeze_support

import requests

import conversion

def askBasePath():
    folder = ""
    while not folder:
        print("Drag the AO base folder to this console window then press ENTER:")
        folder = input("> ")
        if not os.path.exists(folder+"/characters"):
            print("AO data does not exist there.")
            folder = ""

    print("The selected base folder '%s' will be saved in aopath.txt" % folder)
    print("To bring up this dialog again, delete that text file.")
    print("Press enter to continue")
    input("")

    open("aopath.txt", "w").write(folder)
    return folder

if __name__ == "__main__":
    freeze_support()
    if os.path.exists("log.txt"):
        os.remove("log.txt")

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

    folder = open("aopath.txt", "r").read() if os.path.exists("aopath.txt") else ""
    if folder and not os.path.exists(folder):
        print("The AO base folder '%s' no longer exists." % folder)
        folder = ""

    if not folder:
        folder = askBasePath()


    while 1:
        os.system("cls||clear")

        print("AO NDS converter tool")
        print("This tool will convert your AO 'base' folder to formats that will work with the Nintendo DS.")
        print("Current AO base path: '%s'\n" % folder)

        print("Make a decision:")
        print("  1. Convert everything")
        print("  2. Convert backgrounds")
        print("  3. Convert characters")
        print("  4. Convert evidence images")
        print("  5. Convert sounds and blips")
        print("  6. Convert music")
        print("  7. Convert misc folder")
        print("  8. Exit")

        option = input("> ")
        if not option.isdigit(): continue
        option = int(option)
        if option < 1 or option > 8: continue

        for path in ["background", "characters", "evidence", "sounds/general", "sounds/music", "sounds/blips", "misc"]:
            if not os.path.exists("converted/data/ao-nds/" + path):
                os.makedirs("converted/data/ao-nds/" + path)

        if option == 1:
            print("Converting backgrounds...")
            conversion.convertBackgrounds(folder)

            print("\nConverting characters...")
            for f in os.listdir("converted/data/ao-nds/characters"):
                shutil.rmtree("converted/data/ao-nds/characters/" + f)

            pool = Pool(processes=(cpu_count()))
            for i in range(cpu_count()):
                pool.apply_async(conversion.convertCharacters, args=(folder+"/characters", "converted/data/ao-nds/characters", i))

            pool.close()
            pool.join()

            print("\nConverting evidence images...")
            conversion.convertEvidenceImages(folder+"/evidence", "converted/data/ao-nds/evidence")

            print("\nConverting sounds...")
            conversion.convertSounds(folder+"/sounds/blips", "converted/data/ao-nds/sounds/blips")
            conversion.convertSounds(folder+"/sounds/general", "converted/data/ao-nds/sounds/general")

            print("\nConverting music...")
            conversion.convertMusic(folder+"/sounds/music", "converted/data/ao-nds/sounds/music")

            print("\nConverting objection images and chatbox...")
            conversion.convertChatbox(folder)
            conversion.convertShout(folder+"/misc/default/objection_bubble.gif", "converted/data/ao-nds/misc/objection_bubble")
            conversion.convertShout(folder+"/misc/default/holdit_bubble.gif", "converted/data/ao-nds/misc/holdit_bubble")
            conversion.convertShout(folder+"/misc/default/takethat_bubble.gif", "converted/data/ao-nds/misc/takethat_bubble")
            conversion.convertSpeedlines(folder+"/themes/default/defense_speedlines.gif", "converted/data/ao-nds/misc/speedlines")

            print("Converting placeholder.gif...")
            if os.path.exists("converted/data/ao-nds/misc/nds.cfg"): os.remove("converted/data/ao-nds/misc/nds.cfg")
            conversion.convertEmoteGIF(folder+"/themes/default/placeholder.gif", "converted/data/ao-nds/misc/placeholder.gif", "converted/data/ao-nds/misc", 0, "")

            print("Cleaning up temporary files...")
            if os.path.exists("temp.png"): os.remove("temp.png")
            for i in range(cpu_count()):
                if os.path.exists("temp%d.png" % i): os.remove("temp%d.png" % i)

            print("Done!")
            print("Inside the folder named 'converted', you will see a folder named 'data'.")
            print("Copy this 'data' folder to the root of your SD card.")
            print("Press enter to return to the main menu...")
            input()

        elif option == 2:
            print("Enter the name of the background you wish to convert, or leave empty to convert all backgrounds")
            option = input("> ")

            if len(option):
                if os.path.exists(folder+"/background/"+option):
                    conversion.convertBackground(folder+"/background/"+option, "converted/data/ao-nds/background/"+option)
                else:
                    print("'%s' does not exist" % (option))
                    input("Press enter to continue...\n")
            else:
                conversion.convertBackgrounds(folder)

            if os.path.exists("temp.png"): os.remove("temp.png")

        elif option == 3:
            print("Enter the name of the character you wish to convert, or leave empty to convert all characters")
            option = input("> ")

            if len(option):
                if os.path.exists(folder+"/characters/"+option):
                    conversion.recursiveCharacter(folder+"/characters/"+option, "converted/data/ao-nds/characters/"+option, "converted/data/ao-nds/characters/"+option, 0)
                else:
                    print("'%s' does not exist" % (option))
                    input("Press enter to continue...\n")
            else:
                for f in os.listdir("converted/data/ao-nds/characters"):
                    shutil.rmtree("converted/data/ao-nds/characters/" + f)

                pool = Pool(processes=(cpu_count()))
                for i in range(cpu_count()):
                    pool.apply_async(conversion.convertCharacters, args=(folder+"/characters", "converted/data/ao-nds/characters", i))

                pool.close()
                pool.join()

            for i in range(cpu_count()):
                if os.path.exists("temp%d.png" % i): os.remove("temp%d.png" % i)

        elif option == 4:
            conversion.convertEvidenceImages(folder+"/evidence", "converted/data/ao-nds/evidence")
            if os.path.exists("temp.png"): os.remove("temp.png")

        elif option == 5:
            conversion.convertSounds(folder+"/sounds/blips", "converted/data/ao-nds/sounds/blips")
            conversion.convertSounds(folder+"/sounds/general", "converted/data/ao-nds/sounds/general")
            if os.path.exists("temp.png"): os.remove("temp.png")

        elif option == 6:
            conversion.convertMusic(folder+"/sounds/music", "converted/data/ao-nds/sounds/music")
            if os.path.exists("temp.png"): os.remove("temp.png")

        elif option == 7:
            conversion.convertChatbox(folder)
            conversion.convertShout(folder+"/misc/default/objection_bubble.gif", "converted/data/ao-nds/misc/objection_bubble")
            conversion.convertShout(folder+"/misc/default/holdit_bubble.gif", "converted/data/ao-nds/misc/holdit_bubble")
            conversion.convertShout(folder+"/misc/default/takethat_bubble.gif", "converted/data/ao-nds/misc/takethat_bubble")
            conversion.convertSpeedlines(folder+"/themes/default/defense_speedlines.gif", "converted/data/ao-nds/misc/speedlines")
            if os.path.exists("converted/data/ao-nds/misc/nds.cfg"): os.remove("converted/data/ao-nds/misc/nds.cfg")
            conversion.convertEmoteGIF(folder+"/themes/default/placeholder.gif", "converted/data/ao-nds/misc/placeholder.gif", "converted/data/ao-nds/misc", 0, "")
            if os.path.exists("temp.png"): os.remove("temp.png")
            if os.path.exists("temp0.png"): os.remove("temp0.png")

        elif option == 8:
            break
