TARGET	= getconfig
SRC	= getconfig.c
CFLAGS += -Wall -O2

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^
clean:
	@rm $(TARGET) -f
