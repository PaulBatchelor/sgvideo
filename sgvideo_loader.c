/*
 * Copyright (c) 2021 Muvik Labs, LLC
 * Distributed under the MIT license.
 */

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "video.h"

static sg_video * check_vg(lua_State *L, int index)
{
    sg_video *v;

    v = lua_touserdata(L, index);
    if (v == NULL) luaL_error(L, "Invalid sg_video argument.\n");

    return v;
}

sg_video * sg_video_check(lua_State *L, int index)
{
    return check_vg(L, index);
}

static us_vec3 get_vec3(lua_State *L, int index)
{

    us_vec3 v;

    luaL_checktype(L, index, LUA_TTABLE);

    lua_rawgeti(L, index, 1);
    v.x = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, index, 2);
    v.y = lua_tonumber(L, -1);
    lua_pop(L, 1);
    lua_rawgeti(L, index, 3);
    v.z = lua_tonumber(L, -1);
    lua_pop(L, 1);

    return v;
}

static int l_vg_new(lua_State *L)
{
    sg_video *v;
    sg_video_new(&v);
    lua_pushlightuserdata(L, v);
    return 1;
}

static int l_vg_del(lua_State *L)
{
    sg_video *v;
    v = check_vg(L, 1);
    sg_video_del(&v);
    return 0;
}

static int l_vg_open(lua_State *L)
{
    sg_video *v;
    const char *filename;
    int w, h, fps;

    v = check_vg(L, 1);
    filename = luaL_checkstring(L, 2);
    w = luaL_checkinteger(L, 3);
    h = luaL_checkinteger(L, 4);
    fps = luaL_checkinteger(L, 5);

    sg_video_open(v, filename, w, h, fps);
    return 0;
}

static int l_vg_cairo_init(lua_State *L)
{
    sg_video *v;
    int w, h;

    v = check_vg(L, 1);
    w = luaL_checkinteger(L, 2);
    h = luaL_checkinteger(L, 3);

    sg_video_cairo_init(v, w, h);
    return 0;
}

static int l_vg_fontstash_init(lua_State *L)
{
    sg_video *v;
    v = check_vg(L, 1);
    sg_video_fontstash_init(v);
    return 0;
}

static int l_vg_close(lua_State *L)
{
    sg_video *v;
    v = check_vg(L, 1);
    sg_video_close(v);
    return 0;
}

static int l_vg_append(lua_State *L)
{
    sg_video *v;
    v = check_vg(L, 1);
    sg_video_append(v);
    return 0;
}

static int l_vg_color(lua_State *L)
{
    sg_video *v;
    float r, g, b, a;
    v = check_vg(L, 1);
    r = luaL_checknumber(L, 2);
    g = luaL_checknumber(L, 3);
    b = luaL_checknumber(L, 4);
    a = luaL_checknumber(L, 5);
    sg_video_color(v, r, g, b, a);
    return 0;
}

static int l_vg_paint(lua_State *L)
{
    sg_video *v;
    v = check_vg(L, 1);
    sg_video_paint(v);
    return 0;
}

static int l_vg_fill(lua_State *L)
{
    sg_video *v;
    v = check_vg(L, 1);
    sg_video_fill(v);
    return 0;
}

