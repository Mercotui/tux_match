#include "game_logic.hpp"
#include <algorithm>
#include <iostream>

GameLogic::GameLogic()
    : _random_generator(), _board_width(9), _board_height(9) {
  create_board();
}

GameLogic::~GameLogic() { ; }

void GameLogic::mouse_click(float x, float y) {
  _click_pos = {x, y};
  _board[_click_pos.x][_click_pos.y].animation = kSATIONARY;
}

void GameLogic::mouse_move(float x, float y) {
  BoardTile &tile = _board[_click_pos.x][_click_pos.y];
  tile.offset_x = std::clamp(x - _click_pos.x, -1.0f, 1.0f);
  tile.offset_y = std::clamp(y - _click_pos.y, -1.0f, 1.0f);
}

void GameLogic::mouse_release(float x, float y) {
  _board[_click_pos.x][_click_pos.y].animation = kRETURN;
}

void GameLogic::physics_tick() {
  for (auto &column : _board) {
    column.resize(_board_height);
    for (auto &piece : column) {
      switch (piece.animation) {
        case kSATIONARY:
          break;
        case kRETURN:
          piece.offset_x *= 0.8f;
          piece.offset_y *= 0.8f;
          break;
      }
    }
  }
}

void GameLogic::create_board() {
  std::uniform_int_distribution<> random_distribution(kTUX, kWILDEBEEST);
  std::uniform_int_distribution<> random_offset(-1, 1);

  _board.resize(_board_width);
  for (auto &column : _board) {
    column.resize(_board_height);
    for (auto &piece : column) {
      piece.type =
          static_cast<PieceType>(random_distribution(_random_generator));
      piece.offset_x = 0;
      piece.offset_y = 0;
      piece.animation = kSATIONARY;
    }
  }
}

int GameLogic::width() { return _board_width; }

int GameLogic::height() { return _board_height; }

const std::vector<std::vector<GameLogic::BoardTile>> &GameLogic::game_board() {
  return _board;
}
