#include "game_logic.hpp"

#include <algorithm>
#include <array>
#include <iostream>

GameLogic::GameLogic() : _board(9, 9), _goal(50), _score(0) { ; }

GameLogic::~GameLogic() { ; }

void GameLogic::mouse_click(float x, float y) { _board.drag_start({x, y}); }

void GameLogic::mouse_move(float x, float y) { _board.drag_move({x, y}); }

void GameLogic::mouse_release(float x, float y) { _board.drag_release({x, y}); }

const GameBoard &GameLogic::game_board() { return _board; }

int GameLogic::goal() { return _goal; }

int GameLogic::score() { return _score; }
