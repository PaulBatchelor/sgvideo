/*
 * Copyright (c) 2021 Muvik Labs, LLC
 * Distributed under the MIT license.
 */

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <x264.h>
#include <cairo/cairo.h>
#include <stdio.h>
#include <pthread.h>

#include "lodepng/lodepng.h"

#include "fontstash/fontstash.h"

#define SG_VIDEO_PRIVATE
#include "video.h"

#include "colorlerp.h"

/* included after video.h for sg_video def */
#include "fontstash/sgfontstash.h"

void sg_video_new(sg_video **pv)
{
    sg_video *v;
    v = calloc(1, sizeof(sg_video));
    v->fp = NULL;
    v->cairo_buf = NULL;
    v->fs = NULL;
    *pv = v;
    v->usbuf = NULL;
}

void sg_video_del(sg_video **pv)
{
    free(*pv);
}

void sg_video_cairo_init(sg_video *v, int w, int h)
{
    cairo_surface_t *surface;
    cairo_t *cr;
    int stride;
    int format;

    if (v->cairo_buf != NULL) {
        fprintf(stderr, "Cairo is already initialized!\n");
        return;
    }

    format = CAIRO_FORMAT_RGB24;

    stride = cairo_format_stride_for_width(format, w);
    v->cairo_buf = calloc(1, stride * h);
    v->stride = stride;
    surface = cairo_image_surface_create_for_data(
        (unsigned char *)v->cairo_buf,
        format,
        w, h,
        stride);

    cr = cairo_create(surface);

    v->cr = cr;
    v->surface = surface;
    v->width = w;
    v->height = h;

    /* turn on hi-res anti-aliasing by default */

    cairo_set_antialias(cr, CAIRO_ANTIALIAS_BEST);
}

void sg_video_fontstash_init(sg_video *v)
{
    if (v->fs != NULL) {
        fprintf(stderr,
                "Looks like fontstash is already created\n");
        return;
    }

    v->fs = sgfons_create(512, 512, FONS_ZERO_TOPLEFT, v);
}

void sg_video_open(sg_video *v,
                   const char *filename,
                   int w, int h,
                   int fps)
{
    if (v->fp != NULL) sg_video_close(v);

    v->fp = fopen(filename, "w");

    /* set up cairo */
    sg_video_cairo_init(v, w, h);

    /* set up fontstash */
    sg_video_fontstash_init(v);

    /* set up x264 */
    {

        unsigned int sz;
        unsigned int szd4;
        x264_param_t *p;

        sz = w * h;
        szd4 = sz/4;

        v->sz = sz;
        /* v->szd4 = szd4; */
        p = &v->param;

        v->i_frame = 0;
        v->ybuf = calloc(1, sz);
        /* v->ubuf = calloc(1, szd4); */
        /* v->vbuf = calloc(1, szd4); */
        v->ubuf = calloc(1, sz);
        v->vbuf = calloc(1, sz);

        if (x264_param_default_preset(p, "ultrafast", NULL) < 0)
            return;

        /* p->i_bitdepth = 8; */
        /* p->i_csp = X264_CSP_I420; */
        p->i_csp = X264_CSP_I444;
        p->rc.i_rc_method = X264_RC_CRF;
        p->rc.f_rf_constant_max = 2;
        p->i_width  = w;
        p->i_height = h;
        p->b_vfr_input = 0;
        p->b_repeat_headers = 1;
        p->b_annexb = 1;
        p->i_fps_num = fps;

        /* had to change threads to stop from crashing on Linux */
        p->i_threads = 1;
        p->i_lookahead_threads = 1;

        /* try to make bitrate 7.5 mbps */
        p->rc.i_bitrate = 7500;

        /* they say this means lossless */
        p->rc.i_qp_constant = 0;

        /* silence output */
        p->i_log_level = X264_LOG_NONE;

        if (x264_param_apply_profile(p, "high444") < 0 )
            return;

        if (x264_picture_alloc(&v->pic, p->i_csp, p->i_width, p->i_height) < 0 )
            return;

        v->h = x264_encoder_open(p);
        if (!h) return;
    }
}

