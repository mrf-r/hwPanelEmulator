TARGET_PE := build/test

#######################################
# additional libraries besides sdl2
LIBS := 
LIBS += portmidi
LIBS += libserialport
LIBS += portaudio-2.0

LIBS_FLAGS := 
LIBS_FLAGS += $(shell sdl2-config --cflags)
LIBS_FLAGS += $(shell sdl2-config --libs) # separated with cflags for -mconsole
LIBS_FLAGS += -mconsole # part of --libs, uncomment it if printf console is needed
LIBS_FLAGS += $(shell pkg-config $(LIBS) --cflags --libs)

#######################################
ifeq (,$(DIR_USER))
DIR_USER := user
endif
DIR_SRC := src
DIRS_INCLUDE := $(DIR_SRC) $(DIR_USER)

SOURCES_C :=
SOURCES_C += $(wildcard $(DIR_SRC)/*.c)
# SOURCES_C += $(DIR_SRC)/main.c
# SOURCES_C += $(wildcard $(DIR_USER)/*.c)
# SOURCES_C += $(DIR_USER)/userpanel_empty.c
SOURCES_C += $(DIR_USER)/userpanel.c

#######################################
# user libraries
DIR_MGL := libs/minimalgraphics
DIRS_INCLUDE += $(DIR_MGL)
SOURCES_C += $(DIR_MGL)/mgl.c
SOURCES_C += $(DIR_MGL)/5monotxt.c
SOURCES_C += $(DIR_MGL)/5x7mod.c

DIR_MIDI := libs/mbwmidi
DIRS_INCLUDE += $(DIR_MIDI)
SOURCES_C += $(wildcard $(DIR_MIDI)/*.c)

#######################################
FLAGS_C := 
FLAGS_C += $(addprefix -I,$(DIRS_INCLUDE))
#FLAGS_C += -std=gnu11
FLAGS_C += -O3
FLAGS_C += -g3
FLAGS_C += -Wall -Wpedantic
FLAGS_C += -DSDL_ASSERT_LEVEL=3
# FLAGS_C += -Xlinker -Map=$(TARGET_PE).map

#######################################
$(TARGET_PE): .force_remake

$(TARGET_PE): $(SOURCES_C)
	@echo "userdir: $(DIR_USER)"
	mkdir -p $(dir $(TARGET_PE))
	gcc $(FLAGS_C) $(SOURCES_C) -o $@ $(LIBS_FLAGS)
	./$(TARGET_PE)

.force_remake:

clean:
	rm -rf $(TARGET_PE)


