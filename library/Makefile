CC ?= gcc
CFLAGS = -I../common -I. -I../ -Wall -Wextra
DEPS =
SHARED = $(wildcard ../shared/*.c)
OBJS = bbgl.o gl.o $(SHARED:.c=.o)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

libbbgl.a: $(OBJS)
	ar rcs $@ $^

clean:
	rm -f libbbgl.a
	rm -f $(OBJS)

all: libbbgl
