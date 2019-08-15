#ifndef COORDINATES_HPP
#define COORDINATES_HPP

class Coordinates {
 public:
  Coordinates(int x = 0, int y = 0) : x(x), y(y){};
  bool operator==(const Coordinates& b) const { return x == b.x && y == b.y; }
  bool operator!=(const Coordinates& b) const { return x != b.x || y != b.y; }

  int x;
  int y;
};

class CoordinatesF {
 public:
  CoordinatesF(float x = 0, float y = 0) : x(x), y(y){};
  bool operator==(const CoordinatesF& b) const { return x == b.x && y == b.y; }
  bool operator!=(const CoordinatesF& b) const { return x != b.x || y != b.y; }
  operator Coordinates() { return {static_cast<int>(x), static_cast<int>(y)}; }

  float x;
  float y;
};

#endif  // COORDINATES_HPP
