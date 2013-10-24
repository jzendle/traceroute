TARGET=traceroute
OBJECT=main.o traceroute.o Socket.o Logger.o Timer.o Icmp.o

CFLAGS=-O0 -ggdb -Wall -ansi
LIBS=

CC=gcc

.h.c:
	touch $@

.c.o:
	$(CC) $< $(CFLAGS) -c -o $@

all: $(TARGET)

$(TARGET): $(OBJECT)
	$(CC) $^ -o $(TARGET) $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET)
	rm -f *.o
