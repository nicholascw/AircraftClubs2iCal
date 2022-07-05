CFLAGS=-O2 -fPIC -Wall -Wextra -D _GNU_SOURCE -pthread $$(pkg-config --cflags libical libcurl)
LDFLAGS=$$(pkg-config --libs libical libcurl)
TARGET=ac2ical
SRC=$(wildcard *.c)
HDR=$(wildcard *.h)
OBJS=$(patsubst %.c, obj/%.o, $(SRC))
DEBUG=

ifeq ($(DEBUG),1)
	CFLAGS += -g -DDEBUG
endif

.PHONY: all clean format run install

all: clean $(TARGET) format

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

obj/%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c -o $@ $<

format:
	$(foreach n, $(SRC), clang-format -style=google -i $(n); )
	$(foreach n, $(HDR), clang-format -style=google -i $(n); )

clean:
	$(RM) -r vgcore.* obj $(TARGET)

run: $(TARGET)
	./$(TARGET)

install:
	echo "Installing is not supported"