/* source: https://www.fourcc.org/fccyvrgb.php */

void rgb2yuv(uint8_t r, uint8_t g, uint8_t b,
             uint8_t *y, uint8_t *u, uint8_t *v)
{
    double Ey;
    double Ecr;
    double Ecb;
    double norm;

    norm = 1.0/255;

    Ey = (0.299*r + 0.587*g + 0.114*b)*norm;
    Ecr = 0.713 * (r*norm - Ey);
    Ecb = 0.564 * (b*norm - Ey);


    *y = Ey * 255;
    *u = (0.5 + Ecb) * 255;
    *v = (0.5 + Ecr) * 255;
}

/* yuv function designed to be applied to cairo surfaces
 * This assumes the format is CAIRO_FORMAT_RGB24
 */

void cairo2yuv(uint32_t *pix,
               unsigned int w, unsigned int h,
               uint8_t *ybuf,
               uint8_t *ubuf,
               uint8_t *vbuf)
{
    unsigned int x, y;
    unsigned int pos;
    /* unsigned int posB; */
    uint8_t yv, uv, vv;
    unsigned char r, g, b;
    uint32_t tmp;

    pos = 0;
    /* posB = 0; */
    for(y = 0; y < h; y++) {
        for(x = 0; x < w; x++) {
            tmp = pix[y * w + x];

            b = tmp & 0xff;
            g = (tmp >> 8) & 0xff;
            r = (tmp >> 16) & 0xff;
            rgb2yuv(r, g, b, &yv, &uv, &vv);
            ybuf[pos] = yv;
            /* if(x % 2 == 0 && y % 2 == 0) { */
            /*     ubuf[posB] = uv; */
            /*     vbuf[posB] = vv; */
            /*     posB++; */
            /* } */
            ubuf[pos] = uv;
            vbuf[pos] = vv;

            pos++;
        }
    }
}

void sg_video_append(sg_video *v)
{
    int i_frame_size;

    cairo2yuv(v->cairo_buf,
              v->width, v->height,
              v->pic.img.plane[0],
              v->pic.img.plane[1],
              v->pic.img.plane[2]);

    v->pic.i_pts = v->i_frame;

    v->i_frame++;


    i_frame_size = x264_encoder_encode(v->h,
                                       &v->nal,
                                       &v->i_nal,
                                       &v->pic,
                                       &v->pic_out);

    if(i_frame_size < 0) return;
    else if(i_frame_size) {
        fwrite(v->nal->p_payload, i_frame_size, 1, v->fp);
    }
}

void sg_video_close(sg_video *v)
{
    /* cairo cleanup */
    if (v->cairo_buf != NULL) {
        cairo_destroy(v->cr);
        cairo_surface_destroy (v->surface);
        free(v->cairo_buf);
        v->cairo_buf = NULL;
    }

    /* x264 cleanup */
    if (v->fp != NULL) {
        int i_frame_size;
        while (x264_encoder_delayed_frames(v->h)) {
            i_frame_size = x264_encoder_encode(
                v->h,
                &v->nal,
                &v->i_nal,
                NULL,
                &v->pic_out);
            if (i_frame_size) {
                fwrite(v->nal->p_payload,
                       i_frame_size,
                       1,
                       v->fp);
            }
        }

        x264_encoder_close(v->h);
        x264_picture_clean(&v->pic);
        free(v->ybuf);
        free(v->ubuf);
        free(v->vbuf);
        fclose(v->fp);
        v->fp = NULL;
    }

    /* fontstash cleanup */
    if (v->fs != NULL) {
        sgfons_delete(v->fs);
        v->fs = NULL;
    }

    /* unshade cleanup */

    if (v->usbuf != NULL) {
        free(v->usbuf);
        v->usbuf = NULL;
    }
}

void sg_video_color(sg_video *v,
                    float r,
                    float g,
                    float b,
                    float a)
{
    cairo_t *cr;

    cr = v->cr;
    cairo_set_source_rgba(cr, r, g, b, a);
}

void sg_video_paint(sg_video *v)
{
    cairo_t *cr;

    cr = v->cr;
    cairo_paint(cr);
}

