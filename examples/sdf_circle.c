#include <math.h>
#include <stdlib.h>

#define UNSHADE_SHORTCUTS
#include "unshade.h"

/* https://www.shadertoy.com/view/3ltSW2 */

float sdCircle(vec2 p, float r)
{
    return length2(p) - r;
}


static void draw(vec3 *fragColor, vec2 fragCoord, us_image_data *id)
{
    vec2 p;
    float d;
    vec3 col;
    vec3 white, blue;
    float alpha;

    p = mul2s(fragCoord, 2.0);
    p = sub2(p, id->iResolution);
    p = div2vs(p, id->iResolution.y);

    d = -sdCircle(p, 0.5);

    /* col = mkvec3(1., 1., 1.); */
    /* col = sub3(col, mul3s(mkvec3(0., 0.25, 0.25), sign(d))); */

    alpha = 0;
    alpha = sign(d) > 0;
    alpha += smoothstep(0.01, 0.0, fabs(d));
    alpha = clamp(alpha, 0, 1);

    white = mkvec3(1., 1., 1.);
    blue = mkvec3(0., 0.25, 0.25);
    col = mix3(blue, white, alpha);

    *fragColor = col;
}

int main(int argc, char *argv[])
{
    vec3 *buf;
    int width, height;
    vec2 res;

    width = 640;
    height = 480;

    res = mkvec2(width, height);

    buf = malloc(width * height * sizeof(vec3));

    us_draw(buf, res, 0, 60, draw, NULL);

    us_write_ppm(buf, res, "circle.ppm");

    free(buf);
    return 0;
}