static int l_vg_circ(lua_State *L)
{
    sg_video *v;
    float x, y, r;

    v = check_vg(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    r = luaL_checknumber(L, 4);

    sg_video_circle(v, x, y, r);
    return 0;
}

static int l_vg_font_face(lua_State *L)
{
    sg_video *v;
    const char *face;

    v = check_vg(L, 1);
    face = luaL_checkstring(L, 2);
    sg_video_font_face(v, face);
    return 0;
}

static int l_vg_font_size(lua_State *L)
{
    sg_video *v;
    int size;

    v = check_vg(L, 1);
    size = luaL_checkinteger(L, 2);
    sg_video_font_size(v, size);
    return 0;
}

static int l_vg_text(lua_State *L)
{
    sg_video *v;
    const char *text;
    float x, y;

    v = check_vg(L, 1);
    text = luaL_checkstring(L, 2);
    x = luaL_checknumber(L, 3);
    y = luaL_checknumber(L, 4);
    sg_video_text(v, text, x, y);
    return 0;
}

static void pushval(lua_State *L, char *key, double val) {
    lua_pushstring(L, key);
    lua_pushnumber(L, val);
    lua_settable(L, -3);
}

static int l_vg_text_extents(lua_State *L)
{
    sg_video *v;
    sg_text_extents e;
    const char *text;

    v = check_vg(L, 1);
    text = luaL_checkstring(L, 2);
    sg_video_text_extents(v, text, &e);

    lua_newtable(L);
    pushval(L, "x_bearing", e.x_bearing);
    pushval(L, "y_bearing", e.y_bearing);
    pushval(L, "width", e.width);
    pushval(L, "height", e.height);
    pushval(L, "x_advance", e.x_advance);
    pushval(L, "y_advance", e.y_advance);

    return 1;
}

static int l_vg_img_new(lua_State *L)
{
    sg_image *i;
    const char *filename;
    int rc;

    filename = luaL_checkstring(L, 1);
    rc = sg_image_new(&i, filename);

    if (!rc) {
        luaL_error(L, "img_new failed.\n");
    }

    lua_pushlightuserdata(L, i);
    return 1;
}

static sg_image * check_img(lua_State *L, int index)
{
    sg_image *i;

    i = lua_touserdata(L, index);

    if (i == NULL) luaL_error(L, "Invalid sg_image argument.\n");

    return i;
}

static int l_vg_img_del(lua_State *L)
{
    sg_image *i;
    i = check_img(L, 1);
    sg_image_del(&i);
    return 0;
}

static int l_vg_img(lua_State *L)
{
    sg_image *i;
    sg_video *v;
    float x, y;
    float a;

    v = check_vg(L, 1);
    i = check_img(L, 2);
    x = luaL_checknumber(L, 3);
    y = luaL_checknumber(L, 4);
    a = luaL_checknumber(L, 5);


    if (a == 1) sg_video_image(v, i, x, y);
    else sg_video_image_withalpha(v, i, x, y, a);

    return 0;
}

static int l_vg_img_stencil(lua_State *L)
{
    sg_image *i;
    sg_video *v;
    float x, y;
    int r, g, b;
    float a;

    v = check_vg(L, 1);
    i = check_img(L, 2);
    x = luaL_checknumber(L, 3);
    y = luaL_checknumber(L, 4);

    r = luaL_checkinteger(L, 5);
    g = luaL_checkinteger(L, 6);
    b = luaL_checkinteger(L, 7);
    a = luaL_checknumber(L, 8);

    sg_video_stencil(v, i, x, y, r, g, b, a);
    return 0;
}

static int l_vg_img_dims(lua_State *L)
{
    sg_image *i;
    int w, h;
    i = check_img(L, 1);
    w = h = 0;
    sg_image_dims(i, &w, &h);
    lua_pushinteger(L, w);
    lua_pushinteger(L, h);
    return 2;
}

static int l_vg_scale(lua_State *L)
{
    sg_video *v;
    float sx, sy;

    v = check_vg(L, 1);
    sx = luaL_checknumber(L, 2);
    sy = luaL_checknumber(L, 3);

    sg_video_scale(v, sx, sy);
    return 0;
}

static int l_vg_rect(lua_State *L)
{
    sg_video *v;
    float x, y, w, h;

    v = check_vg(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    w = luaL_checknumber(L, 4);
    h = luaL_checknumber(L, 5);

    sg_video_rect(v, x, y, w, h);
    return 0;
}

static int l_vg_arc(lua_State *L)
{
    sg_video *v;
    float x, y, r, a1, a2;

    v = check_vg(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    r = luaL_checknumber(L, 4);
    a1 = luaL_checknumber(L, 5);
    a2 = luaL_checknumber(L, 6);

    sg_video_arc(v, x, y, r, a1, a2);
    return 0;
}

static int l_vg_arc_neg(lua_State *L)
{
    sg_video *v;
    float x, y, r, a1, a2;

    v = check_vg(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    r = luaL_checknumber(L, 4);
    a1 = luaL_checknumber(L, 5);
    a2 = luaL_checknumber(L, 6);

    sg_video_arc(v, x, y, r, a1, a2);
    return 0;
}

static int l_vg_evenodd(lua_State *L)
{
    sg_video *v;

    v = check_vg(L, 1);

    sg_video_evenodd(v);
    return 0;
}

static int l_vg_write_png(lua_State *L)
{
    sg_video *v;
    const char *filename;

    v = check_vg(L, 1);
    filename = luaL_checkstring(L, 2);

    sg_video_write_png(v, filename);
    return 0;
}

static int l_vg_set(lua_State *L)
{
    sg_video *v;
    int x, y;
    int r, g, b;

    v = check_vg(L, 1);
    x = lua_tointeger(L, 2);
    y = lua_tointeger(L, 3);
    r = lua_tointeger(L, 4);
    g = lua_tointeger(L, 5);
    b = lua_tointeger(L, 6);

    sg_video_set(v, x, y, r, g, b);
    return 0;
}

static int l_vg_get(lua_State *L)
{
    sg_video *v;
    int x, y;
    int r, g, b;

    v = check_vg(L, 1);
    x = lua_tointeger(L, 2);
    y = lua_tointeger(L, 3);

    r = 0;
    g = 0;
    b = 0;

    sg_video_get(v, x, y, &r, &g, &b);

    lua_pushinteger(L, r);
    lua_pushinteger(L, g);
    lua_pushinteger(L, b);
    return 3;
}

static int l_vg_text_add_font(lua_State *L)
{
    sg_video *v;
    int font;
    const char *name;
    const char *path;

    v = check_vg(L, 1);
    name = luaL_checkstring(L, 2);
    path = luaL_checkstring(L, 3);

    font = sg_video_text_add_font(v, name, path);

    if (sg_video_invalidfont(v, font)) {
        luaL_error(L, "Could not add font '%s' from file '%s'.\n",
                   name, path);
    }

    lua_pushinteger(L, font);
    return 1;
}

static int l_vg_text_rgba(lua_State *L)
{
    int r, g, b, a;
    unsigned int clr;

    r = luaL_checkinteger(L, 1);
    g = luaL_checkinteger(L, 2);
    b = luaL_checkinteger(L, 3);
    a = luaL_checkinteger(L, 4);

    clr = sg_video_text_rgba(r, g, b, a);

    lua_pushinteger(L, clr);
    return 1;
}

static int l_vg_text_clearstate(lua_State *L)
{
    sg_video *v;

    v = check_vg(L, 1);

    sg_video_text_clearstate(v);

    return 0;
}

static int l_vg_text_setsize(lua_State *L)
{
    sg_video *v;
    float size;

    v = check_vg(L, 1);

    size = luaL_checknumber(L, 2);
    sg_video_text_setsize(v, size);

    return 0;
}

static int l_vg_text_setalign(lua_State *L)
{
    sg_video *v;
    float align;

    v = check_vg(L, 1);
    align = luaL_checknumber(L, 2);
    sg_video_text_setalign(v, align);
    return 0;
}

static int l_vg_text_vertmetrics(lua_State *L)
{
    sg_video *v;
    float ascend, descend, lineh;

    v = check_vg(L, 1);

    ascend = 0;
    descend = 0;
    lineh = 0;

    sg_video_text_vertmetrics(v, &ascend, &descend, &lineh);

    lua_pushnumber(L, ascend);
    lua_pushnumber(L, descend);
    lua_pushnumber(L, lineh);

    return 3;
}

static int l_vg_text_setblur(lua_State *L)
{
    sg_video *v;
    float blur;

    v = check_vg(L, 1);
    blur = luaL_checknumber(L, 2);
    sg_video_text_setblur(v, blur);
    return 0;
}

static int l_vg_text_draw(lua_State *L)
{
    sg_video *v;
    float x, y;
    const char *str;
    float dx;

    v = check_vg(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    str = luaL_checkstring(L, 4);

    dx = sg_video_text_draw(v, x, y, str, NULL);

    lua_pushinteger(L, dx);
    return 1;
}

static int l_vg_text_setfont(lua_State *L)
{
    sg_video *v;
    int font;

    v = check_vg(L, 1);
    font = luaL_checkinteger(L, 2);
    sg_video_text_setfont(v, font);
    return 0;
}


static int l_vg_text_setcolor(lua_State *L)
{
    sg_video *v;
    unsigned int color;

    v = check_vg(L, 1);
    color = luaL_checkinteger(L, 2);
    sg_video_text_setcolor(v, color);
    return 0;
}

static int l_vg_move_to(lua_State *L)
{
    sg_video *v;
    float x, y;

    v = check_vg(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);

    sg_video_move_to(v, x, y);
    return 0;
}

static int l_vg_line_to(lua_State *L)
{
    sg_video *v;
    float x, y;

    v = check_vg(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);

    sg_video_line_to(v, x, y);
    return 0;
}

static int l_vg_stroke(lua_State *L)
{
    sg_video *v;

    v = check_vg(L, 1);
    sg_video_stroke(v);
    return 0;
}

static int l_vg_line_width(lua_State *L)
{
    sg_video *v;
    float w;

    v = check_vg(L, 1);
    w = luaL_checknumber(L, 2);
    sg_video_line_width(v, w);
    return 0;
}

static int l_vg_line_rounded(lua_State *L)
{
    sg_video *v;

    v = check_vg(L, 1);
    sg_video_line_rounded(v);
    return 0;
}

static int l_vg_line_defaults(lua_State *L)
{
    sg_video *v;

    v = check_vg(L, 1);
    sg_video_line_defaults(v);
    return 0;
}

static int l_vg_roundrect(lua_State *L)
{
    sg_video *v;
    float x, y;
    float w, h;
    float round;

    v = check_vg(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    w = luaL_checknumber(L, 4);
    h = luaL_checknumber(L, 5);
    round = luaL_checknumber(L, 6);

    if (round <= 0) {
        luaL_error(L, "Round value of %g is too small\n", round);
    }

    sg_video_roundrect(v, x, y, w, h, round);
    return 0;
}

static int l_vg_roundtri(lua_State *L)
{
    sg_video *v;
    float x, y;
    float s;
    float round;

    v = check_vg(L, 1);
    x = luaL_checknumber(L, 2);
    y = luaL_checknumber(L, 3);
    s = luaL_checknumber(L, 4);
    round = luaL_checknumber(L, 5);

    if (round <= 0) {
        luaL_error(L, "Round value of %g is too small\n", round);
    }

    sg_video_roundtri(v, x, y, s, round);
    return 0;
}

static int l_vg_fbmfill(lua_State *L)
{
    sg_video *v;
    int oct;
    int r, g, b;
    float t;

    v = check_vg(L, 1);
    r = luaL_checkinteger(L, 2);
    g = luaL_checkinteger(L, 3);
    b = luaL_checkinteger(L, 4);
    oct = luaL_checkinteger(L, 5);
    t = luaL_checknumber(L, 6);

    sg_video_fbm(v, r, g, b, oct, t);
    return 0;
}

static int l_vg_unshade_init(lua_State *L)
{
    sg_video *v;

    v = check_vg(L, 1);

    sg_video_unshade_init(v);
    return 0;
}

static int l_vg_unshade_clear(lua_State *L)
{
    sg_video *v;
    float r, g, b;

    v = check_vg(L, 1);
    r = luaL_checknumber(L, 2);
    g = luaL_checknumber(L, 3);
    b = luaL_checknumber(L, 4);

    sg_video_unshade_clear(v, us_mkvec3(r, g, b));
    return 0;
}

static int l_vg_unshade_transfer(lua_State *L)
{
    sg_video *v;

    v = check_vg(L, 1);

    sg_video_unshade_transfer(v);
    return 0;
}

void sg_video_startest(sg_video *v);

static int l_vg_unshade_test(lua_State *L)
{
    sg_video *v;

    v = check_vg(L, 1);

    sg_video_startest(v);

    return 0;
}

void sg_video_star(sg_video *v,
                   us_vec3 color,
                   us_vec3 bg,
                   us_vec3 tint,
                   float radius);

static int l_vg_star(lua_State *L)
{
    sg_video *v;
    us_vec3 color;
    us_vec3 bg;
    us_vec3 tint;
    float radius;

    v = check_vg(L, 1);
    color = get_vec3(L, 2);
    bg = get_vec3(L, 3);
    tint = get_vec3(L, 4);
    radius = luaL_checknumber(L, 5);

    sg_video_star(v, color, bg, tint, radius);

    return 0;
}

static const luaL_Reg vglib[] = {
    {"new", l_vg_new},
    {"del", l_vg_del},
    {"open", l_vg_open},
    {"cairo_init", l_vg_cairo_init},
    {"fontstash_init", l_vg_fontstash_init},
    {"close", l_vg_close},
    {"append", l_vg_append},
    {"color", l_vg_color},
    {"paint", l_vg_paint},
    {"fill", l_vg_fill},
    {"circ", l_vg_circ},
    {"font_size", l_vg_font_size},
    {"font_face", l_vg_font_face},
    {"text_extents", l_vg_text_extents},
    {"text", l_vg_text},
    {"img_new", l_vg_img_new},
    {"img_del", l_vg_img_del},
    {"img_dims", l_vg_img_dims},
    {"img_stencil", l_vg_img_stencil},
    {"img", l_vg_img},
    {"scale", l_vg_scale},
    {"rect", l_vg_rect},
    {"arc", l_vg_arc},
    {"arc_neg", l_vg_arc_neg},
    {"evenodd", l_vg_evenodd},
    {"write_png", l_vg_write_png},
    {"set", l_vg_set},
    {"get", l_vg_get},
    {"moveto", l_vg_move_to},
    {"lineto", l_vg_line_to},
    {"stroke", l_vg_stroke},
    {"line_width", l_vg_line_width},
    {"line_rounded", l_vg_line_rounded},
    {"line_defaults", l_vg_line_defaults},
    {"roundrect", l_vg_roundrect},
    {"roundtri", l_vg_roundtri},
    {"fbmfill", l_vg_fbmfill},
    {"star", l_vg_star},

    /* text/fontstash stuff */
    {"text_add_font", l_vg_text_add_font},
    {"text_rgba", l_vg_text_rgba},
    {"text_clearstate", l_vg_text_clearstate},
    {"text_setsize", l_vg_text_setsize},
    {"text_setcolor", l_vg_text_setcolor},
    {"text_setalign", l_vg_text_setalign},
    {"text_vertmetrics", l_vg_text_vertmetrics},
    {"text_setblur", l_vg_text_setblur},
    {"text_draw", l_vg_text_draw},
    {"text_setfont", l_vg_text_setfont},


    /* unshade operations */
    {"unshade_init", l_vg_unshade_init},
    {"unshade_clear", l_vg_unshade_clear},
    {"unshade_transfer", l_vg_unshade_transfer},
    {"unshade_test", l_vg_unshade_test},

    {NULL, NULL}
};

void sg_lua_video_setfuncs(lua_State *L)
{
    luaL_setfuncs(L, vglib, 0);
}

#define tablen(t) (sizeof(t)/sizeof((t)[0]) - 1)

int sg_lua_video_nfuncs(void)
{
    return tablen(vglib);
}

int sg_lua_video(lua_State *L)
{
    luaL_newlib(L, vglib);
    return 1;
}
