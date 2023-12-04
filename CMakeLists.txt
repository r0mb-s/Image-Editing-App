cmake_minimum_required(VERSION 3.5)

project(QT_App VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(OpenCV REQUIRED)
find_package(ALSA REQUIRED)
find_package(PulseAudio REQUIRED)

include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( ${ALSA_INCLUDE_DIRS} )
include_directories( ${PULSEAUDIO_INCLUDE_DIR} )
include_directories( /usr/include/speech_tools )
include_directories( /usr/include/festival )

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

qt_add_executable(QT_App
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(QT_App PUBLIC
    Qt6::Widgets
    ${OpenCV_LIBS}
    tesseract
    /usr/lib/libFestival.a
    /usr/lib/speech_tools/lib/libestools.a
    /usr/lib/speech_tools/lib/libeststring.a
    /usr/lib/speech_tools/lib/libestbase.a
    -fopenmp
    -lncurses
    -lpulse-simple
    -lpulse
    ${ALSA_LIBRARIES}
)

set_target_properties(QT_App PROPERTIES WIN32_EXECUTABLE TRUE)

install(TARGETS QT_App
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

qt_finalize_executable(QT_App)
