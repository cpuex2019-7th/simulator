CC = riscv64-unknown-elf-gcc
CFLAGS = -mabi=ilp32 -march=rv32im -c
LDFLAGS = 

SRCS = $(shell find . -type f -name "*.c")
OBJS = $(SRCS:.c=.o)
BINS = $(SRCS:.c=.bin)

default: $(BINS)

$(BINS): $(OBJS)
	riscv64-unknown-elf-objcopy -O binary -j .text	$^ $@

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(BINS) *~
