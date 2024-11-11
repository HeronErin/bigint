CC      = gcc
CFLAGS  = -I "$(shell pwd)" -g -mavx2 -lm
DEBUG_FLAGS = -DDEBUG -g
RM      = rm -f
C_FILES     = $(wildcard lib/*.c)
O_FILES     = $(C_FILES:.c=.o)

INTERFACE_C_FILES = $(wildcard interface/*.c)
INTERFACE_O_FILES = $(INTERFACE_C_FILES:.c=.o)

default: bigint
all: bigint

objs: $(O_FILES)

bigint: objs $(INTERFACE_O_FILES)
	$(CC) $(CFLAGS) -o bigint $(O_FILES) $(INTERFACE_O_FILES)

%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

run: bigint
	./bigint

debug_make:  CFLAGS += $(DEBUG_FLAGS)
debug_make: bigint


debug_run: CFLAGS += $(DEBUG_FLAGS)
debug_run: run light_clean

clean: light_clean
	$(RM) */*.o | true

light_clean:
	$(RM) ./bigint | true
