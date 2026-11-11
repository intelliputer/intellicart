#ifndef CHECKERS_GEOMETRY_GEOMETRY_HPP
#define CHECKERS_GEOMETRY_GEOMETRY_HPP

#include <stdint.h>

namespace checkers_geometry {

typedef uint8_t Square;
static const Square kInvalidSquare = 0xFFU;
static const uint8_t kBoardRows = 8;
static const uint8_t kBoardColumns = 8;
static const uint8_t kPlayableSquares = 32;

enum class Side : uint8_t {
    HUMAN = 0,
    AGENT = 1
};

enum class ValidationError : uint8_t {
    NONE = 0,
    OVERLAPPING_PIECES,
    ORPHAN_KING,
    CONTINUATION_OUT_OF_RANGE,
    CONTINUATION_EMPTY,
    CONTINUATION_WRONG_SIDE
};

struct Coordinate {
    uint8_t row;
    uint8_t column;
};

struct Position {
    uint32_t humanMen;
    uint32_t agentMen;
    uint32_t kings;
    Side sideToMove;
    bool jumpContinuation;
    Square continuationSquare;
};

bool isPlayableSquare(Square square);
bool toCoordinate(Square square, Coordinate* coordinate);
bool fromCoordinate(uint8_t row, uint8_t column, Square* square);
Square step(Square square, int8_t rowDelta, int8_t columnDelta);
Square jump(Square square, int8_t rowDelta, int8_t columnDelta);
ValidationError validate(const Position& position);

}  // namespace checkers_geometry

#endif
