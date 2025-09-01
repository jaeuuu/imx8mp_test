import gi, sys, argparse
gi.require_version("Gst", "1.0")
from gi.repository import Gst, GObject

parser = argparse.ArgumentParser()
parser.add_argument("--volume", type=float, default=0.1, help="Volume")
parser.add_argument("--freq", type=int, default=1000, help="Frequency")
args = parser.parse_args()

Gst.init(None)

# PIPELINE = f"""
# alsasrc device=plughw:dir9001audio,0 !
#   audio/x-raw,rate=96000,channels=1,format=S32LE ! volume volume={args.volume} !
#   tee name=t
#     t. ! queue ! audioconvert !
#         wavescope shader=0 style=0 !
#         videoconvert ! autovideosink sync=false
#     t. ! queue ! audioconvert !
#         level interval=50000000 post-messages=true ! fakesink
# """

PIPELINE = f"""
audiotestsrc wave=sine freq={args.freq} is-live=true !
  audio/x-raw,rate=48000,channels=2,format=S16LE ! volume volume={args.volume} !
  tee name=t
    t. ! queue ! audioconvert !
        wavescope shader=0 style=0 !
        videoconvert ! autovideosink sync=false
    t. ! queue ! audioconvert !
        level interval=50000000 post-messages=true ! fakesink
"""


#print("pipeline = ", PIPELINE)

pipe = Gst.parse_launch(PIPELINE)
bus = pipe.get_bus()
bus.add_signal_watch()

def bar(db, width=60, min_db=-60.0, max_db=0.0):
    rel = (db - min_db) / (max_db - min_db)
    n = max(0, min(width, int(rel * width)))
    return "#"*n + " "*(width-n)

def on_msg(bus, msg):
    if msg.type == Gst.MessageType.ELEMENT and msg.has_name("level"):
        s = msg.get_structure()
        rms  = s.get_value("rms")
        peak = s.get_value("peak")
        # 화면 지우고 커서를 맨 위로 (ANSI escape)
        line = []
        for ch in range(len(rms)):
            rms_bar  = bar(rms[ch])
            peak_bar = bar(peak[ch])
            line.append(f"CH{ch} RMS {rms[ch]:6.1f} dB {rms_bar} PEAK {peak[ch]:6.1f} dB {peak_bar}")

        sys.stdout.write("\r" + " | ".join(line))
        sys.stdout.flush()

bus.connect("message", on_msg)

pipe.set_state(Gst.State.PLAYING)
loop = GObject.MainLoop()
try:
    loop.run()
except KeyboardInterrupt:
    pass
pipe.set_state(Gst.State.NULL)