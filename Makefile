CC = gcc
CFLAGS = -O0 -g -Wall -I.

# Emit a warning should any variable-length array be found within the code.
CFLAGS += -Wvla

all: cli

TTT_OBJS := game.o mt19937-64.o zobrist.o agents/mcts.o agents/fpmath.o coro_ttt.o

OBJS := cli.o console.o report.o linenoise.o \
		$(TTT_OBJS)

cli: $(OBJS)
	$(CC) $(CFLAGS) -o cli $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) *~ cli

distclean: clean
	rm -f .cmd_history