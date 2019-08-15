#ifndef GAME_LOGIC_HPP
#define GAME_LOGIC_HPP

#include <random>
#include <set>
#include <unordered_set>

#include "coordinates.hpp"
#include "game_board.hpp"

class GameLogic {
 public:
  GameLogic();
  ~GameLogic();

  void mouse_click(float x, float y);
  void mouse_move(float x, float y);
  void mouse_release(float x, float y);
  void physics_tick() { _board.physics_tick(); };

  void create_board();
  int width() { return _board.width(); };
  int height() { return _board.height(); };
  const GameBoard &game_board();

  int score();
  int goal();

 private:
  GameBoard _board;
  CoordinatesF _click_pos;
  int _goal;
  int _score;
};

#endif  // GAME_LOGIC_HPP
