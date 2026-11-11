#ifndef CHECKERS_BOARD_HPP
#define CHECKERS_BOARD_HPP

#include "checkers/types.hpp"

namespace checkers {

static const uint8_t kPlayableSquares = 32;
static const uint8_t kBoardRows = 8;
static const uint8_t kBoardColumns = 8;
static const uint8_t kInvalidSquare = 0xFFU;

enum class BoardError : uint8_t {
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

bool squareToCoordinate(uint8_t square, Coordinate* coordinate);
bool coordinateToSquare(uint8_t row, uint8_t column, uint8_t* square);
uint8_t stepSquare(uint8_t square, int8_t rowDelta, int8_t columnDelta);
uint8_t jumpSquare(uint8_t square, int8_t rowDelta, int8_t columnDelta);
BoardError validateBoard(const Board& board);
Error toProtocolError(BoardError error);

}  // namespace checkers

#endif
