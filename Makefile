CC = gcc
CFLAGS = -Wall -O
LDFLAGS = 
SRCS = $(shell find . -type f -name "*.c")
OBJS = $(SRCS:.c=.o)

TARGET = sim

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TARGET)  $(OBJS) *~
