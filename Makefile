SOURCES := $(wildcard *.cpp)
OBJECTS := $(SOURCES:.cpp=.o)
EXECUTABLE := screambot

CC := g++
COMMONFLAGS := -Wall -Wextra -Wpedantic -std=c++20
CFLAGS := -O3 $(COMMONFLAGS)
DEBUGFLAGS := -g $(COMMONFLAGS)
LDFLAGS := -ldpp

all: $(EXECUTABLE)

debug: CFLAGS := $(DEBUGFLAGS)
debug: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean

clean:
	rm -rf $(OBJECTS) $(EXECUTABLE)
