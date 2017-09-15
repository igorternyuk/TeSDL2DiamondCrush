TEMPLATE = app
CONFIG += c++1z
CONFIG -= app_bundle
CONFIG -= qt

DEFINES -= DEBUG
LIBS += -lSDL2 -lSDL2_image -lSDL2_mixer

SOURCES += main.cpp \
    game.cpp \
    random.cpp

HEADERS += \
    game.hpp \
    random.h
