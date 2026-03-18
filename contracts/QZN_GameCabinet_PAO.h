// ============================================================
//  QZN GAME CABINET PAO
//  Contract: Programmable Arcade Object — Game Cabinet
//  Network:  Qubic (QPI / C++ Smart Contract)
//  Version:  2.0.0
//
//  Supported Games:
//    - snaQe       (single player, score-based)
//    - paQman      (single player, score-based)
//    - TANQ-BATTLE (1v1 and multi-player PvP, stake-based)
//
//  Match Types:
//    - SOLO:  1 player, score threshold triggers reward
//    - DUEL:  2 players stake QU, winner takes routed share
//    - MULTI: 3-4 players stake QU, winner takes routed share
//
//  Validation Model: DUAL SIGNATURE
//    - Game server submits signed match result hash
//    - Player(s) must countersign within confirmation window
//    - Both signatures required before settlement fires
//    - Prevents server manipulation AND client cheating
//
//  Settlement: Cross-contract call → QZN Token_v2 SettleMatch
//    - GameCabinet calls Token_v2 via INVOKE_OTHER_CONTRACT_PROCEDURE
//    - Token_v2 handles all BPS routing deterministically
//    - Cabinet only tracks match state and validates signatures
//    - All routing constants live in Token_v2 (single source of truth)
//
//  DEPLOYMENT ORDER (critical):
//    Token_v2 contract index MUST be lower than GameCabinet index.
//    Cross-contract calls only go to lower-index contracts on Qubic.
//    Fill QZN_TOKEN_CONTRACT_INDEX_LOCAL after IPO assignment.
// ============================================================

using namespace QPI;

// ============================================================
//  CONSTANTS — GAME / MATCH IDENTITY
// ============================================================

constexpr uint8  GAME_SNAQE             = 1;
constexpr uint8  GAME_PAQMAN            = 2;
constexpr uint8  GAME_TANQBATTLE        = 3;

constexpr uint8  MATCH_SOLO             = 1;
constexpr uint8  MATCH_DUEL             = 2;
constexpr uint8  MATCH_MULTI            = 3;

constexpr uint8  STATE_EMPTY            = 0;
constexpr uint8  STATE_PENDING          = 1;
constexpr uint8  STATE_CONFIRMED        = 2;
constexpr uint8  STATE_SETTLED          = 3;
constexpr uint8  STATE_DISPUTED         = 4;
constexpr uint8  STATE_EXPIRED          = 5;

// ============================================================
//  CONSTANTS — LIMITS & THRESHOLDS
// ============================================================

constexpr sint64 MAX_ACTIVE_MATCHES     = 16LL;
constexpr sint64 CONFIRM_WINDOW_TICKS   = 676LL;
constexpr sint64 MIN_STAKE_QU           = 25000LL;
constexpr sint64 MAX_STAKE_QU           = 50000000LL;
constexpr sint64 MAX_PLAYERS_PER_MATCH  = 4LL;

constexpr sint64 SOLO_BASE_REWARD_QU    = 10LL;
constexpr sint64 SOLO_SCORE_MULTIPLIER  = 100LL;
constexpr sint64 SOLO_MAX_REWARD_QU     = 1000LL;

constexpr sint64 SNAQE_MAX_SCORE        = 100000LL;
constexpr sint64 PAQMAN_MAX_SCORE       = 999999LL;
constexpr sint64 TANQ_MAX_SCORE         = 100000LL;

// ============================================================
//  CONSTANTS — CROSS-CONTRACT
// ============================================================
//  Fill these in after IPO — deployment order must place
//  Token_v2 at a LOWER contract index than GameCabinet.

constexpr uint32 QZN_TOKEN_CONTRACT_INDEX_LOCAL = 0;   // TODO: set after IPO
constexpr uint8  QZN_SETTLE_MATCH_PROC_ID  = 2;   // SettleMatch = procedure 2 in Token_v2

// ============================================================
//  DATA STRUCTURES
// ============================================================

struct MatchRecord
{
    uint8   gameId;
    uint8   matchType;
    uint8   state;
    uint8   playerCount;

    id      players_0;
    id      players_1;
    id      players_2;
    id      players_3;

    sint64  stakes_0;
    sint64  stakes_1;
    sint64  stakes_2;
    sint64  stakes_3;
    sint64  totalStake;

    id      winnerAddress;
    sint64  winnerScore;
    sint64  resultHash;

    bit     serverSigned;
    bit     player0Confirmed;
    bit     player1Confirmed;
    bit     player2Confirmed;
    bit     player3Confirmed;

    sint64  openedTick;
    sint64  serverSignTick;
    sint64  settledTick;

    sint64  prizeAwarded;
    sint64  burnedAmount;
};

// ============================================================
//  CONTRACT STATE
// ============================================================

struct QZNCABINET2
{
};

struct QZNCABINET : public ContractBase
{
    struct StateData
    {
    MatchRecord matches_0;
    MatchRecord matches_1;
    MatchRecord matches_2;
    MatchRecord matches_3;
    MatchRecord matches_4;
    MatchRecord matches_5;
    MatchRecord matches_6;
    MatchRecord matches_7;
    MatchRecord matches_8;
    MatchRecord matches_9;
    MatchRecord matches_10;
    MatchRecord matches_11;
    MatchRecord matches_12;
    MatchRecord matches_13;
    MatchRecord matches_14;
    MatchRecord matches_15;

    // ---- Cabinet Stats ----
    sint64 totalMatchesPlayed;
    sint64 totalMatchesSolo;
    sint64 totalMatchesDuel;
    sint64 totalMatchesMulti;
    sint64 totalQUStaked;
    sint64 totalQUBurned;
    sint64 totalPrizesAwarded;
    sint64 totalDisputes;
    sint64 totalExpired;

    // ---- Per-Game Stats ----
    sint64 snaqeMatchCount;
    sint64 paqmanMatchCount;
    sint64 tanqMatchCount;

    // ---- Authority ----
    id     adminAddress;
    id     gameServerAddress;
    bit    initialized;
    bit    cabinetActive;

