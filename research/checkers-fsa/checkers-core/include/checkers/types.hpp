#ifndef CHECKERS_TYPES_HPP
#define CHECKERS_TYPES_HPP

#include <stdint.h>

namespace checkers {

enum class Phase : uint8_t {
    WAIT_REQUEST,
    SNAPSHOT_REQUEST,
    RECONCILE_RESULT,
    GENERATE,
    PROJECT,
    EVALUATE,
    SELECT,
    COMMIT_RESPONSE,
    COMMIT_ERROR,
    WAIT_RESULT
};

enum class Command : uint8_t {
    NONE = 0,
    PING = 1,
    BOARD = 2
};

enum class Status : uint8_t {
    EMPTY = 0,
    BUSY = 1,
    READY = 2,
    ERROR = 3
};

enum class Error : uint16_t {
    NONE = 0,
    BAD_MAGIC = 1,
    BAD_VERSION = 2,
    BAD_COMMAND = 3,
    STALE_SEQUENCE = 4,
    INVALID_BOARD = 5,
    UNEXPECTED_BOARD_RESULT = 6,
    NO_LEGAL_MOVE = 7,
    UNIMPLEMENTED = 8
};

enum class Side : uint8_t {
    HUMAN = 0,
    AGENT = 1
};

static const uint8_t kBoardSquares = 32;
static const uint8_t kMaxLandings = 12;

struct Board {
    uint32_t humanMen;
    uint32_t agentMen;
    uint32_t kings;
    Side sideToMove;
    bool jumpContinuation;
    uint8_t continuationSquare;
};

struct Move {
    uint8_t origin;
    uint8_t landingCount;
    uint8_t landings[kMaxLandings];
    uint8_t capturedCount;
    uint8_t capturedSquares[kMaxLandings];
    uint16_t diagnostic;
};

struct MailboxRequest {
    uint16_t sequence;
    Command command;
    Board board;
};

struct MailboxResponse {
    Status status;
    uint16_t sequence;
    Move move;
    Error error;
};

}  // namespace checkers

#endif
