#ifndef GAME_LOGIC_HPP
#define GAME_LOGIC_HPP

#include <random>
#include <set>
#include <unordered_set>

class GameLogic {
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

  typedef struct {
    int x;
    int y;
  } Coordinates;

  typedef struct {
    float x;
    float y;
  } CoordinatesF;

  GameLogic();
  ~GameLogic();

  void mouse_click(float x, float y);
  void mouse_move(float x, float y);
  void mouse_release(float x, float y);
  void physics_tick();

  void create_board();
  int width();
  int height();
  const std::vector<std::vector<BoardTile>> &game_board();

  int score();
  int goal();

 private:
  static constexpr float kEvadeThreshold = 0.9f;
  static constexpr float kReturnSpeed = 0.8f;
  static constexpr float kStationaryThreshold = 0.1f;
  static constexpr float kFallSpeed = 0.2f;
  static constexpr float kDeleteThreshold = 2.0f;
  static constexpr int kBlobThreshold = 3;

  inline BoardTile &tile_at(CoordinatesF pos);
  void evade_tile();
  void swap_tile(Coordinates source, Coordinates destination);
  void delete_and_replenish();
  bool check_move(CoordinatesF source_f, CoordinatesF destination_f);
  void execute_move(CoordinatesF source_f, CoordinatesF destination_f);
  void label_blobs();
  void label_tile(int x, int y);
  inline std::set<int> get_neighbour_blobs(Coordinates pos);
  inline std::set<int> get_neighbour_blobs(Coordinates pos, PieceType type);
  inline std::set<int> get_past_neighbour_blobs(Coordinates pos);
  inline std::set<int> get_past_neighbour_blobs(Coordinates pos,
                                                PieceType type);
  inline int mark_blobs_for_deletion(std::set<int> marked_labels);

  std::vector<std::vector<BoardTile>> _board;
  std::vector<int> _blob_histogram;
  std::default_random_engine _random_generator;
  int _board_width;
  int _board_height;
  CoordinatesF _click_pos;
  int _goal;
  int _score;
  bool _board_tiles_changed;
};

#endif  // GAME_LOGIC_HPP
