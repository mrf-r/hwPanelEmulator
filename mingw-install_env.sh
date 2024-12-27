# MINGW_PACKAGE_PREFIX=mingw-w64-x86_64

# build and flash ------------------------------------------
# pacman -S --noconfirm \
# make
# $MINGW_PACKAGE_PREFIX-arm-none-eabi-gcc \
# $MINGW_PACKAGE_PREFIX-openocd \
# $MINGW_PACKAGE_PREFIX-putty \
# $MINGW_PACKAGE_PREFIX-gdb-multiarch \
# $MINGW_PACKAGE_PREFIX-python-pip \

# pip install terminaltables cmsis-svd

# additional libs for emulators ----------------------------
pacman -S --noconfirm \
pkg-config \
$MINGW_PACKAGE_PREFIX-SDL2 \
$MINGW_PACKAGE_PREFIX-libserialport \
$MINGW_PACKAGE_PREFIX-portaudio \
$MINGW_PACKAGE_PREFIX-portmidi \
