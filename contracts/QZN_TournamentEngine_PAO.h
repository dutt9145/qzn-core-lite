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
//    - Added state.get().scratchIds[] buffer for bracket generation
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

struct TournamentTournamentPlayerRecord
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
    TournamentPlayerRecord    players[QZN_TOURNAMENT_MAX_PLAYERS];
    TournamentMatch matches[QZN_TOURNAMENT_MAX_MATCHES];
};

// ============================================================
//  CONTRACT STATE
// ============================================================

struct QZNTOUR2
{
};

struct QZNTOUR : public ContractBase
{
    struct StateData
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

public:


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
    TournamentPlayerRecord record;
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
    if (state.get().initialized)
    {
        output.success = 0;
        return;
    }

    if (input.adminOverride == NULL_ID)
    {
        state.mut().protocolAdmin = qpi.invocator();
    }
    else
    {
        state.mut().protocolAdmin = input.adminOverride;
    }

    state.mut().tournamentCount = 0;
    state.mut().totalPrizesDistributed = 0;
    state.mut().initialized = 1;
    output.success               = 1;
}

PUBLIC_PROCEDURE(CreateTournament)
/*
 * Admin only. Creates a new tournament in REGISTRATION state.
 * maxPlayers must be a power of 2 between 4 and 64.
 * entryFee must meet minimum threshold.
 */
{
    if (qpi.invocator() != state.get().protocolAdmin)
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

    if (state.get().tournamentCount >= QZN_TOURNAMENT_MAX_TOURNAMENTS)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx = state.get().tournamentCount;

    state.mut().tournaments[tIdx].tournamentId = tIdx;
    state.mut().tournaments[tIdx].format = input.format;
    state.mut().tournaments[tIdx].tstate = TSTATE_REGISTRATION;
    state.mut().tournaments[tIdx].registeredCount = 0;
    state.mut().tournaments[tIdx].maxPlayers = input.maxPlayers;
    state.mut().tournaments[tIdx].entryFee = input.entryFee;
    state.mut().tournaments[tIdx].prizePool = 0;
    state.mut().tournaments[tIdx].currentRound = 0;
    state.mut().tournaments[tIdx].totalRounds = 0;
    state.mut().tournaments[tIdx].matchCount = 0;
    state.mut().tournaments[tIdx].settledMatchCount = 0;
    state.mut().tournaments[tIdx].admin = qpi.invocator();
    state.mut().tournaments[tIdx].cabinetPAO = input.cabinetPAO;
    state.mut().tournaments[tIdx].prizesDistributed = 0;
    state.mut().tournaments[tIdx].first = NULL_ID;
    state.mut().tournaments[tIdx].second = NULL_ID;
    state.mut().tournaments[tIdx].third = NULL_ID;

    state.mut().tournamentCount = state.get().tournamentCount + 1;

    output.tournamentId = tIdx;
    output.success      = 1;
}

