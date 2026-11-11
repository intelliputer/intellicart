#ifndef CHECKERS_TEST_MAILBOX_HARNESS_HPP
#define CHECKERS_TEST_MAILBOX_HARNESS_HPP

#include <stdint.h>

#include "checkers/agent.hpp"
#include "checkers/mailbox_abi.hpp"

namespace checkers_test {

enum class WriteEvent : uint8_t {
    BUSY,
    MOVE_AND_PATH,
    DIAGNOSTIC,
    RESPONSE_COMMIT
};

class MailboxHarness {
public:
    MailboxHarness() : eventCount_(0) {
        clearWords();
    }

    uint16_t* words() { return words_; }
    const uint16_t* words() const { return words_; }

    bool commitRequest() {
        checkers::MailboxRequest request = {};
        checkers::Error error = checkers::Error::NONE;
        checkers::abi::decodeRequest(words_, &request, &error);
        if (!agent_.acceptRequest(request, error)) {
            return false;
        }
        checkers::abi::encodeBusy(request.sequence, words_);
        record(WriteEvent::BUSY);
        return true;
    }

    bool tick() {
        const bool advanced = agent_.step();
        if (!agent_.responseReady()) {
            return advanced;
        }

        const checkers::MailboxResponse response = agent_.response();
        // Record the intended adapter ordering. encodeResponse follows this
        // same order and writes RESPONSE_STATUS_SEQUENCE last.
        record(WriteEvent::MOVE_AND_PATH);
        record(WriteEvent::DIAGNOSTIC);
        checkers::abi::encodeResponse(response, words_);
        record(WriteEvent::RESPONSE_COMMIT);
        return advanced;
    }

    bool acknowledgeResponse() {
        return agent_.acknowledgeResponse();
    }

    uint8_t eventCount() const { return eventCount_; }
    WriteEvent event(uint8_t index) const { return events_[index]; }

private:
    void clearWords() {
        for (uint8_t index = 0; index < checkers::abi::kMailboxWords; ++index) {
            words_[index] = 0;
        }
    }

    void record(WriteEvent event) {
        if (eventCount_ < sizeof(events_) / sizeof(events_[0])) {
            events_[eventCount_++] = event;
        }
    }

    uint16_t words_[checkers::abi::kMailboxWords];
    checkers::Agent agent_;
    WriteEvent events_[8];
    uint8_t eventCount_;
};

}  // namespace checkers_test

#endif
