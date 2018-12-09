# makefile
# compiler

CC = gcc

CFLAGS = -Wall	-g

HEADERS =  central.h drone_movement.h
OBJECTS =  central.o drone_movement.o main.o


default: main

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

main: $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ -D_REENTRANT -lm -lpthread

clean:
	-rm -f $(OBJECTS)
	-rm -f main
