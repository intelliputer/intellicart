#ifndef CHECKERS_RULES_HPP
#define CHECKERS_RULES_HPP

#include "checkers/board.hpp"

namespace checkers {

static const uint8_t kMaxLegalMoves = 32;

enum class RulesError : uint8_t {
    NONE = 0,
    INVALID_BOARD,
    MOVE_NOT_LEGAL,
    MULTI_JUMP_NOT_IMPLEMENTED,
    PROMOTION_NOT_IMPLEMENTED
};

struct MoveList {
    Move moves[kMaxLegalMoves];
    uint8_t count;
};

// Generates moves for uncrowned men only. Captures, when available, replace
// every quiet move. The list has stable origin/left/right ordering.
RulesError generateMenMoves(const Board& board, MoveList* moves);

// Applies exactly one generated move. It rejects a position requiring a
// multi-jump continuation or promotion; those are separate upcoming rules.
RulesError applyMenMove(const Board& board, const Move& move, Board* result);

}  // namespace checkers

#endif
