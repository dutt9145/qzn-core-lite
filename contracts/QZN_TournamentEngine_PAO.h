// ============================================================
//  QZN TOURNAMENT ENGINE PAO
//  Contract: Programmable Arcade Object — Tournament Engine
//  Network:  Qubic (QPI / C++ Smart Contract)
//  Version:  2.0.0
//
//  QPI Compliance Fixes (v1 → v2):
//    - Removed #pragma once and #include "qpi.h"
//      → replaced with: using namespace QPI;
//    - Replaced all #define constants with constexpr
//    - Replaced typed enums (enum X : uint8) with constexpr uint8
//    - Replaced true/false with 1/0 for bit types
//    - Replaced division operator / with div(), modulo % with mod()
//    - Removed all C++ references (&) — direct state indexing used
//    - Removed internal helper methods from struct body
//      → all logic inlined into PUBLIC_PROCEDURE blocks
//    - Replaced switch statements with if/else chains
//    - Renamed Tournament.state → Tournament.tstate
//      (avoids collision with QPI 'state' keyword)
//    - Added state.scratchIds[] buffer for bracket generation
//      (replaces local stack arrays in helpers)
//    - Added InitializeTournamentEngine procedure
//    - Added REGISTER_USER_FUNCTIONS_AND_PROCEDURES block
//    - Added BEGIN_EPOCH and END_TICK hooks
//
//  Deterministic tournament coordination contract.
//  Supports single elimination, double elimination, and
//  round robin formats. Prize distribution: 60 / 30 / 10.
//  Tournament creation restricted to PAO admin / protocol.
// ============================================================

using namespace QPI;

// ============================================================
//  CONSTANTS
// ============================================================

constexpr uint32 QZN_TOURNAMENT_MAX_TOURNAMENTS  = 64;
constexpr uint32 QZN_TOURNAMENT_MAX_PLAYERS      = 64;   // must be power of 2 for bracket formats
constexpr uint32 QZN_TOURNAMENT_MAX_MATCHES      = 256;  // covers double-elim with 64 players
constexpr uint64 QZN_TOURNAMENT_PRIZE_FIRST      = 60ULL; // %
constexpr uint64 QZN_TOURNAMENT_PRIZE_SECOND     = 30ULL; // %
constexpr uint64 QZN_TOURNAMENT_MIN_ENTRY_FEE    = 100ULL; // QZN units
constexpr uint32 QZN_TOURNAMENT_MIN_PLAYERS      = 4;

// TournamentFormat constants (replaces typed enum)
constexpr uint8  FORMAT_SINGLE_ELIMINATION       = 0;
constexpr uint8  FORMAT_DOUBLE_ELIMINATION       = 1;
constexpr uint8  FORMAT_ROUND_ROBIN              = 2;

// TournamentState constants (replaces typed enum)
// Prefixed TSTATE_ to avoid collision with GameCabinet STATE_ constants
constexpr uint8  TSTATE_REGISTRATION             = 0;
constexpr uint8  TSTATE_IN_PROGRESS              = 1;
constexpr uint8  TSTATE_FINALIZING               = 2;
constexpr uint8  TSTATE_COMPLETE                 = 3;
constexpr uint8  TSTATE_CANCELLED                = 4;

// MatchResult constants (replaces typed enum)
constexpr uint8  TMATCH_PENDING                  = 0;
constexpr uint8  TMATCH_PLAYER_A                 = 1;
constexpr uint8  TMATCH_PLAYER_B                 = 2;
constexpr uint8  TMATCH_DRAW                     = 3;   // round robin only

// BracketSide constants (replaces typed enum)
constexpr uint8  BRACKET_WINNERS                 = 0;
constexpr uint8  BRACKET_LOSERS                  = 1;   // double elimination only

// ============================================================
//  DATA STRUCTURES
// ============================================================

struct TournamentMatch
{
    uint32 matchIndex;
    id     playerA;
    id     playerB;
    id     winner;
    id     loser;
    uint8  result;        // TMATCH_* constant
    uint8  bracketSide;   // BRACKET_* constant
    uint32 round;
    uint64 resultHash;    // deterministic hash submitted by admin
    bit    settled;
};

struct PlayerRecord
{
    id     wallet;
    uint32 wins;
    uint32 losses;
    uint32 draws;           // round robin only
    uint32 points;          // round robin: win=3, draw=1, loss=0
    bit    eliminated;
    bit    inLosersBracket; // double elim
    bit    registered;
};

struct TournamentRecord
{
    uint32          tournamentId;
    uint8           format;           // FORMAT_* constant
    uint8           tstate;           // TSTATE_* constant (renamed from 'state')
    uint8           registeredCount;
    uint8           maxPlayers;
    uint64          entryFee;         // per player in QZN units
    uint64          prizePool;        // total accumulated
    uint32          currentRound;
    uint32          totalRounds;
    uint32          matchCount;
    uint32          settledMatchCount;
    id              admin;
    id              cabinetPAO;       // GameCabinet PAO governing match results
    id              first;            // prize winners
    id              second;
    id              third;
    bit             prizesDistributed;
    PlayerRecord    players[QZN_TOURNAMENT_MAX_PLAYERS];
    TournamentMatch matches[QZN_TOURNAMENT_MAX_MATCHES];
};

