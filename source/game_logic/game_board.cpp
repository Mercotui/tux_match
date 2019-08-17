#include "game_board.hpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <utility>

GameBoard::GameBoard(int width, int height)
    : _random_generator(),
      _board_width(width),
      _board_height(height),
      _board_tiles_changed(true) {
  Create(width, height);
}

GameBoard::~GameBoard() { ; }

void GameBoard::DragStart(CoordinatesF pos) {
  _drag_start_pos = ClampToBoard(pos);
  TileAt(_drag_start_pos).animation = kStationary;
}

void GameBoard::DragMove(CoordinatesF pos) {
  CoordinatesF clamped_pos = ClampToBoard(pos);

  // limit dragging to 4 connected neigbours, by an arbitrary algorithm
  float delta_x = clamped_pos.x - _drag_start_pos.x;
  float delta_y = clamped_pos.y - _drag_start_pos.y;
  float delta_total = fabs(delta_x) + fabs(delta_y);
  float delta_factor = std::max(1.0f, delta_total - 0.2f);
  delta_x = std::clamp(delta_x / delta_factor, -1.0f, 1.0f);
  delta_y = std::clamp(delta_y / delta_factor, -1.0f, 1.0f);

  // assign dragging offset
  BoardTile &tile = TileAt(_drag_start_pos);
  tile.offset_x = delta_x;
  tile.offset_y = delta_y;

  // see if evading should happen
  if (fabs(tile.offset_x) > kEvadeThreshold ||
      fabs(tile.offset_y) > kEvadeThreshold) {
    EvadeTile();
  } else {
    EvadeCancel(_drag_start_pos);
  }
}

int GameBoard::DragReleaseAndCheckMove(CoordinatesF pos) {
  EvadeCancel(_drag_start_pos);

  CoordinatesF clamped_pos = ClampToBoard(pos);
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
    if (CheckMove(_drag_start_pos, destination_tile)) {
      score = ExecuteMove(_drag_start_pos, destination_tile);
    } else {
      _board[_drag_start_pos.x][_drag_start_pos.y].animation = kReturn;
    }
  } else {
    _board[_drag_start_pos.x][_drag_start_pos.y].animation = kReturn;
  }

  return score;
}

void GameBoard::PhysicsTick() {
  bool deletes_done = false;
  for (auto &column : _board) {
    column.resize(_board_height);
    for (auto &piece : column) {
      switch (piece.animation) {
        case kStationary: {
          break;
        }
        case kReturn: {
          piece.offset_x *= kReturnSpeed;
          piece.offset_y *= kReturnSpeed;

          if (fabs(piece.offset_x) < kStationaryThreshold &&
              fabs(piece.offset_y) < kStationaryThreshold) {
            piece.offset_x = 0.0f;
            piece.offset_y = 0.0f;
            piece.animation = kStationary;
          }
          break;
        }
        case kFall: {
          std::uniform_int_distribution<> random_distribution(0, 1000);
          if (random_distribution(_random_generator) == 1) {
            piece.offset_y = _board_height + 1;
          }
          piece.offset_y -= kFallSpeed;
          break;
        }
        case kDelete:
        case kDeleteDone: {
          piece.offset_x += 0.2f;
          if (piece.offset_x > kDeleteThreshold) {
            piece.animation = kDeleteDone;
            deletes_done = true;
          }
          break;
          case kEvadeUp:
            piece.offset_y = std::max(piece.offset_y - 0.1f, -1.0f);
            break;
          case kEvadeDown:
            piece.offset_y = std::min(piece.offset_y + 0.1f, 1.0f);
            break;
          case kEvadeLeft:
            piece.offset_x = std::max(piece.offset_x - 0.1f, -1.0f);
            break;
          case kEvadeRight:
            piece.offset_x = std::min(piece.offset_x + 0.1f, 1.0f);
            break;
        }
      }
    }
  }
  if (deletes_done) {
    DeleteAndReplenish();
  }
}

void GameBoard::Create(int width, int height) {
  _board_width = width;
  _board_height = height;
  std::uniform_int_distribution<> random_distribution(kTux, kWildebeest);

  _board.resize(_board_width);
  for (auto &column : _board) {
    column.resize(_board_height);
    for (auto &piece : column) {
      piece.type =
          static_cast<PieceType>(random_distribution(_random_generator));
      piece.offset_x = 0;
      piece.offset_y = 0;
      piece.animation = kStationary;
    }
  }
}

void GameBoard::clear() {
  for (auto &column : _board) {
    for (auto &piece : column) {
      piece.animation = kFall;
    }
  }
}

int GameBoard::width() { return _board_width; }

int GameBoard::height() { return _board_height; }

const std::vector<std::vector<GameBoard::BoardTile>> &GameBoard::board() {
  return _board;
}

CoordinatesF GameBoard::ClampToBoard(CoordinatesF pos) {
  // limit dragging to board
  return {std::clamp(pos.x, 0.5f, _board_width - 0.5f),
          std::clamp(pos.y, 0.5f, _board_height - 0.5f)};
}

GameBoard::BoardTile &GameBoard::TileAt(CoordinatesF pos) {
  return _board.at(static_cast<int>(pos.x)).at(static_cast<int>(pos.y));
}

