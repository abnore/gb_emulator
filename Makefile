CFILES 	= emulator.c cpu.c rom.c clock.c bus.c
HFILES 	= $(CFILES:.c=.h)
OFILES 	= $(CFILES:.c=.o)

CFLAGS = -Wall -Wextra
LDFLAGS = -lblackbox

all: emulator
	@rm *.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

emulator: $(OFILES)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f emulator *.o
