CC = gcc
CFLAGS = -Wall -O3
LDFLAGS = -lm
SRCS = $(shell find . -type f -name "*.c")
OBJS = $(shell find . -type f -name "*.c" | grep -v "_test" | grep -v "sim.c" | grep -v "disasm.c" | sed "s/\.c/\.o/")

.PHONY: all

all: sim disasm

disasm: $(OBJS) disasm.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

sim: $(OBJS) sim.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

clean:
	$(RM) $(OBJS) disasm.o sim.o disasm sim *~

test:
	./test.sh

install: sim disasm
	cp sim /usr/local/bin/cpuex_sim
	chmod 755 /usr/local/bin/cpuex_sim
	cp disasm /usr/local/bin/cpuex_disasm
	chmod 755 /usr/local/bin/cpuex_disasm
	echo "installed: cpuex_sim, cpuex_disam in /usr/bin"

uninstall:
	rm -rf /usr/local/bin/cpuex_sim
	rm -rf /usr/local/bin/cpuex_disasm
