TARGET_PE := build/test
ifeq ($(OS), Windows_NT)
	MINGW_PREFIX := /mingw64/bin/
else
	MINGW_PREFIX :=
endif
#######################################
# system libraries
LIBS := 
LIBS += sdl2
# LIBS += portmidi
# LIBS += libserialport
# LIBS += portaudio-2.0

# LIBS_FLAGS := $(shell sdl2-config --cflags --libs)
LIBS_FLAGS := $(shell sdl2-config --cflags --libs | sed -E s/-mwindows//) # console output
# LIBS_FLAGS := $(shell pkg-config $(LIBS) --cflags --libs)

#######################################
DIR_SRC := src
DIRS_INCLUDE := $(DIR_SRC) user

SOURCES_C :=
SOURCES_C += $(DIR_SRC)/wid_main.c
# SOURCES_C += $(DIR_SRC)/mgldisp_sdl.c
# SOURCES_C += $(wildcard $(DIR_SRC)/*.c)

#######################################
# user libraries
DIR_MGL := libs/minimalgraphics
DIRS_INCLUDE += $(DIR_MGL)
SOURCES_C += $(DIR_MGL)/mgl.c
SOURCES_C += $(DIR_MGL)/5monotxt.c
SOURCES_C += $(DIR_MGL)/5x7mod.c

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
	mkdir -p $(dir $(TARGET_PE))
	$(MINGW_PREFIX)gcc $(FLAGS_C) $(SOURCES_C) -o $@ $(LIBS_FLAGS)
	./$(TARGET_PE)

.force_remake:

clean:
	rm -rf $(TARGET_PE)


