local vid = sgvideo

function setcolor(v, c)
    vid.color(v, c[1], c[2], c[3], 1)
end

function rgb(r, g, b)
    return {r / 0xff, g / 0xff, b / 0xff}
end

function setup_font(v)
    local font = vid.text_add_font(v, "bold", "font/Roboto-Bold.ttf")
    vid.text_setfont(v, font)

    local ALIGN_LEFT = 1
    local ALIGN_CENTER = 2
    local ALIGN_RIGHT= 4
    local ALIGN_TOP = 8
    local ALIGN_MIDDLE = 16
    local ALIGN_BOTTOM = 32
    local ALIGN_BASELINE = 64

    vid.text_clearstate(v)
    vid.text_setalign(v, ALIGN_CENTER | ALIGN_MIDDLE)
    local txtsize = height * 0.1
    vid.text_setsize(v, txtsize)
    vid.text_setcolor(v, vid.text_rgba(0xff, 0xff, 0xff, 0xff))
end


blue = {0x3b/255, 0x42/255, 0x52/255}
white = {1, 1, 1}

width = 320
height = 240
fps = 30
dur = 5 -- seconds
nframes = dur * fps

v = vid.new()
vid.open(v, "star.h264", width, height, 30)
setup_font(v)
vid.unshade_init(v)

periwink = rgb(0x7f, 0xa9, 0xfd)
sunless = rgb(0x09, 0x02, 0x1d)

--nframes = 1

timescale = 1/(fps * 2.5)
vid.unshade_clear(v, sunless[1], sunless[2], sunless[3])


local a, d, lh = vid.text_vertmetrics(v)
local tx = width / 2
local ty = height - lh

print(tx, ty)
for n=1,nframes do
    --radius = (1 + math.sin(2 * math.pi * n *timescale)) * 0.5
    radius = 1
    if ((n - 1) % fps == 0) then print((n - 1)) end
    vid.star(v, periwink, sunless, {0.4, 0.4, 0.4}, radius)
    vid.unshade_transfer(v)
    vid.text_draw(v, tx, ty, "Hello Muvik!")
    vid.append(v)
end

vid.close(v)
vid.del(v)

os.execute("ffmpeg -y -i star.h264 -vf format=yuv420p star.mp4")