void sg_video_fill(sg_video *v)
{

    cairo_t *cr;

    cr = v->cr;
    cairo_fill(cr);
}

void sg_video_circle(sg_video *v,
                     float cx,
                     float cy,
                     float r)
{

    cairo_t *cr;

    cr = v->cr;
    cairo_arc(cr, cx, cy, r, 0, 2 * M_PI);
}

void sg_video_font_size(sg_video *v, int size)
{
    cairo_t *cr;

    cr = v->cr;
    cairo_set_font_size(cr, size);
}

void sg_video_font_face(sg_video *v, const char *face)
{
    cairo_t *cr;

    cr = v->cr;
    cairo_select_font_face(cr,
                           face,
                           CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_BOLD);

}

void sg_video_text(sg_video *v,
                   const char *txt,
                   float x, float y)
{
    cairo_t *cr;

    cr = v->cr;
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, txt);
}

void sg_video_text_extents(sg_video *v,
                           const char *text,
                           sg_text_extents *e)
{
    cairo_text_extents_t ce;
    cairo_t *cr;

    cr = v->cr;
    cairo_text_extents(cr, text, &ce);

    e->x_bearing = ce.x_bearing;
    e->y_bearing = ce.x_bearing;
    e->width = ce.width;
    e->height = ce.height;
    e->x_advance = ce.x_advance;
    e->y_advance = ce.y_advance;
}

int sg_image_new(sg_image **pimg, const char *filename)
{
    sg_image *img;
    int rc;
    img = calloc(1, sizeof(sg_image));

    rc = lodepng_decode32_file(&img->img,
                               &img->w,
                               &img->h,
                               filename);
    if (rc) {
        fprintf(stderr,
                "error %u: %s\n",
                rc,
                lodepng_error_text(rc));
        return 0;
    }
    *pimg = img;

    return 1;
}

void sg_image_dims(sg_image *i, int *w, int *h)
{
    *w = i->w;
    *h = i->h;
}

void sg_image_del(sg_image **pimg)
{
    sg_image *img;

    img = *pimg;

    free(img->img);
    free(img);
}

/* transfers lodepng image RGBA block to cairo ARGB block */

void sg_video_image(sg_video *v,
                    sg_image *i,
                    float x_pos,
                    float y_pos)
{

    int width;
    int height;
    int x, y;
    unsigned int ix, iy;
    uint32_t *cairo_buf;
    unsigned char *img_buf;
    uint32_t val;
    unsigned int pos;

    width = i->w;
    height = i->h;

    cairo_buf = v->cairo_buf;
    img_buf = i->img;

    ix = (unsigned int)x_pos;
    iy = (unsigned int)y_pos;
    /* printf(">>ix %d, iy %d\n", ix, iy); */

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            pos = (y * i->w * 4) + x * 4;
            val = 0;

            /* blue */
            val |= img_buf[pos + 2];
            /* green */
            val |= (img_buf[pos + 1] << 8);
            /* red */
            val |= (img_buf[pos] << 16);

            /* TODO: handle transparency */
            val |= 255 << 24;

            /* pos = v->stride * (iy + y) + x; */
            pos = v->width * (iy + y) + (ix + x);
            cairo_buf[pos] = val;
        }
    }
}

static void getpixel(sg_video *v, int x, int y,
                     uint8_t *r, uint8_t *g, uint8_t *b)
{
    int bufpos;
    uint32_t clr;

    *r = *g = *b = 0;

    bufpos = v->width * y + x;
    clr = v->cairo_buf[bufpos];
    *r = (clr >> 16) & 0xff;
    *g = (clr >> 8) & 0xff;
    *b = (clr) & 0xff;
}

static void setpixel(sg_video *v, int x, int y,
                     uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t clr;
    int bufpos;

    clr = 0;
    bufpos = v->width * y + x;

    clr |= b;
    clr |= (g << 8);
    clr |= (r << 16);
    clr |= 255 << 24;

    v->cairo_buf[bufpos] = clr;
}

float sg_fbm(float x, float y, int oct);

