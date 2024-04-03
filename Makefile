CC = gcc
CFLAGS = -O1 -g -Wall -I.

# Emit a warning should any variable-length array be found within the code.
CFLAGS += -Wvla

all: cli

OBJS := cli.o console.o report.o linenoise.o

cli: $(OBJS)
	$(CC) $(CFLAGS) -o cli $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) *~ cli

distclean: clean
	rm -f .cmd_history