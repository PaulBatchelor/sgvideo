local vid = sgvideo

blue = {0x3b/255, 0x42/255, 0x52/255}
white = {1, 1, 1}

width = 320
height = 240
fps = 30
dur = 6 -- seconds
nframes = dur * fps

v = vid.new()
vid.open(v, "test.h264", width, height, 30)

function setcolor(v, c)
    vid.color(v, c[1], c[2], c[3], 1)
end

for n=1,nframes do
    scale = (1 + math.sin(2 * math.pi * (n / fps) * 0.2)) * 0.5
    setcolor(v, blue)
    vid.paint(v)
    setcolor(v, white)
    vid.circ(v, width/2, height/2, 10 + 60*scale)
    vid.fill(v)
    vid.append(v)
end

vid.close(v)
vid.del(v)

os.execute("ffmpeg -y -i test.h264 -vf format=yuv420p test.mp4")