void GameBoard::EvadeTile() {
  BoardTile &tile = TileAt(_drag_start_pos);
  Animation evade_animation;
  CoordinatesF evading_tile = _drag_start_pos;

  // evade to the direction from which the dragged tile came
  if (fabs(tile.offset_x) > fabs(tile.offset_y)) {
    if (tile.offset_x > 0) {
      evade_animation = kEvadeLeft;
      evading_tile.x++;
    } else {
      evade_animation = kEvadeRight;
      evading_tile.x--;
    }
  } else {
    if (tile.offset_y > 0) {
      evade_animation = kEvadeUp;
      evading_tile.y++;
    } else {
      evade_animation = kEvadeDown;
      evading_tile.y--;
    }
  }
  // reset other tiles
  EvadeCancel(_drag_start_pos);

  TileAt(evading_tile).animation = evade_animation;
}

void GameBoard::EvadeCancel(Coordinates pos) {
  if (pos.x >= 1) {
    _board[pos.x - 1][pos.y].animation = kReturn;
  }
  if (pos.x < _board_width - 1) {
    _board[pos.x + 1][pos.y].animation = kReturn;
  }
  if (pos.y >= 1) {
    _board[pos.x][pos.y - 1].animation = kReturn;
  }
  if (pos.y < _board_height - 1) {
    _board[pos.x][pos.y + 1].animation = kReturn;
  }
}

void GameBoard::SwapTile(Coordinates source, Coordinates destination) {
  int delta_x = source.x - destination.x;
  int delta_y = source.y - destination.y;
  _board[source.x][source.y].offset_x += delta_x;
  _board[source.x][source.y].offset_y += delta_y;
  _board[destination.x][destination.y].offset_x -= delta_x;
  _board[destination.x][destination.y].offset_y -= delta_y;
  std::swap(_board[source.x][source.y], _board[destination.x][destination.y]);
}

void GameBoard::DeleteAndReplenish() {
  std::uniform_int_distribution<> random_distribution(kTux, kWildebeest);
  BoardTile new_tile = {0, 0, kTux, kReturn, 0};

  for (auto &column : _board) {
    int colum_deletion_count = 0;
    for (auto piece_it = column.begin(); piece_it != column.end();) {
      if (piece_it->animation == kDeleteDone) {
        ++colum_deletion_count;

        piece_it = column.erase(piece_it);
        new_tile.type =
            static_cast<PieceType>(random_distribution(_random_generator));
        column.push_back(new_tile);
      } else {
        if (piece_it->animation != kDelete) {
          piece_it->offset_y += colum_deletion_count;
          piece_it->animation = kReturn;
        }
        piece_it++;
      }
    }
  }
}

bool GameBoard::CheckMove(Coordinates source, Coordinates destination) {
  bool move_valid = false;
  if (_board_tiles_changed) {
    LabelBlobs();
    _board_tiles_changed = false;
  }

  auto source_blobs =
      GetNeighbourBlobs(source, _board[destination.x][destination.x].type);
  auto destination_blobs =
      GetNeighbourBlobs(destination, _board[source.x][source.y].type);

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

int GameBoard::ExecuteMove(Coordinates source, Coordinates destination) {
  SwapTile(source, destination);
  _board_tiles_changed = true;

  auto source_blobs = GetNeighbourBlobs(source);
  auto destination_blobs = GetNeighbourBlobs(destination);

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
    _board[source.x][source.y].animation = kDelete;
    MarkBlobsForDeletion(source_blobs);
    score += source_total_blob_size;
  }
  if (destination_total_blob_size >= kBlobThreshold) {
    _board[destination.x][destination.y].animation = kDelete;
    MarkBlobsForDeletion(destination_blobs);
    score += destination_total_blob_size;
  }
  return score;
}

void GameBoard::LabelBlobs() {
  // This is a naive labeling algorithm that loops through the 2D board multiple
  // times. As this only happens when the user attempts a move AND the playing
  // field has changed since last attempt, it should not be a performance
  // bottleneck.

  // first pass assign labels, and only check against previously assigned labels
  int blob_label = 0;
  for (int x = 0; x < _board_width; x++) {
    for (int y = 0; y < _board_height; y++) {
      auto neighbours = GetPastNeighbourBlobs({x, y});
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
        auto neighbours = GetNeighbourBlobs({x, y});
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

std::set<int> GameBoard::GetNeighbourBlobs(Coordinates pos) {
  return GetNeighbourBlobs(pos, _board[pos.x][pos.y].type);
}

std::set<int> GameBoard::GetNeighbourBlobs(Coordinates pos, PieceType type) {
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

std::set<int> GameBoard::GetPastNeighbourBlobs(Coordinates pos) {
  return GetPastNeighbourBlobs(pos, _board[pos.x][pos.y].type);
}

std::set<int> GameBoard::GetPastNeighbourBlobs(Coordinates pos,
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

int GameBoard::MarkBlobsForDeletion(std::set<int> marked_labels) {
  for (auto &column : _board) {
    for (auto &tile : column) {
      if (marked_labels.find(tile.blob_label) != marked_labels.end()) {
        tile.animation = kDelete;
      }
    }
  }
  return 1;
}
