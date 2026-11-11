#include <cstdlib>
#include <iostream>

#include "checkers/agent.hpp"
#include "checkers/mailbox_abi.hpp"
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
    require(board.commitRequest(), "BOARD transaction is accepted");
    runHarness(&board);
    requireError(board, checkers::Error::UNIMPLEMENTED, 4,
                 "BOARD is explicit until board rules exist");
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

    if (failures == 0) {
        std::cout << "All Checkers core fixtures passed.\n";
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