    // ---- Solo Reward Reserve ----
    sint64 rewardReserveBalance;
    };

public:


// ============================================================
//  INPUT / OUTPUT STRUCTS — SettleMatch cross-contract
// ============================================================

// Must match Token_v2 SettleMatch_input exactly
struct TokenSettleMatch_input
{
    id     winnerAddress;
    sint64 totalStake;
};

// ============================================================
//  INPUT / OUTPUT STRUCTS — Cabinet Procedures
// ============================================================

struct InitializeCabinet_input
{
    id gameServerAddr;
};
struct InitializeCabinet_output
{
    bit success;
};

struct RegisterMatch_input
{
    uint8   gameId;
    uint8   matchType;
    id      player1;
    id      player2;
    id      player3;
    sint64  stakePerPlayer;
};
struct RegisterMatch_output
{
    sint64  matchSlot;
    sint64  totalStake;
    bit     success;
};

struct SubmitResult_input
{
    sint64  matchSlot;
    id      winner;
    sint64  winnerScore;
    sint64  resultHash;
};
struct SubmitResult_output
{
    bit     success;
    sint64  confirmWindowTicks;
};

struct ConfirmResult_input
{
    sint64  matchSlot;
    sint64  resultHash;
};
struct ConfirmResult_output
{
    bit     confirmed;
    bit     settled;
    sint64  prizeAwarded;
    sint64  burnedAmount;
};

struct DisputeResult_input
{
    sint64  matchSlot;
    sint64  playerHash;
};
struct DisputeResult_output
{
    bit     disputeRecorded;
    sint64  refundAmount;
};

struct GetMatch_input
{
    sint64 matchSlot;
};
struct GetMatch_output
{
    uint8   gameId;
    uint8   matchType;
    uint8   state;
    id      winner;
    sint64  totalStake;
    sint64  prizeAwarded;
    sint64  burnedAmount;
    bit     serverSigned;
    bit     fullyConfirmed;
};

struct GetCabinetStats_input {};
struct GetCabinetStats_output
{
    sint64 totalMatchesPlayed;
    sint64 totalQUBurned;
    sint64 totalPrizesAwarded;
    sint64 totalDisputes;
    sint64 snaqeCount;
    sint64 paqmanCount;
    sint64 tanqCount;
};

// ============================================================
//  CONTRACT PROCEDURES
// ============================================================

PUBLIC_PROCEDURE(InitializeCabinet)
{
    if (state.get().initialized)
    {
        output.success = 0;
        return;
    }

    state.mut().adminAddress = qpi.invocator();
    state.mut().gameServerAddress = input.gameServerAddr;
    state.mut().cabinetActive = 1;
    state.mut().initialized = 1;

    state.mut().totalMatchesPlayed = 0;
    state.mut().totalMatchesSolo = 0;
    state.mut().totalMatchesDuel = 0;
    state.mut().totalMatchesMulti = 0;
    state.mut().totalQUStaked = 0;
    state.mut().totalQUBurned = 0;
    state.mut().totalPrizesAwarded = 0;
    state.mut().totalDisputes = 0;
    state.mut().totalExpired = 0;
    state.mut().rewardReserveBalance = 0;
    state.mut().snaqeMatchCount = 0;
    state.mut().paqmanMatchCount = 0;
    state.mut().tanqMatchCount = 0;

    output.success = 1;
}

PUBLIC_PROCEDURE(RegisterMatch)
/*
 * Players call this before the game starts to lock stakes.
 * Assigns a match slot and returns slot index for future calls.
 * Stakes are held in this contract until settlement or refund.
 */
{
    if (!state.get().initialized || !state.get().cabinetActive)
    {
        output.success = 0;
        return;
    }

    if (input.gameId < GAME_SNAQE || input.gameId > GAME_TANQBATTLE)
    {
        output.success = 0;
        return;
    }

    if (input.gameId == GAME_SNAQE || input.gameId == GAME_PAQMAN)
    {
        if (input.matchType == MATCH_MULTI) { output.success = 0; return; }
    }
    if (input.gameId == GAME_TANQBATTLE)
    {
        if (input.matchType == MATCH_SOLO) { output.success = 0; return; }
    }

    if (input.matchType != MATCH_SOLO)
    {
        if (input.stakePerPlayer < MIN_STAKE_QU || input.stakePerPlayer > MAX_STAKE_QU)
        {
            output.success = 0;
            return;
        }
    }

    // ---- FIND FREE SLOT ----
    sint64 slot;
    slot = -1;

    if      (state.get().matches_0.state == STATE_EMPTY) { slot = 0; }
    else if (state.get().matches_1.state == STATE_EMPTY) { slot = 1; }
    else if (state.get().matches_2.state == STATE_EMPTY) { slot = 2; }
    else if (state.get().matches_3.state == STATE_EMPTY) { slot = 3; }
    else if (state.get().matches_4.state == STATE_EMPTY) { slot = 4; }
    else if (state.get().matches_5.state == STATE_EMPTY) { slot = 5; }
    else if (state.get().matches_6.state == STATE_EMPTY) { slot = 6; }
    else if (state.get().matches_7.state == STATE_EMPTY) { slot = 7; }
    else if (state.get().matches_8.state == STATE_EMPTY) { slot = 8; }
    else if (state.get().matches_9.state == STATE_EMPTY) { slot = 9; }
    else if (state.get().matches_10.state == STATE_EMPTY) { slot = 10; }
    else if (state.get().matches_11.state == STATE_EMPTY) { slot = 11; }
    else if (state.get().matches_12.state == STATE_EMPTY) { slot = 12; }
    else if (state.get().matches_13.state == STATE_EMPTY) { slot = 13; }
    else if (state.get().matches_14.state == STATE_EMPTY) { slot = 14; }
    else if (state.get().matches_15.state == STATE_EMPTY) { slot = 15; }

    if (slot < 0) { output.success = 0; return; }

    // ---- COMPUTE PLAYER COUNT & TOTAL STAKE ----
    uint8  playerCount;
    sint64 totalStake;

    playerCount = 1;
    totalStake  = input.stakePerPlayer;

    if (input.matchType == MATCH_DUEL)
    {
        playerCount = 2;
        totalStake  = input.stakePerPlayer * 2;
    }
    if (input.matchType == MATCH_MULTI)
    {
        playerCount = 2;
        totalStake  = input.stakePerPlayer * 2;
        if (input.player2 != NULL_ID) { playerCount = 3; totalStake = input.stakePerPlayer * 3; }
        if (input.player3 != NULL_ID) { playerCount = 4; totalStake = input.stakePerPlayer * 4; }
    }

    // ---- WRITE TO SLOT ----
    if (slot == 0)
    {
        state.mut().matches_0.gameId           = input.gameId;
        state.mut().matches_0.matchType        = input.matchType;
        state.mut().matches_0.state            = STATE_PENDING;
        state.mut().matches_0.playerCount      = playerCount;
        state.mut().matches_0.players_0        = qpi.invocator();
        state.mut().matches_0.players_1        = input.player1;
        state.mut().matches_0.players_2        = input.player2;
        state.mut().matches_0.players_3        = input.player3;
        state.mut().matches_0.stakes_0         = input.stakePerPlayer;
        state.mut().matches_0.stakes_1         = input.stakePerPlayer;
        state.mut().matches_0.stakes_2         = input.stakePerPlayer;
        state.mut().matches_0.stakes_3         = input.stakePerPlayer;
        state.mut().matches_0.totalStake       = totalStake;
        state.mut().matches_0.openedTick       = qpi.tick();
        state.mut().matches_0.serverSigned     = 0;
        state.mut().matches_0.player0Confirmed = 0;
        state.mut().matches_0.player1Confirmed = 0;
        state.mut().matches_0.player2Confirmed = 0;
        state.mut().matches_0.player3Confirmed = 0;
        state.mut().matches_0.prizeAwarded     = 0;
        state.mut().matches_0.burnedAmount     = 0;
    }
    else if (slot == 1)
    {
        state.mut().matches_1.gameId           = input.gameId;
        state.mut().matches_1.matchType        = input.matchType;
        state.mut().matches_1.state            = STATE_PENDING;
        state.mut().matches_1.playerCount      = playerCount;
        state.mut().matches_1.players_0        = qpi.invocator();
        state.mut().matches_1.players_1        = input.player1;
        state.mut().matches_1.players_2        = input.player2;
        state.mut().matches_1.players_3        = input.player3;
        state.mut().matches_1.stakes_0         = input.stakePerPlayer;
        state.mut().matches_1.stakes_1         = input.stakePerPlayer;
        state.mut().matches_1.stakes_2         = input.stakePerPlayer;
        state.mut().matches_1.stakes_3         = input.stakePerPlayer;
        state.mut().matches_1.totalStake       = totalStake;
        state.mut().matches_1.openedTick       = qpi.tick();
        state.mut().matches_1.serverSigned     = 0;
        state.mut().matches_1.player0Confirmed = 0;
        state.mut().matches_1.player1Confirmed = 0;
        state.mut().matches_1.player2Confirmed = 0;
        state.mut().matches_1.player3Confirmed = 0;
        state.mut().matches_1.prizeAwarded     = 0;
        state.mut().matches_1.burnedAmount     = 0;
    }
    else if (slot == 2)
    {
        state.mut().matches_2.gameId           = input.gameId;
        state.mut().matches_2.matchType        = input.matchType;
        state.mut().matches_2.state            = STATE_PENDING;
        state.mut().matches_2.playerCount      = playerCount;
        state.mut().matches_2.players_0        = qpi.invocator();
        state.mut().matches_2.players_1        = input.player1;
        state.mut().matches_2.players_2        = input.player2;
        state.mut().matches_2.players_3        = input.player3;
        state.mut().matches_2.stakes_0         = input.stakePerPlayer;
        state.mut().matches_2.stakes_1         = input.stakePerPlayer;
        state.mut().matches_2.stakes_2         = input.stakePerPlayer;
        state.mut().matches_2.stakes_3         = input.stakePerPlayer;
        state.mut().matches_2.totalStake       = totalStake;
        state.mut().matches_2.openedTick       = qpi.tick();
        state.mut().matches_2.serverSigned     = 0;
        state.mut().matches_2.player0Confirmed = 0;
        state.mut().matches_2.player1Confirmed = 0;
        state.mut().matches_2.player2Confirmed = 0;
        state.mut().matches_2.player3Confirmed = 0;
        state.mut().matches_2.prizeAwarded     = 0;
        state.mut().matches_2.burnedAmount     = 0;
    }
    else if (slot == 3)
    {
        state.mut().matches_3.gameId           = input.gameId;
        state.mut().matches_3.matchType        = input.matchType;
        state.mut().matches_3.state            = STATE_PENDING;
        state.mut().matches_3.playerCount      = playerCount;
        state.mut().matches_3.players_0        = qpi.invocator();
        state.mut().matches_3.players_1        = input.player1;
        state.mut().matches_3.players_2        = input.player2;
        state.mut().matches_3.players_3        = input.player3;
        state.mut().matches_3.stakes_0         = input.stakePerPlayer;
        state.mut().matches_3.stakes_1         = input.stakePerPlayer;
        state.mut().matches_3.stakes_2         = input.stakePerPlayer;
        state.mut().matches_3.stakes_3         = input.stakePerPlayer;
        state.mut().matches_3.totalStake       = totalStake;
        state.mut().matches_3.openedTick       = qpi.tick();
        state.mut().matches_3.serverSigned     = 0;
        state.mut().matches_3.player0Confirmed = 0;
        state.mut().matches_3.player1Confirmed = 0;
        state.mut().matches_3.player2Confirmed = 0;
        state.mut().matches_3.player3Confirmed = 0;
        state.mut().matches_3.prizeAwarded     = 0;
        state.mut().matches_3.burnedAmount     = 0;
    }
    else if (slot == 4)
    {
        state.mut().matches_4.gameId           = input.gameId;
        state.mut().matches_4.matchType        = input.matchType;
        state.mut().matches_4.state            = STATE_PENDING;
        state.mut().matches_4.playerCount      = playerCount;
        state.mut().matches_4.players_0        = qpi.invocator();
        state.mut().matches_4.players_1        = input.player1;
        state.mut().matches_4.players_2        = input.player2;
        state.mut().matches_4.players_3        = input.player3;
        state.mut().matches_4.stakes_0         = input.stakePerPlayer;
        state.mut().matches_4.stakes_1         = input.stakePerPlayer;
        state.mut().matches_4.stakes_2         = input.stakePerPlayer;
        state.mut().matches_4.stakes_3         = input.stakePerPlayer;
        state.mut().matches_4.totalStake       = totalStake;
        state.mut().matches_4.openedTick       = qpi.tick();
        state.mut().matches_4.serverSigned     = 0;
        state.mut().matches_4.player0Confirmed = 0;
        state.mut().matches_4.player1Confirmed = 0;
        state.mut().matches_4.player2Confirmed = 0;
        state.mut().matches_4.player3Confirmed = 0;
        state.mut().matches_4.prizeAwarded     = 0;
        state.mut().matches_4.burnedAmount     = 0;
    }
    else if (slot == 5)
    {
        state.mut().matches_5.gameId           = input.gameId;
        state.mut().matches_5.matchType        = input.matchType;
        state.mut().matches_5.state            = STATE_PENDING;
        state.mut().matches_5.playerCount      = playerCount;
        state.mut().matches_5.players_0        = qpi.invocator();
        state.mut().matches_5.players_1        = input.player1;
        state.mut().matches_5.players_2        = input.player2;
        state.mut().matches_5.players_3        = input.player3;
        state.mut().matches_5.stakes_0         = input.stakePerPlayer;
        state.mut().matches_5.stakes_1         = input.stakePerPlayer;
        state.mut().matches_5.stakes_2         = input.stakePerPlayer;
        state.mut().matches_5.stakes_3         = input.stakePerPlayer;
        state.mut().matches_5.totalStake       = totalStake;
        state.mut().matches_5.openedTick       = qpi.tick();
        state.mut().matches_5.serverSigned     = 0;
        state.mut().matches_5.player0Confirmed = 0;
        state.mut().matches_5.player1Confirmed = 0;
        state.mut().matches_5.player2Confirmed = 0;
        state.mut().matches_5.player3Confirmed = 0;
        state.mut().matches_5.prizeAwarded     = 0;
        state.mut().matches_5.burnedAmount     = 0;
    }
    else if (slot == 6)
    {
        state.mut().matches_6.gameId           = input.gameId;
        state.mut().matches_6.matchType        = input.matchType;
        state.mut().matches_6.state            = STATE_PENDING;
        state.mut().matches_6.playerCount      = playerCount;
        state.mut().matches_6.players_0        = qpi.invocator();
        state.mut().matches_6.players_1        = input.player1;
        state.mut().matches_6.players_2        = input.player2;
        state.mut().matches_6.players_3        = input.player3;
        state.mut().matches_6.stakes_0         = input.stakePerPlayer;
        state.mut().matches_6.stakes_1         = input.stakePerPlayer;
        state.mut().matches_6.stakes_2         = input.stakePerPlayer;
        state.mut().matches_6.stakes_3         = input.stakePerPlayer;
        state.mut().matches_6.totalStake       = totalStake;
        state.mut().matches_6.openedTick       = qpi.tick();
        state.mut().matches_6.serverSigned     = 0;
        state.mut().matches_6.player0Confirmed = 0;
        state.mut().matches_6.player1Confirmed = 0;
        state.mut().matches_6.player2Confirmed = 0;
        state.mut().matches_6.player3Confirmed = 0;
        state.mut().matches_6.prizeAwarded     = 0;
        state.mut().matches_6.burnedAmount     = 0;
    }
    else if (slot == 7)
    {
        state.mut().matches_7.gameId           = input.gameId;
        state.mut().matches_7.matchType        = input.matchType;
        state.mut().matches_7.state            = STATE_PENDING;
        state.mut().matches_7.playerCount      = playerCount;
        state.mut().matches_7.players_0        = qpi.invocator();
        state.mut().matches_7.players_1        = input.player1;
        state.mut().matches_7.players_2        = input.player2;
        state.mut().matches_7.players_3        = input.player3;
        state.mut().matches_7.stakes_0         = input.stakePerPlayer;
        state.mut().matches_7.stakes_1         = input.stakePerPlayer;
        state.mut().matches_7.stakes_2         = input.stakePerPlayer;
        state.mut().matches_7.stakes_3         = input.stakePerPlayer;
        state.mut().matches_7.totalStake       = totalStake;
        state.mut().matches_7.openedTick       = qpi.tick();
        state.mut().matches_7.serverSigned     = 0;
        state.mut().matches_7.player0Confirmed = 0;
        state.mut().matches_7.player1Confirmed = 0;
        state.mut().matches_7.player2Confirmed = 0;
        state.mut().matches_7.player3Confirmed = 0;
        state.mut().matches_7.prizeAwarded     = 0;
        state.mut().matches_7.burnedAmount     = 0;
    }
    else if (slot == 8)
    {
        state.mut().matches_8.gameId           = input.gameId;
        state.mut().matches_8.matchType        = input.matchType;
        state.mut().matches_8.state            = STATE_PENDING;
        state.mut().matches_8.playerCount      = playerCount;
        state.mut().matches_8.players_0        = qpi.invocator();
        state.mut().matches_8.players_1        = input.player1;
        state.mut().matches_8.players_2        = input.player2;
        state.mut().matches_8.players_3        = input.player3;
        state.mut().matches_8.stakes_0         = input.stakePerPlayer;
        state.mut().matches_8.stakes_1         = input.stakePerPlayer;
        state.mut().matches_8.stakes_2         = input.stakePerPlayer;
        state.mut().matches_8.stakes_3         = input.stakePerPlayer;
        state.mut().matches_8.totalStake       = totalStake;
        state.mut().matches_8.openedTick       = qpi.tick();
        state.mut().matches_8.serverSigned     = 0;
        state.mut().matches_8.player0Confirmed = 0;
        state.mut().matches_8.player1Confirmed = 0;
        state.mut().matches_8.player2Confirmed = 0;
        state.mut().matches_8.player3Confirmed = 0;
        state.mut().matches_8.prizeAwarded     = 0;
        state.mut().matches_8.burnedAmount     = 0;
    }
    else if (slot == 9)
    {
        state.mut().matches_9.gameId           = input.gameId;
        state.mut().matches_9.matchType        = input.matchType;
        state.mut().matches_9.state            = STATE_PENDING;
        state.mut().matches_9.playerCount      = playerCount;
        state.mut().matches_9.players_0        = qpi.invocator();
        state.mut().matches_9.players_1        = input.player1;
        state.mut().matches_9.players_2        = input.player2;
        state.mut().matches_9.players_3        = input.player3;
        state.mut().matches_9.stakes_0         = input.stakePerPlayer;
        state.mut().matches_9.stakes_1         = input.stakePerPlayer;
        state.mut().matches_9.stakes_2         = input.stakePerPlayer;
        state.mut().matches_9.stakes_3         = input.stakePerPlayer;
        state.mut().matches_9.totalStake       = totalStake;
        state.mut().matches_9.openedTick       = qpi.tick();
        state.mut().matches_9.serverSigned     = 0;
        state.mut().matches_9.player0Confirmed = 0;
        state.mut().matches_9.player1Confirmed = 0;
        state.mut().matches_9.player2Confirmed = 0;
        state.mut().matches_9.player3Confirmed = 0;
        state.mut().matches_9.prizeAwarded     = 0;
        state.mut().matches_9.burnedAmount     = 0;
    }
    else if (slot == 10)
    {
        state.mut().matches_10.gameId           = input.gameId;
        state.mut().matches_10.matchType        = input.matchType;
        state.mut().matches_10.state            = STATE_PENDING;
        state.mut().matches_10.playerCount      = playerCount;
        state.mut().matches_10.players_0        = qpi.invocator();
        state.mut().matches_10.players_1        = input.player1;
        state.mut().matches_10.players_2        = input.player2;
        state.mut().matches_10.players_3        = input.player3;
        state.mut().matches_10.stakes_0         = input.stakePerPlayer;
        state.mut().matches_10.stakes_1         = input.stakePerPlayer;
        state.mut().matches_10.stakes_2         = input.stakePerPlayer;
        state.mut().matches_10.stakes_3         = input.stakePerPlayer;
        state.mut().matches_10.totalStake       = totalStake;
        state.mut().matches_10.openedTick       = qpi.tick();
        state.mut().matches_10.serverSigned     = 0;
        state.mut().matches_10.player0Confirmed = 0;
        state.mut().matches_10.player1Confirmed = 0;
        state.mut().matches_10.player2Confirmed = 0;
        state.mut().matches_10.player3Confirmed = 0;
        state.mut().matches_10.prizeAwarded     = 0;
        state.mut().matches_10.burnedAmount     = 0;
    }
    else if (slot == 11)
    {
        state.mut().matches_11.gameId           = input.gameId;
        state.mut().matches_11.matchType        = input.matchType;
        state.mut().matches_11.state            = STATE_PENDING;
        state.mut().matches_11.playerCount      = playerCount;
        state.mut().matches_11.players_0        = qpi.invocator();
        state.mut().matches_11.players_1        = input.player1;
        state.mut().matches_11.players_2        = input.player2;
        state.mut().matches_11.players_3        = input.player3;
        state.mut().matches_11.stakes_0         = input.stakePerPlayer;
        state.mut().matches_11.stakes_1         = input.stakePerPlayer;
        state.mut().matches_11.stakes_2         = input.stakePerPlayer;
        state.mut().matches_11.stakes_3         = input.stakePerPlayer;
        state.mut().matches_11.totalStake       = totalStake;
        state.mut().matches_11.openedTick       = qpi.tick();
        state.mut().matches_11.serverSigned     = 0;
        state.mut().matches_11.player0Confirmed = 0;
        state.mut().matches_11.player1Confirmed = 0;
        state.mut().matches_11.player2Confirmed = 0;
        state.mut().matches_11.player3Confirmed = 0;
        state.mut().matches_11.prizeAwarded     = 0;
        state.mut().matches_11.burnedAmount     = 0;
    }
    else if (slot == 12)
    {
        state.mut().matches_12.gameId           = input.gameId;
        state.mut().matches_12.matchType        = input.matchType;
        state.mut().matches_12.state            = STATE_PENDING;
        state.mut().matches_12.playerCount      = playerCount;
        state.mut().matches_12.players_0        = qpi.invocator();
        state.mut().matches_12.players_1        = input.player1;
        state.mut().matches_12.players_2        = input.player2;
        state.mut().matches_12.players_3        = input.player3;
        state.mut().matches_12.stakes_0         = input.stakePerPlayer;
        state.mut().matches_12.stakes_1         = input.stakePerPlayer;
        state.mut().matches_12.stakes_2         = input.stakePerPlayer;
        state.mut().matches_12.stakes_3         = input.stakePerPlayer;
        state.mut().matches_12.totalStake       = totalStake;
        state.mut().matches_12.openedTick       = qpi.tick();
        state.mut().matches_12.serverSigned     = 0;
        state.mut().matches_12.player0Confirmed = 0;
        state.mut().matches_12.player1Confirmed = 0;
        state.mut().matches_12.player2Confirmed = 0;
        state.mut().matches_12.player3Confirmed = 0;
        state.mut().matches_12.prizeAwarded     = 0;
        state.mut().matches_12.burnedAmount     = 0;
    }
    else if (slot == 13)
    {
        state.mut().matches_13.gameId           = input.gameId;
        state.mut().matches_13.matchType        = input.matchType;
        state.mut().matches_13.state            = STATE_PENDING;
        state.mut().matches_13.playerCount      = playerCount;
        state.mut().matches_13.players_0        = qpi.invocator();
        state.mut().matches_13.players_1        = input.player1;
        state.mut().matches_13.players_2        = input.player2;
        state.mut().matches_13.players_3        = input.player3;
        state.mut().matches_13.stakes_0         = input.stakePerPlayer;
        state.mut().matches_13.stakes_1         = input.stakePerPlayer;
        state.mut().matches_13.stakes_2         = input.stakePerPlayer;
        state.mut().matches_13.stakes_3         = input.stakePerPlayer;
        state.mut().matches_13.totalStake       = totalStake;
        state.mut().matches_13.openedTick       = qpi.tick();
        state.mut().matches_13.serverSigned     = 0;
        state.mut().matches_13.player0Confirmed = 0;
        state.mut().matches_13.player1Confirmed = 0;
        state.mut().matches_13.player2Confirmed = 0;
        state.mut().matches_13.player3Confirmed = 0;
        state.mut().matches_13.prizeAwarded     = 0;
        state.mut().matches_13.burnedAmount     = 0;
    }
    else if (slot == 14)
    {
        state.mut().matches_14.gameId           = input.gameId;
        state.mut().matches_14.matchType        = input.matchType;
        state.mut().matches_14.state            = STATE_PENDING;
        state.mut().matches_14.playerCount      = playerCount;
        state.mut().matches_14.players_0        = qpi.invocator();
        state.mut().matches_14.players_1        = input.player1;
        state.mut().matches_14.players_2        = input.player2;
        state.mut().matches_14.players_3        = input.player3;
        state.mut().matches_14.stakes_0         = input.stakePerPlayer;
        state.mut().matches_14.stakes_1         = input.stakePerPlayer;
        state.mut().matches_14.stakes_2         = input.stakePerPlayer;
        state.mut().matches_14.stakes_3         = input.stakePerPlayer;
        state.mut().matches_14.totalStake       = totalStake;
        state.mut().matches_14.openedTick       = qpi.tick();
        state.mut().matches_14.serverSigned     = 0;
        state.mut().matches_14.player0Confirmed = 0;
        state.mut().matches_14.player1Confirmed = 0;
        state.mut().matches_14.player2Confirmed = 0;
        state.mut().matches_14.player3Confirmed = 0;
        state.mut().matches_14.prizeAwarded     = 0;
        state.mut().matches_14.burnedAmount     = 0;
    }
    else if (slot == 15)
    {
        state.mut().matches_15.gameId           = input.gameId;
        state.mut().matches_15.matchType        = input.matchType;
        state.mut().matches_15.state            = STATE_PENDING;
        state.mut().matches_15.playerCount      = playerCount;
        state.mut().matches_15.players_0        = qpi.invocator();
        state.mut().matches_15.players_1        = input.player1;
        state.mut().matches_15.players_2        = input.player2;
        state.mut().matches_15.players_3        = input.player3;
        state.mut().matches_15.stakes_0         = input.stakePerPlayer;
        state.mut().matches_15.stakes_1         = input.stakePerPlayer;
        state.mut().matches_15.stakes_2         = input.stakePerPlayer;
        state.mut().matches_15.stakes_3         = input.stakePerPlayer;
        state.mut().matches_15.totalStake       = totalStake;
        state.mut().matches_15.openedTick       = qpi.tick();
        state.mut().matches_15.serverSigned     = 0;
        state.mut().matches_15.player0Confirmed = 0;
        state.mut().matches_15.player1Confirmed = 0;
        state.mut().matches_15.player2Confirmed = 0;
        state.mut().matches_15.player3Confirmed = 0;
        state.mut().matches_15.prizeAwarded     = 0;
        state.mut().matches_15.burnedAmount     = 0;
    }

    state.mut().totalQUStaked = state.get().totalQUStaked + totalStake;
    if (input.gameId == GAME_SNAQE)      { state.mut().snaqeMatchCount = state.get().snaqeMatchCount + 1; }
    if (input.gameId == GAME_PAQMAN)     { state.mut().paqmanMatchCount = state.get().paqmanMatchCount + 1; }
    if (input.gameId == GAME_TANQBATTLE) { state.mut().tanqMatchCount = state.get().tanqMatchCount + 1; }

    output.matchSlot  = slot;
    output.totalStake = totalStake;
    output.success    = 1;
}

PUBLIC_PROCEDURE(SubmitResult)
/*
 * Game server submits signed match result after game ends.
 * SIGNATURE 1 of 2. Settlement does NOT fire — players must countersign.
 * Score sanity-checked against per-game caps.
 */
{
    if (qpi.invocator() != state.get().gameServerAddress)
    {
        output.success = 0;
        return;
    }
    if (input.matchSlot < 0 || input.matchSlot > 15)
    {
        output.success = 0;
        return;
    }

    // Score sanity check
    sint64 maxScore;
    maxScore = TANQ_MAX_SCORE;

    if (input.matchSlot == 0 && state.get().matches_0.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 0 && state.get().matches_0.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 1 && state.get().matches_1.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 1 && state.get().matches_1.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 2 && state.get().matches_2.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 2 && state.get().matches_2.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 3 && state.get().matches_3.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 3 && state.get().matches_3.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 4 && state.get().matches_4.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 4 && state.get().matches_4.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 5 && state.get().matches_5.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 5 && state.get().matches_5.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 6 && state.get().matches_6.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 6 && state.get().matches_6.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 7 && state.get().matches_7.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 7 && state.get().matches_7.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 8 && state.get().matches_8.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 8 && state.get().matches_8.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 9 && state.get().matches_9.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 9 && state.get().matches_9.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 10 && state.get().matches_10.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 10 && state.get().matches_10.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 11 && state.get().matches_11.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 11 && state.get().matches_11.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 12 && state.get().matches_12.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 12 && state.get().matches_12.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 13 && state.get().matches_13.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 13 && state.get().matches_13.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 14 && state.get().matches_14.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 14 && state.get().matches_14.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }
    if (input.matchSlot == 15 && state.get().matches_15.gameId == GAME_SNAQE)  { maxScore = SNAQE_MAX_SCORE; }
    if (input.matchSlot == 15 && state.get().matches_15.gameId == GAME_PAQMAN) { maxScore = PAQMAN_MAX_SCORE; }

    if (input.winnerScore > maxScore || input.winnerScore < 0)
    {
        output.success = 0;
        return;
    }

    if (input.matchSlot == 0)
    {
        if (state.get().matches_0.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_0.winnerAddress  = input.winner;
        state.mut().matches_0.winnerScore    = input.winnerScore;
        state.mut().matches_0.resultHash     = input.resultHash;
        state.mut().matches_0.serverSigned   = 1;
        state.mut().matches_0.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 1)
    {
        if (state.get().matches_1.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_1.winnerAddress  = input.winner;
        state.mut().matches_1.winnerScore    = input.winnerScore;
        state.mut().matches_1.resultHash     = input.resultHash;
        state.mut().matches_1.serverSigned   = 1;
        state.mut().matches_1.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 2)
    {
        if (state.get().matches_2.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_2.winnerAddress  = input.winner;
        state.mut().matches_2.winnerScore    = input.winnerScore;
        state.mut().matches_2.resultHash     = input.resultHash;
        state.mut().matches_2.serverSigned   = 1;
        state.mut().matches_2.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 3)
    {
        if (state.get().matches_3.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_3.winnerAddress  = input.winner;
        state.mut().matches_3.winnerScore    = input.winnerScore;
        state.mut().matches_3.resultHash     = input.resultHash;
        state.mut().matches_3.serverSigned   = 1;
        state.mut().matches_3.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 4)
    {
        if (state.get().matches_4.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_4.winnerAddress  = input.winner;
        state.mut().matches_4.winnerScore    = input.winnerScore;
        state.mut().matches_4.resultHash     = input.resultHash;
        state.mut().matches_4.serverSigned   = 1;
        state.mut().matches_4.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 5)
    {
        if (state.get().matches_5.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_5.winnerAddress  = input.winner;
        state.mut().matches_5.winnerScore    = input.winnerScore;
        state.mut().matches_5.resultHash     = input.resultHash;
        state.mut().matches_5.serverSigned   = 1;
        state.mut().matches_5.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 6)
    {
        if (state.get().matches_6.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_6.winnerAddress  = input.winner;
        state.mut().matches_6.winnerScore    = input.winnerScore;
        state.mut().matches_6.resultHash     = input.resultHash;
        state.mut().matches_6.serverSigned   = 1;
        state.mut().matches_6.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 7)
    {
        if (state.get().matches_7.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_7.winnerAddress  = input.winner;
        state.mut().matches_7.winnerScore    = input.winnerScore;
        state.mut().matches_7.resultHash     = input.resultHash;
        state.mut().matches_7.serverSigned   = 1;
        state.mut().matches_7.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 8)
    {
        if (state.get().matches_8.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_8.winnerAddress  = input.winner;
        state.mut().matches_8.winnerScore    = input.winnerScore;
        state.mut().matches_8.resultHash     = input.resultHash;
        state.mut().matches_8.serverSigned   = 1;
        state.mut().matches_8.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 9)
    {
        if (state.get().matches_9.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_9.winnerAddress  = input.winner;
        state.mut().matches_9.winnerScore    = input.winnerScore;
        state.mut().matches_9.resultHash     = input.resultHash;
        state.mut().matches_9.serverSigned   = 1;
        state.mut().matches_9.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 10)
    {
        if (state.get().matches_10.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_10.winnerAddress  = input.winner;
        state.mut().matches_10.winnerScore    = input.winnerScore;
        state.mut().matches_10.resultHash     = input.resultHash;
        state.mut().matches_10.serverSigned   = 1;
        state.mut().matches_10.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 11)
    {
        if (state.get().matches_11.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_11.winnerAddress  = input.winner;
        state.mut().matches_11.winnerScore    = input.winnerScore;
        state.mut().matches_11.resultHash     = input.resultHash;
        state.mut().matches_11.serverSigned   = 1;
        state.mut().matches_11.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 12)
    {
        if (state.get().matches_12.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_12.winnerAddress  = input.winner;
        state.mut().matches_12.winnerScore    = input.winnerScore;
        state.mut().matches_12.resultHash     = input.resultHash;
        state.mut().matches_12.serverSigned   = 1;
        state.mut().matches_12.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 13)
    {
        if (state.get().matches_13.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_13.winnerAddress  = input.winner;
        state.mut().matches_13.winnerScore    = input.winnerScore;
        state.mut().matches_13.resultHash     = input.resultHash;
        state.mut().matches_13.serverSigned   = 1;
        state.mut().matches_13.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 14)
    {
        if (state.get().matches_14.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_14.winnerAddress  = input.winner;
        state.mut().matches_14.winnerScore    = input.winnerScore;
        state.mut().matches_14.resultHash     = input.resultHash;
        state.mut().matches_14.serverSigned   = 1;
        state.mut().matches_14.serverSignTick = qpi.tick();
    }
    else if (input.matchSlot == 15)
    {
        if (state.get().matches_15.state != STATE_PENDING) { output.success = 0; return; }
        state.mut().matches_15.winnerAddress  = input.winner;
        state.mut().matches_15.winnerScore    = input.winnerScore;
        state.mut().matches_15.resultHash     = input.resultHash;
        state.mut().matches_15.serverSigned   = 1;
        state.mut().matches_15.serverSignTick = qpi.tick();
    }

    output.success            = 1;
    output.confirmWindowTicks = CONFIRM_WINDOW_TICKS;
}

PUBLIC_PROCEDURE(ConfirmResult)
/*
 * Player countersigns the server result. SIGNATURE 2 of 2 (or 2-4 for multi).
 *
 * Hash mismatch  → dispute + full refund to all players.
 * All confirmed  → cross-contract call to Token_v2 SettleMatch.
 *
 * Token_v2 handles all BPS routing. Cabinet only tracks state.
 * SOLO matches pay from rewardReserveBalance (no cross-contract needed).
 */
{
    if (input.matchSlot < 0 || input.matchSlot > 15)
    {
        output.confirmed = 0;
        return;
    }

    if (input.matchSlot == 0)
    {
        if (!state.get().matches_0.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_0.resultHash)
        {
            state.mut().matches_0.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_0.players_0, state.get().matches_0.stakes_0);
            if (state.get().matches_0.playerCount > 1) { qpi.transfer(state.get().matches_0.players_1, state.get().matches_0.stakes_1); }
            if (state.get().matches_0.playerCount > 2) { qpi.transfer(state.get().matches_0.players_2, state.get().matches_0.stakes_2); }
            if (state.get().matches_0.playerCount > 3) { qpi.transfer(state.get().matches_0.players_3, state.get().matches_0.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_0.players_0) { state.mut().matches_0.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_0.players_1) { state.mut().matches_0.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_0.players_2) { state.mut().matches_0.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_0.players_3) { state.mut().matches_0.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_0.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_0.playerCount > 1 && !state.get().matches_0.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_0.playerCount > 2 && !state.get().matches_0.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_0.playerCount > 3 && !state.get().matches_0.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_0.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_0.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_0.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_0.winnerAddress, soloReward);
            }
            state.mut().matches_0.prizeAwarded = soloReward;
            state.mut().matches_0.burnedAmount = 0;
            state.mut().matches_0.state        = STATE_SETTLED;
            state.mut().matches_0.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_0.winnerAddress;
        settlementCall.totalStake    = state.get().matches_0.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_0.prizeAwarded = div(state.get().matches_0.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_0.burnedAmount = div(state.get().matches_0.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_0.state        = STATE_SETTLED;
        state.mut().matches_0.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_0.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_0.prizeAwarded;
        if (state.get().matches_0.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_0.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_0.prizeAwarded;
        output.burnedAmount = state.get().matches_0.burnedAmount;
    }
        else if (input.matchSlot == 1)
    {
        if (!state.get().matches_1.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_1.resultHash)
        {
            state.mut().matches_1.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_1.players_0, state.get().matches_1.stakes_0);
            if (state.get().matches_1.playerCount > 1) { qpi.transfer(state.get().matches_1.players_1, state.get().matches_1.stakes_1); }
            if (state.get().matches_1.playerCount > 2) { qpi.transfer(state.get().matches_1.players_2, state.get().matches_1.stakes_2); }
            if (state.get().matches_1.playerCount > 3) { qpi.transfer(state.get().matches_1.players_3, state.get().matches_1.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_1.players_0) { state.mut().matches_1.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_1.players_1) { state.mut().matches_1.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_1.players_2) { state.mut().matches_1.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_1.players_3) { state.mut().matches_1.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_1.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_1.playerCount > 1 && !state.get().matches_1.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_1.playerCount > 2 && !state.get().matches_1.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_1.playerCount > 3 && !state.get().matches_1.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_1.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_1.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_1.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_1.winnerAddress, soloReward);
            }
            state.mut().matches_1.prizeAwarded = soloReward;
            state.mut().matches_1.burnedAmount = 0;
            state.mut().matches_1.state        = STATE_SETTLED;
            state.mut().matches_1.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_1.winnerAddress;
        settlementCall.totalStake    = state.get().matches_1.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_1.prizeAwarded = div(state.get().matches_1.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_1.burnedAmount = div(state.get().matches_1.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_1.state        = STATE_SETTLED;
        state.mut().matches_1.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_1.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_1.prizeAwarded;
        if (state.get().matches_1.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_1.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_1.prizeAwarded;
        output.burnedAmount = state.get().matches_1.burnedAmount;
    }
        else if (input.matchSlot == 2)
    {
        if (!state.get().matches_2.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_2.resultHash)
        {
            state.mut().matches_2.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_2.players_0, state.get().matches_2.stakes_0);
            if (state.get().matches_2.playerCount > 1) { qpi.transfer(state.get().matches_2.players_1, state.get().matches_2.stakes_1); }
            if (state.get().matches_2.playerCount > 2) { qpi.transfer(state.get().matches_2.players_2, state.get().matches_2.stakes_2); }
            if (state.get().matches_2.playerCount > 3) { qpi.transfer(state.get().matches_2.players_3, state.get().matches_2.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_2.players_0) { state.mut().matches_2.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_2.players_1) { state.mut().matches_2.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_2.players_2) { state.mut().matches_2.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_2.players_3) { state.mut().matches_2.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_2.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_2.playerCount > 1 && !state.get().matches_2.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_2.playerCount > 2 && !state.get().matches_2.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_2.playerCount > 3 && !state.get().matches_2.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_2.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_2.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_2.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_2.winnerAddress, soloReward);
            }
            state.mut().matches_2.prizeAwarded = soloReward;
            state.mut().matches_2.burnedAmount = 0;
            state.mut().matches_2.state        = STATE_SETTLED;
            state.mut().matches_2.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_2.winnerAddress;
        settlementCall.totalStake    = state.get().matches_2.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_2.prizeAwarded = div(state.get().matches_2.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_2.burnedAmount = div(state.get().matches_2.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_2.state        = STATE_SETTLED;
        state.mut().matches_2.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_2.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_2.prizeAwarded;
        if (state.get().matches_2.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_2.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_2.prizeAwarded;
        output.burnedAmount = state.get().matches_2.burnedAmount;
    }
        else if (input.matchSlot == 3)
    {
        if (!state.get().matches_3.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_3.resultHash)
        {
            state.mut().matches_3.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_3.players_0, state.get().matches_3.stakes_0);
            if (state.get().matches_3.playerCount > 1) { qpi.transfer(state.get().matches_3.players_1, state.get().matches_3.stakes_1); }
            if (state.get().matches_3.playerCount > 2) { qpi.transfer(state.get().matches_3.players_2, state.get().matches_3.stakes_2); }
            if (state.get().matches_3.playerCount > 3) { qpi.transfer(state.get().matches_3.players_3, state.get().matches_3.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_3.players_0) { state.mut().matches_3.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_3.players_1) { state.mut().matches_3.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_3.players_2) { state.mut().matches_3.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_3.players_3) { state.mut().matches_3.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_3.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_3.playerCount > 1 && !state.get().matches_3.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_3.playerCount > 2 && !state.get().matches_3.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_3.playerCount > 3 && !state.get().matches_3.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_3.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_3.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_3.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_3.winnerAddress, soloReward);
            }
            state.mut().matches_3.prizeAwarded = soloReward;
            state.mut().matches_3.burnedAmount = 0;
            state.mut().matches_3.state        = STATE_SETTLED;
            state.mut().matches_3.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_3.winnerAddress;
        settlementCall.totalStake    = state.get().matches_3.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_3.prizeAwarded = div(state.get().matches_3.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_3.burnedAmount = div(state.get().matches_3.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_3.state        = STATE_SETTLED;
        state.mut().matches_3.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_3.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_3.prizeAwarded;
        if (state.get().matches_3.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_3.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_3.prizeAwarded;
        output.burnedAmount = state.get().matches_3.burnedAmount;
    }
        else if (input.matchSlot == 4)
    {
        if (!state.get().matches_4.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_4.resultHash)
        {
            state.mut().matches_4.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_4.players_0, state.get().matches_4.stakes_0);
            if (state.get().matches_4.playerCount > 1) { qpi.transfer(state.get().matches_4.players_1, state.get().matches_4.stakes_1); }
            if (state.get().matches_4.playerCount > 2) { qpi.transfer(state.get().matches_4.players_2, state.get().matches_4.stakes_2); }
            if (state.get().matches_4.playerCount > 3) { qpi.transfer(state.get().matches_4.players_3, state.get().matches_4.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_4.players_0) { state.mut().matches_4.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_4.players_1) { state.mut().matches_4.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_4.players_2) { state.mut().matches_4.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_4.players_3) { state.mut().matches_4.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_4.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_4.playerCount > 1 && !state.get().matches_4.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_4.playerCount > 2 && !state.get().matches_4.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_4.playerCount > 3 && !state.get().matches_4.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_4.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_4.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_4.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_4.winnerAddress, soloReward);
            }
            state.mut().matches_4.prizeAwarded = soloReward;
            state.mut().matches_4.burnedAmount = 0;
            state.mut().matches_4.state        = STATE_SETTLED;
            state.mut().matches_4.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_4.winnerAddress;
        settlementCall.totalStake    = state.get().matches_4.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_4.prizeAwarded = div(state.get().matches_4.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_4.burnedAmount = div(state.get().matches_4.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_4.state        = STATE_SETTLED;
        state.mut().matches_4.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_4.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_4.prizeAwarded;
        if (state.get().matches_4.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_4.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_4.prizeAwarded;
        output.burnedAmount = state.get().matches_4.burnedAmount;
    }
        else if (input.matchSlot == 5)
    {
        if (!state.get().matches_5.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_5.resultHash)
        {
            state.mut().matches_5.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_5.players_0, state.get().matches_5.stakes_0);
            if (state.get().matches_5.playerCount > 1) { qpi.transfer(state.get().matches_5.players_1, state.get().matches_5.stakes_1); }
            if (state.get().matches_5.playerCount > 2) { qpi.transfer(state.get().matches_5.players_2, state.get().matches_5.stakes_2); }
            if (state.get().matches_5.playerCount > 3) { qpi.transfer(state.get().matches_5.players_3, state.get().matches_5.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_5.players_0) { state.mut().matches_5.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_5.players_1) { state.mut().matches_5.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_5.players_2) { state.mut().matches_5.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_5.players_3) { state.mut().matches_5.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_5.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_5.playerCount > 1 && !state.get().matches_5.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_5.playerCount > 2 && !state.get().matches_5.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_5.playerCount > 3 && !state.get().matches_5.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_5.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_5.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_5.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_5.winnerAddress, soloReward);
            }
            state.mut().matches_5.prizeAwarded = soloReward;
            state.mut().matches_5.burnedAmount = 0;
            state.mut().matches_5.state        = STATE_SETTLED;
            state.mut().matches_5.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_5.winnerAddress;
        settlementCall.totalStake    = state.get().matches_5.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_5.prizeAwarded = div(state.get().matches_5.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_5.burnedAmount = div(state.get().matches_5.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_5.state        = STATE_SETTLED;
        state.mut().matches_5.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_5.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_5.prizeAwarded;
        if (state.get().matches_5.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_5.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_5.prizeAwarded;
        output.burnedAmount = state.get().matches_5.burnedAmount;
    }
        else if (input.matchSlot == 6)
    {
        if (!state.get().matches_6.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_6.resultHash)
        {
            state.mut().matches_6.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_6.players_0, state.get().matches_6.stakes_0);
            if (state.get().matches_6.playerCount > 1) { qpi.transfer(state.get().matches_6.players_1, state.get().matches_6.stakes_1); }
            if (state.get().matches_6.playerCount > 2) { qpi.transfer(state.get().matches_6.players_2, state.get().matches_6.stakes_2); }
            if (state.get().matches_6.playerCount > 3) { qpi.transfer(state.get().matches_6.players_3, state.get().matches_6.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_6.players_0) { state.mut().matches_6.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_6.players_1) { state.mut().matches_6.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_6.players_2) { state.mut().matches_6.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_6.players_3) { state.mut().matches_6.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_6.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_6.playerCount > 1 && !state.get().matches_6.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_6.playerCount > 2 && !state.get().matches_6.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_6.playerCount > 3 && !state.get().matches_6.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_6.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_6.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_6.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_6.winnerAddress, soloReward);
            }
            state.mut().matches_6.prizeAwarded = soloReward;
            state.mut().matches_6.burnedAmount = 0;
            state.mut().matches_6.state        = STATE_SETTLED;
            state.mut().matches_6.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_6.winnerAddress;
        settlementCall.totalStake    = state.get().matches_6.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_6.prizeAwarded = div(state.get().matches_6.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_6.burnedAmount = div(state.get().matches_6.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_6.state        = STATE_SETTLED;
        state.mut().matches_6.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_6.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_6.prizeAwarded;
        if (state.get().matches_6.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_6.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_6.prizeAwarded;
        output.burnedAmount = state.get().matches_6.burnedAmount;
    }
        else if (input.matchSlot == 7)
    {
        if (!state.get().matches_7.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_7.resultHash)
        {
            state.mut().matches_7.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_7.players_0, state.get().matches_7.stakes_0);
            if (state.get().matches_7.playerCount > 1) { qpi.transfer(state.get().matches_7.players_1, state.get().matches_7.stakes_1); }
            if (state.get().matches_7.playerCount > 2) { qpi.transfer(state.get().matches_7.players_2, state.get().matches_7.stakes_2); }
            if (state.get().matches_7.playerCount > 3) { qpi.transfer(state.get().matches_7.players_3, state.get().matches_7.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_7.players_0) { state.mut().matches_7.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_7.players_1) { state.mut().matches_7.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_7.players_2) { state.mut().matches_7.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_7.players_3) { state.mut().matches_7.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_7.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_7.playerCount > 1 && !state.get().matches_7.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_7.playerCount > 2 && !state.get().matches_7.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_7.playerCount > 3 && !state.get().matches_7.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_7.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_7.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_7.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_7.winnerAddress, soloReward);
            }
            state.mut().matches_7.prizeAwarded = soloReward;
            state.mut().matches_7.burnedAmount = 0;
            state.mut().matches_7.state        = STATE_SETTLED;
            state.mut().matches_7.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_7.winnerAddress;
        settlementCall.totalStake    = state.get().matches_7.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_7.prizeAwarded = div(state.get().matches_7.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_7.burnedAmount = div(state.get().matches_7.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_7.state        = STATE_SETTLED;
        state.mut().matches_7.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_7.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_7.prizeAwarded;
        if (state.get().matches_7.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_7.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_7.prizeAwarded;
        output.burnedAmount = state.get().matches_7.burnedAmount;
    }
        else if (input.matchSlot == 8)
    {
        if (!state.get().matches_8.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_8.resultHash)
        {
            state.mut().matches_8.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_8.players_0, state.get().matches_8.stakes_0);
            if (state.get().matches_8.playerCount > 1) { qpi.transfer(state.get().matches_8.players_1, state.get().matches_8.stakes_1); }
            if (state.get().matches_8.playerCount > 2) { qpi.transfer(state.get().matches_8.players_2, state.get().matches_8.stakes_2); }
            if (state.get().matches_8.playerCount > 3) { qpi.transfer(state.get().matches_8.players_3, state.get().matches_8.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_8.players_0) { state.mut().matches_8.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_8.players_1) { state.mut().matches_8.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_8.players_2) { state.mut().matches_8.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_8.players_3) { state.mut().matches_8.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_8.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_8.playerCount > 1 && !state.get().matches_8.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_8.playerCount > 2 && !state.get().matches_8.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_8.playerCount > 3 && !state.get().matches_8.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_8.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_8.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_8.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_8.winnerAddress, soloReward);
            }
            state.mut().matches_8.prizeAwarded = soloReward;
            state.mut().matches_8.burnedAmount = 0;
            state.mut().matches_8.state        = STATE_SETTLED;
            state.mut().matches_8.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_8.winnerAddress;
        settlementCall.totalStake    = state.get().matches_8.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_8.prizeAwarded = div(state.get().matches_8.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_8.burnedAmount = div(state.get().matches_8.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_8.state        = STATE_SETTLED;
        state.mut().matches_8.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_8.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_8.prizeAwarded;
        if (state.get().matches_8.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_8.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_8.prizeAwarded;
        output.burnedAmount = state.get().matches_8.burnedAmount;
    }
        else if (input.matchSlot == 9)
    {
        if (!state.get().matches_9.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_9.resultHash)
        {
            state.mut().matches_9.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_9.players_0, state.get().matches_9.stakes_0);
            if (state.get().matches_9.playerCount > 1) { qpi.transfer(state.get().matches_9.players_1, state.get().matches_9.stakes_1); }
            if (state.get().matches_9.playerCount > 2) { qpi.transfer(state.get().matches_9.players_2, state.get().matches_9.stakes_2); }
            if (state.get().matches_9.playerCount > 3) { qpi.transfer(state.get().matches_9.players_3, state.get().matches_9.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_9.players_0) { state.mut().matches_9.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_9.players_1) { state.mut().matches_9.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_9.players_2) { state.mut().matches_9.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_9.players_3) { state.mut().matches_9.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_9.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_9.playerCount > 1 && !state.get().matches_9.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_9.playerCount > 2 && !state.get().matches_9.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_9.playerCount > 3 && !state.get().matches_9.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_9.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_9.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_9.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_9.winnerAddress, soloReward);
            }
            state.mut().matches_9.prizeAwarded = soloReward;
            state.mut().matches_9.burnedAmount = 0;
            state.mut().matches_9.state        = STATE_SETTLED;
            state.mut().matches_9.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_9.winnerAddress;
        settlementCall.totalStake    = state.get().matches_9.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_9.prizeAwarded = div(state.get().matches_9.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_9.burnedAmount = div(state.get().matches_9.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_9.state        = STATE_SETTLED;
        state.mut().matches_9.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_9.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_9.prizeAwarded;
        if (state.get().matches_9.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_9.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_9.prizeAwarded;
        output.burnedAmount = state.get().matches_9.burnedAmount;
    }
        else if (input.matchSlot == 10)
    {
        if (!state.get().matches_10.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_10.resultHash)
        {
            state.mut().matches_10.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_10.players_0, state.get().matches_10.stakes_0);
            if (state.get().matches_10.playerCount > 1) { qpi.transfer(state.get().matches_10.players_1, state.get().matches_10.stakes_1); }
            if (state.get().matches_10.playerCount > 2) { qpi.transfer(state.get().matches_10.players_2, state.get().matches_10.stakes_2); }
            if (state.get().matches_10.playerCount > 3) { qpi.transfer(state.get().matches_10.players_3, state.get().matches_10.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_10.players_0) { state.mut().matches_10.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_10.players_1) { state.mut().matches_10.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_10.players_2) { state.mut().matches_10.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_10.players_3) { state.mut().matches_10.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_10.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_10.playerCount > 1 && !state.get().matches_10.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_10.playerCount > 2 && !state.get().matches_10.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_10.playerCount > 3 && !state.get().matches_10.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_10.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_10.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_10.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_10.winnerAddress, soloReward);
            }
            state.mut().matches_10.prizeAwarded = soloReward;
            state.mut().matches_10.burnedAmount = 0;
            state.mut().matches_10.state        = STATE_SETTLED;
            state.mut().matches_10.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_10.winnerAddress;
        settlementCall.totalStake    = state.get().matches_10.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_10.prizeAwarded = div(state.get().matches_10.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_10.burnedAmount = div(state.get().matches_10.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_10.state        = STATE_SETTLED;
        state.mut().matches_10.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_10.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_10.prizeAwarded;
        if (state.get().matches_10.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_10.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_10.prizeAwarded;
        output.burnedAmount = state.get().matches_10.burnedAmount;
    }
        else if (input.matchSlot == 11)
    {
        if (!state.get().matches_11.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_11.resultHash)
        {
            state.mut().matches_11.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_11.players_0, state.get().matches_11.stakes_0);
            if (state.get().matches_11.playerCount > 1) { qpi.transfer(state.get().matches_11.players_1, state.get().matches_11.stakes_1); }
            if (state.get().matches_11.playerCount > 2) { qpi.transfer(state.get().matches_11.players_2, state.get().matches_11.stakes_2); }
            if (state.get().matches_11.playerCount > 3) { qpi.transfer(state.get().matches_11.players_3, state.get().matches_11.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_11.players_0) { state.mut().matches_11.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_11.players_1) { state.mut().matches_11.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_11.players_2) { state.mut().matches_11.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_11.players_3) { state.mut().matches_11.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_11.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_11.playerCount > 1 && !state.get().matches_11.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_11.playerCount > 2 && !state.get().matches_11.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_11.playerCount > 3 && !state.get().matches_11.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_11.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_11.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_11.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_11.winnerAddress, soloReward);
            }
            state.mut().matches_11.prizeAwarded = soloReward;
            state.mut().matches_11.burnedAmount = 0;
            state.mut().matches_11.state        = STATE_SETTLED;
            state.mut().matches_11.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_11.winnerAddress;
        settlementCall.totalStake    = state.get().matches_11.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_11.prizeAwarded = div(state.get().matches_11.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_11.burnedAmount = div(state.get().matches_11.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_11.state        = STATE_SETTLED;
        state.mut().matches_11.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_11.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_11.prizeAwarded;
        if (state.get().matches_11.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_11.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_11.prizeAwarded;
        output.burnedAmount = state.get().matches_11.burnedAmount;
    }
        else if (input.matchSlot == 12)
    {
        if (!state.get().matches_12.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_12.resultHash)
        {
            state.mut().matches_12.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_12.players_0, state.get().matches_12.stakes_0);
            if (state.get().matches_12.playerCount > 1) { qpi.transfer(state.get().matches_12.players_1, state.get().matches_12.stakes_1); }
            if (state.get().matches_12.playerCount > 2) { qpi.transfer(state.get().matches_12.players_2, state.get().matches_12.stakes_2); }
            if (state.get().matches_12.playerCount > 3) { qpi.transfer(state.get().matches_12.players_3, state.get().matches_12.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_12.players_0) { state.mut().matches_12.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_12.players_1) { state.mut().matches_12.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_12.players_2) { state.mut().matches_12.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_12.players_3) { state.mut().matches_12.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_12.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_12.playerCount > 1 && !state.get().matches_12.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_12.playerCount > 2 && !state.get().matches_12.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_12.playerCount > 3 && !state.get().matches_12.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_12.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_12.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_12.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_12.winnerAddress, soloReward);
            }
            state.mut().matches_12.prizeAwarded = soloReward;
            state.mut().matches_12.burnedAmount = 0;
            state.mut().matches_12.state        = STATE_SETTLED;
            state.mut().matches_12.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_12.winnerAddress;
        settlementCall.totalStake    = state.get().matches_12.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_12.prizeAwarded = div(state.get().matches_12.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_12.burnedAmount = div(state.get().matches_12.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_12.state        = STATE_SETTLED;
        state.mut().matches_12.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_12.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_12.prizeAwarded;
        if (state.get().matches_12.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_12.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_12.prizeAwarded;
        output.burnedAmount = state.get().matches_12.burnedAmount;
    }
        else if (input.matchSlot == 13)
    {
        if (!state.get().matches_13.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_13.resultHash)
        {
            state.mut().matches_13.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_13.players_0, state.get().matches_13.stakes_0);
            if (state.get().matches_13.playerCount > 1) { qpi.transfer(state.get().matches_13.players_1, state.get().matches_13.stakes_1); }
            if (state.get().matches_13.playerCount > 2) { qpi.transfer(state.get().matches_13.players_2, state.get().matches_13.stakes_2); }
            if (state.get().matches_13.playerCount > 3) { qpi.transfer(state.get().matches_13.players_3, state.get().matches_13.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_13.players_0) { state.mut().matches_13.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_13.players_1) { state.mut().matches_13.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_13.players_2) { state.mut().matches_13.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_13.players_3) { state.mut().matches_13.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_13.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_13.playerCount > 1 && !state.get().matches_13.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_13.playerCount > 2 && !state.get().matches_13.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_13.playerCount > 3 && !state.get().matches_13.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_13.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_13.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_13.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_13.winnerAddress, soloReward);
            }
            state.mut().matches_13.prizeAwarded = soloReward;
            state.mut().matches_13.burnedAmount = 0;
            state.mut().matches_13.state        = STATE_SETTLED;
            state.mut().matches_13.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_13.winnerAddress;
        settlementCall.totalStake    = state.get().matches_13.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_13.prizeAwarded = div(state.get().matches_13.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_13.burnedAmount = div(state.get().matches_13.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_13.state        = STATE_SETTLED;
        state.mut().matches_13.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_13.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_13.prizeAwarded;
        if (state.get().matches_13.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_13.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_13.prizeAwarded;
        output.burnedAmount = state.get().matches_13.burnedAmount;
    }
        else if (input.matchSlot == 14)
    {
        if (!state.get().matches_14.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_14.resultHash)
        {
            state.mut().matches_14.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_14.players_0, state.get().matches_14.stakes_0);
            if (state.get().matches_14.playerCount > 1) { qpi.transfer(state.get().matches_14.players_1, state.get().matches_14.stakes_1); }
            if (state.get().matches_14.playerCount > 2) { qpi.transfer(state.get().matches_14.players_2, state.get().matches_14.stakes_2); }
            if (state.get().matches_14.playerCount > 3) { qpi.transfer(state.get().matches_14.players_3, state.get().matches_14.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_14.players_0) { state.mut().matches_14.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_14.players_1) { state.mut().matches_14.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_14.players_2) { state.mut().matches_14.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_14.players_3) { state.mut().matches_14.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_14.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_14.playerCount > 1 && !state.get().matches_14.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_14.playerCount > 2 && !state.get().matches_14.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_14.playerCount > 3 && !state.get().matches_14.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_14.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_14.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_14.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_14.winnerAddress, soloReward);
            }
            state.mut().matches_14.prizeAwarded = soloReward;
            state.mut().matches_14.burnedAmount = 0;
            state.mut().matches_14.state        = STATE_SETTLED;
            state.mut().matches_14.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_14.winnerAddress;
        settlementCall.totalStake    = state.get().matches_14.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_14.prizeAwarded = div(state.get().matches_14.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_14.burnedAmount = div(state.get().matches_14.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_14.state        = STATE_SETTLED;
        state.mut().matches_14.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_14.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_14.prizeAwarded;
        if (state.get().matches_14.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_14.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_14.prizeAwarded;
        output.burnedAmount = state.get().matches_14.burnedAmount;
    }
        else if (input.matchSlot == 15)
    {
        if (!state.get().matches_15.serverSigned) { output.confirmed = 0; return; }

        // Hash mismatch → dispute + refund
        if (input.resultHash != state.get().matches_15.resultHash)
        {
            state.mut().matches_15.state = STATE_DISPUTED;
            state.mut().totalDisputes = state.get().totalDisputes + 1;
            qpi.transfer(state.get().matches_15.players_0, state.get().matches_15.stakes_0);
            if (state.get().matches_15.playerCount > 1) { qpi.transfer(state.get().matches_15.players_1, state.get().matches_15.stakes_1); }
            if (state.get().matches_15.playerCount > 2) { qpi.transfer(state.get().matches_15.players_2, state.get().matches_15.stakes_2); }
            if (state.get().matches_15.playerCount > 3) { qpi.transfer(state.get().matches_15.players_3, state.get().matches_15.stakes_3); }
            output.confirmed = 0;
            output.settled   = 0;
            return;
        }

        // Record this player's confirmation
        if (qpi.invocator() == state.get().matches_15.players_0) { state.mut().matches_15.player0Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_15.players_1) { state.mut().matches_15.player1Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_15.players_2) { state.mut().matches_15.player2Confirmed = 1; }
        if (qpi.invocator() == state.get().matches_15.players_3) { state.mut().matches_15.player3Confirmed = 1; }

        output.confirmed = 1;

        // Check if all required players have confirmed
        bit allConfirmed;
        allConfirmed = 1;
        if (!state.get().matches_15.player0Confirmed) { allConfirmed = 0; }
        if (state.get().matches_15.playerCount > 1 && !state.get().matches_15.player1Confirmed) { allConfirmed = 0; }
        if (state.get().matches_15.playerCount > 2 && !state.get().matches_15.player2Confirmed) { allConfirmed = 0; }
        if (state.get().matches_15.playerCount > 3 && !state.get().matches_15.player3Confirmed) { allConfirmed = 0; }

        if (!allConfirmed) { output.settled = 0; return; }


        state.mut().matches_15.state = STATE_CONFIRMED;

        // ---- SOLO: pay from reserve, no routing ----
        if (state.get().matches_15.matchType == MATCH_SOLO)
        {
            sint64 soloReward;
            soloReward = SOLO_BASE_REWARD_QU;
            soloReward = soloReward + div(state.get().matches_15.winnerScore, SOLO_SCORE_MULTIPLIER).quot;
            if (soloReward > SOLO_MAX_REWARD_QU) { soloReward = SOLO_MAX_REWARD_QU; }
            if (state.get().rewardReserveBalance >= soloReward)
            {
                state.mut().rewardReserveBalance = state.get().rewardReserveBalance - soloReward;
                qpi.transfer(state.get().matches_15.winnerAddress, soloReward);
            }
            state.mut().matches_15.prizeAwarded = soloReward;
            state.mut().matches_15.burnedAmount = 0;
            state.mut().matches_15.state        = STATE_SETTLED;
            state.mut().matches_15.settledTick  = qpi.tick();
            state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
            state.mut().totalMatchesSolo = state.get().totalMatchesSolo + 1;
            state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + soloReward;
            output.settled      = 1;
            output.prizeAwarded = soloReward;
            output.burnedAmount = 0;
            return;
        }

        // ---- DUEL / MULTI: cross-contract call → Token_v2 SettleMatch ----
        QZN::SettleMatch_input settlementCall;
        settlementCall.winnerAddress = state.get().matches_15.winnerAddress;
        settlementCall.totalStake    = state.get().matches_15.totalStake;

        QZN::SettleMatch_output settlementResult;
        INVOKE_OTHER_CONTRACT_PROCEDURE(
            QZN,
            SettleMatch,
            settlementCall,
            settlementResult,
            0
        );

        state.mut().matches_15.prizeAwarded = div(state.get().matches_15.totalStake * 4500LL, 10000LL).quot;
        state.mut().matches_15.burnedAmount = div(state.get().matches_15.totalStake * 1000LL, 10000LL).quot;
        state.mut().matches_15.state        = STATE_SETTLED;
        state.mut().matches_15.settledTick  = qpi.tick();

        state.mut().totalMatchesPlayed = state.get().totalMatchesPlayed + 1;
        state.mut().totalQUBurned = state.get().totalQUBurned + state.get().matches_15.burnedAmount;
        state.mut().totalPrizesAwarded = state.get().totalPrizesAwarded + state.get().matches_15.prizeAwarded;
        if (state.get().matches_15.matchType == MATCH_DUEL)  { state.mut().totalMatchesDuel = state.get().totalMatchesDuel + 1; }
        if (state.get().matches_15.matchType == MATCH_MULTI) { state.mut().totalMatchesMulti = state.get().totalMatchesMulti + 1; }

        output.settled      = 1;
        output.prizeAwarded = state.get().matches_15.prizeAwarded;
        output.burnedAmount = state.get().matches_15.burnedAmount;
    }

}

PUBLIC_PROCEDURE(DisputeResult)
/*
 * Player explicitly disputes result before confirmation window closes.
 * Triggers full refund to all players. No burn on dispute.
 */
{
    if (input.matchSlot < 0 || input.matchSlot > 15)
    {
        output.disputeRecorded = 0;
        return;
    }

    if (input.matchSlot == 0)
    {
        if (state.get().matches_0.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_0.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_0.players_0, state.get().matches_0.stakes_0);
        if (state.get().matches_0.playerCount > 1) { qpi.transfer(state.get().matches_0.players_1, state.get().matches_0.stakes_1); }
        if (state.get().matches_0.playerCount > 2) { qpi.transfer(state.get().matches_0.players_2, state.get().matches_0.stakes_2); }
        if (state.get().matches_0.playerCount > 3) { qpi.transfer(state.get().matches_0.players_3, state.get().matches_0.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_0.stakes_0;
    }
        else if (input.matchSlot == 1)
    {
        if (state.get().matches_1.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_1.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_1.players_0, state.get().matches_1.stakes_0);
        if (state.get().matches_1.playerCount > 1) { qpi.transfer(state.get().matches_1.players_1, state.get().matches_1.stakes_1); }
        if (state.get().matches_1.playerCount > 2) { qpi.transfer(state.get().matches_1.players_2, state.get().matches_1.stakes_2); }
        if (state.get().matches_1.playerCount > 3) { qpi.transfer(state.get().matches_1.players_3, state.get().matches_1.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_1.stakes_0;
    }
        else if (input.matchSlot == 2)
    {
        if (state.get().matches_2.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_2.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_2.players_0, state.get().matches_2.stakes_0);
        if (state.get().matches_2.playerCount > 1) { qpi.transfer(state.get().matches_2.players_1, state.get().matches_2.stakes_1); }
        if (state.get().matches_2.playerCount > 2) { qpi.transfer(state.get().matches_2.players_2, state.get().matches_2.stakes_2); }
        if (state.get().matches_2.playerCount > 3) { qpi.transfer(state.get().matches_2.players_3, state.get().matches_2.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_2.stakes_0;
    }
        else if (input.matchSlot == 3)
    {
        if (state.get().matches_3.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_3.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_3.players_0, state.get().matches_3.stakes_0);
        if (state.get().matches_3.playerCount > 1) { qpi.transfer(state.get().matches_3.players_1, state.get().matches_3.stakes_1); }
        if (state.get().matches_3.playerCount > 2) { qpi.transfer(state.get().matches_3.players_2, state.get().matches_3.stakes_2); }
        if (state.get().matches_3.playerCount > 3) { qpi.transfer(state.get().matches_3.players_3, state.get().matches_3.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_3.stakes_0;
    }
        else if (input.matchSlot == 4)
    {
        if (state.get().matches_4.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_4.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_4.players_0, state.get().matches_4.stakes_0);
        if (state.get().matches_4.playerCount > 1) { qpi.transfer(state.get().matches_4.players_1, state.get().matches_4.stakes_1); }
        if (state.get().matches_4.playerCount > 2) { qpi.transfer(state.get().matches_4.players_2, state.get().matches_4.stakes_2); }
        if (state.get().matches_4.playerCount > 3) { qpi.transfer(state.get().matches_4.players_3, state.get().matches_4.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_4.stakes_0;
    }
        else if (input.matchSlot == 5)
    {
        if (state.get().matches_5.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_5.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_5.players_0, state.get().matches_5.stakes_0);
        if (state.get().matches_5.playerCount > 1) { qpi.transfer(state.get().matches_5.players_1, state.get().matches_5.stakes_1); }
        if (state.get().matches_5.playerCount > 2) { qpi.transfer(state.get().matches_5.players_2, state.get().matches_5.stakes_2); }
        if (state.get().matches_5.playerCount > 3) { qpi.transfer(state.get().matches_5.players_3, state.get().matches_5.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_5.stakes_0;
    }
        else if (input.matchSlot == 6)
    {
        if (state.get().matches_6.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_6.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_6.players_0, state.get().matches_6.stakes_0);
        if (state.get().matches_6.playerCount > 1) { qpi.transfer(state.get().matches_6.players_1, state.get().matches_6.stakes_1); }
        if (state.get().matches_6.playerCount > 2) { qpi.transfer(state.get().matches_6.players_2, state.get().matches_6.stakes_2); }
        if (state.get().matches_6.playerCount > 3) { qpi.transfer(state.get().matches_6.players_3, state.get().matches_6.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_6.stakes_0;
    }
        else if (input.matchSlot == 7)
    {
        if (state.get().matches_7.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_7.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_7.players_0, state.get().matches_7.stakes_0);
        if (state.get().matches_7.playerCount > 1) { qpi.transfer(state.get().matches_7.players_1, state.get().matches_7.stakes_1); }
        if (state.get().matches_7.playerCount > 2) { qpi.transfer(state.get().matches_7.players_2, state.get().matches_7.stakes_2); }
        if (state.get().matches_7.playerCount > 3) { qpi.transfer(state.get().matches_7.players_3, state.get().matches_7.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_7.stakes_0;
    }
        else if (input.matchSlot == 8)
    {
        if (state.get().matches_8.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_8.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_8.players_0, state.get().matches_8.stakes_0);
        if (state.get().matches_8.playerCount > 1) { qpi.transfer(state.get().matches_8.players_1, state.get().matches_8.stakes_1); }
        if (state.get().matches_8.playerCount > 2) { qpi.transfer(state.get().matches_8.players_2, state.get().matches_8.stakes_2); }
        if (state.get().matches_8.playerCount > 3) { qpi.transfer(state.get().matches_8.players_3, state.get().matches_8.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_8.stakes_0;
    }
        else if (input.matchSlot == 9)
    {
        if (state.get().matches_9.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_9.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_9.players_0, state.get().matches_9.stakes_0);
        if (state.get().matches_9.playerCount > 1) { qpi.transfer(state.get().matches_9.players_1, state.get().matches_9.stakes_1); }
        if (state.get().matches_9.playerCount > 2) { qpi.transfer(state.get().matches_9.players_2, state.get().matches_9.stakes_2); }
        if (state.get().matches_9.playerCount > 3) { qpi.transfer(state.get().matches_9.players_3, state.get().matches_9.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_9.stakes_0;
    }
        else if (input.matchSlot == 10)
    {
        if (state.get().matches_10.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_10.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_10.players_0, state.get().matches_10.stakes_0);
        if (state.get().matches_10.playerCount > 1) { qpi.transfer(state.get().matches_10.players_1, state.get().matches_10.stakes_1); }
        if (state.get().matches_10.playerCount > 2) { qpi.transfer(state.get().matches_10.players_2, state.get().matches_10.stakes_2); }
        if (state.get().matches_10.playerCount > 3) { qpi.transfer(state.get().matches_10.players_3, state.get().matches_10.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_10.stakes_0;
    }
        else if (input.matchSlot == 11)
    {
        if (state.get().matches_11.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_11.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_11.players_0, state.get().matches_11.stakes_0);
        if (state.get().matches_11.playerCount > 1) { qpi.transfer(state.get().matches_11.players_1, state.get().matches_11.stakes_1); }
        if (state.get().matches_11.playerCount > 2) { qpi.transfer(state.get().matches_11.players_2, state.get().matches_11.stakes_2); }
        if (state.get().matches_11.playerCount > 3) { qpi.transfer(state.get().matches_11.players_3, state.get().matches_11.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_11.stakes_0;
    }
        else if (input.matchSlot == 12)
    {
        if (state.get().matches_12.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_12.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_12.players_0, state.get().matches_12.stakes_0);
        if (state.get().matches_12.playerCount > 1) { qpi.transfer(state.get().matches_12.players_1, state.get().matches_12.stakes_1); }
        if (state.get().matches_12.playerCount > 2) { qpi.transfer(state.get().matches_12.players_2, state.get().matches_12.stakes_2); }
        if (state.get().matches_12.playerCount > 3) { qpi.transfer(state.get().matches_12.players_3, state.get().matches_12.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_12.stakes_0;
    }
        else if (input.matchSlot == 13)
    {
        if (state.get().matches_13.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_13.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_13.players_0, state.get().matches_13.stakes_0);
        if (state.get().matches_13.playerCount > 1) { qpi.transfer(state.get().matches_13.players_1, state.get().matches_13.stakes_1); }
        if (state.get().matches_13.playerCount > 2) { qpi.transfer(state.get().matches_13.players_2, state.get().matches_13.stakes_2); }
        if (state.get().matches_13.playerCount > 3) { qpi.transfer(state.get().matches_13.players_3, state.get().matches_13.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_13.stakes_0;
    }
        else if (input.matchSlot == 14)
    {
        if (state.get().matches_14.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_14.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_14.players_0, state.get().matches_14.stakes_0);
        if (state.get().matches_14.playerCount > 1) { qpi.transfer(state.get().matches_14.players_1, state.get().matches_14.stakes_1); }
        if (state.get().matches_14.playerCount > 2) { qpi.transfer(state.get().matches_14.players_2, state.get().matches_14.stakes_2); }
        if (state.get().matches_14.playerCount > 3) { qpi.transfer(state.get().matches_14.players_3, state.get().matches_14.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_14.stakes_0;
    }
        else if (input.matchSlot == 15)
    {
        if (state.get().matches_15.state != STATE_PENDING) { output.disputeRecorded = 0; return; }
        state.mut().matches_15.state = STATE_DISPUTED;
        state.mut().totalDisputes = state.get().totalDisputes + 1;
        qpi.transfer(state.get().matches_15.players_0, state.get().matches_15.stakes_0);
        if (state.get().matches_15.playerCount > 1) { qpi.transfer(state.get().matches_15.players_1, state.get().matches_15.stakes_1); }
        if (state.get().matches_15.playerCount > 2) { qpi.transfer(state.get().matches_15.players_2, state.get().matches_15.stakes_2); }
        if (state.get().matches_15.playerCount > 3) { qpi.transfer(state.get().matches_15.players_3, state.get().matches_15.stakes_3); }
        output.disputeRecorded = 1;
        output.refundAmount    = state.get().matches_15.stakes_0;
    }

}

PUBLIC_FUNCTION(GetMatch)
{
    if (input.matchSlot < 0 || input.matchSlot > 15) { return; }

    if (input.matchSlot == 0)
    {
        output.gameId        = state.get().matches_0.gameId;
        output.matchType     = state.get().matches_0.matchType;
        output.state         = state.get().matches_0.state;
        output.winner        = state.get().matches_0.winnerAddress;
        output.totalStake    = state.get().matches_0.totalStake;
        output.prizeAwarded  = state.get().matches_0.prizeAwarded;
        output.burnedAmount  = state.get().matches_0.burnedAmount;
        output.serverSigned  = state.get().matches_0.serverSigned;
        bit fc; fc = state.get().matches_0.player0Confirmed;
        if (state.get().matches_0.playerCount > 1 && !state.get().matches_0.player1Confirmed) { fc = 0; }
        if (state.get().matches_0.playerCount > 2 && !state.get().matches_0.player2Confirmed) { fc = 0; }
        if (state.get().matches_0.playerCount > 3 && !state.get().matches_0.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 1)
    {
        output.gameId        = state.get().matches_1.gameId;
        output.matchType     = state.get().matches_1.matchType;
        output.state         = state.get().matches_1.state;
        output.winner        = state.get().matches_1.winnerAddress;
        output.totalStake    = state.get().matches_1.totalStake;
        output.prizeAwarded  = state.get().matches_1.prizeAwarded;
        output.burnedAmount  = state.get().matches_1.burnedAmount;
        output.serverSigned  = state.get().matches_1.serverSigned;
        bit fc; fc = state.get().matches_1.player0Confirmed;
        if (state.get().matches_1.playerCount > 1 && !state.get().matches_1.player1Confirmed) { fc = 0; }
        if (state.get().matches_1.playerCount > 2 && !state.get().matches_1.player2Confirmed) { fc = 0; }
        if (state.get().matches_1.playerCount > 3 && !state.get().matches_1.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 2)
    {
        output.gameId        = state.get().matches_2.gameId;
        output.matchType     = state.get().matches_2.matchType;
        output.state         = state.get().matches_2.state;
        output.winner        = state.get().matches_2.winnerAddress;
        output.totalStake    = state.get().matches_2.totalStake;
        output.prizeAwarded  = state.get().matches_2.prizeAwarded;
        output.burnedAmount  = state.get().matches_2.burnedAmount;
        output.serverSigned  = state.get().matches_2.serverSigned;
        bit fc; fc = state.get().matches_2.player0Confirmed;
        if (state.get().matches_2.playerCount > 1 && !state.get().matches_2.player1Confirmed) { fc = 0; }
        if (state.get().matches_2.playerCount > 2 && !state.get().matches_2.player2Confirmed) { fc = 0; }
        if (state.get().matches_2.playerCount > 3 && !state.get().matches_2.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 3)
    {
        output.gameId        = state.get().matches_3.gameId;
        output.matchType     = state.get().matches_3.matchType;
        output.state         = state.get().matches_3.state;
        output.winner        = state.get().matches_3.winnerAddress;
        output.totalStake    = state.get().matches_3.totalStake;
        output.prizeAwarded  = state.get().matches_3.prizeAwarded;
        output.burnedAmount  = state.get().matches_3.burnedAmount;
        output.serverSigned  = state.get().matches_3.serverSigned;
        bit fc; fc = state.get().matches_3.player0Confirmed;
        if (state.get().matches_3.playerCount > 1 && !state.get().matches_3.player1Confirmed) { fc = 0; }
        if (state.get().matches_3.playerCount > 2 && !state.get().matches_3.player2Confirmed) { fc = 0; }
        if (state.get().matches_3.playerCount > 3 && !state.get().matches_3.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 4)
    {
        output.gameId        = state.get().matches_4.gameId;
        output.matchType     = state.get().matches_4.matchType;
        output.state         = state.get().matches_4.state;
        output.winner        = state.get().matches_4.winnerAddress;
        output.totalStake    = state.get().matches_4.totalStake;
        output.prizeAwarded  = state.get().matches_4.prizeAwarded;
        output.burnedAmount  = state.get().matches_4.burnedAmount;
        output.serverSigned  = state.get().matches_4.serverSigned;
        bit fc; fc = state.get().matches_4.player0Confirmed;
        if (state.get().matches_4.playerCount > 1 && !state.get().matches_4.player1Confirmed) { fc = 0; }
        if (state.get().matches_4.playerCount > 2 && !state.get().matches_4.player2Confirmed) { fc = 0; }
        if (state.get().matches_4.playerCount > 3 && !state.get().matches_4.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 5)
    {
        output.gameId        = state.get().matches_5.gameId;
        output.matchType     = state.get().matches_5.matchType;
        output.state         = state.get().matches_5.state;
        output.winner        = state.get().matches_5.winnerAddress;
        output.totalStake    = state.get().matches_5.totalStake;
        output.prizeAwarded  = state.get().matches_5.prizeAwarded;
        output.burnedAmount  = state.get().matches_5.burnedAmount;
        output.serverSigned  = state.get().matches_5.serverSigned;
        bit fc; fc = state.get().matches_5.player0Confirmed;
        if (state.get().matches_5.playerCount > 1 && !state.get().matches_5.player1Confirmed) { fc = 0; }
        if (state.get().matches_5.playerCount > 2 && !state.get().matches_5.player2Confirmed) { fc = 0; }
        if (state.get().matches_5.playerCount > 3 && !state.get().matches_5.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 6)
    {
        output.gameId        = state.get().matches_6.gameId;
        output.matchType     = state.get().matches_6.matchType;
        output.state         = state.get().matches_6.state;
        output.winner        = state.get().matches_6.winnerAddress;
        output.totalStake    = state.get().matches_6.totalStake;
        output.prizeAwarded  = state.get().matches_6.prizeAwarded;
        output.burnedAmount  = state.get().matches_6.burnedAmount;
        output.serverSigned  = state.get().matches_6.serverSigned;
        bit fc; fc = state.get().matches_6.player0Confirmed;
        if (state.get().matches_6.playerCount > 1 && !state.get().matches_6.player1Confirmed) { fc = 0; }
        if (state.get().matches_6.playerCount > 2 && !state.get().matches_6.player2Confirmed) { fc = 0; }
        if (state.get().matches_6.playerCount > 3 && !state.get().matches_6.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 7)
    {
        output.gameId        = state.get().matches_7.gameId;
        output.matchType     = state.get().matches_7.matchType;
        output.state         = state.get().matches_7.state;
        output.winner        = state.get().matches_7.winnerAddress;
        output.totalStake    = state.get().matches_7.totalStake;
        output.prizeAwarded  = state.get().matches_7.prizeAwarded;
        output.burnedAmount  = state.get().matches_7.burnedAmount;
        output.serverSigned  = state.get().matches_7.serverSigned;
        bit fc; fc = state.get().matches_7.player0Confirmed;
        if (state.get().matches_7.playerCount > 1 && !state.get().matches_7.player1Confirmed) { fc = 0; }
        if (state.get().matches_7.playerCount > 2 && !state.get().matches_7.player2Confirmed) { fc = 0; }
        if (state.get().matches_7.playerCount > 3 && !state.get().matches_7.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 8)
    {
        output.gameId        = state.get().matches_8.gameId;
        output.matchType     = state.get().matches_8.matchType;
        output.state         = state.get().matches_8.state;
        output.winner        = state.get().matches_8.winnerAddress;
        output.totalStake    = state.get().matches_8.totalStake;
        output.prizeAwarded  = state.get().matches_8.prizeAwarded;
        output.burnedAmount  = state.get().matches_8.burnedAmount;
        output.serverSigned  = state.get().matches_8.serverSigned;
        bit fc; fc = state.get().matches_8.player0Confirmed;
        if (state.get().matches_8.playerCount > 1 && !state.get().matches_8.player1Confirmed) { fc = 0; }
        if (state.get().matches_8.playerCount > 2 && !state.get().matches_8.player2Confirmed) { fc = 0; }
        if (state.get().matches_8.playerCount > 3 && !state.get().matches_8.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 9)
    {
        output.gameId        = state.get().matches_9.gameId;
        output.matchType     = state.get().matches_9.matchType;
        output.state         = state.get().matches_9.state;
        output.winner        = state.get().matches_9.winnerAddress;
        output.totalStake    = state.get().matches_9.totalStake;
        output.prizeAwarded  = state.get().matches_9.prizeAwarded;
        output.burnedAmount  = state.get().matches_9.burnedAmount;
        output.serverSigned  = state.get().matches_9.serverSigned;
        bit fc; fc = state.get().matches_9.player0Confirmed;
        if (state.get().matches_9.playerCount > 1 && !state.get().matches_9.player1Confirmed) { fc = 0; }
        if (state.get().matches_9.playerCount > 2 && !state.get().matches_9.player2Confirmed) { fc = 0; }
        if (state.get().matches_9.playerCount > 3 && !state.get().matches_9.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 10)
    {
        output.gameId        = state.get().matches_10.gameId;
        output.matchType     = state.get().matches_10.matchType;
        output.state         = state.get().matches_10.state;
        output.winner        = state.get().matches_10.winnerAddress;
        output.totalStake    = state.get().matches_10.totalStake;
        output.prizeAwarded  = state.get().matches_10.prizeAwarded;
        output.burnedAmount  = state.get().matches_10.burnedAmount;
        output.serverSigned  = state.get().matches_10.serverSigned;
        bit fc; fc = state.get().matches_10.player0Confirmed;
        if (state.get().matches_10.playerCount > 1 && !state.get().matches_10.player1Confirmed) { fc = 0; }
        if (state.get().matches_10.playerCount > 2 && !state.get().matches_10.player2Confirmed) { fc = 0; }
        if (state.get().matches_10.playerCount > 3 && !state.get().matches_10.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 11)
    {
        output.gameId        = state.get().matches_11.gameId;
        output.matchType     = state.get().matches_11.matchType;
        output.state         = state.get().matches_11.state;
        output.winner        = state.get().matches_11.winnerAddress;
        output.totalStake    = state.get().matches_11.totalStake;
        output.prizeAwarded  = state.get().matches_11.prizeAwarded;
        output.burnedAmount  = state.get().matches_11.burnedAmount;
        output.serverSigned  = state.get().matches_11.serverSigned;
        bit fc; fc = state.get().matches_11.player0Confirmed;
        if (state.get().matches_11.playerCount > 1 && !state.get().matches_11.player1Confirmed) { fc = 0; }
        if (state.get().matches_11.playerCount > 2 && !state.get().matches_11.player2Confirmed) { fc = 0; }
        if (state.get().matches_11.playerCount > 3 && !state.get().matches_11.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 12)
    {
        output.gameId        = state.get().matches_12.gameId;
        output.matchType     = state.get().matches_12.matchType;
        output.state         = state.get().matches_12.state;
        output.winner        = state.get().matches_12.winnerAddress;
        output.totalStake    = state.get().matches_12.totalStake;
        output.prizeAwarded  = state.get().matches_12.prizeAwarded;
        output.burnedAmount  = state.get().matches_12.burnedAmount;
        output.serverSigned  = state.get().matches_12.serverSigned;
        bit fc; fc = state.get().matches_12.player0Confirmed;
        if (state.get().matches_12.playerCount > 1 && !state.get().matches_12.player1Confirmed) { fc = 0; }
        if (state.get().matches_12.playerCount > 2 && !state.get().matches_12.player2Confirmed) { fc = 0; }
        if (state.get().matches_12.playerCount > 3 && !state.get().matches_12.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 13)
    {
        output.gameId        = state.get().matches_13.gameId;
        output.matchType     = state.get().matches_13.matchType;
        output.state         = state.get().matches_13.state;
        output.winner        = state.get().matches_13.winnerAddress;
        output.totalStake    = state.get().matches_13.totalStake;
        output.prizeAwarded  = state.get().matches_13.prizeAwarded;
        output.burnedAmount  = state.get().matches_13.burnedAmount;
        output.serverSigned  = state.get().matches_13.serverSigned;
        bit fc; fc = state.get().matches_13.player0Confirmed;
        if (state.get().matches_13.playerCount > 1 && !state.get().matches_13.player1Confirmed) { fc = 0; }
        if (state.get().matches_13.playerCount > 2 && !state.get().matches_13.player2Confirmed) { fc = 0; }
        if (state.get().matches_13.playerCount > 3 && !state.get().matches_13.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 14)
    {
        output.gameId        = state.get().matches_14.gameId;
        output.matchType     = state.get().matches_14.matchType;
        output.state         = state.get().matches_14.state;
        output.winner        = state.get().matches_14.winnerAddress;
        output.totalStake    = state.get().matches_14.totalStake;
        output.prizeAwarded  = state.get().matches_14.prizeAwarded;
        output.burnedAmount  = state.get().matches_14.burnedAmount;
        output.serverSigned  = state.get().matches_14.serverSigned;
        bit fc; fc = state.get().matches_14.player0Confirmed;
        if (state.get().matches_14.playerCount > 1 && !state.get().matches_14.player1Confirmed) { fc = 0; }
        if (state.get().matches_14.playerCount > 2 && !state.get().matches_14.player2Confirmed) { fc = 0; }
        if (state.get().matches_14.playerCount > 3 && !state.get().matches_14.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }
        else if (input.matchSlot == 15)
    {
        output.gameId        = state.get().matches_15.gameId;
        output.matchType     = state.get().matches_15.matchType;
        output.state         = state.get().matches_15.state;
        output.winner        = state.get().matches_15.winnerAddress;
        output.totalStake    = state.get().matches_15.totalStake;
        output.prizeAwarded  = state.get().matches_15.prizeAwarded;
        output.burnedAmount  = state.get().matches_15.burnedAmount;
        output.serverSigned  = state.get().matches_15.serverSigned;
        bit fc; fc = state.get().matches_15.player0Confirmed;
        if (state.get().matches_15.playerCount > 1 && !state.get().matches_15.player1Confirmed) { fc = 0; }
        if (state.get().matches_15.playerCount > 2 && !state.get().matches_15.player2Confirmed) { fc = 0; }
        if (state.get().matches_15.playerCount > 3 && !state.get().matches_15.player3Confirmed) { fc = 0; }
        output.fullyConfirmed = fc;
    }

}

PUBLIC_FUNCTION(GetCabinetStats)
{
    output.totalMatchesPlayed = state.get().totalMatchesPlayed;
    output.totalQUBurned      = state.get().totalQUBurned;
    output.totalPrizesAwarded = state.get().totalPrizesAwarded;
    output.totalDisputes      = state.get().totalDisputes;
    output.snaqeCount         = state.get().snaqeMatchCount;
    output.paqmanCount        = state.get().paqmanMatchCount;
    output.tanqCount          = state.get().tanqMatchCount;
}

// ============================================================
//  REGISTRATION
// ============================================================

REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
{
    REGISTER_USER_PROCEDURE(InitializeCabinet,  1);
    REGISTER_USER_PROCEDURE(RegisterMatch,      2);
    REGISTER_USER_PROCEDURE(SubmitResult,       3);
    REGISTER_USER_PROCEDURE(ConfirmResult,      4);
    REGISTER_USER_PROCEDURE(DisputeResult,      5);
    REGISTER_USER_FUNCTION(GetMatch,            6);
    REGISTER_USER_FUNCTION(GetCabinetStats,     7);
}

// ============================================================
//  SYSTEM HOOKS
// ============================================================

BEGIN_EPOCH()
{
}

END_TICK()
/*
 * Expire matches that missed the confirmation window.
 * Fires every tick — checks all 16 slots.
 * Refunds stakes automatically. No burn on expiry.
 */
{
    sint64 currentTick;
    currentTick = qpi.tick();

    if (state.get().matches_0.state == STATE_PENDING && state.get().matches_0.serverSigned == 1)
    {
        sint64 ticksElapsed_0;
        ticksElapsed_0 = currentTick - state.get().matches_0.serverSignTick;
        if (ticksElapsed_0 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_0.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_0.players_0, state.get().matches_0.stakes_0);
            if (state.get().matches_0.playerCount > 1) { qpi.transfer(state.get().matches_0.players_1, state.get().matches_0.stakes_1); }
            if (state.get().matches_0.playerCount > 2) { qpi.transfer(state.get().matches_0.players_2, state.get().matches_0.stakes_2); }
            if (state.get().matches_0.playerCount > 3) { qpi.transfer(state.get().matches_0.players_3, state.get().matches_0.stakes_3); }
        }
    }
    if (state.get().matches_1.state == STATE_PENDING && state.get().matches_1.serverSigned == 1)
    {
        sint64 ticksElapsed_1;
        ticksElapsed_1 = currentTick - state.get().matches_1.serverSignTick;
        if (ticksElapsed_1 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_1.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_1.players_0, state.get().matches_1.stakes_0);
            if (state.get().matches_1.playerCount > 1) { qpi.transfer(state.get().matches_1.players_1, state.get().matches_1.stakes_1); }
            if (state.get().matches_1.playerCount > 2) { qpi.transfer(state.get().matches_1.players_2, state.get().matches_1.stakes_2); }
            if (state.get().matches_1.playerCount > 3) { qpi.transfer(state.get().matches_1.players_3, state.get().matches_1.stakes_3); }
        }
    }
    if (state.get().matches_2.state == STATE_PENDING && state.get().matches_2.serverSigned == 1)
    {
        sint64 ticksElapsed_2;
        ticksElapsed_2 = currentTick - state.get().matches_2.serverSignTick;
        if (ticksElapsed_2 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_2.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_2.players_0, state.get().matches_2.stakes_0);
            if (state.get().matches_2.playerCount > 1) { qpi.transfer(state.get().matches_2.players_1, state.get().matches_2.stakes_1); }
            if (state.get().matches_2.playerCount > 2) { qpi.transfer(state.get().matches_2.players_2, state.get().matches_2.stakes_2); }
            if (state.get().matches_2.playerCount > 3) { qpi.transfer(state.get().matches_2.players_3, state.get().matches_2.stakes_3); }
        }
    }
    if (state.get().matches_3.state == STATE_PENDING && state.get().matches_3.serverSigned == 1)
    {
        sint64 ticksElapsed_3;
        ticksElapsed_3 = currentTick - state.get().matches_3.serverSignTick;
        if (ticksElapsed_3 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_3.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_3.players_0, state.get().matches_3.stakes_0);
            if (state.get().matches_3.playerCount > 1) { qpi.transfer(state.get().matches_3.players_1, state.get().matches_3.stakes_1); }
            if (state.get().matches_3.playerCount > 2) { qpi.transfer(state.get().matches_3.players_2, state.get().matches_3.stakes_2); }
            if (state.get().matches_3.playerCount > 3) { qpi.transfer(state.get().matches_3.players_3, state.get().matches_3.stakes_3); }
        }
    }
    if (state.get().matches_4.state == STATE_PENDING && state.get().matches_4.serverSigned == 1)
    {
        sint64 ticksElapsed_4;
        ticksElapsed_4 = currentTick - state.get().matches_4.serverSignTick;
        if (ticksElapsed_4 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_4.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_4.players_0, state.get().matches_4.stakes_0);
            if (state.get().matches_4.playerCount > 1) { qpi.transfer(state.get().matches_4.players_1, state.get().matches_4.stakes_1); }
            if (state.get().matches_4.playerCount > 2) { qpi.transfer(state.get().matches_4.players_2, state.get().matches_4.stakes_2); }
            if (state.get().matches_4.playerCount > 3) { qpi.transfer(state.get().matches_4.players_3, state.get().matches_4.stakes_3); }
        }
    }
    if (state.get().matches_5.state == STATE_PENDING && state.get().matches_5.serverSigned == 1)
    {
        sint64 ticksElapsed_5;
        ticksElapsed_5 = currentTick - state.get().matches_5.serverSignTick;
        if (ticksElapsed_5 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_5.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_5.players_0, state.get().matches_5.stakes_0);
            if (state.get().matches_5.playerCount > 1) { qpi.transfer(state.get().matches_5.players_1, state.get().matches_5.stakes_1); }
            if (state.get().matches_5.playerCount > 2) { qpi.transfer(state.get().matches_5.players_2, state.get().matches_5.stakes_2); }
            if (state.get().matches_5.playerCount > 3) { qpi.transfer(state.get().matches_5.players_3, state.get().matches_5.stakes_3); }
        }
    }
    if (state.get().matches_6.state == STATE_PENDING && state.get().matches_6.serverSigned == 1)
    {
        sint64 ticksElapsed_6;
        ticksElapsed_6 = currentTick - state.get().matches_6.serverSignTick;
        if (ticksElapsed_6 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_6.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_6.players_0, state.get().matches_6.stakes_0);
            if (state.get().matches_6.playerCount > 1) { qpi.transfer(state.get().matches_6.players_1, state.get().matches_6.stakes_1); }
            if (state.get().matches_6.playerCount > 2) { qpi.transfer(state.get().matches_6.players_2, state.get().matches_6.stakes_2); }
            if (state.get().matches_6.playerCount > 3) { qpi.transfer(state.get().matches_6.players_3, state.get().matches_6.stakes_3); }
        }
    }
    if (state.get().matches_7.state == STATE_PENDING && state.get().matches_7.serverSigned == 1)
    {
        sint64 ticksElapsed_7;
        ticksElapsed_7 = currentTick - state.get().matches_7.serverSignTick;
        if (ticksElapsed_7 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_7.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_7.players_0, state.get().matches_7.stakes_0);
            if (state.get().matches_7.playerCount > 1) { qpi.transfer(state.get().matches_7.players_1, state.get().matches_7.stakes_1); }
            if (state.get().matches_7.playerCount > 2) { qpi.transfer(state.get().matches_7.players_2, state.get().matches_7.stakes_2); }
            if (state.get().matches_7.playerCount > 3) { qpi.transfer(state.get().matches_7.players_3, state.get().matches_7.stakes_3); }
        }
    }
    if (state.get().matches_8.state == STATE_PENDING && state.get().matches_8.serverSigned == 1)
    {
        sint64 ticksElapsed_8;
        ticksElapsed_8 = currentTick - state.get().matches_8.serverSignTick;
        if (ticksElapsed_8 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_8.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_8.players_0, state.get().matches_8.stakes_0);
            if (state.get().matches_8.playerCount > 1) { qpi.transfer(state.get().matches_8.players_1, state.get().matches_8.stakes_1); }
            if (state.get().matches_8.playerCount > 2) { qpi.transfer(state.get().matches_8.players_2, state.get().matches_8.stakes_2); }
            if (state.get().matches_8.playerCount > 3) { qpi.transfer(state.get().matches_8.players_3, state.get().matches_8.stakes_3); }
        }
    }
    if (state.get().matches_9.state == STATE_PENDING && state.get().matches_9.serverSigned == 1)
    {
        sint64 ticksElapsed_9;
        ticksElapsed_9 = currentTick - state.get().matches_9.serverSignTick;
        if (ticksElapsed_9 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_9.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_9.players_0, state.get().matches_9.stakes_0);
            if (state.get().matches_9.playerCount > 1) { qpi.transfer(state.get().matches_9.players_1, state.get().matches_9.stakes_1); }
            if (state.get().matches_9.playerCount > 2) { qpi.transfer(state.get().matches_9.players_2, state.get().matches_9.stakes_2); }
            if (state.get().matches_9.playerCount > 3) { qpi.transfer(state.get().matches_9.players_3, state.get().matches_9.stakes_3); }
        }
    }
    if (state.get().matches_10.state == STATE_PENDING && state.get().matches_10.serverSigned == 1)
    {
        sint64 ticksElapsed_10;
        ticksElapsed_10 = currentTick - state.get().matches_10.serverSignTick;
        if (ticksElapsed_10 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_10.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_10.players_0, state.get().matches_10.stakes_0);
            if (state.get().matches_10.playerCount > 1) { qpi.transfer(state.get().matches_10.players_1, state.get().matches_10.stakes_1); }
            if (state.get().matches_10.playerCount > 2) { qpi.transfer(state.get().matches_10.players_2, state.get().matches_10.stakes_2); }
            if (state.get().matches_10.playerCount > 3) { qpi.transfer(state.get().matches_10.players_3, state.get().matches_10.stakes_3); }
        }
    }
    if (state.get().matches_11.state == STATE_PENDING && state.get().matches_11.serverSigned == 1)
    {
        sint64 ticksElapsed_11;
        ticksElapsed_11 = currentTick - state.get().matches_11.serverSignTick;
        if (ticksElapsed_11 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_11.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_11.players_0, state.get().matches_11.stakes_0);
            if (state.get().matches_11.playerCount > 1) { qpi.transfer(state.get().matches_11.players_1, state.get().matches_11.stakes_1); }
            if (state.get().matches_11.playerCount > 2) { qpi.transfer(state.get().matches_11.players_2, state.get().matches_11.stakes_2); }
            if (state.get().matches_11.playerCount > 3) { qpi.transfer(state.get().matches_11.players_3, state.get().matches_11.stakes_3); }
        }
    }
    if (state.get().matches_12.state == STATE_PENDING && state.get().matches_12.serverSigned == 1)
    {
        sint64 ticksElapsed_12;
        ticksElapsed_12 = currentTick - state.get().matches_12.serverSignTick;
        if (ticksElapsed_12 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_12.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_12.players_0, state.get().matches_12.stakes_0);
            if (state.get().matches_12.playerCount > 1) { qpi.transfer(state.get().matches_12.players_1, state.get().matches_12.stakes_1); }
            if (state.get().matches_12.playerCount > 2) { qpi.transfer(state.get().matches_12.players_2, state.get().matches_12.stakes_2); }
            if (state.get().matches_12.playerCount > 3) { qpi.transfer(state.get().matches_12.players_3, state.get().matches_12.stakes_3); }
        }
    }
    if (state.get().matches_13.state == STATE_PENDING && state.get().matches_13.serverSigned == 1)
    {
        sint64 ticksElapsed_13;
        ticksElapsed_13 = currentTick - state.get().matches_13.serverSignTick;
        if (ticksElapsed_13 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_13.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_13.players_0, state.get().matches_13.stakes_0);
            if (state.get().matches_13.playerCount > 1) { qpi.transfer(state.get().matches_13.players_1, state.get().matches_13.stakes_1); }
            if (state.get().matches_13.playerCount > 2) { qpi.transfer(state.get().matches_13.players_2, state.get().matches_13.stakes_2); }
            if (state.get().matches_13.playerCount > 3) { qpi.transfer(state.get().matches_13.players_3, state.get().matches_13.stakes_3); }
        }
    }
    if (state.get().matches_14.state == STATE_PENDING && state.get().matches_14.serverSigned == 1)
    {
        sint64 ticksElapsed_14;
        ticksElapsed_14 = currentTick - state.get().matches_14.serverSignTick;
        if (ticksElapsed_14 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_14.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_14.players_0, state.get().matches_14.stakes_0);
            if (state.get().matches_14.playerCount > 1) { qpi.transfer(state.get().matches_14.players_1, state.get().matches_14.stakes_1); }
            if (state.get().matches_14.playerCount > 2) { qpi.transfer(state.get().matches_14.players_2, state.get().matches_14.stakes_2); }
            if (state.get().matches_14.playerCount > 3) { qpi.transfer(state.get().matches_14.players_3, state.get().matches_14.stakes_3); }
        }
    }
    if (state.get().matches_15.state == STATE_PENDING && state.get().matches_15.serverSigned == 1)
    {
        sint64 ticksElapsed_15;
        ticksElapsed_15 = currentTick - state.get().matches_15.serverSignTick;
        if (ticksElapsed_15 > CONFIRM_WINDOW_TICKS)
        {
            state.mut().matches_15.state = STATE_EXPIRED;
            state.mut().totalExpired = state.get().totalExpired + 1;
            qpi.transfer(state.get().matches_15.players_0, state.get().matches_15.stakes_0);
            if (state.get().matches_15.playerCount > 1) { qpi.transfer(state.get().matches_15.players_1, state.get().matches_15.stakes_1); }
            if (state.get().matches_15.playerCount > 2) { qpi.transfer(state.get().matches_15.players_2, state.get().matches_15.stakes_2); }
            if (state.get().matches_15.playerCount > 3) { qpi.transfer(state.get().matches_15.players_3, state.get().matches_15.stakes_3); }
        }
    }
}
};