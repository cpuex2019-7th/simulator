CC = gcc
CFLAGS = -Wall -O
LDFLAGS = 
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

TARGET = sim

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) $(TARGET)  $(OBJS) *~

test:
	./test.sh
