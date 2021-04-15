#ifndef SG_VIDEO_H
#define SG_VIDEO_H
typedef struct sg_video sg_video;
typedef struct sg_image sg_image;

#include "unshade.h"

#ifdef SG_VIDEO_PRIVATE
struct sg_video {
    /* cairo */
    cairo_surface_t *surface;
    cairo_t *cr;
    uint32_t *cairo_buf;
    int stride;
    int width, height;

    /* x264 video */
    FILE *fp;
    x264_param_t param;
    x264_picture_t pic;
    x264_picture_t pic_out;
    x264_t *h;
    int i_frame;
    x264_nal_t *nal;
    int i_nal;
    uint8_t *ybuf;
    uint8_t *ubuf;
    uint8_t *vbuf;
    unsigned int sz;

    /* fontstash */
    FONScontext *fs;

    /* unshade buffer */
    us_vec3 *usbuf;
};

struct sg_image {
    unsigned char *img;
    unsigned int w;
    unsigned int h;
};
#endif
void sg_video_new(sg_video **pv);
void sg_video_del(sg_video **pv);
void sg_video_open(sg_video *v,
                   const char *filename,
                   int w, int h,
                   int fps);
void sg_video_append(sg_video *v);
void sg_video_close(sg_video *v);

/* drawing routines */
void sg_video_color(sg_video *v,
                    float r,
                    float g,
                    float b,
                    float a);
void sg_video_paint(sg_video *v);
void sg_video_fill(sg_video *v);
void sg_video_circle(sg_video *v,
                     float cx,
                     float cy,
                     float r);

void sg_video_arc(sg_video *v,
                  float cx,
                  float cy,
                  float r,
                  float a1,
                  float a2);

void sg_video_arc_neg(sg_video *v,
                      float cx,
                      float cy,
                      float r,
                      float a1,
                      float a2);

/* Text */
void sg_video_font_size(sg_video *v, int size);
void sg_video_font_face(sg_video *v, const char *face);
void sg_video_text(sg_video *v,
                   const char *txt,
                   float x, float y);

typedef struct {
    double x_bearing;
    double y_bearing;
    double width;
    double height;
    double x_advance;
    double y_advance;
} sg_text_extents;

void sg_video_text_extents(sg_video *v,
                           const char *text,
                           sg_text_extents *e);

int sg_image_new(sg_image **pimg, const char *filename);
void sg_image_del(sg_image **pimg);
void sg_video_image(sg_video *v,
                    sg_image *i,
                    float x,
                    float y);


void sg_video_stencil(sg_video *v,
                      sg_image *i,
                      float x_pos,
                      float y_pos,
                      int r, int g, int b,
                      double alpha);

void sg_video_image_withalpha(sg_video *v,
                              sg_image *i,
                              float x_pos,
                              float y_pos,
                              float alpha);


void sg_image_dims(sg_image *i, int *w, int *h);
void sg_video_scale(sg_video *v, float sx, float xy);
void sg_video_rect(sg_video *v, float x, float y, float w, float h);
void sg_video_evenodd(sg_video *v);
void sg_video_cairo_init(sg_video *v, int w, int h);
void sg_video_write_png(sg_video *v, const char *filename);
void sg_video_set(sg_video *v, int x, int y, int r, int g, int b);
int sg_video_get(sg_video *v, int x, int y, int *r, int *g, int *b);
void sg_video_fontstash_init(sg_video *v);
void sg_video_move_to(sg_video *v, float x, float y);
void sg_video_line_to(sg_video *v, float x, float y);
void sg_video_stroke(sg_video *v);
void sg_video_line_width(sg_video *v, float width);
void sg_video_line_rounded(sg_video *v);
void sg_video_line_defaults(sg_video *v);
void sg_video_roundrect(sg_video *v,
                        float x, float y,
                        float w, float h,
                        float round);
void sg_video_roundtri(sg_video *v,
                       float cx, float cy,
                       float s,
                       float round);
void sg_video_fbm(sg_video *v, int r, int g, int b, int noct, float t);


/* fontstash wrappers */

int sg_video_text_add_font(sg_video *v,
                           const char *name,
                           const char *path);

int sg_video_invalidfont(sg_video *v, int font);
unsigned int sg_video_text_rgba(unsigned char r,
                                unsigned char g,
                                unsigned char b,
                                unsigned char a);
void sg_video_text_clearstate(sg_video *v);

void sg_video_text_setsize(sg_video *v, float size);
void sg_video_text_setfont(sg_video *v, int font);
void sg_video_text_setcolor(sg_video *v, unsigned int color);
void sg_video_text_setalign(sg_video *v, int align);
void sg_video_text_vertmetrics(sg_video *v,
                               float *ascender,
                               float *descender,
                               float *lineh);
void sg_video_text_setblur(sg_video *v, float blur);
float sg_video_text_draw(sg_video *v,
                         float x, float y,
                         const char *str,
                         const char *end);

void sg_video_dims(sg_video *v, int *w, int *h);

int sg_video_fps(sg_video *v);
int sg_video_framepos(sg_video *v);

us_vec3 * sg_video_unshadebuf(sg_video *v);

void sg_video_unshade_init(sg_video *v);

void sg_video_unshade_clear(sg_video *v, us_vec3 color);

void sg_video_unshade_transfer(sg_video *v);

#endif
