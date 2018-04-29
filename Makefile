CC = gcc
EXE = dis
SOURCES = $(wildcard *.c include/*.c arch/*.c common/*.c)
OBJS = $(SOURCES:.c=.o)
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	CXXFLAGS = -O3 -g -std=c99 -Wall -Wextra -pedantic
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
