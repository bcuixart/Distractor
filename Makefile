CC      := gcc
CFLAGS  := -Wall -Wextra -Ilib -MMD -MP -O3
LDFLAGS := -mwindows
LDLIBS  := -lgdi32
BUILD   := build

APPS := dvd_logo grandma

OBJS := $(BUILD)/lib/window.o \
        $(BUILD)/lib/bitmap.o \
        $(BUILD)/src/main.o \
        $(BUILD)/src/config.o \
        $(APPS:%=$(BUILD)/example_apps/%/main.o)

.PHONY: all apps distractor clean $(APPS)

all: distractor apps

distractor: $(BUILD)/distractor.exe
apps: $(APPS)
$(APPS): %: $(BUILD)/%.exe

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.rc
	@mkdir -p $(dir $@)
	windres --include-dir $(dir $<) $< -o $@

$(BUILD)/libwindow.a: $(BUILD)/lib/window.o $(BUILD)/lib/bitmap.o
	ar rcs $@ $^

$(BUILD)/distractor.exe: $(BUILD)/src/main.o $(BUILD)/src/config.o
	$(CC) $^ -o $@

$(BUILD)/%.exe: $(BUILD)/example_apps/%/main.o \
                $(BUILD)/example_apps/%/resources.o \
                $(BUILD)/libwindow.a
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -rf $(BUILD)

-include $(OBJS:.o=.d)

.SECONDARY: