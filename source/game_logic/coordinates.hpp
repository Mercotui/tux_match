#ifndef SOURCE_GAME_LOGIC_COORDINATES_HPP_
#define SOURCE_GAME_LOGIC_COORDINATES_HPP_

class Coordinates {
 public:
  Coordinates() : x(0), y(0) {}
  Coordinates(int x, int y) : x(x), y(y) {}
  bool operator==(const Coordinates& b) const { return x == b.x && y == b.y; }
  bool operator!=(const Coordinates& b) const { return x != b.x || y != b.y; }

  int x;
  int y;
};

class CoordinatesF {
 public:
  CoordinatesF() : x(0.0f), y(0.0f) {}
  CoordinatesF(float x, float y) : x(x), y(y) {}
  bool operator==(const CoordinatesF& b) const { return x == b.x && y == b.y; }
  bool operator!=(const CoordinatesF& b) const { return x != b.x || y != b.y; }
  operator Coordinates() { return {static_cast<int>(x), static_cast<int>(y)}; }

  float x;
  float y;
};

#endif  // SOURCE_GAME_LOGIC_COORDINATES_HPP_
