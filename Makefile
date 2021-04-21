default: sgvideo

LUA_PATH=lua
LUA_CORE_O=	lapi.o lcode.o lctype.o ldebug.o ldo.o ldump.o lfunc.o lgc.o llex.o \
	lmem.o lobject.o lopcodes.o lparser.o lstate.o lstring.o ltable.o \
	ltm.o lundump.o lvm.o lzio.o lua.o
LUA_LIB_O=	lauxlib.o lbaselib.o lbitlib.o lcorolib.o ldblib.o liolib.o \
	lmathlib.o loslib.o lstrlib.o ltablib.o lutf8lib.o loadlib.o linit.o
LUA_BASE_O= $(LUA_CORE_O) $(LUA_LIB_O)

OBJ += $(addprefix $(LUA_PATH)/, $(LUA_BASE_O))

CFLAGS+=-DLUA_USE_C89 -DLUA_COMPAT_5_2
CFLAGS+=-O3 -Wall -pedantic
CFLAGS+=-I$(LUA_PATH)

C99=$(CC) -std=c99
C89=$(CC) -std=c89

OBJ += colorlerp.o fbm.o sgvideo_loader.o simplex.c99 video.c99 main.o
OBJ += fontstash/sgfontstash.c99

OBJ += lodepng/lodepng.c99

OBJ += unshade.o star.o fill.o

LIBS+=-lx264 -lcairo

sgvideo: $(OBJ)
	$(C89) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LIBS)

%.c99: %.c
	$(C99) -c $(CFLAGS) $< -o $@

%.o: %.c
	$(C89) -c $(CFLAGS) $< -o $@

clean:
	$(RM) $(OBJ)
	$(RM) sgvideo
