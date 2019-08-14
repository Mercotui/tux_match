#include "game_logic.hpp"

#include <algorithm>
#include <array>
#include <iostream>

GameLogic::GameLogic()
    : _random_generator(),
      _board_width(9),
      _board_height(9),
      _goal(50),
      _score(0),
      _board_tiles_changed(true) {
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

  float x_clamped = _click_pos.x + std::clamp(x - _click_pos.x, -1.0f, 1.0f);
  float y_clamped = _click_pos.y + std::clamp(y - _click_pos.y, -1.0f, 1.0f);
  CoordinatesF destination_tile = {x_clamped, y_clamped};

  if (check_move(_click_pos, destination_tile)) {
    execute_move(_click_pos, destination_tile);
  }
}

void GameLogic::physics_tick() {
  bool deletes_done = false;
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
          piece.offset_y -= 0.2f;
          break;
        case kDELETE:
        case kDELETE_DONE:
          piece.offset_x += 0.2f;
          if (piece.offset_x > 2.0f) {
            piece.animation = kDELETE_DONE;
            deletes_done = true;
          }
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
  if (deletes_done) {
    delete_and_replenish();
  }
}

void GameLogic::create_board() {
  std::uniform_int_distribution<> random_distribution(kTUX, kWILDEBEEST);

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

GameLogic::BoardTile &GameLogic::tile_at(CoordinatesF pos) {
  return _board.at(static_cast<int>(pos.x)).at(static_cast<int>(pos.y));
}

void GameLogic::evade_tile() {
  BoardTile &tile = tile_at(_click_pos);
  Animation evade_animation;
  CoordinatesF evading_tile = _click_pos;

  // evade to the direction from which the dragged tile came
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
  if (_click_pos.x >= 1) {
    tile_at({_click_pos.x - 1, _click_pos.y}).animation = kRETURN;
  }
  if (_click_pos.x < _board_width - 1) {
    tile_at({_click_pos.x + 1, _click_pos.y}).animation = kRETURN;
  }
  if (_click_pos.y >= 1) {
    tile_at({_click_pos.x, _click_pos.y - 1}).animation = kRETURN;
  }
  if (_click_pos.y < _board_height - 1) {
    tile_at({_click_pos.x, _click_pos.y + 1}).animation = kRETURN;
  }

  // set evading tile
  tile_at(evading_tile).animation = evade_animation;
}

void GameLogic::swap_tile(Coordinates source, Coordinates destination) {
  int delta_x = source.x - destination.x;
  int delta_y = source.y - destination.y;
  _board[source.x][source.y].offset_x += delta_x;
  _board[source.x][source.y].offset_y += delta_y;
  _board[destination.x][destination.y].offset_x -= delta_x;
  _board[destination.x][destination.y].offset_y -= delta_y;
  std::swap(_board[source.x][source.y], _board[destination.x][destination.y]);
}

void GameLogic::delete_and_replenish() {
  std::uniform_int_distribution<> random_distribution(kTUX, kWILDEBEEST);
  BoardTile new_tile = {0, 0, kTUX, kRETURN, 0};

  for (auto &column : _board) {
    int colum_deletion_count = 0;
    for (auto piece_it = column.begin(); piece_it != column.end();) {
      if (piece_it->animation == kDELETE_DONE) {
        ++colum_deletion_count;

        piece_it = column.erase(piece_it);
        new_tile.type =
            static_cast<PieceType>(random_distribution(_random_generator));
        column.push_back(new_tile);
      } else {
        if (piece_it->animation != kDELETE) {
          piece_it->offset_y += colum_deletion_count;
          piece_it->animation = kRETURN;
        }
        piece_it++;
      }
    }
  }
}

bool GameLogic::check_move(CoordinatesF source_f, CoordinatesF destination_f) {
  bool move_valid = false;
  if (_board_tiles_changed) {
    label_blobs();
    _board_tiles_changed = false;
  }

  Coordinates source = {static_cast<int>(source_f.x),
                        static_cast<int>(source_f.y)};
  Coordinates destination = {static_cast<int>(destination_f.x),
                             static_cast<int>(destination_f.y)};
  auto source_blobs = get_neighbour_blobs(source, tile_at(destination_f).type);
  auto destination_blobs =
      get_neighbour_blobs(destination, tile_at(source_f).type);

  int source_total_blob_size = 0;
  for (auto label : source_blobs) {
    source_total_blob_size += _blob_histogram.at(label);
  }
  int destination_total_blob_size = 0;
  for (auto label : destination_blobs) {
    destination_total_blob_size += _blob_histogram.at(label);
  }

  if (source_total_blob_size >= 3) {
    move_valid = true;
  } else if (destination_total_blob_size >= 3) {
    move_valid = true;
  }

  return move_valid;
}

void GameLogic::execute_move(CoordinatesF source_f,
                             CoordinatesF destination_f) {
  Coordinates source = {static_cast<int>(source_f.x),
                        static_cast<int>(source_f.y)};
  Coordinates destination = {static_cast<int>(destination_f.x),
                             static_cast<int>(destination_f.y)};
  swap_tile(source, destination);
  _board_tiles_changed = true;

  auto source_blobs = get_neighbour_blobs(source);
  auto destination_blobs = get_neighbour_blobs(destination);

  int source_total_blob_size = 1;
  for (auto label : source_blobs) {
    source_total_blob_size += _blob_histogram.at(label);
  }
  int destination_total_blob_size = 1;
  for (auto label : destination_blobs) {
    destination_total_blob_size += _blob_histogram.at(label);
  }

  if (source_total_blob_size >= 3) {
    _board[source.x][source.y].animation = kDELETE;
    mark_blobs_for_deletion(source_blobs);
    _score += source_total_blob_size;
  }
  if (destination_total_blob_size >= 3) {
    _board[destination.x][destination.y].animation = kDELETE;
    mark_blobs_for_deletion(destination_blobs);
    _score += destination_total_blob_size;
  }
}

void GameLogic::label_blobs() {
  // This is a naive labeling algorithm that loops through the 2D board multiple
  // times. As this only happens when the user attempts a move AND the playing
  // field has changed since last attempt, it should not be a performance
  // bottleneck.

  // first pass assign labels, and only check against previously assigned labels
  int blob_label = 0;
  for (int x = 0; x < _board_width; x++) {
    for (int y = 0; y < _board_height; y++) {
      auto neighbours = get_past_neighbour_blobs({x, y});
      auto lowest_neighbour_label = neighbours.cbegin();
      if (lowest_neighbour_label != neighbours.cend()) {
        _board[x][y].blob_label = *lowest_neighbour_label;
      } else {
        _board[x][y].blob_label = blob_label;
        ++blob_label;
      }
    }
  }

  // consecutive passes, reduce to continuous blobs and build histogram
  bool changed;
  do {
    changed = false;
    _blob_histogram.clear();
    _blob_histogram.resize(blob_label, 0);

    for (int x = 0; x < _board_width; x++) {
      for (int y = 0; y < _board_height; y++) {
        auto neighbours = get_neighbour_blobs({x, y});
        auto lowest_neighbour_label = neighbours.cbegin();
        if (lowest_neighbour_label != neighbours.cend()) {
          if (*lowest_neighbour_label < _board[x][y].blob_label) {
            _board[x][y].blob_label = *lowest_neighbour_label;
            changed = true;
          }
        }
        _blob_histogram[_board[x][y].blob_label]++;
      }
    }
  } while (changed);
}

std::set<int> GameLogic::get_neighbour_blobs(Coordinates pos) {
  return get_neighbour_blobs(pos, _board[pos.x][pos.y].type);
}

std::set<int> GameLogic::get_neighbour_blobs(Coordinates pos, PieceType type) {
  std::set<int> ret;
  if (pos.x > 0) {
    if (_board[pos.x - 1][pos.y].type == type) {
      ret.insert(_board[pos.x - 1][pos.y].blob_label);
    }
  }
  if (pos.y > 0) {
    if (_board[pos.x][pos.y - 1].type == type) {
      ret.insert(_board[pos.x][pos.y - 1].blob_label);
    }
  }
  if (pos.x < (_board_width - 1)) {
    if (_board[pos.x + 1][pos.y].type == type) {
      ret.insert(_board[pos.x + 1][pos.y].blob_label);
    }
  }
  if (pos.y < (_board_height - 1)) {
    if (_board[pos.x][pos.y + 1].type == type) {
      ret.insert(_board[pos.x][pos.y + 1].blob_label);
    }
  }
  return ret;
}

std::set<int> GameLogic::get_past_neighbour_blobs(Coordinates pos) {
  return get_past_neighbour_blobs(pos, _board[pos.x][pos.y].type);
}

std::set<int> GameLogic::get_past_neighbour_blobs(Coordinates pos,
                                                  PieceType type) {
  std::set<int> ret;
  if (pos.x > 0) {
    if (_board[pos.x - 1][pos.y].type == type) {
      ret.insert(_board[pos.x - 1][pos.y].blob_label);
    }
  }
  if (pos.y > 0) {
    if (_board[pos.x][pos.y - 1].type == type) {
      ret.insert(_board[pos.x][pos.y - 1].blob_label);
    }
  }
  return ret;
}

int GameLogic::mark_blobs_for_deletion(std::set<int> marked_labels) {
  std::cout << std::endl;
  for (auto &column : _board) {
    for (auto &tile : column) {
      if (marked_labels.find(tile.blob_label) != marked_labels.end()) {
        tile.animation = kDELETE;
      }
    }
  }
  return 1;
}
