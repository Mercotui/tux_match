#include "game_board.hpp"

#include <algorithm>
#include <array>
#include <iostream>

GameBoard::GameBoard(int width, int height)
    : _random_generator(),
      _board_width(width),
      _board_height(height),
      _board_tiles_changed(true) {
  create_board();
}

GameBoard::~GameBoard() { ; }

void GameBoard::drag_start(CoordinatesF pos) {
  _drag_start_pos = clamp_to_board(pos);
  tile_at(_drag_start_pos).animation = kSTATIONARY;
}

void GameBoard::drag_move(CoordinatesF pos) {
  CoordinatesF clamped_pos = clamp_to_board(pos);

  // limit dragging to 4 connected neigbours, by an arbitrary algorithm
  float delta_x = clamped_pos.x - _drag_start_pos.x;
  float delta_y = clamped_pos.y - _drag_start_pos.y;
  float delta_total = fabs(delta_x) + fabs(delta_y);
  float delta_factor = std::max(1.0f, delta_total - 0.2f);
  delta_x = std::clamp(delta_x / delta_factor, -1.0f, 1.0f);
  delta_y = std::clamp(delta_y / delta_factor, -1.0f, 1.0f);

  // assign dragging offset
  BoardTile &tile = tile_at(_drag_start_pos);
  tile.offset_x = delta_x;
  tile.offset_y = delta_y;

  // see if evading should happen
  if (fabs(tile.offset_x) > kEvadeThreshold ||
      fabs(tile.offset_y) > kEvadeThreshold) {
    evade_tile();
  } else {
    evade_cancel(_drag_start_pos);
  }
}

int GameBoard::drag_release_and_check_move(CoordinatesF pos) {
  evade_cancel(_drag_start_pos);

  CoordinatesF clamped_pos = clamp_to_board(pos);
  Coordinates source_tile = _drag_start_pos;
  Coordinates destination_tile = source_tile;

  // determine destination tile
  float delta_x = clamped_pos.x - _drag_start_pos.x;
  float delta_y = clamped_pos.y - _drag_start_pos.y;
  if (fabs(delta_x) > fabs(delta_y)) {
    if (delta_x > 0.1f) {
      destination_tile.x++;
    } else if (delta_x < -0.1f) {
      destination_tile.x--;
    }
  } else {
    if (delta_y > 0.1f) {
      destination_tile.y++;
    } else if (delta_y < -0.1f) {
      destination_tile.y--;
    }
  }

  // check and execute move
  int score = 0;
  if (source_tile != destination_tile) {
    if (check_move(_drag_start_pos, destination_tile)) {
      score = execute_move(_drag_start_pos, destination_tile);
    } else {
      _board[_drag_start_pos.x][_drag_start_pos.y].animation = kRETURN;
    }
  } else {
    _board[_drag_start_pos.x][_drag_start_pos.y].animation = kRETURN;
  }

  return score;
}

