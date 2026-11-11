#include "checkers/mailbox_abi.hpp"

namespace checkers {
namespace abi {

namespace {

uint32_t readBits(uint16_t low, uint16_t high) {
    return static_cast<uint32_t>(low) |
           (static_cast<uint32_t>(high) << 16);
}

}  // namespace

uint16_t packVersionCommand(uint8_t version, Command command) {
    return static_cast<uint16_t>((static_cast<uint16_t>(version) << 8) |
                                 static_cast<uint8_t>(command));
}

uint16_t packStatusSequence(Status status, uint16_t sequence) {
    return static_cast<uint16_t>((static_cast<uint16_t>(status) << 12) |
                                 (sequence & kSequenceMask));
}

bool decodeRequest(const uint16_t words[kMailboxWords], MailboxRequest* request,
                   Error* error) {
    // Sequence is deliberately captured before validation so ERROR can echo
    // the transaction which caused it.
    *request = {};
    request->sequence = static_cast<uint16_t>(words[REQUEST_SEQUENCE] & kSequenceMask);
    if (words[MAGIC] != kMagic) {
        *error = Error::BAD_MAGIC;
        return false;
    }

    const uint8_t version = static_cast<uint8_t>(words[VERSION_COMMAND] >> 8);
    if (version != kVersion) {
        *error = Error::BAD_VERSION;
        return false;
    }

    const uint8_t rawCommand =
        static_cast<uint8_t>(words[VERSION_COMMAND] & 0x00FFU);
    if (rawCommand != static_cast<uint8_t>(Command::PING) &&
        rawCommand != static_cast<uint8_t>(Command::BOARD)) {
        *error = Error::BAD_COMMAND;
        return false;
    }

    request->command = static_cast<Command>(rawCommand);
    request->board.humanMen = readBits(words[HUMAN_MEN_LO], words[HUMAN_MEN_HI]);
    request->board.agentMen = readBits(words[AGENT_MEN_LO], words[AGENT_MEN_HI]);
    request->board.kings = readBits(words[KINGS_LO], words[KINGS_HI]);
    request->board.sideToMove = (words[TURN_FLAGS] & 0x0001U) == 0U
        ? Side::HUMAN : Side::AGENT;
    request->board.jumpContinuation = (words[TURN_FLAGS] & 0x0002U) != 0U;
    request->board.continuationSquare =
        static_cast<uint8_t>((words[TURN_FLAGS] >> 8) & 0x001FU);
    *error = Error::NONE;
    return true;
}

void encodeBusy(uint16_t sequence, uint16_t words[kMailboxWords]) {
    words[RESPONSE_STATUS_SEQUENCE] = packStatusSequence(Status::BUSY, sequence);
}

void encodeResponse(const MailboxResponse& response,
                    uint16_t words[kMailboxWords]) {
    words[MOVE_ORIGIN] = response.move.origin;
    for (uint8_t index = 0; index < kMaxLandings; ++index) {
        words[MOVE_LANDING_0 + index] =
            index < response.move.landingCount ? response.move.landings[index] : kTerminator;
    }
    words[DIAGNOSTIC] = response.error == Error::NONE
        ? response.move.diagnostic
        : static_cast<uint16_t>(response.error);
    // This is deliberately last: it commits all preceding response words.
    words[RESPONSE_STATUS_SEQUENCE] =
        packStatusSequence(response.status, response.sequence);
}

}  // namespace abi
}  // namespace checkers
