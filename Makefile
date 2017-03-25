CFLAGS:=-Wall
CFILES:=$(wildcard src/*.c)
OBJS:=$(patsubst src/%.c,objs/%.o,$(CFILES))
HEADERS:=$(wildcard src/*.h)
LIBS:=$(wildcard $(OS)/*.a)

OS:=$(shell uname)
ifeq ($(OS),Darwin)
	CFLAGS:=$(CFLAGS) -I /opt/local/include/ -framework CoreAudio -framework AudioToolbox -framework AudioUnit -framework CoreServices
	LIBS:=$(LIBS) -L /opt/local/lib/ -l portaudio
else ifeq ($(OS),Linux)
	LIBS:=$(LIBS) -L /usr/lib/x86_64-linux-gnu/ -l portaudio -l pthread
endif

all: BuddyBox-PPM

BuddyBox-PPM: $(HEADERS) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

objs:
	-rm -rf objs
	mkdir objs

objs/%.o: src/%.c $(HEADERS) objs
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f BuddyBox-PPM
	-rm -rf objs