void GameBoard::physics_tick() {
  bool deletes_done = false;
  for (auto &column : _board) {
    column.resize(_board_height);
    for (auto &piece : column) {
      switch (piece.animation) {
        case kSTATIONARY:
          break;
        case kRETURN:
          piece.offset_x *= kReturnSpeed;
          piece.offset_y *= kReturnSpeed;

          if (fabs(piece.offset_x) < kStationaryThreshold &&
              fabs(piece.offset_y) < kStationaryThreshold) {
            piece.offset_x = 0.0f;
            piece.offset_y = 0.0f;
            piece.animation = kSTATIONARY;
          }
          break;
        case kFALL:
          piece.offset_y -= kFallSpeed;
          break;
        case kDELETE:
        case kDELETE_DONE:
          piece.offset_x += 0.2f;
          if (piece.offset_x > kDeleteThreshold) {
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

void GameBoard::create_board() {
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

int GameBoard::width() { return _board_width; }

int GameBoard::height() { return _board_height; }

const std::vector<std::vector<GameBoard::BoardTile>> &GameBoard::game_board() {
  return _board;
}

CoordinatesF GameBoard::clamp_to_board(CoordinatesF pos) {
  // limit dragging to board
  return {std::clamp(pos.x, 0.5f, _board_width - 0.5f),
          std::clamp(pos.y, 0.5f, _board_height - 0.5f)};
}

GameBoard::BoardTile &GameBoard::tile_at(CoordinatesF pos) {
  return _board.at(static_cast<int>(pos.x)).at(static_cast<int>(pos.y));
}

void GameBoard::evade_tile() {
  BoardTile &tile = tile_at(_drag_start_pos);
  Animation evade_animation;
  CoordinatesF evading_tile = _drag_start_pos;

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
  evade_cancel(_drag_start_pos);

  tile_at(evading_tile).animation = evade_animation;
}

void GameBoard::evade_cancel(Coordinates pos) {
  if (pos.x >= 1) {
    _board[pos.x - 1][pos.y].animation = kRETURN;
  }
  if (pos.x < _board_width - 1) {
    _board[pos.x + 1][pos.y].animation = kRETURN;
  }
  if (pos.y >= 1) {
    _board[pos.x][pos.y - 1].animation = kRETURN;
  }
  if (pos.y < _board_height - 1) {
    _board[pos.x][pos.y + 1].animation = kRETURN;
  }
}

void GameBoard::swap_tile(Coordinates source, Coordinates destination) {
  int delta_x = source.x - destination.x;
  int delta_y = source.y - destination.y;
  _board[source.x][source.y].offset_x += delta_x;
  _board[source.x][source.y].offset_y += delta_y;
  _board[destination.x][destination.y].offset_x -= delta_x;
  _board[destination.x][destination.y].offset_y -= delta_y;
  std::swap(_board[source.x][source.y], _board[destination.x][destination.y]);
}

void GameBoard::delete_and_replenish() {
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

bool GameBoard::check_move(Coordinates source, Coordinates destination) {
  bool move_valid = false;
  if (_board_tiles_changed) {
    label_blobs();
    _board_tiles_changed = false;
  }

  auto source_blobs =
      get_neighbour_blobs(source, _board[destination.x][destination.x].type);
  auto destination_blobs =
      get_neighbour_blobs(destination, _board[source.x][source.y].type);

  int source_total_blob_size = 0;
  for (auto label : source_blobs) {
    source_total_blob_size += _blob_histogram.at(label);
  }
  int destination_total_blob_size = 0;
  for (auto label : destination_blobs) {
    destination_total_blob_size += _blob_histogram.at(label);
  }

  if (source_total_blob_size >= kBlobThreshold) {
    move_valid = true;
  } else if (destination_total_blob_size >= kBlobThreshold) {
    move_valid = true;
  }

  return move_valid;
}

int GameBoard::execute_move(Coordinates source, Coordinates destination) {
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

  int score = 0;
  if (source_total_blob_size >= kBlobThreshold) {
    _board[source.x][source.y].animation = kDELETE;
    mark_blobs_for_deletion(source_blobs);
    score += source_total_blob_size;
  }
  if (destination_total_blob_size >= kBlobThreshold) {
    _board[destination.x][destination.y].animation = kDELETE;
    mark_blobs_for_deletion(destination_blobs);
    score += destination_total_blob_size;
  }
  return score;
}

void GameBoard::label_blobs() {
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

std::set<int> GameBoard::get_neighbour_blobs(Coordinates pos) {
  return get_neighbour_blobs(pos, _board[pos.x][pos.y].type);
}

std::set<int> GameBoard::get_neighbour_blobs(Coordinates pos, PieceType type) {
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

std::set<int> GameBoard::get_past_neighbour_blobs(Coordinates pos) {
  return get_past_neighbour_blobs(pos, _board[pos.x][pos.y].type);
}

std::set<int> GameBoard::get_past_neighbour_blobs(Coordinates pos,
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

int GameBoard::mark_blobs_for_deletion(std::set<int> marked_labels) {
  for (auto &column : _board) {
    for (auto &tile : column) {
      if (marked_labels.find(tile.blob_label) != marked_labels.end()) {
        tile.animation = kDELETE;
      }
    }
  }
  return 1;
}
