#include "game_logic.hpp"

#include <algorithm>
#include <array>
#include <iostream>

GameLogic::GameLogic() : _board(9, 9), _state(kPaused), _goal(50), _score(0) {}

GameLogic::~GameLogic() {}

void GameLogic::MouseClick(float x, float y) {
  if (_state == kPlaying) {
    _board.DragStart({x, y});
  }
}

void GameLogic::MouseMove(float x, float y) {
  if (_state == kPlaying) {
    _board.DragMove({x, y});
  }
}

void GameLogic::MouseRelease(float x, float y) {
  switch (_state) {
    case kPlaying: {
      _score += _board.DragReleaseAndCheckMove({x, y});
      if (_score >= _goal) {
        _board.Clear();
        _state = kLevelComplete;
      }
      break;
    }
    case kPaused: {
      _state = kPlaying;
      break;
    }
    case kLevelComplete: {
      _goal *= 1.5f;
      _board.Create(_board.width() + 3, _board.height() + 3);
      _score = 0;
      _state = kPlaying;
      break;
    }
  }
}