// ============================================================
//  CONTRACT STATE
// ============================================================

struct QZNTOUR
{
    TournamentRecord tournaments[QZN_TOURNAMENT_MAX_TOURNAMENTS];
    uint32           tournamentCount;
    id               protocolAdmin;
    uint64           totalPrizesDistributed;
    bit              initialized;

    // Scratch buffer for bracket generation and winner collection.
    // Used in StartTournament (round-robin rotation) and
    // SubmitMatchResult (single/double elim advancement).
    // Indices 0-31: winners bracket scratch
    // Indices 32-63: losers bracket scratch (double elim only)
    id               scratchIds[QZN_TOURNAMENT_MAX_PLAYERS];
};

// ============================================================
//  INPUT / OUTPUT STRUCTS
// ============================================================

struct InitializeTournamentEngine_input
{
    id adminOverride; // Set to zero-id to use invocator address
};
struct InitializeTournamentEngine_output
{
    bit success;
};

struct CreateTournament_input
{
    uint8  format;       // FORMAT_* constant
    uint8  maxPlayers;   // 4, 8, 16, 32, or 64
    uint64 entryFee;     // QZN units per player
    id     cabinetPAO;   // GameCabinet PAO governing match results
};
struct CreateTournament_output
{
    uint32 tournamentId;
    bit    success;
};

struct RegisterPlayer_input
{
    uint32 tournamentId;
};
struct RegisterPlayer_output
{
    uint8 slotIndex;
    bit   success;
};

struct StartTournament_input
{
    uint32 tournamentId;
};
struct StartTournament_output
{
    uint32 matchCount;
    bit    success;
};

struct SubmitMatchResult_input
{
    uint32 tournamentId;
    uint32 matchIndex;
    id     winner;
    uint64 resultHash;
};
struct SubmitMatchResult_output
{
    bit advancedRound;
    bit tournamentComplete;
    bit success;
};

struct CancelTournament_input
{
    uint32 tournamentId;
};
struct CancelTournament_output
{
    bit success;
};

struct GetTournament_input
{
    uint32 tournamentId;
};
struct GetTournament_output
{
    uint32 tournamentId;
    uint8  format;
    uint8  tstate;
    uint8  registeredCount;
    uint8  maxPlayers;
    uint64 entryFee;
    uint64 prizePool;
    uint32 currentRound;
    uint32 matchCount;
    uint32 settledMatchCount;
    id     first;
    id     second;
    id     third;
    bit    prizesDistributed;
};

struct GetMatch_input
{
    uint32 tournamentId;
    uint32 matchIndex;
};
struct GetMatch_output
{
    TournamentMatch match;
    bit             found;
};

struct GetPlayerRecord_input
{
    uint32 tournamentId;
    id     wallet;
};
struct GetPlayerRecord_output
{
    PlayerRecord record;
    bit          found;
};

// ============================================================
//  PUBLIC PROCEDURES
// ============================================================

PUBLIC_PROCEDURE(InitializeTournamentEngine)
/*
 * One-time initialization. Sets protocol admin.
 * If adminOverride is zero-id, uses the invoking address.
 * Guard: can only be called once.
 */
{
    if (state.initialized)
    {
        output.success = 0;
        return;
    }

    if (input.adminOverride == NULL_ID)
    {
        state.protocolAdmin = qpi.invocator();
    }
    else
    {
        state.protocolAdmin = input.adminOverride;
    }

    state.tournamentCount        = 0;
    state.totalPrizesDistributed = 0;
    state.initialized            = 1;
    output.success               = 1;
}
_

PUBLIC_PROCEDURE(CreateTournament)
/*
 * Admin only. Creates a new tournament in REGISTRATION state.
 * maxPlayers must be a power of 2 between 4 and 64.
 * entryFee must meet minimum threshold.
 */
{
    if (qpi.invocator() != state.protocolAdmin)
    {
        output.success = 0;
        return;
    }

    if (input.format > FORMAT_ROUND_ROBIN)
    {
        output.success = 0;
        return;
    }

    // Validate player count: must be power of 2, within [MIN, MAX]
    uint8 mp = input.maxPlayers;
    if (mp < (uint8)QZN_TOURNAMENT_MIN_PLAYERS || mp > (uint8)QZN_TOURNAMENT_MAX_PLAYERS)
    {
        output.success = 0;
        return;
    }
    // Power-of-2 check via bitwise AND
    if ((mp & (mp - 1)) != 0)
    {
        output.success = 0;
        return;
    }

    if (input.entryFee < QZN_TOURNAMENT_MIN_ENTRY_FEE)
    {
        output.success = 0;
        return;
    }

    if (state.tournamentCount >= QZN_TOURNAMENT_MAX_TOURNAMENTS)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx = state.tournamentCount;

    state.tournaments[tIdx].tournamentId      = tIdx;
    state.tournaments[tIdx].format            = input.format;
    state.tournaments[tIdx].tstate            = TSTATE_REGISTRATION;
    state.tournaments[tIdx].registeredCount   = 0;
    state.tournaments[tIdx].maxPlayers        = input.maxPlayers;
    state.tournaments[tIdx].entryFee          = input.entryFee;
    state.tournaments[tIdx].prizePool         = 0;
    state.tournaments[tIdx].currentRound      = 0;
    state.tournaments[tIdx].totalRounds       = 0;
    state.tournaments[tIdx].matchCount        = 0;
    state.tournaments[tIdx].settledMatchCount = 0;
    state.tournaments[tIdx].admin             = qpi.invocator();
    state.tournaments[tIdx].cabinetPAO        = input.cabinetPAO;
    state.tournaments[tIdx].prizesDistributed = 0;
    state.tournaments[tIdx].first             = NULL_ID;
    state.tournaments[tIdx].second            = NULL_ID;
    state.tournaments[tIdx].third             = NULL_ID;

    state.tournamentCount = state.tournamentCount + 1;

    output.tournamentId = tIdx;
    output.success      = 1;
}
_

