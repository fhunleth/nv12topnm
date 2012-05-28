
TARGET = nv12topnm
OBJECTS = nv12topnm.o
CC = gcc

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $<

clean:
	-rm $(TARGET) $(OBJECTS)
