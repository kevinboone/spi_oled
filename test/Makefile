TARGET  := test 

OBJECTS := test.o 
CFLAGS  := -Wall -pedantic
INCLUDE := -I ../include
LDFLAGS := -L ../lib

all: $(TARGET) 

$(TARGET): $(OBJECTS)
	gcc $(LDFLAGS) -o $(TARGET) $(OBJECTS) -lspi_oled	

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -MD -MF $(@:.o=.deps) -c -o $@ $<

clean:
	rm -rf *.o $(TARGET) 