PUBLIC_PROCEDURE(RegisterPlayer)
/*
 * Any player. Registers for a tournament in REGISTRATION state.
 * Invocation reward must cover entryFee; overpayment is refunded.
 * Duplicate registration is rejected.
 */
{
    if (input.tournamentId >= state.tournamentCount)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx  = input.tournamentId;
    id     caller = qpi.invocator();

    if (state.tournaments[tIdx].tstate != TSTATE_REGISTRATION)
    {
        output.success = 0;
        return;
    }

    if (state.tournaments[tIdx].registeredCount >= state.tournaments[tIdx].maxPlayers)
    {
        output.success = 0;
        return;
    }

    // Check not already registered — inline findPlayerIndex
    uint32 regCount = state.tournaments[tIdx].registeredCount;
    for (uint32 ii = 0; ii < regCount; ii = ii + 1)
    {
        if (state.tournaments[tIdx].players[ii].wallet == caller)
        {
            output.success = 0;
            return;
        }
    }

    // Collect entry fee — refund if insufficient
    if (qpi.invocationReward() < state.tournaments[tIdx].entryFee)
    {
        qpi.transfer(caller, qpi.invocationReward());
        output.success = 0;
        return;
    }

    // Refund overpayment
    if (qpi.invocationReward() > state.tournaments[tIdx].entryFee)
    {
        qpi.transfer(caller, qpi.invocationReward() - state.tournaments[tIdx].entryFee);
    }

    uint8 slot = state.tournaments[tIdx].registeredCount;

    state.tournaments[tIdx].players[slot].wallet          = caller;
    state.tournaments[tIdx].players[slot].wins            = 0;
    state.tournaments[tIdx].players[slot].losses          = 0;
    state.tournaments[tIdx].players[slot].draws           = 0;
    state.tournaments[tIdx].players[slot].points          = 0;
    state.tournaments[tIdx].players[slot].eliminated      = 0;
    state.tournaments[tIdx].players[slot].inLosersBracket = 0;
    state.tournaments[tIdx].players[slot].registered      = 1;

    state.tournaments[tIdx].prizePool        = state.tournaments[tIdx].prizePool + state.tournaments[tIdx].entryFee;
    state.tournaments[tIdx].registeredCount  = state.tournaments[tIdx].registeredCount + 1;

    output.slotIndex = slot;
    output.success   = 1;
}
_

