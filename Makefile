CC=gcc
CFLAGS=-Wall -Werror -Wvla -O0 -std=c11 -g -fsanitize=address,leak
LDFLAGS=-lm -fsanitize=address,leak
TARGET=pe_exchange pe_trader

.PHONY: clean
all: $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o

pe_exchange.o: pe_exchange.c
	$(CC) -c $(CFLAGS) $^ -o $@

dyn_array.o: dyn_array.c
	$(CC) -c $(CFLAGS) $^ -o $@

helpers.o: helpers.c
	$(CC) -c $(CFLAGS) $^ -o $@

pe_trader.o: pe_trader.c
	$(CC) -c $(CFLAGS) $^ -o $@

pe_exchange: pe_exchange.o dyn_array.o helpers.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

pe_trader: pe_trader.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

