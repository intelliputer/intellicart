#include <cstdlib>
#include <iostream>

#include "checkers/agent.hpp"
#include "checkers/mailbox_abi.hpp"

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

}  // namespace

int main() {
    testPingRoundTrip();
    testBadMagic();
    testStaleSequence();

    if (failures == 0) {
        std::cout << "All Checkers core fixtures passed.\n";
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
