#ifndef CHECKERS_MAILBOX_ABI_HPP
#define CHECKERS_MAILBOX_ABI_HPP

#include <stdint.h>

#include "checkers/types.hpp"

namespace checkers {
namespace abi {

static const uint16_t kMagic = 0x4348;       // "CH"
static const uint8_t kVersion = 1;
static const uint16_t kDiagnosticPing = 0xA11E;
static const uint16_t kTerminator = 0xFFFF;
static const uint16_t kSequenceMask = 0x0FFF;
static const uint8_t kMailboxWords = 32;

enum Word : uint8_t {
    MAGIC = 0,
    VERSION_COMMAND = 1,
    REQUEST_SEQUENCE = 2,
    HUMAN_MEN_LO = 3,
    HUMAN_MEN_HI = 4,
    AGENT_MEN_LO = 5,
    AGENT_MEN_HI = 6,
    KINGS_LO = 7,
    KINGS_HI = 8,
    TURN_FLAGS = 9,
    RESPONSE_STATUS_SEQUENCE = 10,
    MOVE_ORIGIN = 11,
    MOVE_LANDING_0 = 12,
    MOVE_LANDING_LAST = MOVE_LANDING_0 + kMaxLandings - 1,
    DIAGNOSTIC = 24
};

uint16_t packVersionCommand(uint8_t version, Command command);
uint16_t packStatusSequence(Status status, uint16_t sequence);
bool decodeRequest(const uint16_t words[kMailboxWords], MailboxRequest* request,
                   Error* error);
void encodeBusy(uint16_t sequence, uint16_t words[kMailboxWords]);
void encodeResponse(const MailboxResponse& response,
                    uint16_t words[kMailboxWords]);

}  // namespace abi
}  // namespace checkers

#endif
