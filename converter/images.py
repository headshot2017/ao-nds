from PIL import Image
from apng import APNG
import io

APNG_DISPOSE_OP_NONE = 0
APNG_DISPOSE_OP_BACKGROUND = 1
APNG_DISPOSE_OP_PREVIOUS = 2
APNG_BLEND_OP_SOURCE = 0
APNG_BLEND_OP_OVER = 1
disposes = ["APNG_DISPOSE_OP_NONE", "APNG_DISPOSE_OP_BACKGROUND", "APNG_DISPOSE_OP_PREVIOUS"]
blends = ["APNG_BLEND_OP_SOURCE", "APNG_BLEND_OP_OVER"]

# layman's terms of above apng constants so i can easily wrap my head around it:
#
# APNG_DISPOSE_OP_NONE: do nothing with next frame, keep it as it is
# APNG_DISPOSE_OP_BACKGROUND: turn the current frame into full transparency before pasting the next frame
# APNG_DISPOSE_OP_PREVIOUS: grab the previous frame, and paste the next one into THAT previous frame
#
# APNG_BLEND_OP_SOURCE: replace frame region
# APNG_BLEND_OP_OVER: blend with frame

def load_apng(file): # this one was hell to implement compared to the three funcs below
    img = APNG.open(file)
    pilframes = []
    dispose_op = 0
    
    width, height = img.frames[0][0].width, img.frames[0][0].height
    outputbuf = Image.new("RGBA", (width, height), (255,255,255,0))
    prev_frame = None

    for frame, frame_info in img.frames:
        i = img.frames.index((frame, frame_info))
        pilframe = Image.open(io.BytesIO(frame.to_bytes())).convert("RGBA")

        if dispose_op == APNG_DISPOSE_OP_BACKGROUND or (i == 0 and dispose_op == APNG_DISPOSE_OP_PREVIOUS):
            prev_frame = outputbuf.copy()
            emptyrect = Image.new("RGBA", (img.frames[i-1][0].width, img.frames[i-1][0].height), (255,255,255,0))
            outputbuf.paste(emptyrect, (img.frames[i-1][1].x_offset, img.frames[i-1][1].y_offset))

        elif dispose_op == APNG_DISPOSE_OP_PREVIOUS:
            outputbuf = prev_frame.copy()
        
        else:
            prev_frame = outputbuf.copy()

        if frame_info:
            outputbuf.paste(pilframe, (frame_info.x_offset, frame_info.y_offset), pilframe.convert("RGBA") if frame_info.blend_op == APNG_BLEND_OP_OVER else None)
        else:
            outputbuf.paste(pilframe, (0, 0))

        final_frame = outputbuf.copy()
        if frame_info:
            pilframes.append([final_frame, frame_info.delay*10]) # convert delay from centiseconds to milliseconds
            dispose_op = frame_info.depose_op
        else:
            pilframes.append([final_frame, 0])

    return pilframes

def load_webp(file):
    img = Image.open(file)
    frames = []

    for i in range(img.n_frames):
        img.seek(i)
        img.load() # strange thing with Pillow and animated webp's is that the img.info dictionary attr doesn't update unless you call a function like this
        frames.append([img.copy(), img.info["duration"]])

    return frames, img.info["loop"]

def get_apng_duration(file):
    img = APNG.open(file)
    dur = 0

    for frame, frame_info in img.frames:
        if frame_info: dur += frame_info.delay*10 # convert delay from centiseconds to milliseconds

    return dur

def get_webp_duration(file):
    img = Image.open(file)
    dur = 0

    for i in range(img.n_frames):
        img.seek(i)
        img.load() # strange thing with Pillow and animated webp's is that the img.info dictionary attr doesn't update unless you call a function like this
        dur += img.info["duration"]

    return dur