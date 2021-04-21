/*
 * Copyright (c) 2021 Muvik Labs, LLC
 * Distributed under the MIT license.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define UNSHADE_SHORTCUTS
#include "video.h"

struct fill_stuff {
    vec3 color;
    float alpha;
};

static void draw(vec3 *fragColor, vec2 fragCoord, us_image_data *id)
{
    struct fill_stuff *fs;

    fs = id->ud;

    if (fs->alpha >= 0) {
        *fragColor = mix3(*fragColor, fs->color, fs->alpha);
    } else {
        *fragColor = fs->color;
    }
}

void sg_video_unshade_fill(sg_video *v,
                           us_vec3 color,
                           float alpha)
{
    int w, h;
    int fps;
    int frame;
    struct fill_stuff fs;
    vec3 *buf;

    sg_video_dims(v, &w, &h);

    buf = sg_video_unshadebuf(v);

    fs.color = color;
    fs.alpha = alpha;

    fps = sg_video_fps(v);
    frame = sg_video_framepos(v);
    us_draw(buf, mkvec2(w, h), frame, fps, draw, &fs);
}
