CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g
TARGET = riscv_sim
SOURCES = main.c riscv_sim.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean