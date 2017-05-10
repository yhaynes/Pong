TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    pong.cpp

# Command
# -L[Directory path of "lib" folder] -lSDL2
LIBS += -LC://SDL2_libs/SDL2-2.0.5//i686-w64-mingw32//lib -lSDL2

# [Directory of "include"]
INCLUDEPATH += C://SDL2_libs/SDL2-2.0.5//i686-w64-mingw32//include//SDL2

# Command
# -L[Directory path of "lib" folder] -lSDL2
LIBS += -LC://SDL2_libs/SDL2_image-2.0.1//i686-w64-mingw32//lib -lSDL2_image
LIBS += -LC://SDL2_libs/SDL2_mixer-2.0.1//i686-w64-mingw32//lib -lSDL2_mixer
LIBS += -LC://SDL2_libs/SDL2_ttf-2.0.14//i686-w64-mingw32//lib -lSDL2_ttf 


# [Directory of "include"]
INCLUDEPATH += C://SDL2_libs/SDL2_image-2.0.1//i686-w64-mingw32//include//SDL2
INCLUDEPATH += C://SDL2_libs/SDL2_mixer-2.0.1//i686-w64-mingw32//include//SDL2
INCLUDEPATH += C://SDL2_libs/SDL2_ttf-2.0.14//i686-w64-mingw32//include//SDL2