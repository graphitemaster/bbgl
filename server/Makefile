CC ?= gcc
CFLAGS  = -I../common -I../ -Wall -Wextra
DEPS =
LIBS = -lrt -lX11 -lGL
SHARED = $(wildcard ../shared/*.c)
OBJS = main.o dispatch.o context.o $(SHARED:.c=.o)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

server: $(OBJS)
	$(CC) -o $@ $^ $(LIBS) $(CFLAGS)

clean:
	rm -f server
	rm -f $(OBJS)

all: server