PUBLIC_PROCEDURE(RegisterPlayer)
/*
 * Any player. Registers for a tournament in REGISTRATION state.
 * Invocation reward must cover entryFee; overpayment is refunded.
 * Duplicate registration is rejected.
 */
{
    if (input.tournamentId >= state.get().tournamentCount)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx  = input.tournamentId;
    id     caller = qpi.invocator();

    if (state.get().tournaments[tIdx].tstate != TSTATE_REGISTRATION)
    {
        output.success = 0;
        return;
    }

    if (state.get().tournaments[tIdx].registeredCount >= state.get().tournaments[tIdx].maxPlayers)
    {
        output.success = 0;
        return;
    }

    // Check not already registered — inline findPlayerIndex
    uint32 regCount = state.get().tournaments[tIdx].registeredCount;
    for (uint32 ii = 0; ii < regCount; ii = ii + 1)
    {
        if (state.get().tournaments[tIdx].players[ii].wallet == caller)
        {
            output.success = 0;
            return;
        }
    }

    // Collect entry fee — refund if insufficient
    if (qpi.invocationReward() < state.get().tournaments[tIdx].entryFee)
    {
        qpi.transfer(caller, qpi.invocationReward());
        output.success = 0;
        return;
    }

    // Refund overpayment
    if (qpi.invocationReward() > state.get().tournaments[tIdx].entryFee)
    {
        qpi.transfer(caller, qpi.invocationReward() - state.get().tournaments[tIdx].entryFee);
    }

    uint8 slot = state.get().tournaments[tIdx].registeredCount;

    state.mut().tournaments[tIdx].players[slot].wallet = caller;
    state.mut().tournaments[tIdx].players[slot].wins = 0;
    state.mut().tournaments[tIdx].players[slot].losses = 0;
    state.mut().tournaments[tIdx].players[slot].draws = 0;
    state.mut().tournaments[tIdx].players[slot].points = 0;
    state.mut().tournaments[tIdx].players[slot].eliminated = 0;
    state.mut().tournaments[tIdx].players[slot].inLosersBracket = 0;
    state.mut().tournaments[tIdx].players[slot].registered = 1;

    state.mut().tournaments[tIdx].prizePool = state.get().tournaments[tIdx].prizePool + state.get().tournaments[tIdx].entryFee;
    state.mut().tournaments[tIdx].registeredCount = state.get().tournaments[tIdx].registeredCount + 1;

    output.slotIndex = slot;
    output.success   = 1;
}

