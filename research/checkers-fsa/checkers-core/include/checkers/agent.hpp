#ifndef CHECKERS_AGENT_HPP
#define CHECKERS_AGENT_HPP

#include "checkers/types.hpp"

namespace checkers {

class Agent {
public:
    Agent();

    void reset();
    bool acceptRequest(const MailboxRequest& request, Error requestError);
    bool step();
    bool acknowledgeResponse();
    bool responseReady() const;
    MailboxResponse response() const;
    Phase phase() const;

private:
    Phase phase_;
    uint16_t activeSequence_;
    MailboxRequest request_;
    MailboxResponse response_;
    Error pendingError_;
    bool hasRequest_;
    bool responseReady_;
};

}  // namespace checkers

#endif
