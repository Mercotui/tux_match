# Set up QT modules
QT       += core gui widgets opengl

# Set up application details
TARGET = tux_match
TEMPLATE = app
CONFIG += c++1z

# add include dirs
INCLUDEPATH += source
# Link and include the application source files
SOURCES += \
        source/main.cpp \
        source/graphics_engine/graphics_engine.cpp \
        source/graphics_engine/board_renderer.cpp \
        source/game_logic/game_logic.cpp \
        source/game_logic/game_board.cpp

HEADERS += \
        source/graphics_engine/graphics_engine.hpp \
        source/graphics_engine/board_renderer.hpp \
        source/game_logic/game_logic.hpp \
        source/game_logic/game_board.hpp \
        source/game_logic/coordinates.hpp

RESOURCES += \
    resources/app_resources.qrc \
    source/graphics_engine/shaders/GL_shaders.qrc
