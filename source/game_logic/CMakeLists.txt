set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( game_logic_SOURCES game_logic.cpp )
set( game_logic_HEADERS game_logic.hpp )

add_library( game_logic STATIC ${game_logic_SOURCES} )
target_include_directories ( game_logic PUBLIC ${INCLUDE_DIR} )
target_compile_options(game_logic PRIVATE -std=c++17 -Wall -Wextra)
