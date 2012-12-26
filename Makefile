CFLAGS:=-Wall
CFILES:=$(wildcard src/*.c)
OBJS:=$(patsubst src/%.c,objs/%.o,$(CFILES))
HEADERS:=$(wildcard src/*.h)
INCLUDES:=/opt/local/include
LIBS:=$(wildcard $(OS)/*.a)
LINK_FLAGS:=/opt/local/lib/libportaudio.a

OS:=$(shell uname)
ifeq ($(OS),Darwin)
	LINK_FLAGS:=$(LINK_FLAGS) -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices
endif

all: BuddyBox

BuddyBox: $(HEADERS) $(OBJS) $(LIBS)
	$(CC) $(LINK_FLAGS) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

objs:
	-rm -rf objs
	mkdir objs

objs/%.o: src/%.c $(HEADERS) objs
	$(CC) $(CFLAGS) -I $(INCLUDES) -c -o $@ $<

clean:
	-rm -f BuddyBox
	-rm -rf objs
