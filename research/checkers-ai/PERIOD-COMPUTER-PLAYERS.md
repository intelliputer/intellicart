# Period Intellivision computer players

## Scope

This is a working historical list of released Intellivision-era games that let
a human play **against a selectable computer opponent**. It does not count a
game merely because it contains computer-controlled teammates, referees,
enemies, scorekeeping, or a demo mode. The distinction matters for the
Checkers project: we want games with an opponent decision loop, turn hand-off,
and a visible response to a human move.

It is deliberately a conservative starting list, not yet a claim to be the
complete 125-cartridge catalogue.

## Direct predecessors: board and strategy games

| Title | Era | Documented computer-player behaviour | Why it matters here |
| --- | --- | --- | --- |
| [ABPA Backgammon](https://history.blueskyrangers.com/instructions/backgammoninst.html) | 1979 launch era | One player can compete with the computer at two skill levels; two-player play is also supported. | The closest early example of selectable level, human/computer turn exchange, and a board-game UI. |
| [Checkers / Draughts](https://history.blueskyrangers.com/instructions/checkers.html) | 1980 | Human-first or computer-first modes, high/low skill, and `COMPUTER SUGGESTS MOVE`. | The direct behavioural reference. Its optional suggestion feature is especially interesting for exposing an FSA's selected move without committing it. |
| [Reversi](https://history.blueskyrangers.com/instructions/reversi.html) | 1982 | Computer-first or human-first modes, three skill levels, three board sizes, and a suggested-move feature. | A compact board representation with explicit thinking levels; useful comparative material for our fixture and trace design. |
| [USCF Chess](https://history.blueskyrangers.com/mattelelectronics/games/chess.html) | 1982 | Nine computer difficulty levels; one- or two-player play. | Important hardware precedent: it used 2K of cartridge RAM because the base console's RAM was insufficient, and difficult moves could take hours or days. |
| [Mind Strike](https://history.blueskyrangers.com/mattelelectronics/games/mindstrike.html) | 1983, ECS | Computer or human opponent, computer-vs-computer viewing, and keyboard-programmable computer-opponent skill. | Strong conceptual precedent for a configurable opponent. Its history explicitly says the base system lacked memory for a computer player, while the ECS extra RAM enabled it. |
| [Triple Challenge](https://history.blueskyrangers.com/instructions/triplechallenge.html) | 1987 | Chess, Checkers, and Backgammon each playable against a friend or the computer. | A later repackaging of the three original strategy opponents; useful proof that this group was perceived as a coherent feature set. |

## Sports games with a selectable computer opponent

| Title | Era | Documented computer-player behaviour | Relevance |
| --- | --- | --- | --- |
| [World Series Major League Baseball](https://www.gamesdatabase.org/Media/SYSTEM/Mattel_Intellivision/Manual/formated/Major_League_Baseball_-_1980_-_Mattel_Electronics.pdf) | 1983, ECS/Keyboard Component | The keyboard selects a human league while the computer controls the other; it also has a computer-demo mode. | A useful asynchronous, phase-based opponent: batting, fielding, roster, and strategy phases. |
| [World Cup Soccer](https://history.blueskyrangers.com/dextell/worldcupsoccer.html) | 1985 | The catalogue describes play against an opponent **or against the Intellivision unit**. | Shows computer play in a real-time team game rather than a turn-based board game. |
| [Super Pro Football](https://history.blueskyrangers.com/intvcorp/superprofootball.html) | 1986 | The product description says the player may outwit an opponent or play against the computer. | A later enhancement of the original two-player NFL Football. |
| [World Championship Baseball](https://history.blueskyrangers.com/game-list.html) | 1986 | The release notes say it added a computer opponent to the earlier Major League Baseball game. | Good example of a targeted computer-player retrofit to an existing design. |
| [Slam Dunk: Super Pro Basketball](https://history.blueskyrangers.com/instructions/slamdunk.html) | 1987 | Either team can be assigned to the computer; the manual also allows computer-created rosters. | An explicit per-side controller assignment model, useful for a future computer-vs-computer regression mode. |
| [Slap Shot: Super Pro Hockey](https://history.blueskyrangers.com/instructions/slapshot.html) | 1987 | Each side can be assigned to a human controller or to the computer, at selectable team skill levels. | Another clear controller-to-computer assignment model. |
| [Super Pro Baseball](https://www.intellivision.us/intvgames/baseball/baseball.php) | late INTV era | One-player, two-player, and computer-vs-computer title-screen modes are documented. | Direct precedent for an automated fixture/demo mode that uses the same game surface. |

## Computer-controlled does not necessarily mean computer opponent

These are useful implementation references but do **not** belong in the main
list under this definition:

| Title | What the computer does | Why excluded |
| --- | --- | --- |
| [Utopia](https://www.intvfunhouse.com/games/utop.php) | Runs simulation events; one human can play one island. | The documented Intellivision version has no computer AI controlling the other island. |
| [NFL Football](https://history.blueskyrangers.com/instructions/nfl.html) | Controls the non-captain players on both human teams. | Its manual is for two players; the computer fills out each side rather than replacing an opponent. |
| [Tennis](https://history.blueskyrangers.com/instructions/tennis.html) | Plays line judge and keeps score. | The original is explicitly a two-player game. |
| [Snafu](https://history.blueskyrangers.com/instructions/snafu.html) | Can trap a player with computer-controlled trails or serpents. | It has an autonomous adversary, but not the alternating board-game opponent model we are studying. |

## What to study next

For the Checkers FSA, the first four titles are the priority comparison set:

1. **Checkers** — user-facing mode selection, skill selection, move suggestion,
   and full move exchange.
2. **Reversi** — board sizes, suggestion facility, and three-level policy.
3. **ABPA Backgammon** — small-board representation and two-level opponent.
4. **USCF Chess** — the historical warning: do not let deep search block the
   surface or bus; use the Teensy's asynchronous service loop instead.

The next research pass should obtain and annotate their manuals/screens in
Mira, then compare each title's turn-state UI with our `BUSY`, `READY`,
suggestion, and error states. That will give the CP1610 surface an
Intellivision-native interaction vocabulary without copying any original ROM
logic.
