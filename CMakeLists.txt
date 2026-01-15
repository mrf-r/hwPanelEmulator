
file(GLOB HWPANELEMULATOR_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.c)

add_library(hwPanelEmulator OBJECT ${HWPANELEMULATOR_SOURCES})

target_link_libraries(hwPanelEmulator
    PUBLIC
        mbwmidi
        minimalgraphics
        hwPanelEmulator_config
)

target_compile_options(hwPanelEmulator
    PRIVATE
        -std=c11 -Wdouble-promotion
        -Wall -Wextra -Wpedantic
)

find_package(PkgConfig REQUIRED)

set(LIBS sdl2 portmidi libserialport portaudio-2.0)
foreach(lib ${LIBS})
    pkg_check_modules(${lib} REQUIRED ${lib})
    target_include_directories(hwPanelEmulator PUBLIC ${${lib}_INCLUDE_DIRS})
    target_link_libraries(hwPanelEmulator PUBLIC ${${lib}_LIBRARIES})
    link_directories(${${lib}_LIBRARY_DIRS})
endforeach(lib)

add_library(mbwmidi_config INTERFACE)
target_sources(mbwmidi_config
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS
        ${CMAKE_CURRENT_LIST_DIR}
        ${sdl2_INCLUDE_DIRS}
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/midi_conf.h
)
    
add_library(minimalgraphics_config INTERFACE)
target_sources(minimalgraphics_config
    INTERFACE
    FILE_SET HEADERS
    BASE_DIRS
        ${CMAKE_CURRENT_LIST_DIR}
        ${sdl2_INCLUDE_DIRS}
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/mgl_conf.h
)
