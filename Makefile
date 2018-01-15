CC=gcc
CFLAGS=-c -g -Wall -Isrc
LDFLAGS=-lm -leditline
EXECUTABLE=dsptree
SOURCES=src/numbers.c src/tokens.c src/ast.c src/parser.c dsptree.c
OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	rm src/*.o *.o

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm dsptree

