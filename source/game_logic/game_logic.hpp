#ifndef SOURCE_GAME_LOGIC_GAME_LOGIC_HPP_
#define SOURCE_GAME_LOGIC_GAME_LOGIC_HPP_

#include <random>
#include <set>
#include <unordered_set>

#include "coordinates.hpp"
#include "game_board.hpp"

class GameLogic {
 public:
  enum GameState { kPlaying = 0, kPaused, kLevelComplete };

  GameLogic();
  ~GameLogic();

  void MouseClick(float x, float y);
  void MouseMove(float x, float y);
  void MouseRelease(float x, float y);
  void PhysicsTick() { _board.PhysicsTick(); }

  int width() { return _board.width(); }
  int height() { return _board.height(); }
  const GameBoard &board();

  int score();
  int goal();
  GameState state();

 private:
  GameBoard _board;
  GameState _state;
  CoordinatesF _click_pos;
  int _goal;
  int _score;
};

#endif  // SOURCE_GAME_LOGIC_GAME_LOGIC_HPP_
