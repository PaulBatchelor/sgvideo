#ifndef SGFONTSTASH_H
#define SGFONTSTASH_H

FONScontext* sgfons_create(int width, int height,
                           int flags,
                           sg_video *v);

void sgfons_delete(FONScontext* ctx);

unsigned int sgfons_rgba(unsigned char r,
                         unsigned char g,
                         unsigned char b,
                         unsigned char a);

#endif

#ifdef SG_FONTSTASH_IMPLEMENTATION
#include <stdint.h>
#include "../colorlerp.h"
struct SGFONScontext {
	int width, height;
    const unsigned char *tex;
    sg_video *v;
};
typedef struct SGFONScontext SGFONScontext;

static int renderCreate(void* ud, int width, int height)
{
	SGFONScontext* sgf = (SGFONScontext*)ud;
	sgf->width = width;
	sgf->height = height;
    sgf->tex = NULL;
	return 1;
}

static int renderResize(void* ud, int width, int height)
{
	return renderCreate(ud, width, height);
}

static void renderUpdate(void* ud, int* rect, const unsigned char* data)
{
	SGFONScontext* sgf = (SGFONScontext*)ud;
    sgf->tex = data;

}

static void draw_tex(sg_video *v,
                     const unsigned char *tex,
                     int tstride,
                     int tx, int ty,
                     int xpos, int ypos,
                     int w, int h,
                     int r, int g, int b, int a)
{
    int x, y;

    /* pos = ypos * stride + xpos * 3; */
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            double c;
            unsigned char oclr[3];
            int iclr[3];
            int rc;


            iclr[0] = 0;
            iclr[1] = 0;
            iclr[2] = 0;

            rc = sg_video_get(v, xpos + x, ypos + y,
                              &iclr[0], &iclr[1], &iclr[2]);

            if (!rc) continue;

            oclr[0] = iclr[0];
            oclr[1] = iclr[1];
            oclr[2] = iclr[2];

            c = (tex[(ty + y) * tstride + (tx + x)]/(double)0xff);
            if (a < 0xff) c *= (double) a / 0xff;

            /* oclr[0] = (c * r + (1 - c)*iclr[0]); */
            /* oclr[1] = (c * g + (1 - c)*iclr[1]); */
            /* oclr[2] = (c * b + (1 - c)*iclr[2]); */

            sg_colorlerp(iclr[0], iclr[1], iclr[2],
                         r, g, b,
                         c,
                         &oclr[0], &oclr[1], &oclr[2]);

            sg_video_set(v,
                         xpos + x,
                         ypos + y,
                         oclr[0], oclr[1], oclr[2]);
        }
    }
}

static void renderDraw(void* ud,
                       const float* verts,
                       const float* tcoords,
                       const unsigned int* colors,
                       int nverts)
{
	SGFONScontext* sgf = (SGFONScontext*)ud;
    int vpos = 0;
    int tpos = 0;
    int cpos = 0;
    int n;
    int nquads;

    nquads = nverts / 6;

    for (n = 0; n < nquads; n++) {
        int xpos, ypos;
        /* int w, h; */
        int r, g, b, a;
        unsigned int c;
        int tx, ty;
        int tw, th;

        c = colors[cpos];

        r = c & 0xff;
        g = (c >> 8) & 0xff;
        b = (c >> 16) & 0xff;
        a = (c >> 24) & 0xff;

        xpos = verts[vpos];
        ypos = verts[vpos + 1];
        tx = tcoords[tpos] * sgf->width;
        ty = tcoords[tpos + 1] * sgf->height;
        tw = tcoords[tpos + 2] * sgf->width - tx;
        th = tcoords[tpos + 3] * sgf->height - ty;

        draw_tex(sgf->v, sgf->tex, sgf->width,
                 tx, ty,
                 xpos, ypos,
                 tw, th,
                 r, g, b, a);
        vpos+=12;
        tpos+=12;
        cpos++;
    }

}

static void renderDelete(void* userPtr)
{
	SGFONScontext* sgf = (SGFONScontext*)userPtr;
	free(sgf);
}

FONScontext* sgfons_create(int width, int height,
                           int flags,
                           sg_video *v)
{
    FONSparams params;
	SGFONScontext* sgf;

	sgf = (SGFONScontext*)malloc(sizeof(SGFONScontext));
	if (sgf == NULL) goto error;

	memset(sgf, 0, sizeof(SGFONScontext));

	memset(&params, 0, sizeof(params));
	params.width = width;
	params.height = height;
	params.flags = (unsigned char)flags;
	params.renderCreate = renderCreate;
	params.renderResize = renderResize;
	params.renderUpdate = renderUpdate;
	params.renderDraw = renderDraw;
	params.renderDelete = renderDelete;
	params.userPtr = sgf;

    sgf->v = v;

	return fonsCreateInternal(&params);

error:
	if (sgf != NULL) {
        free(sgf);
    }
	return NULL;
}

void sgfons_delete(FONScontext* ctx)
{
	fonsDeleteInternal(ctx);
}

unsigned int sgfons_rgba(unsigned char r,
                          unsigned char g,
                          unsigned char b,
                          unsigned char a)
{
	return (r) | (g << 8) | (b << 16) | (a << 24);
}
#endif
