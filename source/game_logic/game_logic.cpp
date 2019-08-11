#include "game_logic.hpp"

GameLogic::GameLogic()
    : _random_generator(), _board_width(9), _board_height(9) {
  create_board();
}

GameLogic::~GameLogic() { ; }

void GameLogic::mouse_click(float x, float y) { ; }

void GameLogic::mouse_release() { ; }

void GameLogic::create_board() {
  std::uniform_int_distribution<> random_distribution(kTUX, kWILDEBEEST);
  std::uniform_int_distribution<> random_offset(-1, 1);

  _board.resize(_board_width);
  for (auto &column : _board) {
    column.resize(_board_height);
    for (auto &piece : column) {
      piece.type =
          static_cast<PieceType>(random_distribution(_random_generator));
      piece.offset_x = random_offset(_random_generator);
      piece.offset_y = random_offset(_random_generator);
    }
  }
}

int GameLogic::width() { return _board_width; }

int GameLogic::height() { return _board_height; }

const std::vector<std::vector<GameLogic::BoardTile>> &GameLogic::game_board() {
  return _board;
}