PUBLIC_PROCEDURE(StartTournament)
/*
 * Admin only. Generates bracket / schedule and moves to IN_PROGRESS.
 * Single Elim:    pairs players 0v1, 2v3, ... round 1 matches only.
 *                 Subsequent rounds generated dynamically in SubmitMatchResult.
 * Double Elim:    same winners-bracket R1 as single elim.
 *                 Losers bracket generated dynamically.
 * Round Robin:    full schedule generated up front using rotation algorithm.
 *                 Uses state.scratchIds as rotation buffer.
 */
{
    if (input.tournamentId >= state.tournamentCount)
    {
        output.success = 0;
        return;
    }

    if (qpi.invocator() != state.protocolAdmin)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx = input.tournamentId;

    if (state.tournaments[tIdx].tstate != TSTATE_REGISTRATION)
    {
        output.success = 0;
        return;
    }

    if (state.tournaments[tIdx].registeredCount < QZN_TOURNAMENT_MIN_PLAYERS)
    {
        output.success = 0;
        return;
    }

    uint32 n    = state.tournaments[tIdx].registeredCount;
    uint32 mIdx = 0;

    if (state.tournaments[tIdx].format == FORMAT_SINGLE_ELIMINATION)
    {
        // Pair players: 0v1, 2v3, 4v5, ... for round 1
        for (uint32 i = 0; i < n; i = i + 2)
        {
            state.tournaments[tIdx].matches[mIdx].matchIndex  = mIdx;
            state.tournaments[tIdx].matches[mIdx].playerA     = state.tournaments[tIdx].players[i].wallet;
            state.tournaments[tIdx].matches[mIdx].playerB     = state.tournaments[tIdx].players[i + 1].wallet;
            state.tournaments[tIdx].matches[mIdx].result      = TMATCH_PENDING;
            state.tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
            state.tournaments[tIdx].matches[mIdx].round       = 1;
            state.tournaments[tIdx].matches[mIdx].settled     = 0;
            mIdx = mIdx + 1;
        }

        // Calculate total rounds: log2(n) via right-shift
        uint32 roundsCalc = 0;
        uint32 nCalc      = n;
        while (nCalc > 1)
        {
            nCalc      = nCalc >> 1;
            roundsCalc = roundsCalc + 1;
        }

        state.tournaments[tIdx].matchCount   = mIdx;
        state.tournaments[tIdx].totalRounds  = roundsCalc;
        state.tournaments[tIdx].currentRound = 1;
    }
    else if (state.tournaments[tIdx].format == FORMAT_DOUBLE_ELIMINATION)
    {
        // Winners bracket round 1 only — losers bracket generated dynamically
        for (uint32 i = 0; i < n; i = i + 2)
        {
            state.tournaments[tIdx].matches[mIdx].matchIndex  = mIdx;
            state.tournaments[tIdx].matches[mIdx].playerA     = state.tournaments[tIdx].players[i].wallet;
            state.tournaments[tIdx].matches[mIdx].playerB     = state.tournaments[tIdx].players[i + 1].wallet;
            state.tournaments[tIdx].matches[mIdx].result      = TMATCH_PENDING;
            state.tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
            state.tournaments[tIdx].matches[mIdx].round       = 1;
            state.tournaments[tIdx].matches[mIdx].settled     = 0;
            mIdx = mIdx + 1;
        }

        uint32 roundsCalc = 0;
        uint32 nCalc      = n;
        while (nCalc > 1)
        {
            nCalc      = nCalc >> 1;
            roundsCalc = roundsCalc + 1;
        }

        state.tournaments[tIdx].matchCount   = mIdx;
        state.tournaments[tIdx].totalRounds  = roundsCalc * 2 + 1; // approx for double elim
        state.tournaments[tIdx].currentRound = 1;
    }
    else // FORMAT_ROUND_ROBIN
    {
        // Classic round-robin rotation algorithm.
        // Fix player[0], rotate positions 1..n-1.
        // Uses state.scratchIds as the rotation buffer.

        for (uint32 i = 0; i < n; i = i + 1)
        {
            state.scratchIds[i] = state.tournaments[tIdx].players[i].wallet;
        }

        uint32 round = 1;

        for (uint32 r = 0; r < n - 1; r = r + 1)
        {
            // Generate matches for this round
            for (uint32 i = 0; i < n / 2; i = i + 1)
            {
                uint32 j = n - 1 - i;

                state.tournaments[tIdx].matches[mIdx].matchIndex  = mIdx;
                state.tournaments[tIdx].matches[mIdx].playerA     = state.scratchIds[i];
                state.tournaments[tIdx].matches[mIdx].playerB     = state.scratchIds[j];
                state.tournaments[tIdx].matches[mIdx].result      = TMATCH_PENDING;
                state.tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
                state.tournaments[tIdx].matches[mIdx].round       = round;
                state.tournaments[tIdx].matches[mIdx].settled     = 0;
                mIdx = mIdx + 1;
            }

            round = round + 1;

            // Rotate: fix index 0, rotate indices 1..n-1
            id lastId = state.scratchIds[n - 1];
            for (uint32 k = n - 1; k > 1; k = k - 1)
            {
                state.scratchIds[k] = state.scratchIds[k - 1];
            }
            state.scratchIds[1] = lastId;
        }

        state.tournaments[tIdx].matchCount   = mIdx;
        state.tournaments[tIdx].totalRounds  = n - 1;
        state.tournaments[tIdx].currentRound = 1;
    }

    state.tournaments[tIdx].tstate = TSTATE_IN_PROGRESS;

    output.matchCount = state.tournaments[tIdx].matchCount;
    output.success    = 1;
}
_

