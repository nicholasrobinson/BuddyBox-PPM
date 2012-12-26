CFLAGS:=-Wall
CFILES:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(CFILES))
HEADERS:=$(wildcard *.h)
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
	-rm *.o

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -I $(INCLUDES) -c -o $@ $<

clean:
	-rm -f BuddyBox
	-rm -f *.o
