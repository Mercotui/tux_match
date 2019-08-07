find_package(Qt5 REQUIRED Core Gui Widgets OpenGL)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( graphics_engine_SOURCES graphics_engine.cpp )
set( graphics_engine_HEADERS graphics_engine.hpp )
qt5_wrap_cpp(graphics_engine_MOC ${graphics_engine_HEADERS})
qt5_add_resources(graphics_engine_RRC shaders/GL_shaders.qrc)

add_library( graphics_engine STATIC ${graphics_engine_SOURCES} ${graphics_engine_RRC} ${graphics_engine_MOC} )
if(ANDROID)
    target_link_libraries( graphics_engine ${QT_LIBRARIES} GLESv3)
else()
    target_link_libraries( graphics_engine ${QT_LIBRARIES} )
endif()
target_include_directories ( graphics_engine PUBLIC ${INCLUDE_DIR} )

qt5_use_modules( graphics_engine Core Gui Widgets OpenGL )

target_compile_options(graphics_engine PRIVATE -std=c++11 -Wall -Wextra)
