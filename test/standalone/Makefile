
all: trap_cpuid

CC ?= gcc
CFLAGS = -std=gnu99 -march=broadwell

trap_cpuid: trap_cpuid.c
	$(CC) $(CFLAGS) $^ -o $@

from_forum:
	nasm -Wall -f elf64 -o utils.o utils.asm
	$(CC) $(CFLAGS) -Wall -c -o cpuid.o cpuid.c
	$(CC) $(CFLAGS) -Wall -g -o cpuid cpuid.o utils.o

clean:
	$(RM) trap_cpuid

.PHONY: all
