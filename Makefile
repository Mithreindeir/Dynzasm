CC = gcc
EXE = dis
SOURCES = $(wildcard src/*.c src/arch/*.c src/arch/x86/*.c src/common/*.c)
OBJS = $(SOURCES:.c=.o)
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	CXXFLAGS = -ggdb3 -std=c99 -Wall -Wextra -pedantic
	CFLAGS = $(CXXFLAGS)
endif

.c.o:
	$(CC) $(CXXFLAGS) -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CC) -o $(EXE) $(OBJS) $(CXXFLAGS) $(LIBS)

clean:
	rm $(EXE) $(OBJS)
