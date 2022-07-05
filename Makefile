CFLAGS=-O2 -fPIC -Wall -Wextra -D _GNU_SOURCE -pthread $$(pkg-config --cflags telebot libical)
LDFLAGS=$$(pkg-config --libs telebot libical)
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
	$(RM) -r vgcore.* obj $(TARGET) config*.h

run: $(TARGET)
	./$(TARGET)

install:
	echo "Installing is not supported"

