#ifndef GAME_BOARD_HPP
#define GAME_BOARD_HPP

#include <random>
#include <set>

#include "coordinates.hpp"

class GameBoard {
 public:
  enum PieceType { kTUX = 0, kHAT, kCHAMELEON, kWILDEBEEST };
  enum Animation {
    kSTATIONARY = 0,
    kRETURN,
    kFALL,
    kDELETE,
    kDELETE_DONE,
    kEVADE_UP,
    kEVADE_DOWN,
    kEVADE_LEFT,
    kEVADE_RIGHT,
  };

  typedef struct {
    float offset_x;
    float offset_y;
    PieceType type;
    Animation animation;
    int blob_label;
  } BoardTile;

  GameBoard(int width, int height);
  ~GameBoard();

  void drag_start(CoordinatesF pos);
  void drag_move(CoordinatesF pos);
  int drag_release_and_check_move(CoordinatesF pos);
  void physics_tick();

  void create();
  void clear();
  int width();
  int height();
  const std::vector<std::vector<BoardTile>> &game_board();

 private:
  static constexpr float kEvadeThreshold = 0.9f;
  static constexpr float kReturnSpeed = 0.8f;
  static constexpr float kStationaryThreshold = 0.1f;
  static constexpr float kFallSpeed = 0.2f;
  static constexpr float kDeleteThreshold = 2.0f;
  static constexpr int kBlobThreshold = 3;

  BoardTile &tile_at(CoordinatesF pos);
  CoordinatesF clamp_to_board(CoordinatesF pos);
  void evade_tile();
  void evade_cancel(Coordinates pos);
  void swap_tile(Coordinates source, Coordinates destination);
  void delete_and_replenish();
  bool check_move(Coordinates source, Coordinates destination);
  int execute_move(Coordinates source, Coordinates destination);
  void label_blobs();
  void label_tile(int x, int y);
  std::set<int> get_neighbour_blobs(Coordinates pos);
  std::set<int> get_neighbour_blobs(Coordinates pos, PieceType type);
  std::set<int> get_past_neighbour_blobs(Coordinates pos);
  std::set<int> get_past_neighbour_blobs(Coordinates pos, PieceType type);
  int mark_blobs_for_deletion(std::set<int> marked_labels);

  std::vector<std::vector<BoardTile>> _board;
  std::vector<int> _blob_histogram;
  std::default_random_engine _random_generator;
  int _board_width;
  int _board_height;
  CoordinatesF _drag_start_pos;
  bool _board_tiles_changed;
};

#endif  // GAME_BOARD_HPP