PUBLIC_PROCEDURE(StartTournament)
/*
 * Admin only. Generates bracket / schedule and moves to IN_PROGRESS.
 * Single Elim:    pairs players 0v1, 2v3, ... round 1 matches only.
 *                 Subsequent rounds generated dynamically in SubmitMatchResult.
 * Double Elim:    same winners-bracket R1 as single elim.
 *                 Losers bracket generated dynamically.
 * Round Robin:    full schedule generated up front using rotation algorithm.
 *                 Uses state.get().scratchIds as rotation buffer.
 */
{
    if (input.tournamentId >= state.get().tournamentCount)
    {
        output.success = 0;
        return;
    }

    if (qpi.invocator() != state.get().protocolAdmin)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx = input.tournamentId;

    if (state.get().tournaments[tIdx].tstate != TSTATE_REGISTRATION)
    {
        output.success = 0;
        return;
    }

    if (state.get().tournaments[tIdx].registeredCount < QZN_TOURNAMENT_MIN_PLAYERS)
    {
        output.success = 0;
        return;
    }

    uint32 n    = state.get().tournaments[tIdx].registeredCount;
    uint32 mIdx = 0;

    if (state.get().tournaments[tIdx].format == FORMAT_SINGLE_ELIMINATION)
    {
        // Pair players: 0v1, 2v3, 4v5, ... for round 1
        for (uint32 i = 0; i < n; i = i + 2)
        {
            state.mut().tournaments[tIdx].matches[mIdx].matchIndex = mIdx;
            state.mut().tournaments[tIdx].matches[mIdx].playerA = state.get().tournaments[tIdx].players[i].wallet;
            state.mut().tournaments[tIdx].matches[mIdx].playerB = state.get().tournaments[tIdx].players[i + 1].wallet;
            state.mut().tournaments[tIdx].matches[mIdx].result = TMATCH_PENDING;
            state.mut().tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
            state.mut().tournaments[tIdx].matches[mIdx].round = 1;
            state.mut().tournaments[tIdx].matches[mIdx].settled = 0;
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

        state.mut().tournaments[tIdx].matchCount = mIdx;
        state.mut().tournaments[tIdx].totalRounds = roundsCalc;
        state.mut().tournaments[tIdx].currentRound = 1;
    }
    else if (state.get().tournaments[tIdx].format == FORMAT_DOUBLE_ELIMINATION)
    {
        // Winners bracket round 1 only — losers bracket generated dynamically
        for (uint32 i = 0; i < n; i = i + 2)
        {
            state.mut().tournaments[tIdx].matches[mIdx].matchIndex = mIdx;
            state.mut().tournaments[tIdx].matches[mIdx].playerA = state.get().tournaments[tIdx].players[i].wallet;
            state.mut().tournaments[tIdx].matches[mIdx].playerB = state.get().tournaments[tIdx].players[i + 1].wallet;
            state.mut().tournaments[tIdx].matches[mIdx].result = TMATCH_PENDING;
            state.mut().tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
            state.mut().tournaments[tIdx].matches[mIdx].round = 1;
            state.mut().tournaments[tIdx].matches[mIdx].settled = 0;
            mIdx = mIdx + 1;
        }

        uint32 roundsCalc = 0;
        uint32 nCalc      = n;
        while (nCalc > 1)
        {
            nCalc      = nCalc >> 1;
            roundsCalc = roundsCalc + 1;
        }

        state.mut().tournaments[tIdx].matchCount = mIdx;
        state.mut().tournaments[tIdx].totalRounds = roundsCalc * 2 + 1; // approx for double elim
        state.mut().tournaments[tIdx].currentRound = 1;
    }
    else // FORMAT_ROUND_ROBIN
    {
        // Classic round-robin rotation algorithm.
        // Fix player[0], rotate positions 1..n-1.
        // Uses state.get().scratchIds as the rotation buffer.

        for (uint32 i = 0; i < n; i = i + 1)
        {
            state.mut().scratchIds[i] = state.get().tournaments[tIdx].players[i].wallet;
        }

        uint32 round = 1;

        for (uint32 r = 0; r < n - 1; r = r + 1)
        {
            // Generate matches for this round
            for (uint32 i = 0; i < n / 2; i = i + 1)
            {
                uint32 j = n - 1 - i;

                state.mut().tournaments[tIdx].matches[mIdx].matchIndex = mIdx;
                state.mut().tournaments[tIdx].matches[mIdx].playerA = state.get().scratchIds[i];
                state.mut().tournaments[tIdx].matches[mIdx].playerB = state.get().scratchIds[j];
                state.mut().tournaments[tIdx].matches[mIdx].result = TMATCH_PENDING;
                state.mut().tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
                state.mut().tournaments[tIdx].matches[mIdx].round = round;
                state.mut().tournaments[tIdx].matches[mIdx].settled = 0;
                mIdx = mIdx + 1;
            }

            round = round + 1;

            // Rotate: fix index 0, rotate indices 1..n-1
            id lastId = state.get().scratchIds[n - 1];
            for (uint32 k = n - 1; k > 1; k = k - 1)
            {
                state.mut().scratchIds[k] = state.get().scratchIds[k - 1];
            }
            state.mut().scratchIds[1] = lastId;
        }

        state.mut().tournaments[tIdx].matchCount = mIdx;
        state.mut().tournaments[tIdx].totalRounds = n - 1;
        state.mut().tournaments[tIdx].currentRound = 1;
    }

    state.mut().tournaments[tIdx].tstate = TSTATE_IN_PROGRESS;

    output.matchCount = state.get().tournaments[tIdx].matchCount;
    output.success    = 1;
}

PUBLIC_PROCEDURE(SubmitMatchResult)
/*
 * Admin only. Records match result, updates player records,
 * and if the current round is fully settled, advances the bracket
 * or finalizes the tournament with prize distribution.
 *
 * Single Elim:
 *   Winners collected into state.get().scratchIds[0..wCount-1].
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
    if (input.tournamentId >= state.get().tournamentCount)
    {
        output.success = 0;
        return;
    }

    if (qpi.invocator() != state.get().protocolAdmin)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx = input.tournamentId;

    if (state.get().tournaments[tIdx].tstate != TSTATE_IN_PROGRESS)
    {
        output.success = 0;
        return;
    }

    if (input.matchIndex >= state.get().tournaments[tIdx].matchCount)
    {
        output.success = 0;
        return;
    }

    if (state.get().tournaments[tIdx].matches[input.matchIndex].settled)
    {
        output.success = 0;
        return;
    }

    // Validate winner is one of the two match players
    id mPlayerA = state.get().tournaments[tIdx].matches[input.matchIndex].playerA;
    id mPlayerB = state.get().tournaments[tIdx].matches[input.matchIndex].playerB;

    if (input.winner != mPlayerA && input.winner != mPlayerB)
    {
        output.success = 0;
        return;
    }

    // Record match result
    state.mut().tournaments[tIdx].matches[input.matchIndex].winner = input.winner;
    state.mut().tournaments[tIdx].matches[input.matchIndex].resultHash = input.resultHash;
    state.mut().tournaments[tIdx].matches[input.matchIndex].settled = 1;

    if (input.winner == mPlayerA)
    {
        state.mut().tournaments[tIdx].matches[input.matchIndex].loser = mPlayerB;
        state.mut().tournaments[tIdx].matches[input.matchIndex].result = TMATCH_PLAYER_A;
    }
    else
    {
        state.mut().tournaments[tIdx].matches[input.matchIndex].loser = mPlayerA;
        state.mut().tournaments[tIdx].matches[input.matchIndex].result = TMATCH_PLAYER_B;
    }

    state.mut().tournaments[tIdx].settledMatchCount = state.get().tournaments[tIdx].settledMatchCount + 1;

    id matchLoser  = state.get().tournaments[tIdx].matches[input.matchIndex].loser;
    uint32 regCount = state.get().tournaments[tIdx].registeredCount;

    // Update winner player record — inline findPlayerIndex
    for (uint32 ii = 0; ii < regCount; ii = ii + 1)
    {
        if (state.get().tournaments[tIdx].players[ii].wallet == input.winner)
        {
            state.mut().tournaments[tIdx].players[ii].wins = state.get().tournaments[tIdx].players[ii].wins + 1;
            state.mut().tournaments[tIdx].players[ii].points = state.get().tournaments[tIdx].players[ii].points + 3;
        }
    }

    // Update loser player record — inline findPlayerIndex
    for (uint32 ii = 0; ii < regCount; ii = ii + 1)
    {
        if (state.get().tournaments[tIdx].players[ii].wallet == matchLoser)
        {
            state.mut().tournaments[tIdx].players[ii].losses = state.get().tournaments[tIdx].players[ii].losses + 1;

            // Eliminate only if single elim, or already in losers bracket (double elim)
            if (state.get().tournaments[tIdx].format != FORMAT_DOUBLE_ELIMINATION ||
                state.get().tournaments[tIdx].players[ii].inLosersBracket)
            {
                state.mut().tournaments[tIdx].players[ii].eliminated = 1;
            }
        }
    }

    output.advancedRound      = 0;
    output.tournamentComplete = 0;
    output.success            = 1;

    // Check if all matches in the current round are settled — inline _currentRoundComplete
    uint32 currentRound = state.get().tournaments[tIdx].currentRound;
    bit    roundComplete = 1;

    for (uint32 ii = 0; ii < state.get().tournaments[tIdx].matchCount; ii = ii + 1)
    {
        if (state.get().tournaments[tIdx].matches[ii].round == currentRound &&
            state.get().tournaments[tIdx].matches[ii].settled == 0)
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
    if (state.get().tournaments[tIdx].format == FORMAT_SINGLE_ELIMINATION)
    {
        // Collect winners from current round into scratchIds
        uint32 winnerCount = 0;

        for (uint32 ii = 0; ii < state.get().tournaments[tIdx].matchCount; ii = ii + 1)
        {
            if (state.get().tournaments[tIdx].matches[ii].round == currentRound &&
                state.get().tournaments[tIdx].matches[ii].bracketSide == BRACKET_WINNERS)
            {
                state.mut().scratchIds[winnerCount] = state.get().tournaments[tIdx].matches[ii].winner;
                winnerCount = winnerCount + 1;
            }
        }

        if (winnerCount == 1)
        {
            // Finals complete — determine placements
            state.mut().tournaments[tIdx].first = state.get().scratchIds[0];

            // Second = loser of the grand final match
            for (uint32 ii = 0; ii < state.get().tournaments[tIdx].matchCount; ii = ii + 1)
            {
                if (state.get().tournaments[tIdx].matches[ii].round == currentRound)
                {
                    state.mut().tournaments[tIdx].second = state.get().tournaments[tIdx].matches[ii].loser;
                }
            }

            // Third = loser of the semifinal who lost to the champion's path
            id finalist = state.get().tournaments[tIdx].second;
            for (uint32 ii = 0; ii < state.get().tournaments[tIdx].matchCount; ii = ii + 1)
            {
                if (state.get().tournaments[tIdx].matches[ii].round == currentRound - 1 &&
                    state.get().tournaments[tIdx].matches[ii].loser != finalist &&
                    state.get().tournaments[tIdx].third == NULL_ID)
                {
                    state.mut().tournaments[tIdx].third = state.get().tournaments[tIdx].matches[ii].loser;
                }
            }

            // Distribute prizes: 60 / 30 / 10 via div()
            if (state.get().tournaments[tIdx].prizesDistributed == 0)
            {
                state.mut().tournaments[tIdx].tstate = TSTATE_FINALIZING;

                uint64 total  = state.get().tournaments[tIdx].prizePool;
                uint64 first  = div(total * QZN_TOURNAMENT_PRIZE_FIRST,  (uint64).quot100);
                uint64 second = div(total * QZN_TOURNAMENT_PRIZE_SECOND, (uint64).quot100);
                uint64 third  = total - first - second; // remainder prevents rounding dust

                if (state.get().tournaments[tIdx].first  != NULL_ID) qpi.transfer(state.get().tournaments[tIdx].first,  first);
                if (state.get().tournaments[tIdx].second != NULL_ID) qpi.transfer(state.get().tournaments[tIdx].second, second);
                if (state.get().tournaments[tIdx].third  != NULL_ID) qpi.transfer(state.get().tournaments[tIdx].third,  third);

                state.mut().tournaments[tIdx].prizesDistributed = 1;
                state.mut().totalPrizesDistributed = state.get().totalPrizesDistributed + total;
                state.mut().tournaments[tIdx].tstate = TSTATE_COMPLETE;
            }

            output.tournamentComplete = 1;
            return;
        }

        // Not final round yet — generate next-round matches from winners
        uint32 nextRound = currentRound + 1;
        uint32 mIdx      = state.get().tournaments[tIdx].matchCount;

        for (uint32 i = 0; i + 1 < winnerCount; i = i + 2)
        {
            state.mut().tournaments[tIdx].matches[mIdx].matchIndex = mIdx;
            state.mut().tournaments[tIdx].matches[mIdx].playerA = state.get().scratchIds[i];
            state.mut().tournaments[tIdx].matches[mIdx].playerB = state.get().scratchIds[i + 1];
            state.mut().tournaments[tIdx].matches[mIdx].result = TMATCH_PENDING;
            state.mut().tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
            state.mut().tournaments[tIdx].matches[mIdx].round = nextRound;
            state.mut().tournaments[tIdx].matches[mIdx].settled = 0;
            mIdx = mIdx + 1;
        }

        state.mut().tournaments[tIdx].matchCount = mIdx;
        state.mut().tournaments[tIdx].currentRound = nextRound;
    }

    // ── Double Elimination advancement ───────────────────────
    else if (state.get().tournaments[tIdx].format == FORMAT_DOUBLE_ELIMINATION)
    {
        uint32 nextRound = currentRound + 1;
        uint32 mIdx      = state.get().tournaments[tIdx].matchCount;
        uint32 wCount    = 0;
        uint32 lCount    = 0;

        // Collect winners/losers from current round
        // Winners go into scratchIds[0..wCount-1]
        // Losers go into scratchIds[32..32+lCount-1]
        for (uint32 ii = 0; ii < state.get().tournaments[tIdx].matchCount; ii = ii + 1)
        {
            if (state.get().tournaments[tIdx].matches[ii].round == currentRound &&
                state.get().tournaments[tIdx].matches[ii].bracketSide == BRACKET_WINNERS)
            {
                state.mut().scratchIds[wCount] = state.get().tournaments[tIdx].matches[ii].winner;
                state.mut().scratchIds[32 + lCount] = state.get().tournaments[tIdx].matches[ii].loser;
                wCount = wCount + 1;
                lCount = lCount + 1;

                // Mark loser as entering losers bracket — inline findPlayerIndex
                for (uint32 jj = 0; jj < state.get().tournaments[tIdx].registeredCount; jj = jj + 1)
                {
                    if (state.get().tournaments[tIdx].players[jj].wallet == state.get().tournaments[tIdx].matches[ii].loser)
                    {
                        state.mut().tournaments[tIdx].players[jj].inLosersBracket = 1;
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
            uint32 mc       = state.get().tournaments[tIdx].matchCount;

            for (uint32 ii = 0; ii < mc && lbFound == 0; ii = ii + 1)
            {
                // Scan in reverse via index arithmetic
                uint32 revIdx = mc - 1 - ii;
                if (state.get().tournaments[tIdx].matches[revIdx].bracketSide == BRACKET_LOSERS &&
                    state.get().tournaments[tIdx].matches[revIdx].settled)
                {
                    lbFinalist = state.get().tournaments[tIdx].matches[revIdx].winner;
                    lbFound    = 1;
                }
            }

            if (lbFound && lbFinalist != NULL_ID)
            {
                // Grand final match
                state.mut().tournaments[tIdx].matches[mIdx].matchIndex = mIdx;
                state.mut().tournaments[tIdx].matches[mIdx].playerA = state.get().scratchIds[0];
                state.mut().tournaments[tIdx].matches[mIdx].playerB = lbFinalist;
                state.mut().tournaments[tIdx].matches[mIdx].result = TMATCH_PENDING;
                state.mut().tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
                state.mut().tournaments[tIdx].matches[mIdx].round = nextRound;
                state.mut().tournaments[tIdx].matches[mIdx].settled = 0;

                state.mut().tournaments[tIdx].matchCount = mIdx + 1;
                state.mut().tournaments[tIdx].currentRound = nextRound;
                return;
            }

            // No losers-bracket finalist — tournament complete
            state.mut().tournaments[tIdx].first = state.get().scratchIds[0];

            if (state.get().tournaments[tIdx].prizesDistributed == 0)
            {
                state.mut().tournaments[tIdx].tstate = TSTATE_FINALIZING;

                uint64 total  = state.get().tournaments[tIdx].prizePool;
                uint64 first  = div(total * QZN_TOURNAMENT_PRIZE_FIRST,  (uint64).quot100);
                uint64 second = div(total * QZN_TOURNAMENT_PRIZE_SECOND, (uint64).quot100);
                uint64 third  = total - first - second;

                if (state.get().tournaments[tIdx].first  != NULL_ID) qpi.transfer(state.get().tournaments[tIdx].first,  first);
                if (state.get().tournaments[tIdx].second != NULL_ID) qpi.transfer(state.get().tournaments[tIdx].second, second);
                if (state.get().tournaments[tIdx].third  != NULL_ID) qpi.transfer(state.get().tournaments[tIdx].third,  third);

                state.mut().tournaments[tIdx].prizesDistributed = 1;
                state.mut().totalPrizesDistributed = state.get().totalPrizesDistributed + total;
                state.mut().tournaments[tIdx].tstate = TSTATE_COMPLETE;
            }

            output.tournamentComplete = 1;
            return;
        }

        // Pair winners-bracket survivors (scratchIds[0..wCount-1])
        for (uint32 i = 0; i + 1 < wCount; i = i + 2)
        {
            state.mut().tournaments[tIdx].matches[mIdx].matchIndex = mIdx;
            state.mut().tournaments[tIdx].matches[mIdx].playerA = state.get().scratchIds[i];
            state.mut().tournaments[tIdx].matches[mIdx].playerB = state.get().scratchIds[i + 1];
            state.mut().tournaments[tIdx].matches[mIdx].result = TMATCH_PENDING;
            state.mut().tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_WINNERS;
            state.mut().tournaments[tIdx].matches[mIdx].round = nextRound;
            state.mut().tournaments[tIdx].matches[mIdx].settled = 0;
            mIdx = mIdx + 1;
        }

        // Pair losers-bracket players (scratchIds[32..32+lCount-1])
        for (uint32 i = 0; i + 1 < lCount; i = i + 2)
        {
            state.mut().tournaments[tIdx].matches[mIdx].matchIndex = mIdx;
            state.mut().tournaments[tIdx].matches[mIdx].playerA = state.get().scratchIds[32 + i];
            state.mut().tournaments[tIdx].matches[mIdx].playerB = state.get().scratchIds[32 + i + 1];
            state.mut().tournaments[tIdx].matches[mIdx].result = TMATCH_PENDING;
            state.mut().tournaments[tIdx].matches[mIdx].bracketSide = BRACKET_LOSERS;
            state.mut().tournaments[tIdx].matches[mIdx].round = nextRound;
            state.mut().tournaments[tIdx].matches[mIdx].settled = 0;
            mIdx = mIdx + 1;
        }

        state.mut().tournaments[tIdx].matchCount = mIdx;
        state.mut().tournaments[tIdx].currentRound = nextRound;
    }

    // ── Round Robin advancement ───────────────────────────────
    else // FORMAT_ROUND_ROBIN
    {
        if (state.get().tournaments[tIdx].currentRound >= state.get().tournaments[tIdx].totalRounds)
        {
            // All rounds complete — sort by points (insertion sort, deterministic)
            uint32 rc = state.get().tournaments[tIdx].registeredCount;

            for (uint32 i = 1; i < rc; i = i + 1)
            {
                TournamentPlayerRecord keyRecord = state.get().tournaments[tIdx].players[i];
                sint32 j = (sint32)i - 1;

                while (j >= 0 && state.get().tournaments[tIdx].players[j].points < keyRecord.points)
                {
                    state.mut().tournaments[tIdx].players[j + 1] = state.get().tournaments[tIdx].players[j];
                    j = j - 1;
                }

                state.mut().tournaments[tIdx].players[j + 1] = keyRecord;
            }

            state.mut().tournaments[tIdx].first = state.get().tournaments[tIdx].players[0].wallet;
            state.mut().tournaments[tIdx].second = state.get().tournaments[tIdx].players[1].wallet;
            state.mut().tournaments[tIdx].third = state.get().tournaments[tIdx].players[2].wallet;

            if (state.get().tournaments[tIdx].prizesDistributed == 0)
            {
                state.mut().tournaments[tIdx].tstate = TSTATE_FINALIZING;

                uint64 total  = state.get().tournaments[tIdx].prizePool;
                uint64 first  = div(total * QZN_TOURNAMENT_PRIZE_FIRST,  (uint64).quot100);
                uint64 second = div(total * QZN_TOURNAMENT_PRIZE_SECOND, (uint64).quot100);
                uint64 third  = total - first - second;

                if (state.get().tournaments[tIdx].first  != NULL_ID) qpi.transfer(state.get().tournaments[tIdx].first,  first);
                if (state.get().tournaments[tIdx].second != NULL_ID) qpi.transfer(state.get().tournaments[tIdx].second, second);
                if (state.get().tournaments[tIdx].third  != NULL_ID) qpi.transfer(state.get().tournaments[tIdx].third,  third);

                state.mut().tournaments[tIdx].prizesDistributed = 1;
                state.mut().totalPrizesDistributed = state.get().totalPrizesDistributed + total;
                state.mut().tournaments[tIdx].tstate = TSTATE_COMPLETE;
            }

            output.tournamentComplete = 1;
        }
        else
        {
            state.mut().tournaments[tIdx].currentRound = state.get().tournaments[tIdx].currentRound + 1;
        }
    }
}

PUBLIC_PROCEDURE(CancelTournament)
/*
 * Admin only. Cancels a tournament and refunds all registered players.
 * Cannot cancel a completed or already-cancelled tournament.
 */
{
    if (input.tournamentId >= state.get().tournamentCount)
    {
        output.success = 0;
        return;
    }

    if (qpi.invocator() != state.get().protocolAdmin)
    {
        output.success = 0;
        return;
    }

    uint32 tIdx = input.tournamentId;

    if (state.get().tournaments[tIdx].tstate == TSTATE_COMPLETE ||
        state.get().tournaments[tIdx].tstate == TSTATE_CANCELLED)
    {
        output.success = 0;
        return;
    }

    // Refund all registered players
    for (uint32 i = 0; i < state.get().tournaments[tIdx].registeredCount; i = i + 1)
    {
        if (state.get().tournaments[tIdx].players[i].registered)
        {
            qpi.transfer(state.get().tournaments[tIdx].players[i].wallet, state.get().tournaments[tIdx].entryFee);
        }
    }

    state.mut().tournaments[tIdx].tstate = TSTATE_CANCELLED;
    state.mut().tournaments[tIdx].prizePool = 0;
    output.success = 1;
}

// ============================================================
//  PUBLIC FUNCTIONS (read-only)
// ============================================================

PUBLIC_FUNCTION(GetTournament)
/*
 * Returns core tournament metadata by ID.
 * Returns zeroed output (tournamentId == 0) if index is out of range.
 */
{
    if (input.tournamentId >= state.get().tournamentCount)
    {
        output.tournamentId = 0;
        return;
    }

    uint32 tIdx = input.tournamentId;

    output.tournamentId      = state.get().tournaments[tIdx].tournamentId;
    output.format            = state.get().tournaments[tIdx].format;
    output.tstate            = state.get().tournaments[tIdx].tstate;
    output.registeredCount   = state.get().tournaments[tIdx].registeredCount;
    output.maxPlayers        = state.get().tournaments[tIdx].maxPlayers;
    output.entryFee          = state.get().tournaments[tIdx].entryFee;
    output.prizePool         = state.get().tournaments[tIdx].prizePool;
    output.currentRound      = state.get().tournaments[tIdx].currentRound;
    output.matchCount        = state.get().tournaments[tIdx].matchCount;
    output.settledMatchCount = state.get().tournaments[tIdx].settledMatchCount;
    output.first             = state.get().tournaments[tIdx].first;
    output.second            = state.get().tournaments[tIdx].second;
    output.third             = state.get().tournaments[tIdx].third;
    output.prizesDistributed = state.get().tournaments[tIdx].prizesDistributed;
}

PUBLIC_FUNCTION(GetMatch)
/*
 * Returns a specific match record by tournament + match index.
 * Sets output.found = 0 if either index is out of range.
 */
{
    if (input.tournamentId >= state.get().tournamentCount ||
        input.matchIndex >= state.get().tournaments[input.tournamentId].matchCount)
    {
        output.found = 0;
        return;
    }

    output.match = state.get().tournaments[input.tournamentId].matches[input.matchIndex];
    output.found = 1;
}

PUBLIC_FUNCTION(GetPlayerRecord)
/*
 * Returns a player's record within a specific tournament.
 * Searches by wallet address — inline findPlayerIndex.
 * Sets output.found = 0 if player or tournament not found.
 */
{
    if (input.tournamentId >= state.get().tournamentCount)
    {
        output.found = 0;
        return;
    }

    uint32 tIdx     = input.tournamentId;
    uint32 regCount = state.get().tournaments[tIdx].registeredCount;

    for (uint32 ii = 0; ii < regCount; ii = ii + 1)
    {
        if (state.get().tournaments[tIdx].players[ii].wallet == input.wallet)
        {
            output.record = state.get().tournaments[tIdx].players[ii];
            output.found  = 1;
            return;
        }
    }

    output.found = 0;
}

// ============================================================
//  REGISTRATION
// ============================================================

REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
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

// ============================================================
//  SYSTEM HOOKS
// ============================================================

BEGIN_EPOCH()
/*
 * Fires at the start of every epoch (~weekly).
 * Reserved for future: epoch-based tournament scheduling,
 * automated cancellation of stale tournaments.
 */
{
}

END_TICK()
/*
 * Reserved for future: tick-based match expiry enforcement.
 */
{
}

};