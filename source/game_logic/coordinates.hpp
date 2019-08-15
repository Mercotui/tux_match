#ifndef COORDINATES_HPP
#define COORDINATES_HPP

typedef struct {
  int x;
  int y;
} Coordinates;

typedef struct {
  operator Coordinates() { return {static_cast<int>(x), static_cast<int>(y)}; }
  float x;
  float y;
} CoordinatesF;

#endif  // COORDINATES_HPP
