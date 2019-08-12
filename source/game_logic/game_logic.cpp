#include "game_logic.hpp"
#include <algorithm>
#include <iostream>

GameLogic::GameLogic()
    : _random_generator(),
      _board_width(9),
      _board_height(9),
      _goal(4),
      _score(0) {
  create_board();
}

GameLogic::~GameLogic() { ; }

void GameLogic::mouse_click(float x, float y) {
  _click_pos = {x, y};
  tile_at(_click_pos).animation = kSTATIONARY;
}

void GameLogic::mouse_move(float x, float y) {
  float x_clamped = std::clamp(x, 0.0f, 8.9f);
  float y_clamped = std::clamp(y, 0.0f, 8.9f);

  BoardTile &tile = tile_at(_click_pos);
  tile.offset_x = std::clamp(x_clamped - _click_pos.x, -1.0f, 1.0f);
  tile.offset_y = std::clamp(y_clamped - _click_pos.y, -1.0f, 1.0f);

  if (fabs(tile.offset_x) > 0.9f || fabs(tile.offset_y) > 0.9f) {
    evade_tile();
  }
}

void GameLogic::mouse_release(float x, float y) {
  _board[_click_pos.x][_click_pos.y].animation = kRETURN;

  for (auto &column : _board) {
    column.resize(_board_height);
    for (auto &piece : column) {
      if (piece.animation == kEVADE_UP || piece.animation == kEVADE_DOWN ||
          piece.animation == kEVADE_LEFT || piece.animation == kEVADE_RIGHT) {
        piece.animation = kRETURN;
      }
    }
  }

  float x_clamped = std::clamp(x, -1.0f, 1.0f);
  float y_clamped = std::clamp(y, -1.0f, 1.0f);
  check_move(_click_pos, {x_clamped, y_clamped});
}

void GameLogic::physics_tick() {
  for (auto &column : _board) {
    column.resize(_board_height);
    for (auto &piece : column) {
      switch (piece.animation) {
        case kSTATIONARY:
          break;
        case kRETURN:
          piece.offset_x *= 0.8f;
          piece.offset_y *= 0.8f;

          if (fabs(piece.offset_x) < 0.1f && fabs(piece.offset_y) < 0.1f) {
            piece.offset_x = 0.0f;
            piece.offset_y = 0.0f;
            piece.animation = kSTATIONARY;
          }
          break;
        case kFALL:
          piece.offset_x += 0.2f;
          break;
        case kEVADE_UP:
          piece.offset_y = std::max(piece.offset_y - 0.1f, -1.0f);
          break;
        case kEVADE_DOWN:
          piece.offset_y = std::min(piece.offset_y + 0.1f, 1.0f);
          break;
        case kEVADE_LEFT:
          piece.offset_x = std::max(piece.offset_x - 0.1f, -1.0f);
          break;
        case kEVADE_RIGHT:
          piece.offset_x = std::min(piece.offset_x + 0.1f, 1.0f);
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
      piece.animation = kSTATIONARY;
    }
  }
}

int GameLogic::width() { return _board_width; }

int GameLogic::height() { return _board_height; }

const std::vector<std::vector<GameLogic::BoardTile>> &GameLogic::game_board() {
  return _board;
}

int GameLogic::goal() { return _goal; }

int GameLogic::score() { return _score; }

GameLogic::BoardTile &GameLogic::tile_at(Coordinates pos) {
  return _board.at(static_cast<int>(pos.x)).at(static_cast<int>(pos.y));
}

void GameLogic::evade_tile() {
  BoardTile &tile = tile_at(_click_pos);
  Animation evade_animation;
  Coordinates evading_tile = _click_pos;

  if (fabs(tile.offset_x) > fabs(tile.offset_y)) {
    if (tile.offset_x > 0) {
      evade_animation = kEVADE_LEFT;
      evading_tile.x++;
    } else {
      evade_animation = kEVADE_RIGHT;
      evading_tile.x--;
    }
  } else {
    if (tile.offset_y > 0) {
      evade_animation = kEVADE_UP;
      evading_tile.y++;
    } else {
      evade_animation = kEVADE_DOWN;
      evading_tile.y--;
    }
  }

  // reset other tiles
  tile_at({_click_pos.x - 1, _click_pos.y}).animation = kRETURN;
  tile_at({_click_pos.x + 1, _click_pos.y}).animation = kRETURN;
  tile_at({_click_pos.x, _click_pos.y - 1}).animation = kRETURN;
  tile_at({_click_pos.x, _click_pos.y + 1}).animation = kRETURN;

  // set evading tile
  tile_at(evading_tile).animation = evade_animation;
}

void GameLogic::check_move(Coordinates source, Coordinates destination) {
  for (auto column = _board.begin(); column != _board.end(); column++) {
    for (auto tile = column->begin(); tile != column->end(); tile++) {
      std::array<int, 4> blob_ids;
      std::cout << (tile - 1)->blob_id << std::endl;
    }
  }
}
