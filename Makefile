CC := gcc
CFLAGS := -O2 -g -std=c11 -Wall -Wno-missing-braces
LDFLAGS := -lm -lGL -lSDL2
TARGET = main


SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c,obj/%.o,$(SRCS))


g: $(TARGET)
$(TARGET): $(OBJS) $(OBJS)
	$(CC)  -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf obj/ $(TARGET)

.PHONY: all clean

len:
	find . -name '*.c' | xargs wc -l
