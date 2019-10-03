CC = riscv64-unknown-elf-gcc
CC_X86 = gcc
CFLAGS = -mabi=ilp32 -march=rv32im -c
LDFLAGS = 

SRCS = $(filter-out call.c, $(wildcard *.c))
OBJS = $(SRCS:.c=.o)
COES = $(SRCS:.c=.coe)
BINS = $(SRCS:.c=.bin)
EXPECTEDS = $(SRCS:.c=.expected)

default: $(BINS) $(EXPECTEDS) $(COES)

%.coe: %.bin
	python bin2coe.py $^ > $@

%.bin: %.o
	riscv64-unknown-elf-objcopy -O binary -j .text	$^ $@

%.expected: %.c
	$(CC_X86) call.c $^ -o $*.out
	./$*.out > $*.expected
	rm $*.out

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(BINS) $(EXPECTEDS) call.o *.out *~
