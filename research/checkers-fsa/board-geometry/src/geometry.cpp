#include "checkers_geometry/geometry.hpp"

namespace checkers_geometry {

bool isPlayableSquare(Square square) {
    return square < kPlayableSquares;
}

bool toCoordinate(Square square, Coordinate* coordinate) {
    if (!isPlayableSquare(square) || coordinate == 0) {
        return false;
    }

    coordinate->row = static_cast<uint8_t>(square / 4U);
    const uint8_t positionInRow = static_cast<uint8_t>(square % 4U);
    coordinate->column = static_cast<uint8_t>(2U * positionInRow +
        ((coordinate->row + 1U) & 1U));
    return true;
}

bool fromCoordinate(uint8_t row, uint8_t column, Square* square) {
    if (row >= kBoardRows || column >= kBoardColumns || square == 0) {
        return false;
    }
    // Playable squares alternate; row zero contains columns 1, 3, 5, 7.
    if (((row + column) & 1U) == 0U) {
        return false;
    }

    *square = static_cast<Square>(row * 4U + column / 2U);
    return true;
}

Square step(Square square, int8_t rowDelta, int8_t columnDelta) {
    Coordinate source = {};
    if (!toCoordinate(square, &source)) {
        return kInvalidSquare;
    }

    const int16_t targetRow = static_cast<int16_t>(source.row) + rowDelta;
    const int16_t targetColumn = static_cast<int16_t>(source.column) + columnDelta;
    if (targetRow < 0 || targetRow >= kBoardRows ||
        targetColumn < 0 || targetColumn >= kBoardColumns) {
        return kInvalidSquare;
    }

    Square target = kInvalidSquare;
    return fromCoordinate(static_cast<uint8_t>(targetRow),
                          static_cast<uint8_t>(targetColumn), &target)
        ? target : kInvalidSquare;
}

Square jump(Square square, int8_t rowDelta, int8_t columnDelta) {
    return step(square, static_cast<int8_t>(rowDelta * 2),
                static_cast<int8_t>(columnDelta * 2));
}

ValidationError validate(const Position& position) {
    if ((position.humanMen & position.agentMen) != 0U) {
        return ValidationError::OVERLAPPING_PIECES;
    }

    const uint32_t occupied = position.humanMen | position.agentMen;
    if ((position.kings & ~occupied) != 0U) {
        return ValidationError::ORPHAN_KING;
    }

    if (!position.jumpContinuation) {
        return ValidationError::NONE;
    }
    if (!isPlayableSquare(position.continuationSquare)) {
        return ValidationError::CONTINUATION_OUT_OF_RANGE;
    }

    const uint32_t continuationBit =
        static_cast<uint32_t>(1UL) << position.continuationSquare;
    if ((occupied & continuationBit) == 0U) {
        return ValidationError::CONTINUATION_EMPTY;
    }

    const uint32_t moverBits = position.sideToMove == Side::HUMAN
        ? position.humanMen : position.agentMen;
    if ((moverBits & continuationBit) == 0U) {
        return ValidationError::CONTINUATION_WRONG_SIDE;
    }

    return ValidationError::NONE;
}

}  // namespace checkers_geometry
