CC ?= gcc
CFLAGS = -I. -Wall -Wextra `sdl2-config --cflags`
DEPS =
OBJS = test.o
LIBS = -lrt `sdl2-config --libs`

all:
	cd server; $(MAKE)
	cd library; $(MAKE)
	$(MAKE) test

clean:
	cd server; $(MAKE) clean
	cd library; $(MAKE) clean
	rm -f test $(OBJS)

test: $(OBJS) library/libbbgl.a
	$(CC) $(LIBS) -o $@ $^ $(CFLAGS)