struct fbmstrip {
    sg_video *v;
    int r, g, b;
    int noct;
    float t;
    int yoff;
    int size;
};

static void *render_strip(void *ptr)
{
    int x, y;
    sg_video *v;
    int r, g, b, noct;
    float t;
    float iw, ih;
    struct fbmstrip *s;

    s = ptr;
    v = s->v;
    r = s->r;
    g = s->g;
    b = s->b;
    noct = s->noct;
    t = s->t;

    iw = 1.0 / v->width;
    ih = 1.0 / v->height;

    for (y = 0; y < s->size; y++) {
        for (x = 0; x < v->width; x++) {
            uint8_t clr[3];
            float xn, yn;
            float qx, qy;
            float rx, ry;
            float a;
            int ypos;

            ypos = y + s->yoff;

            xn = (float)x * iw;
            yn = (float)(ypos) * ih;
            xn = xn * ((float)v->width * ih);

            xn *= 4.f;
            yn *= 4.f;

            qx = sg_fbm(xn + t, yn + t, noct);
            qy = sg_fbm(xn + 2.f, yn + 1.f, noct);

            rx = sg_fbm(xn + qx + 1.7f + (t * 0.15f),
                        yn + qy + 9.2f + (t * 0.15f),
                        noct);

            ry = sg_fbm(xn + qx + 8.3f + (t * 0.126f),
                        yn + qy + 2.8f + (t * 0.3f),
                        noct);

            a = sg_fbm(xn + rx, yn + ry, noct);

            /* clamp! */
            if (a < 0) a = 0;
            if (a > 1) a = 1;

            getpixel(v, x, ypos, &clr[0], &clr[1], &clr[2]);
            sg_colorlerp(r, g, b,
                         clr[0], clr[1], clr[2], a,
                         &clr[0], &clr[1], &clr[2]);
            setpixel(v, x, ypos, clr[0], clr[1], clr[2]);
        }
    }

    return NULL;
}

#define NTHREADS 8

void sg_video_fbm(sg_video *v, int r, int g, int b, int noct, float t)
{
    struct fbmstrip s[NTHREADS];
    pthread_t th[NTHREADS];
    int size;
    int i;

    size = v->height / NTHREADS;

    for (i = 0; i < NTHREADS; i++) {
        s[i].v = v;
        s[i].r = r;
        s[i].g = g;
        s[i].b = b;
        s[i].noct = noct;
        s[i].t = t;
        s[i].size = size;
        s[i].yoff = i * size;
    }

    for (i = 0; i < NTHREADS; i++) {
        pthread_create(&th[i], NULL, render_strip, &s[i]);
    }
    for (i = 0; i < NTHREADS; i++) {
        pthread_join(th[i], NULL);
    }

    /* for (i = 0; i < NTHREADS; i++) { */
    /*     render_strip((void *)&s[i]); */
    /* } */
}

#undef NTHREADS

void sg_video_image_withalpha(sg_video *v,
                              sg_image *i,
                              float x_pos,
                              float y_pos,
                              float alpha)
{

    int width;
    int height;
    int x, y;
    unsigned int ix, iy;
    uint32_t *cairo_buf;
    unsigned char *img_buf;
    uint32_t val;
    unsigned int imgpos, bufpos;

    width = i->w;
    height = i->h;

    cairo_buf = v->cairo_buf;
    img_buf = i->img;

    ix = (unsigned int)x_pos;
    iy = (unsigned int)y_pos;

    /* printf("ix %d, iy %d\n", ix, iy); */

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            uint8_t in[3];
            uint8_t out[3];
            uint8_t rgb[3];
            uint32_t clr;

            bufpos = v->width * (iy + y) + (ix + x);
            imgpos = (y * i->w * 4) + x * 4;
            val = 0;

            clr = cairo_buf[bufpos];

            in[0] = (clr >> 16) & 0xff;
            in[1] = (clr >> 8) & 0xff;
            in[2] = (clr) & 0xff;

            out[0] = img_buf[imgpos];
            out[1] = img_buf[imgpos + 1];
            out[2] = img_buf[imgpos + 2];

            sg_colorlerp_lin(in[0], in[1], in[2], out[0], out[1], out[2], alpha,
                         &rgb[0], &rgb[1], &rgb[2]);

            /* blue */
            val |= rgb[2];
            /* green */
            val |= (rgb[1] << 8);
            /* red */
            val |= (rgb[0] << 16);

            val |= 255 << 24;

            /* pos = v->stride * (iy + y) + x; */
            cairo_buf[bufpos] = val;
        }
    }
}