PUBLIC_PROCEDURE(SubmitMatchResult)
/*
 * Admin only. Records match result, updates player records,
 * and if the current round is fully settled, advances the bracket
 * or finalizes the tournament with prize distribution.
 *
 * Single Elim:
 *   Winners collected into state.scratchIds[0..wCount-1].
 *   If 1 winner → finalize. Otherwise generate next-round matches.
 *
 * Double Elim:
 *   Winners in scratchIds[0..wCount-1].
 *   Losers in scratchIds[32..32+lCount-1].
 *   Grand final detected when 1 winners-bracket survivor remains.
 *
 * Round Robin:
 *   On final round, sort players by points (insertion sort) and finalize.
 *
 * Prize distribution (inline): 60% / 30% / 10% via div().
 */
{
    if (input.tournamentId >= state.tournamentCount)
    {
        output.success = 0;
        return;
    }

    if (qpi.invocator() != state.protocolAdmin)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx = input.tournamentId;

    if (state.tournaments[tIdx].tstate != TSTATE_IN_PROGRESS)
    {
        output.success = 0;
        return;
    }

    if (input.matchIndex >= state.tournaments[tIdx].matchCount)
    {
        output.success = 0;
        return;
    }

    if (state.tournaments[tIdx].matches[input.matchIndex].settled)
    {
        output.success = 0;
        return;
    }

    // Validate winner is one of the two match players
    id mPlayerA = state.tournaments[tIdx].matches[input.matchIndex].playerA;
    id mPlayerB = state.tournaments[tIdx].matches[input.matchIndex].playerB;

    if (input.winner != mPlayerA && input.winner != mPlayerB)
    {
        output.success = 0;
        return;
    }

    // Record match result
    state.tournaments[tIdx].matches[input.matchIndex].winner     = input.winner;
    state.tournaments[tIdx].matches[input.matchIndex].resultHash = input.resultHash;
    state.tournaments[tIdx].matches[input.matchIndex].settled    = 1;

    if (input.winner == mPlayerA)
    {
        state.tournaments[tIdx].matches[input.matchIndex].loser  = mPlayerB;
        state.tournaments[tIdx].matches[input.matchIndex].result = TMATCH_PLAYER_A;
    }
    else
    {
        state.tournaments[tIdx].matches[input.matchIndex].loser  = mPlayerA;
        state.tournaments[tIdx].matches[input.matchIndex].result = TMATCH_PLAYER_B;
    }

    state.tournaments[tIdx].settledMatchCount = state.tournaments[tIdx].settledMatchCount + 1;

    id matchLoser  = state.tournaments[tIdx].matches[input.matchIndex].loser;
    uint32 regCount = state.tournaments[tIdx].registeredCount;

    // Update winner player record — inline findPlayerIndex
    for (uint32 ii = 0; ii < regCount; ii = ii + 1)
    {
        if (state.tournaments[tIdx].players[ii].wallet == input.winner)
        {
            state.tournaments[tIdx].players[ii].wins   = state.tournaments[tIdx].players[ii].wins + 1;
            state.tournaments[tIdx].players[ii].points = state.tournaments[tIdx].players[ii].points + 3;
        }
    }

    // Update loser player record — inline findPlayerIndex
    for (uint32 ii = 0; ii < regCount; ii = ii + 1)
    {
        if (state.tournaments[tIdx].players[ii].wallet == matchLoser)
        {
            state.tournaments[tIdx].players[ii].losses = state.tournaments[tIdx].players[ii].losses + 1;

            // Eliminate only if single elim, or already in losers bracket (double elim)
            if (state.tournaments[tIdx].format != FORMAT_DOUBLE_ELIMINATION ||
                state.tournaments[tIdx].players[ii].inLosersBracket)
            {
                state.tournaments[tIdx].players[ii].eliminated = 1;
            }
        }
    }

    output.advancedRound      = 0;
    output.tournamentComplete = 0;
    output.success            = 1;

    // Check if all matches in the current round are settled — inline _currentRoundComplete
    uint32 currentRound = state.tournaments[tIdx].currentRound;
    bit    roundComplete = 1;

    for (uint32 ii = 0; ii < state.tournaments[tIdx].matchCount; ii = ii + 1)
    {
        if (state.tournaments[tIdx].matches[ii].round == currentRound &&
            state.tournaments[tIdx].matches[ii].settled == 0)
        {
            roundComplete = 0;
        }
    }

    if (roundComplete == 0)
    {
        return; // Round still in progress — no advancement needed
    }

    output.advancedRound = 1;

    // ── Single Elimination advancement ───────────────────────
    if (state.tournaments[tIdx].format == FORMAT_SINGLE_ELIMINATION)
    {
        // Collect winners from current round into scratchIds
        uint32 winnerCount = 0;

        for (uint32 ii = 0; ii < state.tournaments[tIdx].matchCount; ii = ii + 1)
        {
            if (state.tournaments[tIdx].matches[ii].round == currentRound &&
                state.tournaments[tIdx].matches[ii].bracketSide == BRACKET_WINNERS)
            {
                state.scratchIds[winnerCount] = state.tournaments[tIdx].matches[ii].winner;
                winnerCount = winnerCount + 1;
            }
        }

        if (winnerCount == 1)
        {
            // Finals complete — determine placements
            state.tournaments[tIdx].first = state.scratchIds[0];

            // Second = loser of the grand final match
            for (uint32 ii = 0; ii < state.tournaments[tIdx].matchCount; ii = ii + 1)
            {
                if (state.tournaments[tIdx].matches[ii].round == currentRound)
                {
                    state.tournaments[tIdx].second = state.tournaments[tIdx].matches[ii].loser;
                }
            }

            // Third = loser of the semifinal who lost to the champion's path
            id finalist = state.tournaments[tIdx].second;
            for (uint32 ii = 0; ii < state.tournaments[tIdx].matchCount; ii = ii + 1)
            {
                if (state.tournaments[tIdx].matches[ii].round == currentRound - 1 &&
                    state.tournaments[tIdx].matches[ii].loser != finalist &&
                    state.tournaments[tIdx].third == NULL_ID)
                {
                    state.tournaments[tIdx].third = state.tournaments[tIdx].matches[ii].loser;
                }
            }

            // Distribute prizes: 60 / 30 / 10 via div()
            if (state.tournaments[tIdx].prizesDistributed == 0)
            {
                state.tournaments[tIdx].tstate = TSTATE_FINALIZING;

                uint64 total  = state.tournaments[tIdx].prizePool;
                uint64 first  = div(total * QZN_TOURNAMENT_PRIZE_FIRST,  (uint64)100);
                uint64 second = div(total * QZN_TOURNAMENT_PRIZE_SECOND, (uint64)100);
                uint64 third  = total - first - second; // remainder prevents rounding dust

                if (state.tournaments[tIdx].first  != NULL_ID) qpi.transfer(state.tournaments[tIdx].first,  first);
                if (state.tournaments[tIdx].second != NULL_ID) qpi.transfer(state.tournaments[tIdx].second, second);
                if (state.tournaments[tIdx].third  != NULL_ID) qpi.transfer(state.tournaments[tIdx].third,  third);

                state.tournaments[tIdx].prizesDistributed = 1;
                state.totalPrizesDistributed = state.totalPrizesDistributed + total;
                state.tournaments[tIdx].tstate = TSTATE_COMPLETE;
            }

            output.tournamentComplete = 1;
            return;
        }

        // Not final round yet — generate next-round matches from winners
        uint32 nextRound = currentRound + 1;
        uint32 mIdx      = state.tournaments[tIdx].matchCount;

        for (uint32 i = 0; i + 1 < winnerCount; i = i + 2)
        {
            state.tournaments[tIdx].matches[mIdx].matchIndex  = mIdx;
            state.tournaments[tIdx].matches[mIdx].playerA     = state.scratchIds[i];
            state.tournaments[tIdx].matches[mIdx].playerB     = state.scratchIds[i + 1];
            state.tournaments[tIdx].matches[mIdx].result      = TMATCH_PENDING;
            state.tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
            state.tournaments[tIdx].matches[mIdx].round       = nextRound;
            state.tournaments[tIdx].matches[mIdx].settled     = 0;
            mIdx = mIdx + 1;
        }

        state.tournaments[tIdx].matchCount   = mIdx;
        state.tournaments[tIdx].currentRound = nextRound;
    }

    // ── Double Elimination advancement ───────────────────────
    else if (state.tournaments[tIdx].format == FORMAT_DOUBLE_ELIMINATION)
    {
        uint32 nextRound = currentRound + 1;
        uint32 mIdx      = state.tournaments[tIdx].matchCount;
        uint32 wCount    = 0;
        uint32 lCount    = 0;

        // Collect winners/losers from current round
        // Winners go into scratchIds[0..wCount-1]
        // Losers go into scratchIds[32..32+lCount-1]
        for (uint32 ii = 0; ii < state.tournaments[tIdx].matchCount; ii = ii + 1)
        {
            if (state.tournaments[tIdx].matches[ii].round == currentRound &&
                state.tournaments[tIdx].matches[ii].bracketSide == BRACKET_WINNERS)
            {
                state.scratchIds[wCount]        = state.tournaments[tIdx].matches[ii].winner;
                state.scratchIds[32 + lCount]   = state.tournaments[tIdx].matches[ii].loser;
                wCount = wCount + 1;
                lCount = lCount + 1;

                // Mark loser as entering losers bracket — inline findPlayerIndex
                for (uint32 jj = 0; jj < state.tournaments[tIdx].registeredCount; jj = jj + 1)
                {
                    if (state.tournaments[tIdx].players[jj].wallet == state.tournaments[tIdx].matches[ii].loser)
                    {
                        state.tournaments[tIdx].players[jj].inLosersBracket = 1;
                    }
                }
            }
        }

        // Grand final: 1 winners-bracket survivor, check for losers-bracket finalist
        if (wCount == 1 && lCount == 0)
        {
            // Find the most recent settled losers-bracket match winner
            id   lbFinalist = NULL_ID;
            bit  lbFound    = 0;
            uint32 mc       = state.tournaments[tIdx].matchCount;

            for (uint32 ii = 0; ii < mc && lbFound == 0; ii = ii + 1)
            {
                // Scan in reverse via index arithmetic
                uint32 revIdx = mc - 1 - ii;
                if (state.tournaments[tIdx].matches[revIdx].bracketSide == BRACKET_LOSERS &&
                    state.tournaments[tIdx].matches[revIdx].settled)
                {
                    lbFinalist = state.tournaments[tIdx].matches[revIdx].winner;
                    lbFound    = 1;
                }
            }

            if (lbFound && lbFinalist != NULL_ID)
            {
                // Grand final match
                state.tournaments[tIdx].matches[mIdx].matchIndex  = mIdx;
                state.tournaments[tIdx].matches[mIdx].playerA     = state.scratchIds[0];
                state.tournaments[tIdx].matches[mIdx].playerB     = lbFinalist;
                state.tournaments[tIdx].matches[mIdx].result      = TMATCH_PENDING;
                state.tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
                state.tournaments[tIdx].matches[mIdx].round       = nextRound;
                state.tournaments[tIdx].matches[mIdx].settled     = 0;

                state.tournaments[tIdx].matchCount   = mIdx + 1;
                state.tournaments[tIdx].currentRound = nextRound;
                return;
            }

            // No losers-bracket finalist — tournament complete
            state.tournaments[tIdx].first = state.scratchIds[0];

            if (state.tournaments[tIdx].prizesDistributed == 0)
            {
                state.tournaments[tIdx].tstate = TSTATE_FINALIZING;

                uint64 total  = state.tournaments[tIdx].prizePool;
                uint64 first  = div(total * QZN_TOURNAMENT_PRIZE_FIRST,  (uint64)100);
                uint64 second = div(total * QZN_TOURNAMENT_PRIZE_SECOND, (uint64)100);
                uint64 third  = total - first - second;

                if (state.tournaments[tIdx].first  != NULL_ID) qpi.transfer(state.tournaments[tIdx].first,  first);
                if (state.tournaments[tIdx].second != NULL_ID) qpi.transfer(state.tournaments[tIdx].second, second);
                if (state.tournaments[tIdx].third  != NULL_ID) qpi.transfer(state.tournaments[tIdx].third,  third);

                state.tournaments[tIdx].prizesDistributed = 1;
                state.totalPrizesDistributed = state.totalPrizesDistributed + total;
                state.tournaments[tIdx].tstate = TSTATE_COMPLETE;
            }

            output.tournamentComplete = 1;
            return;
        }

        // Pair winners-bracket survivors (scratchIds[0..wCount-1])
        for (uint32 i = 0; i + 1 < wCount; i = i + 2)
        {
            state.tournaments[tIdx].matches[mIdx].matchIndex  = mIdx;
            state.tournaments[tIdx].matches[mIdx].playerA     = state.scratchIds[i];
            state.tournaments[tIdx].matches[mIdx].playerB     = state.scratchIds[i + 1];
            state.tournaments[tIdx].matches[mIdx].result      = TMATCH_PENDING;
            state.tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
            state.tournaments[tIdx].matches[mIdx].round       = nextRound;
            state.tournaments[tIdx].matches[mIdx].settled     = 0;
            mIdx = mIdx + 1;
        }

        // Pair losers-bracket players (scratchIds[32..32+lCount-1])
        for (uint32 i = 0; i + 1 < lCount; i = i + 2)
        {
            state.tournaments[tIdx].matches[mIdx].matchIndex  = mIdx;
            state.tournaments[tIdx].matches[mIdx].playerA     = state.scratchIds[32 + i];
            state.tournaments[tIdx].matches[mIdx].playerB     = state.scratchIds[32 + i + 1];
            state.tournaments[tIdx].matches[mIdx].result      = TMATCH_PENDING;
            state.tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_LOSERS;
            state.tournaments[tIdx].matches[mIdx].round       = nextRound;
            state.tournaments[tIdx].matches[mIdx].settled     = 0;
            mIdx = mIdx + 1;
        }

        state.tournaments[tIdx].matchCount   = mIdx;
        state.tournaments[tIdx].currentRound = nextRound;
    }

    // ── Round Robin advancement ───────────────────────────────
    else // FORMAT_ROUND_ROBIN
    {
        if (state.tournaments[tIdx].currentRound >= state.tournaments[tIdx].totalRounds)
        {
            // All rounds complete — sort by points (insertion sort, deterministic)
            uint32 rc = state.tournaments[tIdx].registeredCount;

            for (uint32 i = 1; i < rc; i = i + 1)
            {
                PlayerRecord keyRecord = state.tournaments[tIdx].players[i];
                sint32 j = (sint32)i - 1;

                while (j >= 0 && state.tournaments[tIdx].players[j].points < keyRecord.points)
                {
                    state.tournaments[tIdx].players[j + 1] = state.tournaments[tIdx].players[j];
                    j = j - 1;
                }

                state.tournaments[tIdx].players[j + 1] = keyRecord;
            }

            state.tournaments[tIdx].first  = state.tournaments[tIdx].players[0].wallet;
            state.tournaments[tIdx].second = state.tournaments[tIdx].players[1].wallet;
            state.tournaments[tIdx].third  = state.tournaments[tIdx].players[2].wallet;

            if (state.tournaments[tIdx].prizesDistributed == 0)
            {
                state.tournaments[tIdx].tstate = TSTATE_FINALIZING;

                uint64 total  = state.tournaments[tIdx].prizePool;
                uint64 first  = div(total * QZN_TOURNAMENT_PRIZE_FIRST,  (uint64)100);
                uint64 second = div(total * QZN_TOURNAMENT_PRIZE_SECOND, (uint64)100);
                uint64 third  = total - first - second;

                if (state.tournaments[tIdx].first  != NULL_ID) qpi.transfer(state.tournaments[tIdx].first,  first);
                if (state.tournaments[tIdx].second != NULL_ID) qpi.transfer(state.tournaments[tIdx].second, second);
                if (state.tournaments[tIdx].third  != NULL_ID) qpi.transfer(state.tournaments[tIdx].third,  third);

                state.tournaments[tIdx].prizesDistributed = 1;
                state.totalPrizesDistributed = state.totalPrizesDistributed + total;
                state.tournaments[tIdx].tstate = TSTATE_COMPLETE;
            }

            output.tournamentComplete = 1;
        }
        else
        {
            state.tournaments[tIdx].currentRound = state.tournaments[tIdx].currentRound + 1;
        }
    }
}
_

