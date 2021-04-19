/*
 * Copyright (c) 2021 Muvik Labs, LLC
 * Distributed under the MIT license.
 *
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define UNSHADE_SHORTCUTS
#include "video.h"

typedef struct {
    vec3 color;
    vec3 tint;
    vec3 bg;
    float radius;
    int count;
} star_stuff;

static void draw(vec3 *fragColor, vec2 fragCoord, us_image_data *id)
{
    vec3 c;
    float t;
    float f;
    float phase;
    float dir;
    float a;
    float len;
    float d;
    int i;
    vec2 uv;
    vec2 uv_n;
    star_stuff *ss;

    ss = id->ud;

    t = id->iTime * 6.28f * 0.04f;

    fragCoord = sub2(fragCoord, mul2s(id->iResolution, 0.5));

    c = mkvec3(0.f, 0.f, 0.25f);

    uv = div2(fragCoord, mkvec2(id->iResolution.y, id->iResolution.y));
    uv = mul2s(uv, 15.f);
    uv_n = normalize2(uv);

    d = length2(uv) * (0.5 + 1. * (1 - ss->radius));

    f = 0.;

    phase = t;
    dir = 1.;
    a = 0.;
    len = -d*(0.4f*ss->radius);

    for (i = 0; i < ss->count; i++) {
        float p;
        p = phase +(sinf(i+t)-1.)*.05+len;
        a = dot(uv_n,
                normalize2(mkvec2(cosf((p)*dir), sinf((p)*dir))));
        a = max(0.f, a);
        a = powf(a, 10.f);
        dir *= -1;
        phase += fmodf((float)i, 6.28f);
        f += a;
        f = fabsf(fmodf(f + 1.f, 2.f)-1.f);
    }

    f+=1.7-d*(.7+sinf(t+dot(uv_n, mkvec2(1.f, 0.f))*11.f)*(.02f + (1.f - ss->radius)*0.2f));
    f = max(f, 0.f);
    c = mix3(ss->bg, ss->color, f);
    c = sub3sv(1.f,
               mul3(mul3s(ss->tint, 3.f),
               sub3sv(1.0, c))
        );

    c = min3(max3(c, 0.f), 1.f);

    *fragColor = c;
}

void sg_video_star(sg_video *v,
                   us_vec3 color,
                   us_vec3 bg,
                   us_vec3 tint,
                   float radius,
                   int count)
{
    int w, h;
    int fps;
    int frame;
    star_stuff ss;
    vec3 *buf;

    sg_video_dims(v, &w, &h);

    buf = sg_video_unshadebuf(v);

    ss.color = color;
    ss.bg = bg;
    ss.tint = tint;
    ss.radius = radius;
    ss.count = count;

    fps = sg_video_fps(v);
    frame = sg_video_framepos(v);
    us_draw(buf, mkvec2(w, h), frame, fps, draw, &ss);
}

static vec3 rgb(int r, int g, int b)
{
    float oned255 = 1.f / 255.f;
    return mkvec3(r * oned255, g*oned255, b*oned255);
}

void sg_video_startest(sg_video *v)
{

    vec3 color;
    vec3 bg;
    vec3 tint;

    /* periwink/sunless */
    color = rgb(0x7f, 0xa9, 0xfd);
    bg = rgb(0x09, 0x02, 0x1d);

    tint = mkvec3(0.4f, 0.4f, 0.4f);

    sg_video_star(v, color, bg, tint, 1, 48);
}