void sg_video_stencil(sg_video *v,
                      sg_image *i,
                      float x_pos,
                      float y_pos,
                      int r, int g, int b,
                      double alpha)
{

    int width;
    int height;
    int x, y;
    unsigned int ix, iy;
    uint32_t *cairo_buf;
    unsigned char *img_buf;
    uint32_t val;
    unsigned int pos;
    uint32_t clr;
    uint8_t bg[3];
    uint8_t rgb[3];
    double oned255;

    width = i->w;
    height = i->h;

    cairo_buf = v->cairo_buf;
    img_buf = i->img;

    ix = (unsigned int)x_pos;
    iy = (unsigned int)y_pos;

    oned255 = 1.0/255.0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            double amt;
            pos = (y * i->w * 4) + x * 4;
            val = 0;

            amt = img_buf[pos] * oned255 * alpha;

            pos = v->width * (iy + y) + (ix + x);
            clr = cairo_buf[pos];

            bg[0] = (clr >> 16) & 0xff;
            bg[1] = (clr >> 8) & 0xff;
            bg[2] = (clr) & 0xff;


            /* rgb[0] = floor(amt * r + (1 - amt) * bg[0]); */
            /* rgb[1] = floor(amt * g + (1 - amt) * bg[1]); */
            /* rgb[2] = floor(amt * b + (1 - amt) * bg[2]); */

            sg_colorlerp(bg[0], bg[1], bg[2], r, g, b, amt,
                         &rgb[0], &rgb[1], &rgb[2]);

            /* blue */
            val |= rgb[2];
            /* green */
            val |= (rgb[1] << 8);
            /* red */
            val |= (rgb[0] << 16);

            val |= 255 << 24;

            cairo_buf[pos] = val;
        }
    }
}

void sg_video_scale(sg_video *v, float sx, float sy)
{
    cairo_t *cr;

    cr = v->cr;

    cairo_scale(cr, sx, sy);
}

void sg_video_rect(sg_video *v, float x, float y, float w, float h)
{
    cairo_t *cr;

    cr = v->cr;
    cairo_rectangle(cr, x, y, w, h);
}

void sg_video_arc(sg_video *v,
                  float cx,
                  float cy,
                  float r,
                  float a1,
                  float a2)
{
    cairo_t *cr;

    cr = v->cr;
    cairo_arc(cr, cx, cy, r, a1, a2);
    cairo_close_path(cr);
}

void sg_video_arc_neg(sg_video *v,
                      float cx,
                      float cy,
                      float r,
                      float a1,
                      float a2)
{
    cairo_t *cr;

    cr = v->cr;
    cairo_arc_negative(cr, cx, cy, r, a1, a2);
    cairo_close_path(cr);
}

void sg_video_evenodd(sg_video *v)
{
    cairo_t *cr;

    cr = v->cr;
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
}

void sg_video_write_png(sg_video *v, const char *filename)
{
    unsigned char *buf;
    int x, y;
    uint32_t *cairo_buf;

    if (v->cairo_buf == NULL) return;

    buf = calloc(1, v->width * v->height * 4);

    cairo_buf = v->cairo_buf;


    for (y = 0; y < v->height; y++) {
        for (x = 0; x < v->width; x++) {
            int pos;
            unsigned char *pix;
            uint32_t clr;
            unsigned char r, g, b;
            pos = y * v->width * 4 + x * 4;
            pix = &buf[pos];

            pos = v->width * y + x;
            clr = cairo_buf[pos];

            b = clr & 0xff;
            g = (clr >> 8) & 0xff;
            r = (clr >> 16) & 0xff;

            pix[0] = r;
            pix[1] = g;
            pix[2] = b;
            pix[3] = 255;
        }
    }

    lodepng_encode32_file(filename, buf, v->width, v->height);

    free(buf);
}

