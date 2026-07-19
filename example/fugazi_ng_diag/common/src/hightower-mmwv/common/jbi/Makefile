CC            = $(CROSS_COMPILE)gcc
LD            = $(CROSS_COMPILE)ld
CFLAGS        = -Wall -DPORT=UNIX $(PASSIN_INC_DIRS)
LDFLAGS       =
BIN          := jbi

SRCS = $(shell ls --color=no *.c)
OBJS = $(patsubst %.c,%.o,$(SRCS))

ifeq ($(JBI_STANDALONE),1)
CFLAGS  += -DJBI_STANDALONE

$(BIN): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
else

LDFLAGS += -r
BIN := $(BIN).obj

$(BIN): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@
endif

%.o: %.c
	$(CC) $(CFLAGS) -Wp,-MD,.$@.d -c $< -o $@

help:
	@echo "make [CROSS_COMPILE=xxx] [JBI_STANDALONE=0|1]"
	@echo "Where:"
	@echo " xxx             - The cross compile prefix."
	@echo " JBI_STANDALONE  - Compile into executable image or linkable object."

clean:
	rm -f *~
	rm -f *.o .*.d .*.prep
	rm -f $(BIN)

.PHONY: clean $(BIN) help
