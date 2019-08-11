#ifndef GAME_LOGIC_HPP
#define GAME_LOGIC_HPP

#include <random>
#include <vector>

class GameLogic {
 public:
  enum PieceType { kTUX = 0, kHAT, kCHAMELEON, kWILDEBEEST };
  enum Animation { kSATIONARY = 0, kRETURN };

  typedef struct {
    float offset_x;
    float offset_y;
    PieceType type;
    Animation animation;
  } BoardTile;

  typedef struct {
    float x;
    float y;
  } Coordinates;

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

 private:
  std::vector<std::vector<BoardTile>> _board;

  std::default_random_engine _random_generator;
  int _board_width;
  int _board_height;
  Coordinates _click_pos;
};

#endif  // GAME_LOGIC_HPP
