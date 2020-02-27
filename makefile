CC=gcc
CFLAGS=-Wall -g
LDFLAGS=-pthread

all: oss user

clock.o: clock.c clock.h
	$(CC) $(CFLAGS) -c clock.c

shm.o: shm.c shm.h
	$(CC) $(CFLAGS) -c shm.c

oss: oss.c shm.o clock.o
	$(CC) $(CFLAGS) oss.c clock.o shm.o -o oss $(LDFLAGS)

user: user.c shm.o clock.o
	$(CC) $(CFLAGS) user.c clock.o shm.o -o user $(LDFLAGS)

clean:
	rm -rf oss user shm.o clock.o
