CC = gcc
CFLAGS = -Wall -O
LDFLAGS = 
SRCS = $(wildcard *.c)
OBJS = $(filter-out disasm.o sim.o, $(SRCS:.c=.o))

.PHONY: all

all: sim disasm

disasm: $(OBJS) disasm.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

sim: $(OBJS) sim.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ $(LDFLAGS)

clean:
	$(RM) $(OBJS) disasm.o sim.o disasm sim *~

test:
	./test.sh