void sg_video_set(sg_video *v, int x, int y, int r, int g, int b)
{

    uint32_t val;
    int pos;

    if (x > v->width || x < 0) return;
    if (y > v->height || y < 0) return;

    pos = v->width * y + x;

    val = 0;

    /* blue */
    val |= b;
    /* green */
    val |= (g << 8);
    /* red */
    val |= (r << 16);
    /* alpha */
    val |= 255 << 24;

    v->cairo_buf[pos] = val;
}

int sg_video_get(sg_video *v, int x, int y, int *r, int *g, int *b)
{

    uint32_t val;
    int pos;

    if (x > v->width || x < 0) return 0;
    if (y > v->height || y < 0) return 0;

    pos = v->width * y + x;
    val = v->cairo_buf[pos];


    if (b != NULL) *b = val & 0xFF;
    if (g != NULL) *g = (val >> 8) & 0xFF;
    if (r != NULL) *r = (val >> 16) & 0xFF;

    return 1;
}

int sg_video_text_add_font(sg_video *v,
                           const char *name,
                           const char *path)
{
    if (v->fs == NULL) return FONS_INVALID;
    return fonsAddFont(v->fs, name, path);
}

unsigned int sg_video_text_rgba(unsigned char r,
                                unsigned char g,
                                unsigned char b,
                                unsigned char a)
{
    return sgfons_rgba(r, g, b, a);
}

void sg_video_text_clearstate(sg_video *v) {
    if (v->fs == NULL) return;
    fonsClearState(v->fs);
}

void sg_video_text_setsize(sg_video *v, float size)
{
    if (v->fs == NULL) return;
    fonsSetSize(v->fs, size);
}

void sg_video_text_setalign(sg_video *v, int align)
{
    if (v->fs == NULL) return;
    fonsSetAlign(v->fs, align);
}

void sg_video_text_vertmetrics(sg_video *v,
                               float *ascender,
                               float *descender,
                               float *lineh)
{
    if (v->fs == NULL) return;
    fonsVertMetrics(v->fs, ascender, descender, lineh);
}

void sg_video_text_setblur(sg_video *v, float blur)
{
    if (v->fs == NULL) return;
    fonsSetBlur(v->fs, blur);
}

float sg_video_text_draw(sg_video *v,
                         float x, float y,
                         const char *str,
                         const char *end)
{

    if (v->fs == NULL) return -1;
    return fonsDrawText(v->fs, x, y, str, end);
}

int sg_video_invalidfont(sg_video *v, int font)
{
    return font == FONS_INVALID;
}

void sg_video_text_setfont(sg_video *v, int font)
{
    if (v->fs == NULL) return;
    fonsSetFont(v->fs, font);
}

void sg_video_text_setcolor(sg_video *v, unsigned int color)
{
    if (v->fs == NULL) return;
    fonsSetColor(v->fs, color);
}

void sg_video_move_to(sg_video *v, float x, float y)
{
    cairo_move_to(v->cr, x, y);
}

void sg_video_line_to(sg_video *v, float x, float y)
{
    cairo_line_to(v->cr, x, y);
}

void sg_video_stroke(sg_video *v)
{
    cairo_stroke(v->cr);
}

void sg_video_line_width(sg_video *v, float width)
{
    cairo_set_line_width(v->cr, width);
}

void sg_video_line_rounded(sg_video *v)
{
    cairo_set_line_cap(v->cr, CAIRO_LINE_CAP_ROUND);
}

