TARGET  := libspi_oled.a

SOURCES := $(shell find src/ -type f -name *.c)
OBJECTS := $(patsubst src/%,build/%,$(SOURCES:.c=.o))
CFLAGS  := -Wall -pedantic
INCLUDE := -I include

all: $(TARGET) tests

$(TARGET): $(OBJECTS)
	@mkdir -p lib
	ar r lib/$(TARGET) $(OBJECTS)

build/%.o: src/%.c
	@mkdir -p build/
	$(CC) $(CFLAGS) $(INCLUDE) -MD -MF $(@:.o=.deps) -c -o $@ $<

tests:
	make -C test

clean:
	rm -rf build
	rm -f lib/*
	make -C test clean
