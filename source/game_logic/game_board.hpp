#ifndef SOURCE_GAME_LOGIC_GAME_BOARD_HPP_
#define SOURCE_GAME_LOGIC_GAME_BOARD_HPP_

#include <random>
#include <set>
#include <vector>

#include "coordinates.hpp"

class GameBoard {
 public:
  enum PieceType { kTux = 0, kHat, kChameleon, kWildebeest };
  enum Animation {
    kStationary = 0,
    kReturn,
    kFall,
    kDelete,
    kDeleteDone,
    kEvadeUp,
    kEvadeDown,
    kEvadeLeft,
    kEvadeRight,
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

  void DragStart(CoordinatesF pos);
  void DragMove(CoordinatesF pos);
  int DragReleaseAndCheckMove(CoordinatesF pos);
  void PhysicsTick();

  void Create(int width, int height);
  void clear();
  int width();
  int height();
  const std::vector<std::vector<BoardTile>> &board();

 private:
  static constexpr float kEvadeThreshold = 0.9f;
  static constexpr float kReturnSpeed = 0.8f;
  static constexpr float kStationaryThreshold = 0.1f;
  static constexpr float kFallSpeed = 0.2f;
  static constexpr float kDeleteThreshold = 2.0f;
  static constexpr int kBlobThreshold = 3;

  BoardTile &TileAt(CoordinatesF pos);
  CoordinatesF ClampToBoard(CoordinatesF pos);
  void EvadeTile();
  void EvadeCancel(Coordinates pos);
  void SwapTile(Coordinates source, Coordinates destination);
  void DeleteAndReplenish();
  bool CheckMove(Coordinates source, Coordinates destination);
  int ExecuteMove(Coordinates source, Coordinates destination);
  void LabelBlobs();
  std::set<int> GetNeighbourBlobs(Coordinates pos);
  std::set<int> GetNeighbourBlobs(Coordinates pos, PieceType type);
  std::set<int> GetPastNeighbourBlobs(Coordinates pos);
  std::set<int> GetPastNeighbourBlobs(Coordinates pos, PieceType type);
  int MarkBlobsForDeletion(std::set<int> marked_labels);

  std::vector<std::vector<BoardTile>> _board;
  std::vector<int> _blob_histogram;
  std::default_random_engine _random_generator;
  int _board_width;
  int _board_height;
  CoordinatesF _drag_start_pos;
  bool _board_tiles_changed;
};

#endif  // SOURCE_GAME_LOGIC_GAME_BOARD_HPP_
