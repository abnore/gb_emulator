CFILES 	= emulator.c rom.c clock.c decoder.c gameboy.c
HFILES 	= $(CFILES:.c=.h)
OFILES 	= $(CFILES:.c=.o)

CFLAGS = -Wall -Wextra -Wno-unused-function -Wno-unused-parameter
LDFLAGS = -lblackbox

all: emulator
	@rm *.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

emulator: $(OFILES)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f emulator *.o
