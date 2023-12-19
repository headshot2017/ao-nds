from PIL import Image, ImageDraw
from apng import APNG
import io

APNG_DISPOSE_OP_NONE = 0
APNG_DISPOSE_OP_BACKGROUND = 1
APNG_DISPOSE_OP_PREVIOUS = 2
APNG_BLEND_OP_SOURCE = 0
APNG_BLEND_OP_OVER = 1
disposes = ["APNG_DISPOSE_OP_NONE", "APNG_DISPOSE_OP_BACKGROUND", "APNG_DISPOSE_OP_PREVIOUS"]
blends = ["APNG_BLEND_OP_SOURCE", "APNG_BLEND_OP_OVER"]

# https://wiki.mozilla.org/APNG_Specification#.60fcTL.60:_The_Frame_Control_Chunk
#
# `dispose_op` specifies how the output buffer should be changed at the end of the delay (before rendering the next frame).
# APNG_DISPOSE_OP_NONE: no disposal is done on this frame before rendering the next; the contents of the output buffer are left as is.
# APNG_DISPOSE_OP_BACKGROUND: the frame's region of the output buffer is to be cleared to fully transparent black before rendering the next frame.
# APNG_DISPOSE_OP_PREVIOUS: the frame's region of the output buffer is to be reverted to the previous contents before rendering the next frame.
# If the first `fcTL` chunk uses a `dispose_op` of APNG_DISPOSE_OP_PREVIOUS it should be treated as APNG_DISPOSE_OP_BACKGROUND.
#
# `blend_op` specifies whether the frame is to be alpha blended into the current output buffer content, or whether it should completely replace its region in the output buffer.
# APNG_BLEND_OP_SOURCE: replace frame region
# APNG_BLEND_OP_OVER: blend with frame

def load_apng(file):
    img = APNG.open(file)
    pilframes = []
    
    width, height = img.frames[0][0].width, img.frames[0][0].height
    outputbuf = Image.new("RGBA", (width, height), (255,255,255,0))
    prev_frame = None
    prev_frame_info = None

    for frame, frame_info in img.frames:
        i = img.frames.index((frame, frame_info))

        if prev_frame_info and prev_frame_info.depose_op == APNG_DISPOSE_OP_BACKGROUND:
            draw = ImageDraw.Draw(outputbuf)
            draw.rectangle((prev_frame_info.x_offset, prev_frame_info.y_offset, prev_frame_info.x_offset+prev_frame_info.width-1, prev_frame_info.y_offset+prev_frame_info.height-1), fill=(0, 0, 0, 0))
        elif prev_frame_info and prev_frame_info.depose_op == APNG_DISPOSE_OP_PREVIOUS:
            outputbuf.paste(prev_frame, (prev_frame_info.x_offset, prev_frame_info.y_offset))

        pilframe = Image.open(io.BytesIO(frame.to_bytes())).convert("RGBA")
        prev_frame_info = frame_info

        if frame_info:
            if frame_info.depose_op == APNG_DISPOSE_OP_PREVIOUS:
                prev_frame = outputbuf.crop((frame_info.x_offset, frame_info.y_offset, frame_info.x_offset+frame_info.width+1, frame_info.y_offset+frame_info.height+1))
            if frame_info.blend_op == APNG_BLEND_OP_SOURCE:
                draw = ImageDraw.Draw(outputbuf)
                draw.rectangle((frame_info.x_offset, frame_info.y_offset, frame_info.x_offset+frame_info.width-1, frame_info.y_offset+frame_info.height-1), fill=(0, 0, 0, 0))
        
        outputbuf.paste(pilframe, (frame_info.x_offset, frame_info.y_offset) if frame_info else (0,0), pilframe)

        if frame_info:
            pilframes.append([outputbuf.copy(), int(frame_info.delay / float(frame_info.delay_den) * 1000)]) # convert delay to milliseconds
        else:
            pilframes.append([outputbuf.copy(), 0])

    return pilframes

def load_webp(file):
    img = Image.open(file)
    frames = []

    for i in range(img.n_frames):
        img.seek(i)
        img.load() # strange thing with Pillow and animated webp's is that the img.info dictionary attr doesn't update unless you call a function like this
        frames.append([img.copy(), img.info["duration"] if "duration" in img.info else 0])

    return frames

def get_apng_duration(file):
    img = APNG.open(file)
    dur = 0

    for frame, frame_info in img.frames:
        if frame_info: dur += int(frame_info.delay / float(frame_info.delay_den) * 1000) # convert delay to milliseconds

    return dur

def get_webp_duration(file):
    img = Image.open(file)
    dur = 0

    for i in range(img.n_frames):
        img.seek(i)
        img.load() # strange thing with Pillow and animated webp's is that the img.info dictionary attr doesn't update unless you call a function like this
        dur += img.info["duration"]

    return dur