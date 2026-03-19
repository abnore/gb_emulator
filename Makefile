BIN		= Gameboy

CFILES 	= emulator.c rom.c clock.c decoder.c gameboy.c graphics.c sound.c
HFILES 	= $(CFILES:.c=.h)
OFILES 	= $(CFILES:.c=.o)
ICON	= assets/gameboy.png

CFLAGS = -Wall -Wextra -DICON=\"$(ICON)\"
LDFLAGS = -lblackbox -lcanopy -framework AudioToolBox

all: $(BIN)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

$(BIN): $(OFILES)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(BIN) *.o
