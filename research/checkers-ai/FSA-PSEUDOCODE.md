# Checkers FSA pseudocode

This is implementation pseudocode for the architecture in
[FSA-ARCHITECTURE.md](FSA-ARCHITECTURE.md).  It describes a deterministic
Teensy agent that is advanced in small units of work outside the cartridge
bus's time-critical read path.

```text
enum Phase {
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
}

struct Board {
    bits[32] humanMen
    bits[32] agentMen
    bits[32] kings
    Side sideToMove
    Optional<Square> forcedJumpPiece
}

struct Move {
    Square origin
    List<Square> landings
    List<Square> capturedSquares
    Boolean promotes
}

struct Projection {
    Move proposedMove
    Board resultingBoard
    List<Move> opponentReplies
    Integer policyValue
    Trace transitionTrace
}

struct Interaction {
    Move intendedMove
    Board expectedResult
    Board observedResult
    Boolean matched
}

struct AgentMemory {
    Phase phase

    Integer activeSequence
    Board requestedBoard              // immutable while evaluating
    Board observedBoard               // most recently accepted board
    Optional<Interaction> priorInteraction
    History recentHistory

    List<Move> candidates
    Integer generateCursor
    Integer projectCursor
    List<Projection> projections
    Optional<Move> selectedMove
    Optional<Error> error
}


function serviceCheckersMailbox(memory):
    // Called only from a safe, non-time-critical point in the firmware.
    // Execute one small bounded unit of work, then return to bus service.

    switch memory.phase:

        case WAIT_REQUEST:
            if mailbox.commandWasCommitted() == false:
                return

            mailbox.writeStatus(BUSY)
            memory.phase = SNAPSHOT_REQUEST
            return

        case SNAPSHOT_REQUEST:
            request = mailbox.readRequestSnapshot()

            if request.magic != CHECKERS_MAGIC:
                memory.error = BAD_MAGIC
                memory.phase = COMMIT_ERROR
                return

            if request.sequence <= memory.activeSequence:
                memory.error = STALE_SEQUENCE
                memory.phase = COMMIT_ERROR
                return

            board = decodeAndValidateBoard(request)
            if board is INVALID:
                memory.error = INVALID_BOARD
                memory.phase = COMMIT_ERROR
                return

            memory.activeSequence = request.sequence
            memory.requestedBoard = copy(board)
            memory.observedBoard = copy(board)
            memory.candidates.clear()
            memory.projections.clear()
            memory.selectedMove = NONE
            memory.generateCursor = FIRST_PLAYABLE_SQUARE
            memory.projectCursor = 0
            memory.phase = RECONCILE_RESULT
            return

        case RECONCILE_RESULT:
            if memory.priorInteraction exists:
                memory.priorInteraction.observedResult = memory.observedBoard
                memory.priorInteraction.matched = boardsMatch(
                    memory.priorInteraction.expectedResult,
                    memory.priorInteraction.observedResult)

                recordInteraction(memory.recentHistory, memory.priorInteraction)

                if memory.priorInteraction.matched == false:
                    // Never continue a planned line after an unexpected result.
                    // This also catches an out-of-date surface or mailbox error.
                    memory.error = UNEXPECTED_BOARD_RESULT
                    memory.phase = COMMIT_ERROR
                    return

            memory.phase = GENERATE
            return

        case GENERATE:
            // One piece per call: bounded work suitable for the Teensy.
            square = memory.generateCursor
            if square is past LAST_PLAYABLE_SQUARE:
                if memory.candidates is empty:
                    memory.error = NO_LEGAL_MOVE
                    memory.phase = COMMIT_ERROR
                else:
                    memory.phase = PROJECT
                return

            appendLegalMovesForOnePiece(memory.requestedBoard, square,
                                        memory.candidates)
            memory.generateCursor = nextPlayableSquare(square)

            // Once any capture exists, only capture moves may remain.
            enforceCompulsoryCapture(memory.candidates)
            return

        case PROJECT:
            // Anticipation: project exactly one candidate per call.
            if memory.projectCursor == count(memory.candidates):
                memory.phase = EVALUATE
                return

            move = memory.candidates[memory.projectCursor]
            afterMove = applyMove(copy(memory.requestedBoard), move)

            projection = Projection()
            projection.proposedMove = move
            projection.resultingBoard = afterMove
            projection.transitionTrace = trace(move, afterMove)

            // Version 1 can omit this and score only afterMove.
            // A later bounded version generates a small set of opponent replies.
            projection.opponentReplies = generateBoundedOpponentReplies(afterMove)

            append(memory.projections, projection)
            memory.projectCursor += 1
            return

        case EVALUATE:
            // Score one projection per call; do not mix these policy terms
            // into the FSA transition logic.
            projection = nextUnscoredProjection(memory.projections)
            if projection does not exist:
                memory.phase = SELECT
                return

            projection.policyValue = strategyValue(
                memory.requestedBoard,
                projection.proposedMove,
                projection.resultingBoard,
                projection.opponentReplies)
            markScored(projection)
            return

        case SELECT:
            // Deterministic stable tie-break means fixtures are reproducible.
            winner = bestProjection(memory.projections,
                                    by highest policyValue,
                                    then longest capture path,
                                    then lowest origin,
                                    then lowest landing sequence)
            memory.selectedMove = winner.proposedMove
            memory.priorInteraction = Interaction(
                intendedMove = winner.proposedMove,
                expectedResult = winner.resultingBoard,
                observedResult = UNKNOWN,
                matched = false)
            memory.phase = COMMIT_RESPONSE
            return

        case COMMIT_RESPONSE:
            // Commit data first; READY is the final write.
            mailbox.writeMove(memory.selectedMove)
            mailbox.writeDiagnostic(selectedProjection(memory).policyValue)
            mailbox.writeResponse(memory.activeSequence, READY)
            memory.phase = WAIT_RESULT
            return

        case COMMIT_ERROR:
            mailbox.writeDiagnostic(memory.error)
            mailbox.writeResponse(memory.activeSequence, ERROR)
            memory.phase = WAIT_RESULT
            return

        case WAIT_RESULT:
            // The next sequence supplies the surface's result of the move.
            if mailbox.hasNewCommittedSequence(memory.activeSequence):
                memory.phase = WAIT_REQUEST
            return
```

## Strategy policy

The policy is deliberately a function, rather than an FSA phase.  It ranks
anticipated outcomes but has no authority to bypass legality checks, mutate
the request snapshot, or publish a mailbox response.

```text
function strategyValue(before, move, after, opponentReplies):
    score = 0

    if move.capturedSquares is not empty:
        score += 1_000 * count(move.capturedSquares)

    if move.promotes:
        score += 500

    if destinationIsCentral(move):
        score += 25

    if destinationIsImmediatelyCapturable(after, move, opponentReplies):
        score -= 200

    score += materialBalance(after) * 10

    return score
```

## Composite anticipation for a multi-jump

```text
function projectCaptureSequence(board, partialMove):
    afterHop = applyOneCaptureHop(copy(board), partialMove.lastHop)
    nextHops = legalCaptureHops(afterHop, partialMove.currentPiece)

    if nextHops is empty:
        return [completeProjection(partialMove, afterHop)]

    projections = []
    for each hop in nextHops:
        append(projections,
               projectCaptureSequence(afterHop, partialMove extendedBy hop))
    return projections
```

This is the Checkers equivalent of an intended composite interaction: every
hop has a prerequisite projected result.  When the surface later returns its
board, `RECONCILE_RESULT` confirms the completed move rather than assuming
that the intended sequence occurred.
