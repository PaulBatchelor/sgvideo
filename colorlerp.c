/*
 * Copyright (c) 2021 Muvik Labs, LLC
 * Distributed under the MIT license.
 */

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "colorlerp.h"

#include "gammatables.h"

#define FAST_GAMMA

static float ungammait(float u)
{
#ifdef FAST_GAMMA
    float y;
    int ipos;
    float fpos;
    y = 0;

    fpos = u * (GAMMATABLE_SIZE - 1);

    ipos = floor(fpos);
    fpos = fpos - ipos;

    y = (1 - fpos) * ungammatable[ipos] + fpos * ungammatable[ipos + 1];
    return y;
#else
    float y;
    y = 0;

    if (u <= 0.04045) {
        y = u / 12.92;
    } else {
        y = pow((u + 0.055)/1.055, 2.4);
    }
    return y;
#endif
}

static float gammait(float u)
{
#ifdef FAST_GAMMA
    float y;
    int ipos;
    float fpos;
    y = 0;

    fpos = u * (GAMMATABLE_SIZE - 1);

    ipos = floor(fpos);
    fpos = fpos - ipos;

    y = (1 - fpos) * gammatable[ipos] + fpos * gammatable[ipos + 1];
    return y;
#else
    float y;
    y = 0;

    if (u <= 0.0031308) {
        y = u * 12.92;
    } else {
        y = pow(u, 1.0/2.4) * 1.055 - 0.055;
    }

    return y;
#endif
}

/* used to generate lookup tables */

void sg_gammatables(int sz)
{
    FILE *fp;
    int i;
    fp = stdout;

    fprintf(fp, "#define GAMMATABLE_SIZE %d\n", sz);
    fprintf(fp, "static const float gammatable[] = {");
    for (i = 0; i < sz; i++) {
        fprintf(fp, "%g, ", gammait((float)i/sz));
    }
    fprintf(fp, "};");

    fprintf(fp, "static const float ungammatable[] = {");
    for (i = 0; i < sz; i++) {
        fprintf(fp, "%g, ", ungammait((float)i/sz));
    }
    fprintf(fp, "};");
}

void sg_colorlerp(uint8_t c0_r, uint8_t c0_g, uint8_t c0_b,
                  uint8_t c1_r, uint8_t c1_g, uint8_t c1_b,
                  float a,
                  uint8_t *out_r, uint8_t *out_g, uint8_t *out_b)
{
    float c0_n[3];
    float c1_n[3];
    float out_n[3];
    static float oned255 = 1.0 / 255.0;

    if (a == 0) {
        *out_r = c0_r;
        *out_g = c0_g;
        *out_b = c0_b;
        return;
    }

    if (a == 1) {
        *out_r = c1_r;
        *out_g = c1_g;
        *out_b = c1_b;
        return;
    }


    c0_n[0] = c0_r * oned255;
    c0_n[1] = c0_g * oned255;
    c0_n[2] = c0_b * oned255;
    c1_n[0] = c1_r * oned255;
    c1_n[1] = c1_g * oned255;
    c1_n[2] = c1_b * oned255;

    c0_n[0] = ungammait(c0_n[0]);
    c0_n[1] = ungammait(c0_n[1]);
    c0_n[2] = ungammait(c0_n[2]);
    c1_n[0] = ungammait(c1_n[0]);
    c1_n[1] = ungammait(c1_n[1]);
    c1_n[2] = ungammait(c1_n[2]);

    out_n[0] = (1 - a) * c0_n[0] + a * c1_n[0];
    out_n[1] = (1 - a) * c0_n[1] + a * c1_n[1];
    out_n[2] = (1 - a) * c0_n[2] + a * c1_n[2];
    out_n[0] = gammait(out_n[0]);
    out_n[1] = gammait(out_n[1]);
    out_n[2] = gammait(out_n[2]);

    if (out_r != NULL) *out_r = floor(out_n[0] * 255);
    if (out_g != NULL) *out_g = floor(out_n[1] * 255);
    if (out_b != NULL) *out_b = floor(out_n[2] * 255);
}

void sg_colorlerp_lin(uint8_t c0_r, uint8_t c0_g, uint8_t c0_b,
                      uint8_t c1_r, uint8_t c1_g, uint8_t c1_b,
                      float a,
                      uint8_t *out_r, uint8_t *out_g, uint8_t *out_b)
{
    *out_r = floor((1 - a) * c0_r + a * c1_r);
    *out_g = floor((1 - a) * c0_g + a * c1_g);
    *out_b = floor((1 - a) * c0_b + a * c1_b);
}
