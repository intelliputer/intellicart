#include "checkers/rules.hpp"

namespace checkers {

namespace {

uint32_t bitFor(uint8_t square) {
    return static_cast<uint32_t>(1UL) << square;
}

bool occupied(const Board& board, uint8_t square) {
    return (board.humanMen | board.agentMen) & bitFor(square);
}

bool opponentAt(const Board& board, uint8_t square) {
    const uint32_t opponent = board.sideToMove == Side::HUMAN
        ? board.agentMen : board.humanMen;
    return (opponent & bitFor(square)) != 0U;
}

bool isManToMove(const Board& board, uint8_t square) {
    const uint32_t mover = board.sideToMove == Side::HUMAN
        ? board.humanMen : board.agentMen;
    return (mover & bitFor(square)) != 0U && (board.kings & bitFor(square)) == 0U;
}

Move makeMove(uint8_t origin, uint8_t landing, uint8_t captured) {
    Move move = {};
    move.origin = origin;
    move.landingCount = 1;
    move.landings[0] = landing;
    move.capturedCount = captured == kInvalidSquare ? 0 : 1;
    if (move.capturedCount != 0) {
        move.capturedSquares[0] = captured;
    }
    return move;
}

bool addMove(MoveList* moves, const Move& move) {
    if (moves->count >= kMaxLegalMoves) {
        return false;
    }
    moves->moves[moves->count++] = move;
    return true;
}

bool sameMove(const Move& left, const Move& right) {
    if (left.origin != right.origin || left.landingCount != right.landingCount ||
        left.capturedCount != right.capturedCount) {
        return false;
    }
    for (uint8_t index = 0; index < left.landingCount; ++index) {
        if (left.landings[index] != right.landings[index]) {
            return false;
        }
    }
    for (uint8_t index = 0; index < left.capturedCount; ++index) {
        if (left.capturedSquares[index] != right.capturedSquares[index]) {
            return false;
        }
    }
    return true;
}

bool reachesPromotionRow(const Board& board, uint8_t square) {
    Coordinate coordinate = {};
    squareToCoordinate(square, &coordinate);
    return board.sideToMove == Side::HUMAN ? coordinate.row == 0 : coordinate.row == 7;
}

}  // namespace

RulesError generateMenMoves(const Board& board, MoveList* moves) {
    if (moves == 0 || validateBoard(board) != BoardError::NONE) {
        return RulesError::INVALID_BOARD;
    }
    moves->count = 0;
    if (board.jumpContinuation) {
        return RulesError::MULTI_JUMP_NOT_IMPLEMENTED;
    }

    MoveList captures = {};
    const int8_t rowDelta = board.sideToMove == Side::HUMAN ? -1 : 1;
    const int8_t columns[2] = {-1, 1};  // canonical left then right

    for (uint8_t origin = 0; origin < kPlayableSquares; ++origin) {
        if (!isManToMove(board, origin)) {
            continue;
        }
        for (uint8_t direction = 0; direction < 2; ++direction) {
            const uint8_t crossed = stepSquare(origin, rowDelta, columns[direction]);
            const uint8_t landing = jumpSquare(origin, rowDelta, columns[direction]);
            if (crossed != kInvalidSquare && landing != kInvalidSquare &&
                opponentAt(board, crossed) && !occupied(board, landing)) {
                addMove(&captures, makeMove(origin, landing, crossed));
            }
        }
    }

    if (captures.count != 0) {
        *moves = captures;
        return RulesError::NONE;
    }

    for (uint8_t origin = 0; origin < kPlayableSquares; ++origin) {
        if (!isManToMove(board, origin)) {
            continue;
        }
        for (uint8_t direction = 0; direction < 2; ++direction) {
            const uint8_t landing = stepSquare(origin, rowDelta, columns[direction]);
            if (landing != kInvalidSquare && !occupied(board, landing)) {
                addMove(moves, makeMove(origin, landing, kInvalidSquare));
            }
        }
    }
    return RulesError::NONE;
}

RulesError applyMenMove(const Board& board, const Move& move, Board* result) {
    if (result == 0 || validateBoard(board) != BoardError::NONE) {
        return RulesError::INVALID_BOARD;
    }
    if (board.jumpContinuation) {
        return RulesError::MULTI_JUMP_NOT_IMPLEMENTED;
    }

    MoveList legal = {};
    const RulesError generation = generateMenMoves(board, &legal);
    if (generation != RulesError::NONE) {
        return generation;
    }
    bool found = false;
    for (uint8_t index = 0; index < legal.count; ++index) {
        if (sameMove(legal.moves[index], move)) {
            found = true;
            break;
        }
    }
    if (!found) {
        return RulesError::MOVE_NOT_LEGAL;
    }

    const uint8_t landing = move.landings[0];
    if (reachesPromotionRow(board, landing)) {
        return RulesError::PROMOTION_NOT_IMPLEMENTED;
    }

    *result = board;
    const uint32_t originBit = bitFor(move.origin);
    const uint32_t landingBit = bitFor(landing);
    uint32_t* mover = board.sideToMove == Side::HUMAN
        ? &result->humanMen : &result->agentMen;
    uint32_t* opponent = board.sideToMove == Side::HUMAN
        ? &result->agentMen : &result->humanMen;
    *mover = (*mover & ~originBit) | landingBit;
    for (uint8_t index = 0; index < move.capturedCount; ++index) {
        const uint32_t capturedBit = bitFor(move.capturedSquares[index]);
        *opponent &= ~capturedBit;
        result->kings &= ~capturedBit;
    }
    result->sideToMove = board.sideToMove == Side::HUMAN ? Side::AGENT : Side::HUMAN;
    result->jumpContinuation = false;
    result->continuationSquare = kInvalidSquare;
    return RulesError::NONE;
}

}  // namespace checkers
