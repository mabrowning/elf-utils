BIN=elfaddsymbol
LIBS=-lelf
CFLAGS=-O2

all: $(BIN)

clean:
	-@rm $(BIN) 2>/dev/null || true
	
$(BIN): elfaddsymbol.c
	$(CC) $(CFLAGS) $< -o $@ $(LIBS)
