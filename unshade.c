/*
 * Copyright (c) 2021 Muvik Labs, LLC
 * Distributed under the MIT license.
 */

#include <math.h>
#include <stdlib.h>
#include <pthread.h>

#include "unshade.h"

us_vec2 us_mkvec2(float x, float y)
{
    us_vec2 p;
    p.x = x;
    p.y = y;
    return p;
}

us_vec3 us_mkvec3(float x, float y, float z)
{
    us_vec3 p;
    p.x = x;
    p.y = y;
    p.z = z;
    return p;
}

float us_length2(us_vec2 v)
{
    return sqrt(v.x*v.x + v.y*v.y);
}

float us_clamp(float x, float mn, float mx)
{
    if (x < mn) return mn;
    if (x > mx) return mx;
    return x;
}

float us_dot(us_vec2 a, us_vec2 b)
{
    return a.x*b.x + a.y*b.y;
}

us_vec2 us_sub2(us_vec2 a, us_vec2 b)
{
    us_vec2 out;

    out.x = a.x - b.x;
    out.y = a.y - b.y;

    return out;
}

us_vec2 us_sub2vs(us_vec2 v, float s)
{
    us_vec2 out;

    out.x = v.x - s;
    out.y = v.y - s;

    return out;
}

us_vec3 us_sub3sv(float s, us_vec3 v)
{
    us_vec3 out;

    out.x = s - v.x;
    out.y = s - v.y;
    out.z = s - v.z;

    return out;
}

float us_min(float x, float y)
{
    return x < y ? x : y;
}

us_vec3 us_min3(us_vec3 v, float s)
{
    us_vec3 out;

    out.x = us_min(v.x, s);
    out.y = us_min(v.y, s);
    out.z = us_min(v.z, s);

    return out;
}

float us_max(float x, float y)
{
    return x > y ? x : y;
}

us_vec2 us_max2(us_vec2 v, float s)
{
    us_vec2 out;

    out.x = us_max(v.x, s);
    out.y = us_max(v.y, s);

    return out;
}

us_vec3 us_max3(us_vec3 v, float s)
{
    us_vec3 out;

    out.x = us_max(v.x, s);
    out.y = us_max(v.y, s);
    out.z = us_max(v.z, s);

    return out;
}

float us_mix(float x, float y, float a)
{
    return (1-a)*x + a*y;
}

us_vec3 us_mix3(us_vec3 x, us_vec3 y, float a)
{
    us_vec3 out;

    out.x = us_mix(x.x, y.x, a);
    out.y = us_mix(x.y, y.y, a);
    out.z = us_mix(x.z, y.z, a);

    return out;
}

us_vec2 us_div2vs(us_vec2 v, float s)
{
    us_vec2 out;

    out.x = v.x / s;
    out.y = v.y / s;

    return out;
}

us_vec2 us_div2(us_vec2 a, us_vec2 b)
{
    us_vec2 out;

    out.x = a.x / b.x;
    out.y = a.y / b.y;

    return out;
}

us_vec2 us_normalize2(us_vec2 v)
{
    float l;
    us_vec2 out;

    l = us_length2(v);

    if (l == 0) return us_mkvec2(0, 0);

    out = us_div2vs(v, l);

    return out;
}

us_vec2 us_mul2s(us_vec2 v, float s)
{
    us_vec2 out;

    out.x = v.x * s;
    out.y = v.y * s;

    return out;
}

us_vec3 us_mul3s(us_vec3 v, float s)
{
    us_vec3 out;

    out.x = v.x * s;
    out.y = v.y * s;
    out.z = v.z * s;

    return out;
}

us_vec3 us_mul3(us_vec3 a, us_vec3 b)
{
    us_vec3 out;

    out.x = a.x * b.x;
    out.y = a.y * b.y;
    out.z = a.z * b.z;

    return out;
}

us_vec3 us_add3(us_vec3 a, us_vec3 b)
{
    us_vec3 out;

    out.x = a.x + b.x;
    out.y = a.y + b.y;
    out.z = a.z + b.z;

    return out;
}

us_vec2 us_add2sv(float s, us_vec2 v)
{
    us_vec2 out;

    out.x = s + v.x;
    out.y = s + v.y;

    return out;
}

#define US_MAXTHREADS 8

typedef struct {
    us_vec3 *buf;
    us_image_data *data;
    int off;
    void (*draw)(us_vec3 *, us_vec2, us_image_data *);
} thread_data;

void *draw_thread(void *arg)
{
    thread_data *td;
    us_image_data *data;
    int x, y;
    int w, h;
    us_vec3 *buf;
    int nthreads;

    td = arg;
    data = td->data;
    buf = td->buf;

    w = data->iResolution.x;
    h = data->iResolution.y;

    /* This is hard-coded for now */
    nthreads = US_MAXTHREADS;

    for (y = td->off; y < h; y+=nthreads) {
        for (x = 0; x < w; x++) {
            int pos;
            us_vec3 *c;
            pos = y*w+ x;
            c = &buf[pos];
            td->draw(c, us_mkvec2(x, y), data);
        }
    }

    return NULL;
}

void us_draw(us_vec3 *buf,
             us_vec2 res,
             int frame,
             int fps,
             void (*draw)(us_vec3 *, us_vec2, us_image_data *),
             void *ud)
{
    thread_data td[US_MAXTHREADS];
    pthread_t thread[US_MAXTHREADS];
    int t;
    us_image_data data;

    data.iResolution = res;
    data.iTime = (float)(frame)/fps;
    data.ud = ud;

    for (t = 0; t < US_MAXTHREADS; t++) {
        td[t].buf = buf;
        td[t].data = &data;
        td[t].off = t;
        td[t].draw = draw;
        pthread_create(&thread[t], NULL, draw_thread, &td[t]);
    }

    for (t = 0; t < US_MAXTHREADS; t++) {
        pthread_join(thread[t], NULL);
    }
}
