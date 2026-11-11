#include "checkers/agent.hpp"

#include "checkers/board.hpp"
#include "checkers/mailbox_abi.hpp"

namespace checkers {

namespace {

Move emptyMove(uint16_t diagnostic) {
    Move move = {};
    move.origin = 0xFFU;
    move.landingCount = 0;
    move.diagnostic = diagnostic;
    return move;
}

}  // namespace

Agent::Agent() {
    reset();
}

void Agent::reset() {
    phase_ = Phase::WAIT_REQUEST;
    activeSequence_ = 0;
    request_ = {};
    response_ = {};
    pendingError_ = Error::NONE;
    hasRequest_ = false;
    responseReady_ = false;
}

bool Agent::acceptRequest(const MailboxRequest& request, Error requestError) {
    if (phase_ != Phase::WAIT_REQUEST || hasRequest_) {
        return false;
    }

    request_ = request;
    pendingError_ = requestError;
    hasRequest_ = true;
    responseReady_ = false;
    phase_ = Phase::SNAPSHOT_REQUEST;
    return true;
}

bool Agent::step() {
    switch (phase_) {
        case Phase::WAIT_REQUEST:
        case Phase::WAIT_RESULT:
            return false;

        case Phase::SNAPSHOT_REQUEST:
            if (pendingError_ != Error::NONE) {
                phase_ = Phase::COMMIT_ERROR;
            } else if (request_.sequence == activeSequence_) {
                pendingError_ = Error::STALE_SEQUENCE;
                phase_ = Phase::COMMIT_ERROR;
            } else if (request_.command == Command::BOARD &&
                       validateBoard(request_.board) != BoardError::NONE) {
                pendingError_ = Error::INVALID_BOARD;
                phase_ = Phase::COMMIT_ERROR;
            } else {
                activeSequence_ = request_.sequence;
                phase_ = Phase::RECONCILE_RESULT;
            }
            return true;

        case Phase::RECONCILE_RESULT:
            if (request_.command == Command::PING) {
                response_.status = Status::READY;
                response_.sequence = activeSequence_;
                response_.move = emptyMove(abi::kDiagnosticPing);
                response_.error = Error::NONE;
                phase_ = Phase::COMMIT_RESPONSE;
            } else {
                pendingError_ = Error::UNIMPLEMENTED;
                phase_ = Phase::COMMIT_ERROR;
            }
            return true;

        case Phase::COMMIT_RESPONSE:
            responseReady_ = true;
            hasRequest_ = false;
            phase_ = Phase::WAIT_RESULT;
            return true;

        case Phase::COMMIT_ERROR:
            response_.status = Status::ERROR;
            response_.sequence = request_.sequence;
            response_.move = emptyMove(static_cast<uint16_t>(pendingError_));
            response_.error = pendingError_;
            responseReady_ = true;
            hasRequest_ = false;
            phase_ = Phase::WAIT_RESULT;
            return true;

        case Phase::GENERATE:
        case Phase::PROJECT:
        case Phase::EVALUATE:
        case Phase::SELECT:
            pendingError_ = Error::UNIMPLEMENTED;
            phase_ = Phase::COMMIT_ERROR;
            return true;
    }

    return false;
}

bool Agent::acknowledgeResponse() {
    if (phase_ != Phase::WAIT_RESULT || !responseReady_) {
        return false;
    }
    responseReady_ = false;
    phase_ = Phase::WAIT_REQUEST;
    return true;
}

bool Agent::responseReady() const {
    return responseReady_;
}

MailboxResponse Agent::response() const {
    return response_;
}

Phase Agent::phase() const {
    return phase_;
}

}  // namespace checkers
