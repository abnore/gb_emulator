BIN     = Gameboy
OBJ     = obj

CFILES  = emulator.c rom.c clock.c decoder.c gameboy.c graphics.c sound.c
OFILES = $(addprefix $(OBJ)/, $(CFILES:.c=.o))
ICON    = assets/gameboy.png

CFLAGS  = -Wall -Wextra -DICON=\"$(ICON)\"
LDFLAGS = -lblackbox -lcanopy -framework AudioToolBox -framework CoreAudio

all: $(BIN)

$(OBJ):
	mkdir -p $(OBJ)

$(OBJ)/%.o: %.c | $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN): $(OFILES)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm -rf $(BIN) $(OBJ)
