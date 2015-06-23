
CC := gcc

DEFINES +=
CFLAGS = $(DEFINES) -O2 -Wall
INCLUDES += -Icsv

SHARED = -shared

RAGEL = ragel
RAGELFLAGS = -C -G2

.PHONY: clean

TARGETDIR = $(COMMANDER_PATH)/Plugins/wlx/csv
TARGET = $(TARGETDIR)/wlx_csv.wlx

all: $(TARGETDIR) $(TARGET)
	@:

clean:
	rm csv/csv.c
	rm $(TARGET)

$(TARGETDIR):
	mkdir $(subst /,\\,$(TARGETDIR))

csv/csv.c: csv/csv.rl
	$(RAGEL) $(RAGELFLAGS) $< -o $@

$(TARGET): csv/csv.c src/wlx_csv.c src/listplug.def
	$(CC) -o $@ $(CFLAGS) $(SHARED) $(INCLUDES) -Xlinker --enable-stdcall-fixup $^