PUBLIC_PROCEDURE(CancelTournament)
/*
 * Admin only. Cancels a tournament and refunds all registered players.
 * Cannot cancel a completed or already-cancelled tournament.
 */
{
    if (input.tournamentId >= state.tournamentCount)
    {
        output.success = 0;
        return;
    }

    if (qpi.invocator() != state.protocolAdmin)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx = input.tournamentId;

    if (state.tournaments[tIdx].tstate == TSTATE_COMPLETE ||
        state.tournaments[tIdx].tstate == TSTATE_CANCELLED)
    {
        output.success = 0;
        return;
    }

    // Refund all registered players
    for (uint32 i = 0; i < state.tournaments[tIdx].registeredCount; i = i + 1)
    {
        if (state.tournaments[tIdx].players[i].registered)
        {
            qpi.transfer(state.tournaments[tIdx].players[i].wallet, state.tournaments[tIdx].entryFee);
        }
    }

    state.tournaments[tIdx].tstate   = TSTATE_CANCELLED;
    state.tournaments[tIdx].prizePool = 0;
    output.success = 1;
}
_

// ============================================================
//  PUBLIC FUNCTIONS (read-only)
// ============================================================

PUBLIC_FUNCTION(GetTournament)
/*
 * Returns core tournament metadata by ID.
 * Returns zeroed output (tournamentId == 0) if index is out of range.
 */
{
    if (input.tournamentId >= state.tournamentCount)
    {
        output.tournamentId = 0;
        return;
    }

    uint32 tIdx = input.tournamentId;

    output.tournamentId      = state.tournaments[tIdx].tournamentId;
    output.format            = state.tournaments[tIdx].format;
    output.tstate            = state.tournaments[tIdx].tstate;
    output.registeredCount   = state.tournaments[tIdx].registeredCount;
    output.maxPlayers        = state.tournaments[tIdx].maxPlayers;
    output.entryFee          = state.tournaments[tIdx].entryFee;
    output.prizePool         = state.tournaments[tIdx].prizePool;
    output.currentRound      = state.tournaments[tIdx].currentRound;
    output.matchCount        = state.tournaments[tIdx].matchCount;
    output.settledMatchCount = state.tournaments[tIdx].settledMatchCount;
    output.first             = state.tournaments[tIdx].first;
    output.second            = state.tournaments[tIdx].second;
    output.third             = state.tournaments[tIdx].third;
    output.prizesDistributed = state.tournaments[tIdx].prizesDistributed;
}
_

