#include "checkers/board.hpp"

namespace checkers {

bool squareToCoordinate(uint8_t square, Coordinate* coordinate) {
    if (square >= kPlayableSquares || coordinate == 0) {
        return false;
    }

    coordinate->row = static_cast<uint8_t>(square / 4U);
    coordinate->column = static_cast<uint8_t>(2U * (square % 4U) +
        ((coordinate->row + 1U) & 1U));
    return true;
}

bool coordinateToSquare(uint8_t row, uint8_t column, uint8_t* square) {
    if (row >= kBoardRows || column >= kBoardColumns || square == 0 ||
        ((row + column) & 1U) == 0U) {
        return false;
    }

    *square = static_cast<uint8_t>(row * 4U + column / 2U);
    return true;
}

uint8_t stepSquare(uint8_t square, int8_t rowDelta, int8_t columnDelta) {
    Coordinate source = {};
    if (!squareToCoordinate(square, &source)) {
        return kInvalidSquare;
    }

    const int16_t targetRow = static_cast<int16_t>(source.row) + rowDelta;
    const int16_t targetColumn = static_cast<int16_t>(source.column) + columnDelta;
    if (targetRow < 0 || targetRow >= kBoardRows ||
        targetColumn < 0 || targetColumn >= kBoardColumns) {
        return kInvalidSquare;
    }

    uint8_t target = kInvalidSquare;
    return coordinateToSquare(static_cast<uint8_t>(targetRow),
                              static_cast<uint8_t>(targetColumn), &target)
        ? target : kInvalidSquare;
}

uint8_t jumpSquare(uint8_t square, int8_t rowDelta, int8_t columnDelta) {
    return stepSquare(square, static_cast<int8_t>(rowDelta * 2),
                      static_cast<int8_t>(columnDelta * 2));
}

BoardError validateBoard(const Board& board) {
    if ((board.humanMen & board.agentMen) != 0U) {
        return BoardError::OVERLAPPING_PIECES;
    }

    const uint32_t occupied = board.humanMen | board.agentMen;
    if ((board.kings & ~occupied) != 0U) {
        return BoardError::ORPHAN_KING;
    }

    if (!board.jumpContinuation) {
        return BoardError::NONE;
    }
    if (board.continuationSquare >= kPlayableSquares) {
        return BoardError::CONTINUATION_OUT_OF_RANGE;
    }

    const uint32_t continuationBit =
        static_cast<uint32_t>(1UL) << board.continuationSquare;
    if ((occupied & continuationBit) == 0U) {
        return BoardError::CONTINUATION_EMPTY;
    }
    const uint32_t moverBits = board.sideToMove == Side::HUMAN
        ? board.humanMen : board.agentMen;
    return (moverBits & continuationBit) != 0U
        ? BoardError::NONE : BoardError::CONTINUATION_WRONG_SIDE;
}

Error toProtocolError(BoardError error) {
    return error == BoardError::NONE ? Error::NONE : Error::INVALID_BOARD;
}

}  // namespace checkers
