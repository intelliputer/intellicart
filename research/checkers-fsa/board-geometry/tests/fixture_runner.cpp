#include <cstdlib>
#include <iostream>

#include "checkers_geometry/geometry.hpp"

namespace {

int failures = 0;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

checkers_geometry::Position openingPosition() {
    checkers_geometry::Position position = {};
    position.humanMen = 0xFFF00000UL;
    position.agentMen = 0x00000FFFUL;
    position.kings = 0;
    position.sideToMove = checkers_geometry::Side::HUMAN;
    position.jumpContinuation = false;
    position.continuationSquare = checkers_geometry::kInvalidSquare;
    return position;
}

void testAllSquareRoundTrips() {
    for (checkers_geometry::Square square = 0;
         square < checkers_geometry::kPlayableSquares; ++square) {
        checkers_geometry::Coordinate coordinate = {};
        require(checkers_geometry::toCoordinate(square, &coordinate),
                "every numbered square has a coordinate");
        checkers_geometry::Square recovered = checkers_geometry::kInvalidSquare;
        require(checkers_geometry::fromCoordinate(coordinate.row, coordinate.column,
                                                  &recovered),
                "every square coordinate is playable");
        require(recovered == square, "square-coordinate-square round trip");
    }
}

void testCoordinatesAndEdges() {
    checkers_geometry::Coordinate coordinate = {};
    require(checkers_geometry::toCoordinate(0, &coordinate) &&
            coordinate.row == 0 && coordinate.column == 1,
            "square zero is top-row playable column one");
    require(checkers_geometry::toCoordinate(31, &coordinate) &&
            coordinate.row == 7 && coordinate.column == 6,
            "square 31 is bottom-row playable column six");

    checkers_geometry::Square unused = checkers_geometry::kInvalidSquare;
    require(!checkers_geometry::fromCoordinate(0, 0, &unused),
            "top-left light square is rejected");
    require(!checkers_geometry::fromCoordinate(8, 0, &unused),
            "off-board row is rejected");
    require(checkers_geometry::step(12, -1, +1) == 8,
            "square 12 steps up-right to square 8");
    require(checkers_geometry::step(12, +1, +1) == 16,
            "square 12 steps down-right to square 16");
    require(checkers_geometry::step(12, -1, -1) == checkers_geometry::kInvalidSquare,
            "square 12 cannot step off the left edge");
    require(checkers_geometry::jump(12, +1, +1) == 21,
            "square 12 jumps down-right to square 21");
}

void testPositionValidation() {
    checkers_geometry::Position position = openingPosition();
    require(checkers_geometry::validate(position) == checkers_geometry::ValidationError::NONE,
            "opening position is valid");

    position = openingPosition();
    position.humanMen |= 1UL << 12;
    position.agentMen |= 1UL << 12;
    require(checkers_geometry::validate(position) ==
            checkers_geometry::ValidationError::OVERLAPPING_PIECES,
            "overlapping pieces are invalid");

    position = openingPosition();
    position.kings = 1UL << 12;
    require(checkers_geometry::validate(position) ==
            checkers_geometry::ValidationError::ORPHAN_KING,
            "king without a piece is invalid");

    position = openingPosition();
    position.jumpContinuation = true;
    position.continuationSquare = 12;
    require(checkers_geometry::validate(position) ==
            checkers_geometry::ValidationError::CONTINUATION_EMPTY,
            "continuation needs a piece");

    position = openingPosition();
    position.jumpContinuation = true;
    position.continuationSquare = 0;
    require(checkers_geometry::validate(position) ==
            checkers_geometry::ValidationError::CONTINUATION_WRONG_SIDE,
            "continuation must belong to side to move");

    position = openingPosition();
    position.jumpContinuation = true;
    position.continuationSquare = 20;
    require(checkers_geometry::validate(position) == checkers_geometry::ValidationError::NONE,
            "continuation on current side's piece is valid");

    position.continuationSquare = checkers_geometry::kInvalidSquare;
    require(checkers_geometry::validate(position) ==
            checkers_geometry::ValidationError::CONTINUATION_OUT_OF_RANGE,
            "out-of-range continuation is invalid");
}

}  // namespace

int main() {
    testAllSquareRoundTrips();
    testCoordinatesAndEdges();
    testPositionValidation();

    if (failures == 0) {
        std::cout << "All board geometry fixtures passed.\n";
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
