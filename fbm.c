/*
 * Copyright (c) 2021 Muvik Labs, LLC
 * Distributed under the MIT license.
 */

#include <math.h>
#include <stdio.h>

float sg_simplex(float x, float y);

typedef struct {
    float x, y;
} vec2;

static vec2 mkvec2(float x, float y)
{
    vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

static float dot(vec2 a, vec2 b)
{
    return a.x*b.x + a.y*b.y;
}

static vec2 muls(vec2 a, float s)
{
    vec2 out;
    out.x = a.x * s;
    out.y = a.y * s;
    return out;
}

static vec2 mul(vec2 a, vec2 b)
{
    vec2 out;
    out.x = a.x * b.x;
    out.y = a.y * b.y;
    return out;
}

static vec2 add(vec2 a, vec2 b)
{
    vec2 out;

    out.x = a.x + b.x;
    out.y = a.y + b.y;

    return out;
}

static vec2 adds(vec2 a, float s)
{
    vec2 out;

    out.x = a.x + s;
    out.y = a.y + s;

    return out;
}

static float fract(float x)
{
    return x - floorf(x);
}

static float mysin(vec2 st)
{
    return sinf(dot(st, mkvec2(12.9898,78.233)));
}

static float random(vec2 st)
{
    return sg_simplex(st.x, st.y);
}

static float mix(float x, float y, float a)
{
    return x * (1 -a) + y * a;
}

static float noise(vec2 st)
{
    vec2 i;
    vec2 f;

    float a, b, c, d;

    vec2 u;

    i.x = floorf(st.x);
    i.y = floorf(st.y);

    /* TODO: derive from i instead of fract */
    /* f.x = fract(st.x); */
    /* f.y = fract(st.y); */
    f.x = st.x - i.x;
    f.y = st.y - i.y;

    a = random(i);
    b = random(add(i, mkvec2(1.0, 0.0)));
    c = random(add(i, mkvec2(0.0, 1.0)));
    d = random(add(i, mkvec2(1.0, 1.0)));

    /* f * f * (3.0 - 2.0 * f) */
    /* f * f */
    u = mul(f, f);
    /* -2f + (3) */
    u = mul(u, adds(muls(f, -2), 3));
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

float sg_fbm(float x, float y, int oct)
{
    vec2 st;
    float value;
    float amplitude;
    int i;

    st = mkvec2(x, y);
    value = 0;
    amplitude = 0.5;

    for (i = 0; i < oct; i++) {
        value += amplitude * noise(st);
        st = muls(st, 2);
        amplitude *= 0.6;
    }

    return value;
}
