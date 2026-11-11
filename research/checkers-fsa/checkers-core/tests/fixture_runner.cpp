#include <cstdlib>
#include <iostream>

#include "checkers/agent.hpp"
#include "checkers/board.hpp"
#include "checkers/mailbox_abi.hpp"
#include "checkers/rules.hpp"
#include "mailbox_harness.hpp"

namespace {

int failures = 0;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

checkers::MailboxRequest decodePing(uint16_t sequence,
                                    checkers::Error* error) {
    uint16_t words[checkers::abi::kMailboxWords] = {};
    words[checkers::abi::MAGIC] = checkers::abi::kMagic;
    words[checkers::abi::VERSION_COMMAND] = checkers::abi::packVersionCommand(
        checkers::abi::kVersion, checkers::Command::PING);
    words[checkers::abi::REQUEST_SEQUENCE] = sequence;

    checkers::MailboxRequest request = {};
    const bool decoded = checkers::abi::decodeRequest(words, &request, error);
    require(decoded, "PING request decodes");
    return request;
}

void runUntilResponse(checkers::Agent* agent) {
    for (uint8_t steps = 0; steps < 8 && !agent->responseReady(); ++steps) {
        require(agent->step(), "FSA advances while response is pending");
    }
}

void testPingRoundTrip() {
    checkers::Error error = checkers::Error::NONE;
    const checkers::MailboxRequest request = decodePing(1, &error);
    checkers::Agent agent;
    require(agent.acceptRequest(request, error), "agent accepts PING");
    runUntilResponse(&agent);

    const checkers::MailboxResponse response = agent.response();
    require(response.status == checkers::Status::READY, "PING becomes READY");
    require(response.sequence == 1, "PING echoes its sequence");
    require(response.error == checkers::Error::NONE, "PING has no error");
    require(response.move.diagnostic == checkers::abi::kDiagnosticPing,
            "PING has the diagnostic word");

    uint16_t words[checkers::abi::kMailboxWords] = {};
    checkers::abi::encodeResponse(response, words);
    require(words[checkers::abi::DIAGNOSTIC] == checkers::abi::kDiagnosticPing,
            "response encodes diagnostic before READY");
    require(words[checkers::abi::RESPONSE_STATUS_SEQUENCE] ==
            checkers::abi::packStatusSequence(checkers::Status::READY, 1),
            "response commits READY with sequence");
}

void testBadMagic() {
    uint16_t words[checkers::abi::kMailboxWords] = {};
    words[checkers::abi::VERSION_COMMAND] = checkers::abi::packVersionCommand(
        checkers::abi::kVersion, checkers::Command::PING);
    words[checkers::abi::REQUEST_SEQUENCE] = 2;

    checkers::MailboxRequest request = {};
    checkers::Error error = checkers::Error::NONE;
    require(!checkers::abi::decodeRequest(words, &request, &error),
            "bad magic is rejected by decoder");
    require(error == checkers::Error::BAD_MAGIC, "bad magic has the right error");
}

void testStaleSequence() {
    checkers::Error error = checkers::Error::NONE;
    const checkers::MailboxRequest request = decodePing(1, &error);
    checkers::Agent agent;
    require(agent.acceptRequest(request, error), "agent accepts first PING");
    runUntilResponse(&agent);
    require(agent.acknowledgeResponse(), "adapter acknowledges first response");
    require(agent.acceptRequest(request, error), "agent accepts the next transport request");
    runUntilResponse(&agent);
    const checkers::MailboxResponse response = agent.response();
    require(response.status == checkers::Status::ERROR,
            "repeated sequence produces an error response");
    require(response.error == checkers::Error::STALE_SEQUENCE,
            "repeated sequence has the stale-sequence error");
}

void prepareRequest(checkers_test::MailboxHarness* harness, uint16_t sequence,
                    uint8_t version, checkers::Command command) {
    uint16_t* words = harness->words();
    words[checkers::abi::MAGIC] = checkers::abi::kMagic;
    words[checkers::abi::VERSION_COMMAND] =
        checkers::abi::packVersionCommand(version, command);
    words[checkers::abi::REQUEST_SEQUENCE] = sequence;
}

void writeBoard(uint16_t* words, uint32_t humanMen, uint32_t agentMen,
                uint32_t kings, uint16_t turnFlags) {
    words[checkers::abi::HUMAN_MEN_LO] = static_cast<uint16_t>(humanMen);
    words[checkers::abi::HUMAN_MEN_HI] = static_cast<uint16_t>(humanMen >> 16);
    words[checkers::abi::AGENT_MEN_LO] = static_cast<uint16_t>(agentMen);
    words[checkers::abi::AGENT_MEN_HI] = static_cast<uint16_t>(agentMen >> 16);
    words[checkers::abi::KINGS_LO] = static_cast<uint16_t>(kings);
    words[checkers::abi::KINGS_HI] = static_cast<uint16_t>(kings >> 16);
    words[checkers::abi::TURN_FLAGS] = turnFlags;
}

void runHarness(checkers_test::MailboxHarness* harness) {
    for (uint8_t ticks = 0; ticks < 8; ++ticks) {
        harness->tick();
        const uint16_t response =
            harness->words()[checkers::abi::RESPONSE_STATUS_SEQUENCE];
        if ((response >> 12) == static_cast<uint16_t>(checkers::Status::READY) ||
            (response >> 12) == static_cast<uint16_t>(checkers::Status::ERROR)) {
            return;
        }
    }
    require(false, "mailbox harness completes within bounded ticks");
}

void requireError(const checkers_test::MailboxHarness& harness,
                  checkers::Error expected, uint16_t sequence,
                  const char* message) {
    const uint16_t response =
        harness.words()[checkers::abi::RESPONSE_STATUS_SEQUENCE];
    require(response == checkers::abi::packStatusSequence(checkers::Status::ERROR,
                                                           sequence), message);
    require(harness.words()[checkers::abi::DIAGNOSTIC] ==
            static_cast<uint16_t>(expected), "error diagnostic is encoded");
}

void testMailboxHarnessPing() {
    checkers_test::MailboxHarness harness;
    prepareRequest(&harness, 1, checkers::abi::kVersion, checkers::Command::PING);
    require(harness.commitRequest(), "harness accepts a committed request");
    require(harness.words()[checkers::abi::RESPONSE_STATUS_SEQUENCE] ==
            checkers::abi::packStatusSequence(checkers::Status::BUSY, 1),
            "harness publishes BUSY immediately");
    runHarness(&harness);

    require(harness.words()[checkers::abi::RESPONSE_STATUS_SEQUENCE] ==
            checkers::abi::packStatusSequence(checkers::Status::READY, 1),
            "harness publishes READY for PING");
    require(harness.eventCount() == 4, "harness records four adapter writes");
    require(harness.event(0) == checkers_test::WriteEvent::BUSY,
            "BUSY is first adapter write");
    require(harness.event(1) == checkers_test::WriteEvent::MOVE_AND_PATH,
            "move data precedes diagnostic");
    require(harness.event(2) == checkers_test::WriteEvent::DIAGNOSTIC,
            "diagnostic precedes READY");
    require(harness.event(3) == checkers_test::WriteEvent::RESPONSE_COMMIT,
            "READY commit is final adapter write");
    require(harness.acknowledgeResponse(), "adapter acknowledges READY");
}

void testProtocolErrors() {
    checkers_test::MailboxHarness badVersion;
    prepareRequest(&badVersion, 2, checkers::abi::kVersion + 1,
                   checkers::Command::PING);
    require(badVersion.commitRequest(), "bad-version transaction is accepted for error reply");
    runHarness(&badVersion);
    requireError(badVersion, checkers::Error::BAD_VERSION, 2,
                 "bad version produces ERROR with matching sequence");

    checkers_test::MailboxHarness badCommand;
    prepareRequest(&badCommand, 3, checkers::abi::kVersion,
                   static_cast<checkers::Command>(0x7F));
    require(badCommand.commitRequest(), "bad-command transaction is accepted for error reply");
    runHarness(&badCommand);
    requireError(badCommand, checkers::Error::BAD_COMMAND, 3,
                 "bad command produces ERROR with matching sequence");

    checkers_test::MailboxHarness board;
    prepareRequest(&board, 4, checkers::abi::kVersion, checkers::Command::BOARD);
    writeBoard(board.words(), 0xFFF00000UL, 0x00000FFFUL, 0, 0);
    require(board.commitRequest(), "BOARD transaction is accepted");
    runHarness(&board);
    requireError(board, checkers::Error::UNIMPLEMENTED, 4,
                 "BOARD is explicit until board rules exist");
}

void testBoardValidation() {
    checkers_test::MailboxHarness valid;
    prepareRequest(&valid, 5, checkers::abi::kVersion, checkers::Command::BOARD);
    writeBoard(valid.words(), 0xFFF00000UL, 0x00000FFFUL, 0, 0);
    require(valid.commitRequest(), "valid opening board is accepted");
    runHarness(&valid);
    requireError(valid, checkers::Error::UNIMPLEMENTED, 5,
                 "valid board reaches the deliberate no-move boundary");

    checkers_test::MailboxHarness overlap;
    prepareRequest(&overlap, 6, checkers::abi::kVersion, checkers::Command::BOARD);
    writeBoard(overlap.words(), 1UL << 12, 1UL << 12, 0, 0);
    require(overlap.commitRequest(), "overlapping board is accepted for error reply");
    runHarness(&overlap);
    requireError(overlap, checkers::Error::INVALID_BOARD, 6,
                 "overlapping pieces are rejected");

    checkers_test::MailboxHarness orphanKing;
    prepareRequest(&orphanKing, 7, checkers::abi::kVersion, checkers::Command::BOARD);
    writeBoard(orphanKing.words(), 0, 0, 1UL << 12, 0);
    require(orphanKing.commitRequest(), "orphan king is accepted for error reply");
    runHarness(&orphanKing);
    requireError(orphanKing, checkers::Error::INVALID_BOARD, 7,
                 "orphan king is rejected");

    checkers_test::MailboxHarness continuation;
    prepareRequest(&continuation, 8, checkers::abi::kVersion, checkers::Command::BOARD);
    // Bit 1 requests jump continuation; the continuation square is empty.
    writeBoard(continuation.words(), 0xFFF00000UL, 0x00000FFFUL, 0,
               static_cast<uint16_t>(0x0002U | (12U << 8)));
    require(continuation.commitRequest(), "invalid continuation is accepted for error reply");
    runHarness(&continuation);
    requireError(continuation, checkers::Error::INVALID_BOARD, 8,
                 "empty continuation is rejected");
}

void testCoreGeometry() {
    for (uint8_t square = 0; square < checkers::kPlayableSquares; ++square) {
        checkers::Coordinate coordinate = {};
        uint8_t recovered = checkers::kInvalidSquare;
        require(checkers::squareToCoordinate(square, &coordinate),
                "core square has a coordinate");
        require(checkers::coordinateToSquare(coordinate.row, coordinate.column,
                                             &recovered) && recovered == square,
                "core coordinate round trip");
    }
    require(checkers::stepSquare(12, -1, 1) == 8,
            "core geometry uses the validated mapping");
    require(checkers::jumpSquare(12, 1, 1) == 21,
            "core jump geometry uses the validated mapping");
}

checkers::Board makeBoard(uint32_t human, uint32_t agent,
                          checkers::Side side = checkers::Side::HUMAN) {
    checkers::Board board = {};
    board.humanMen = human;
    board.agentMen = agent;
    board.kings = 0;
    board.sideToMove = side;
    board.jumpContinuation = false;
    board.continuationSquare = checkers::kInvalidSquare;
    return board;
}

bool hasMove(const checkers::MoveList& moves, uint8_t origin, uint8_t landing,
             uint8_t captured = checkers::kInvalidSquare) {
    for (uint8_t index = 0; index < moves.count; ++index) {
        const checkers::Move& move = moves.moves[index];
        if (move.origin == origin && move.landingCount == 1 &&
            move.landings[0] == landing &&
            move.capturedCount == (captured == checkers::kInvalidSquare ? 0 : 1) &&
            (captured == checkers::kInvalidSquare || move.capturedSquares[0] == captured)) {
            return true;
        }
    }
    return false;
}

void testMenMoveGeneration() {
    checkers::MoveList moves = {};
    checkers::Board opening = makeBoard(0xFFF00000UL, 0x00000FFFUL);
    require(checkers::generateMenMoves(opening, &moves) == checkers::RulesError::NONE,
            "opening move generation succeeds");
    require(moves.count == 7, "opening position has seven human quiet moves");
    require(hasMove(moves, 20, 16) && hasMove(moves, 21, 16) &&
            hasMove(moves, 21, 17) && hasMove(moves, 22, 17) &&
            hasMove(moves, 22, 18) && hasMove(moves, 23, 18) &&
            hasMove(moves, 23, 19), "opening quiet moves use expected geometry");

    checkers::Board blocked = makeBoard((1UL << 20) | (1UL << 16), 0);
    blocked.kings = 1UL << 16;  // The blocker is outside this men-only module.
    require(checkers::generateMenMoves(blocked, &moves) == checkers::RulesError::NONE &&
            moves.count == 0, "blocked edge man has no quiet move");

    checkers::Board oneCapture = makeBoard(1UL << 20, 1UL << 16);
    require(checkers::generateMenMoves(oneCapture, &moves) == checkers::RulesError::NONE &&
            moves.count == 1 && hasMove(moves, 20, 13, 16),
            "single forward capture is generated");

    checkers::Board captureBeatsQuiet = makeBoard((1UL << 20) | (1UL << 22),
                                                   1UL << 16);
    require(checkers::generateMenMoves(captureBeatsQuiet, &moves) == checkers::RulesError::NONE &&
            moves.count == 1 && hasMove(moves, 20, 13, 16),
            "a capture removes every quiet alternative");

    checkers::Board twoCaptures = makeBoard(1UL << 21, (1UL << 16) | (1UL << 17));
    require(checkers::generateMenMoves(twoCaptures, &moves) == checkers::RulesError::NONE &&
            moves.count == 2 && hasMove(moves, 21, 12, 16) &&
            hasMove(moves, 21, 14, 17), "both captures are generated");
    require(moves.moves[0].landings[0] == 12 && moves.moves[1].landings[0] == 14,
            "capture ordering is canonical left then right");

    checkers::Board noMoves = makeBoard(1UL << 0, 0);
    require(checkers::generateMenMoves(noMoves, &moves) == checkers::RulesError::NONE &&
            moves.count == 0, "man on the wrong edge has no legal move");

    checkers::Board continuation = makeBoard(1UL << 20, 1UL << 16);
    continuation.jumpContinuation = true;
    continuation.continuationSquare = 20;
    require(checkers::generateMenMoves(continuation, &moves) ==
            checkers::RulesError::MULTI_JUMP_NOT_IMPLEMENTED,
            "existing jump continuation is deliberately deferred");
}

void testApplyMenMove() {
    checkers::Board before = makeBoard(1UL << 20, 1UL << 16);
    checkers::MoveList moves = {};
    checkers::Board after = {};
    checkers::generateMenMoves(before, &moves);
    require(checkers::applyMenMove(before, moves.moves[0], &after) == checkers::RulesError::NONE,
            "single capture applies");
    require(after.humanMen == (1UL << 13) && after.agentMen == 0,
            "apply clears origin and captured piece, then occupies landing");
    require(after.sideToMove == checkers::Side::AGENT,
            "apply changes turn after a single capture");

    checkers::Move illegal = {};
    illegal.origin = 20;
    illegal.landingCount = 1;
    illegal.landings[0] = 16;
    require(checkers::applyMenMove(before, illegal, &after) ==
            checkers::RulesError::MOVE_NOT_LEGAL,
            "apply refuses a quiet move when capture is compulsory");

    checkers::Board promotion = makeBoard(1UL << 4, 0);
    checkers::generateMenMoves(promotion, &moves);
    require(moves.count == 1 && moves.moves[0].landings[0] == 0,
            "promotion-row move is identified as a legal geometric move");
    require(checkers::applyMenMove(promotion, moves.moves[0], &after) ==
            checkers::RulesError::PROMOTION_NOT_IMPLEMENTED,
            "promotion is not silently applied before its dedicated increment");
}

void testSequenceWrap() {
    checkers_test::MailboxHarness harness;
    prepareRequest(&harness, checkers::abi::kSequenceMask, checkers::abi::kVersion,
                   checkers::Command::PING);
    require(harness.commitRequest(), "highest sequence is accepted");
    runHarness(&harness);
    require(harness.acknowledgeResponse(), "highest sequence is acknowledged");

    prepareRequest(&harness, 0, checkers::abi::kVersion, checkers::Command::PING);
    require(harness.commitRequest(), "sequence wraps from max to zero");
    runHarness(&harness);
    require(harness.words()[checkers::abi::RESPONSE_STATUS_SEQUENCE] ==
            checkers::abi::packStatusSequence(checkers::Status::READY, 0),
            "wrapped sequence returns READY");
}

}  // namespace

int main() {
    testPingRoundTrip();
    testBadMagic();
    testStaleSequence();
    testMailboxHarnessPing();
    testProtocolErrors();
    testSequenceWrap();
    testBoardValidation();
    testCoreGeometry();
    testMenMoveGeneration();
    testApplyMenMove();

    if (failures == 0) {
        std::cout << "All Checkers core fixtures passed.\n";
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