PUBLIC_FUNCTION(GetMatch)
/*
 * Returns a specific match record by tournament + match index.
 * Sets output.found = 0 if either index is out of range.
 */
{
    if (input.tournamentId >= state.tournamentCount ||
        input.matchIndex >= state.tournaments[input.tournamentId].matchCount)
    {
        output.found = 0;
        return;
    }

    output.match = state.tournaments[input.tournamentId].matches[input.matchIndex];
    output.found = 1;
}
_

PUBLIC_FUNCTION(GetPlayerRecord)
/*
 * Returns a player's record within a specific tournament.
 * Searches by wallet address — inline findPlayerIndex.
 * Sets output.found = 0 if player or tournament not found.
 */
{
    if (input.tournamentId >= state.tournamentCount)
    {
        output.found = 0;
        return;
    }

    uint32 tIdx     = input.tournamentId;
    uint32 regCount = state.tournaments[tIdx].registeredCount;

    for (uint32 ii = 0; ii < regCount; ii = ii + 1)
    {
        if (state.tournaments[tIdx].players[ii].wallet == input.wallet)
        {
            output.record = state.tournaments[tIdx].players[ii];
            output.found  = 1;
            return;
        }
    }

    output.found = 0;
}
_

// ============================================================
//  REGISTRATION
// ============================================================

REGISTER_USER_FUNCTIONS_AND_PROCEDURES
{
    REGISTER_USER_PROCEDURE(InitializeTournamentEngine, 1);
    REGISTER_USER_PROCEDURE(CreateTournament,           2);
    REGISTER_USER_PROCEDURE(RegisterPlayer,             3);
    REGISTER_USER_PROCEDURE(StartTournament,            4);
    REGISTER_USER_PROCEDURE(SubmitMatchResult,          5);
    REGISTER_USER_PROCEDURE(CancelTournament,           6);
    REGISTER_USER_FUNCTION(GetTournament,               7);
    REGISTER_USER_FUNCTION(GetMatch,                    8);
    REGISTER_USER_FUNCTION(GetPlayerRecord,             9);
}
_

// ============================================================
//  SYSTEM HOOKS
// ============================================================

BEGIN_EPOCH
/*
 * Fires at the start of every epoch (~weekly).
 * Reserved for future: epoch-based tournament scheduling,
 * automated cancellation of stale tournaments.
 */
{
}
_

END_TICK
/*
 * Reserved for future: tick-based match expiry enforcement.
 */
{
}
_