void sg_video_line_defaults(sg_video *v)
{
    cairo_set_line_cap(v->cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_width(v->cr, 1);
}

/* https://www.cairographics.org/samples/rounded_rectangle/ */

void sg_video_roundrect(sg_video *v,
                        float x, float y,
                        float w, float h,
                        float round)
{
    float radius;
    float degrees;
    cairo_t *cr;

    if (round == 0) {
        return;
    }


    radius = h / round;
    degrees = M_PI / 180.0;
    cr = v->cr;

    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - radius, y + radius,
              radius, -90 * degrees, 0 * degrees);
    cairo_arc(cr, x + w - radius, y + h - radius,
              radius, 0 * degrees, 90 * degrees);
    cairo_arc(cr, x + radius, y + h - radius,
              radius, 90 * degrees, 180 * degrees);
    cairo_arc(cr, x + radius, y + radius,
              radius, 180 * degrees, 270 * degrees);
    cairo_close_path (cr);
}

/* NOTE: not actually round... yet */
void sg_video_roundtri(sg_video *v,
                       float cx, float cy,
                       float s,
                       float round)
{
    cairo_t *cr;
    float n;
    float xoff, yoff;
    float radius;
    float degrees;

    if (round == 0) {
        return;
    }

    radius = s / round;
    degrees = M_PI / 180.0;
    cr = v->cr;

    /*
     * to visualize this, take equilateral triangle,
     * and build a right angle triangle from the
     * barycentric center.
     *
     * 2n is hypotenuse, n is adjacent (xoff), n sqrt(3)
     * is opposite (yoff).
     */

    n = s * 0.25;
    xoff = s * 0.5;
    yoff = n * sqrt(3);

    cairo_new_sub_path(cr);

    /* TODO: make rounded */
    cairo_move_to(cr, cx, cy - yoff);
    /* cairo_arc_negative(cr, (cx - xoff) - radius, (cy - yoff) + radius, */
    /*           radius, -30*degrees, -150* degrees); */

    cairo_line_to(cr, cx - xoff, cy + yoff);

    /* cairo_arc_negative(cr, (cx - xoff) - radius, (cy + yoff) - radius, */
    /*                    radius, -150 * degrees, -270 * degrees); */
    cairo_line_to(cr, cx + xoff, cy + yoff);

    /* cairo_arc_negative(cr, (cx + xoff) - radius, (cy + yoff) - radius, */
    /*                    radius, -270 * degrees, -30 * degrees); */

    cairo_close_path (cr);
}

int sg_video_fps(sg_video *v)
{
    return v->param.i_fps_num;
}

int sg_video_framepos(sg_video *v)
{
    return v->i_frame;
}

us_vec3 * sg_video_unshadebuf(sg_video *v)
{
    return v->usbuf;
}


void sg_video_dims(sg_video *v, int *w, int *h)
{
    if (w != NULL) *w = v->width;
    if (h != NULL) *h = v->height;
}

void sg_video_unshade_init(sg_video *v)
{
    if (v->usbuf != NULL) return;

    v->usbuf = malloc(sizeof(us_vec3) * v->width * v->height);

    sg_video_unshade_clear(v, us_mkvec3(0.f, 0.f, 0.f));
}

void sg_video_unshade_clear(sg_video *v, us_vec3 color)
{
    int x, y;
    if (v->usbuf == NULL) return;

    for (y = 0; y < v->height; y++) {
        for (x = 0; x < v->width; x++) {
            int pos;
            pos = y * v->width + x;
            v->usbuf[pos] = color;
        }
    }
}

void sg_video_unshade_transfer(sg_video *v)
{
    int x, y;
    uint32_t *cairo_buf;
    us_vec3 *usbuf;

    if (v->usbuf == NULL || v->cairo_buf == NULL) return;

    usbuf = v->usbuf;
    cairo_buf = v->cairo_buf;

    for (y = 0; y < v->height; y++) {
        for (x = 0; x < v->width; x++) {
            int pos;
            uint32_t *val;
            us_vec3 *c;

            pos = v->width * y + x;
            val = &cairo_buf[pos];
            c = &usbuf[pos];

            /* clear pixel */
            *val = 0;
            /* blue (z) */
            *val |= (int)floor(c->z * 255);
            /* green (y) */
            *val |= ((int)floor(c->y * 255)) << 8;
            /* red (x) */
            *val |= ((int)floor(c->x * 255)) << 16;
            /* alpha (100%) */
            *val |= 255 << 24;
        }
    }
}
