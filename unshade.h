#ifndef UNSHADE_H
#define UNSHADE_H

#ifdef UNSHADE_SHORTCUTS
#define vec2 us_vec2
#define vec3 us_vec3
#define vec4 us_vec4
#define sign us_sign
#define length2 us_length2
#define clamp us_clamp
#define dot us_dot
#define add2sv us_add2sv
#define add3 us_add3
#define sub2 us_sub2
#define sub3 us_sub3
#define sub2vs us_sub2vs
#define sub3sv us_sub3sv
#define mul2s us_mul2s
#define mul3s us_mul3s
#define mul3 us_mul3
#define div2 us_div2
#define div2vs us_div2vs
#define mkvec2 us_mkvec2
#define mkvec3 us_mkvec3
#define min us_min
#define min3 us_min3
#define max us_max
#define max2 us_max2
#define max3 us_max3
#define mix us_mix
#define mix3 us_mix3
#define normalize2 us_normalize2
#define smoothstep us_smoothstep
#endif

/* vector types */

typedef struct {
    float x;
    float y;
} us_vec2;

typedef struct {
    float x;
    float y;
    float z;
} us_vec3;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} us_vec4;

/* image data for shader-code like structures */
typedef struct {
    us_vec2 iResolution;
    float iTime;
    void *ud;
} us_image_data;


us_vec2 us_mkvec2(float x, float y);
us_vec3 us_mkvec3(float x, float y, float z);

float us_length2(us_vec2 v);

float us_clamp(float x, float mn, float mx);
us_vec3 us_clamp3(us_vec3 x, float mn, float mx);
float us_dot(us_vec2 a, us_vec2 b);

us_vec3 us_add3(us_vec3 a, us_vec3 b);
us_vec2 us_add2sv(float s, us_vec2 v);

us_vec2 us_sub2(us_vec2 a, us_vec2 b);
us_vec3 us_sub3(us_vec3 a, us_vec3 b);

us_vec2 us_sub2vs(us_vec2 a, float s);
us_vec3 us_sub3sv(float s, us_vec3 v);

us_vec2 us_mul2s(us_vec2 v, float s);
us_vec3 us_mul3s(us_vec3 v, float s);
us_vec3 us_mul3(us_vec3 a, us_vec3 b);

us_vec2 us_div2vs(us_vec2 v, float s);
us_vec2 us_div2(us_vec2 a, us_vec2 b);


float us_min(float x, float y);
us_vec3 us_min3(us_vec3 v, float s);
float us_max(float x, float y);
us_vec2 us_max2(us_vec2 v, float s);
us_vec3 us_max3(us_vec3 v, float s);

float us_mix(float x, float y, float a);
us_vec3 us_mix3(us_vec3 x, us_vec3 y, float a);

us_vec2 us_normalize2(us_vec2 v);

void us_draw(us_vec3 *buf,
             us_vec2 res,
             int frame,
             int fps,
             void (*draw)(us_vec3 *, us_vec2, us_image_data *),
             void *ud);

void us_write_ppm(us_vec3 *buf, us_vec2 res, const char *filename);

float us_radians(float deg);

float us_smoothstep(float e0, float e1, float x);

float us_sign(float x);
#endif
