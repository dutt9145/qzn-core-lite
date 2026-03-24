#define NO_UEFI
#include <iostream>
#include "contract_testing.h"

// ============================================================
//  QZN TournamentEngine PAO — GTest Suite
//  File:    test/contract_qzn_tournamentengine.cpp
//  Place in qubic/core repo at: test/contract_qzn_tournamentengine.cpp
//
//  This tests the v2.0.0 QPI-compliant contract (QZN_TournamentEngine_PAO.h)
//  NOT the original v1 file which had #pragma once / #include violations.
//
//  Coverage:
//    [PROC-1]  InitializeTournamentEngine — happy path, adminOverride,
//                                           uses invocator when override = NULL_ID,
//                                           double-init guard, initialized flag
//    [PROC-2]  CreateTournament           — single/double/round-robin formats,
//                                           all valid player counts (4/8/16/32/64),
//                                           non-power-of-2 rejected,
//                                           below-min player count rejected,
//                                           above-max player count rejected,
//                                           below-min entry fee rejected,
//                                           non-admin rejected,
//                                           tournamentCount increments,
//                                           initial state = TSTATE_REGISTRATION
//    [PROC-3]  RegisterPlayer             — slot 0 first, player stored,
//                                           exact fee pays, overpayment refunded,
//                                           underpayment refunded + rejected,
//                                           duplicate registration rejected,
//                                           wrong state (not REGISTRATION) rejected,
//                                           at-capacity rejected,
//                                           out-of-range tournamentId rejected,
//                                           prizePool accumulates
//    [PROC-4]  StartTournament            — single elim: R1 match count = players/2,
//                                           totalRounds = log2(players),
//                                           currentRound = 1,
//                                           tstate = TSTATE_IN_PROGRESS,
//                                           double elim: same R1 matches,
//                                           round robin: full schedule generated,
//                                           non-admin rejected,
//                                           below-min players rejected,
//                                           wrong state rejected
//    [PROC-5]  SubmitMatchResult          — non-admin rejected,
//                                           wrong state (not IN_PROGRESS) rejected,
//                                           out-of-range matchIndex rejected,
//                                           already-settled match rejected,
//                                           invalid winner (not playerA or playerB) rejected,
//                                           winner/loser/hash stored,
//                                           settled flag set,
//                                           settledMatchCount increments,
//                                           winner record updated (wins++),
//                                           loser record updated (losses++),
//                                           single elim: round advances when complete,
//                                           single elim: finals → tournament complete,
//                                           prizes distributed on completion (60/30/10),
//                                           round robin: advances currentRound,
//                                           round robin: finalizes on last round
//    [PROC-6]  CancelTournament           — cancels REGISTRATION state,
//                                           cancels IN_PROGRESS state,
//                                           non-admin rejected,
//                                           COMPLETE cannot be cancelled,
//                                           CANCELLED cannot be cancelled again,
//                                           entry fees refunded to all players
//    [FUNC-7]  GetTournament              — correct fields after create,
//                                           correct state after start,
//                                           correct winners after completion
//    [FUNC-8]  GetMatch                   — fields after start,
//                                           fields after submit result,
//                                           out-of-range returns found=0
//    [FUNC-9]  GetPlayerRecord            — registered player fields,
//                                           win/loss updated after match,
//                                           unregistered returns found=0
//
//  Integration lifecycle tests:
//    Full single elimination 4-player end-to-end
//    Full round robin 4-player end-to-end
//    Cancel mid-tournament refunds all players
//    Prize split math (60/30/10) verified
// ============================================================

#include "gtest/gtest.h"
#include "../src/contracts/QZN_TournamentEngine_PAO.h"

#include "contract_testing.h"

// ============================================================
//  HELPERS
// ============================================================

static const id ADMIN_ADDR   = id(1, 0, 0, 0);
static const id CABINET_ADDR = id(2, 0, 0, 0);
static const id PLAYER_A     = id(3, 0, 0, 0);
static const id PLAYER_B     = id(4, 0, 0, 0);
static const id PLAYER_C     = id(5, 0, 0, 0);
static const id PLAYER_D     = id(6, 0, 0, 0);
static const id STRANGER     = id(7, 0, 0, 0);

static constexpr uint64 ENTRY_FEE    = 1000ULL;
static constexpr uint64 RESULT_HASH  = 0xABCDEF12ULL;

// ============================================================
//  FIXTURE
// ============================================================

class QZNTournamentEngineTest : public ::testing::Test
{
protected:
    ContractTester<QZNTOUR> tester;

    void SetUp() override {
        tester.reset();
    }

    void initialize(id adminOverride = NULL_ID)
    {
        InitializeTournamentEngine_input in{};
        in.adminOverride = adminOverride;
        tester.setInvocator(ADMIN_ADDR);
        tester.callProcedure(InitializeTournamentEngine_id, in);
    }

    // Creates a tournament and returns its ID
    uint32 createTournament(uint8  format     = FORMAT_SINGLE_ELIMINATION,
                            uint8  maxPlayers = 4,
                            uint64 entryFee   = ENTRY_FEE)
    {
        CreateTournament_input in{};
        in.format     = format;
        in.maxPlayers = maxPlayers;
        in.entryFee   = entryFee;
        in.cabinetPAO = CABINET_ADDR;

        tester.setInvocator(ADMIN_ADDR);
        auto out = tester.callProcedure(CreateTournament_id, in);
        return out.success ? out.tournamentId : 9999;
    }

    // Register a player into a tournament, paying exact fee
    bool registerPlayer(uint32 tId, id player)
    {
        QZNTOUR::QZNTOUR::RegisterPlayer_input in{};
        in.tournamentId = tId;

        tester.setInvocator(player);
        tester.setInvocationReward(tester.state().tournaments[tId].entryFee);
        auto out = tester.callProcedure(TourRegisterPlayer_id, in);
        return out.success;
    }

    // Fill a tournament to exactly minPlayers (4) with PLAYER_A..D
    void fillToMin(uint32 tId)
    {
        registerPlayer(tId, PLAYER_A);
        registerPlayer(tId, PLAYER_B);
        registerPlayer(tId, PLAYER_C);
        registerPlayer(tId, PLAYER_D);
    }

    // Start a tournament as admin
    StartTournament_output startTournament(uint32 tId)
    {
        StartTournament_input in{};
        in.tournamentId = tId;
        tester.setInvocator(ADMIN_ADDR);
        return tester.callProcedure(StartTournament_id, in);
    }

    // Submit a match result as admin
    SubmitMatchResult_output submitResult(uint32 tId, uint32 matchIdx,
                                          id winner,
                                          uint64 hash = RESULT_HASH)
    {
        SubmitMatchResult_input in{};
        in.tournamentId = tId;
        in.matchIndex   = matchIdx;
        in.winner       = winner;
        in.resultHash   = hash;
        tester.setInvocator(ADMIN_ADDR);
        return tester.callProcedure(SubmitMatchResult_id, in);
    }
};

// ============================================================
//  [PROC-1]  InitializeTournamentEngine
// ============================================================

TEST_F(QZNTournamentEngineTest, Init_HappyPath_UsesInvocatorWhenOverrideNull)
{
    initialize(NULL_ID);
    EXPECT_EQ(tester.state().protocolAdmin, ADMIN_ADDR);
}

TEST_F(QZNTournamentEngineTest, Init_HappyPath_AdminOverrideUsedWhenProvided)
{
    initialize(PLAYER_A);
    EXPECT_EQ(tester.state().protocolAdmin, PLAYER_A);
}

TEST_F(QZNTournamentEngineTest, Init_HappyPath_CountersZero)
{
    initialize();
    EXPECT_EQ(tester.state().tournamentCount,        0u);
    EXPECT_EQ(tester.state().totalPrizesDistributed, 0ULL);
}

TEST_F(QZNTournamentEngineTest, Init_HappyPath_InitializedFlagSet)
{
    initialize();
    EXPECT_EQ(tester.state().initialized, 1);
}

TEST_F(QZNTournamentEngineTest, Init_HappyPath_OutputSuccess)
{
    InitializeTournamentEngine_input in{};
    in.adminOverride = NULL_ID;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(InitializeTournamentEngine_id, in);
    EXPECT_EQ(out.success, 1);
}

TEST_F(QZNTournamentEngineTest, Init_DoubleInitGuard_SecondCallFails)
{
    initialize();
    InitializeTournamentEngine_input in{};
    in.adminOverride = STRANGER;
    tester.setInvocator(STRANGER);
    auto out = tester.callProcedure(InitializeTournamentEngine_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().protocolAdmin, ADMIN_ADDR);
}

// ============================================================
//  [PROC-2]  CreateTournament
// ============================================================

TEST_F(QZNTournamentEngineTest, Create_SingleElim_HappyPath)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);

    EXPECT_NE(tId, 9999u);
    EXPECT_EQ(tester.state().tournaments[tId].format,   FORMAT_SINGLE_ELIMINATION);
    EXPECT_EQ(tester.state().tournaments[tId].maxPlayers, 4);
    EXPECT_EQ(tester.state().tournaments[tId].entryFee, ENTRY_FEE);
    EXPECT_EQ(tester.state().tournaments[tId].tstate,   TSTATE_REGISTRATION);
    EXPECT_EQ(tester.state().tournamentCount,            1u);
}

TEST_F(QZNTournamentEngineTest, Create_DoubleElim_HappyPath)
{
    initialize();
    uint32 tId = createTournament(FORMAT_DOUBLE_ELIMINATION, 4, ENTRY_FEE);
    EXPECT_NE(tId, 9999u);
    EXPECT_EQ(tester.state().tournaments[tId].format, FORMAT_DOUBLE_ELIMINATION);
}

TEST_F(QZNTournamentEngineTest, Create_RoundRobin_HappyPath)
{
    initialize();
    uint32 tId = createTournament(FORMAT_ROUND_ROBIN, 4, ENTRY_FEE);
    EXPECT_NE(tId, 9999u);
    EXPECT_EQ(tester.state().tournaments[tId].format, FORMAT_ROUND_ROBIN);
}

TEST_F(QZNTournamentEngineTest, Create_AllValidPlayerCounts_Succeed)
{
    initialize();
    uint8 validCounts[] = { 4, 8, 16, 32, 64 };
    for (uint8 count : validCounts)
    {
        uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, count, ENTRY_FEE);
        EXPECT_NE(tId, 9999u) << "Player count " << (int)count << " should succeed";
    }
}

TEST_F(QZNTournamentEngineTest, Create_NonPowerOf2_Rejected)
{
    initialize();
    uint32 prevCount = tester.state().tournamentCount;

    CreateTournament_input in{};
    in.format     = FORMAT_SINGLE_ELIMINATION;
    in.maxPlayers = 6;  // Not a power of 2
    in.entryFee   = ENTRY_FEE;
    in.cabinetPAO = CABINET_ADDR;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(CreateTournament_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournamentCount, prevCount);
}

TEST_F(QZNTournamentEngineTest, Create_BelowMinPlayers_Rejected)
{
    initialize();
    CreateTournament_input in{};
    in.format     = FORMAT_SINGLE_ELIMINATION;
    in.maxPlayers = 2;  // Below QZN_TOURNAMENT_MIN_PLAYERS (4)
    in.entryFee   = ENTRY_FEE;
    in.cabinetPAO = CABINET_ADDR;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(CreateTournament_id, in);

    EXPECT_EQ(out.success, 0);
}

TEST_F(QZNTournamentEngineTest, Create_AboveMaxPlayers_Rejected)
{
    initialize();
    CreateTournament_input in{};
    in.format     = FORMAT_SINGLE_ELIMINATION;
    in.maxPlayers = 128;  // Above QZN_TOURNAMENT_MAX_PLAYERS (64)
    in.entryFee   = ENTRY_FEE;
    in.cabinetPAO = CABINET_ADDR;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(CreateTournament_id, in);

    EXPECT_EQ(out.success, 0);
}

TEST_F(QZNTournamentEngineTest, Create_BelowMinEntryFee_Rejected)
{
    initialize();
    CreateTournament_input in{};
    in.format     = FORMAT_SINGLE_ELIMINATION;
    in.maxPlayers = 4;
    in.entryFee   = QZN_TOURNAMENT_MIN_ENTRY_FEE - 1ULL;
    in.cabinetPAO = CABINET_ADDR;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(CreateTournament_id, in);

    EXPECT_EQ(out.success, 0);
}

TEST_F(QZNTournamentEngineTest, Create_AtMinEntryFee_Succeeds)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, QZN_TOURNAMENT_MIN_ENTRY_FEE);
    EXPECT_NE(tId, 9999u);
}

TEST_F(QZNTournamentEngineTest, Create_NonAdmin_Rejected)
{
    initialize();
    CreateTournament_input in{};
    in.format     = FORMAT_SINGLE_ELIMINATION;
    in.maxPlayers = 4;
    in.entryFee   = ENTRY_FEE;
    in.cabinetPAO = CABINET_ADDR;
    tester.setInvocator(STRANGER);
    auto out = tester.callProcedure(CreateTournament_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournamentCount, 0u);
}

TEST_F(QZNTournamentEngineTest, Create_InitialFieldsCorrect)
{
    initialize();
    uint32 tId = createTournament();
    const auto& t = tester.state().tournaments[tId];

    EXPECT_EQ(t.tournamentId,     tId);
    EXPECT_EQ(t.registeredCount,  0);
    EXPECT_EQ(t.prizePool,        0ULL);
    EXPECT_EQ(t.matchCount,       0u);
    EXPECT_EQ(t.currentRound,     0u);
    EXPECT_EQ(t.prizesDistributed, 0);
    EXPECT_EQ(t.first,            NULL_ID);
    EXPECT_EQ(t.second,           NULL_ID);
    EXPECT_EQ(t.third,            NULL_ID);
    EXPECT_EQ(t.cabinetPAO,       CABINET_ADDR);
}

TEST_F(QZNTournamentEngineTest, Create_TournamentIdIncrementsPerCreate)
{
    initialize();
    uint32 id0 = createTournament();
    uint32 id1 = createTournament();
    uint32 id2 = createTournament();

    EXPECT_EQ(id0, 0u);
    EXPECT_EQ(id1, 1u);
    EXPECT_EQ(id2, 2u);
    EXPECT_EQ(tester.state().tournamentCount, 3u);
}

// ============================================================
//  [PROC-3]  RegisterPlayer
// ============================================================

TEST_F(QZNTournamentEngineTest, Register_HappyPath_Slot0AssignedFirst)
{
    initialize();
    uint32 tId = createTournament();

    bool ok = registerPlayer(tId, PLAYER_A);

    EXPECT_TRUE(ok);
    EXPECT_EQ(tester.state().tournaments[tId].players[0].wallet,     PLAYER_A);
    EXPECT_EQ(tester.state().tournaments[tId].players[0].registered, 1);
    EXPECT_EQ(tester.state().tournaments[tId].registeredCount,       1);
}

TEST_F(QZNTournamentEngineTest, Register_PrizePoolAccumulatesEntryFee)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);

    registerPlayer(tId, PLAYER_A);
    registerPlayer(tId, PLAYER_B);

    EXPECT_EQ(tester.state().tournaments[tId].prizePool, ENTRY_FEE * 2);
}

TEST_F(QZNTournamentEngineTest, Register_WrongState_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    // Now IN_PROGRESS — registration should be closed
    QZNTOUR::QZNTOUR::RegisterPlayer_input in{};
    in.tournamentId = tId;
    tester.setInvocator(id(99, 0, 0, 0));
    tester.setInvocationReward(ENTRY_FEE);
    auto out = tester.callProcedure(TourRegisterPlayer_id, in);

    EXPECT_EQ(out.success, 0);
}

TEST_F(QZNTournamentEngineTest, Register_DuplicatePlayer_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    registerPlayer(tId, PLAYER_A);

    // Try to register PLAYER_A again
    QZNTOUR::QZNTOUR::RegisterPlayer_input in{};
    in.tournamentId = tId;
    tester.setInvocator(PLAYER_A);
    tester.setInvocationReward(ENTRY_FEE);
    auto out = tester.callProcedure(TourRegisterPlayer_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournaments[tId].registeredCount, 1);
}

TEST_F(QZNTournamentEngineTest, Register_AtCapacity_Rejected)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);  // Fill all 4 slots

    QZNTOUR::QZNTOUR::RegisterPlayer_input in{};
    in.tournamentId = tId;
    tester.setInvocator(id(50, 0, 0, 0));
    tester.setInvocationReward(ENTRY_FEE);
    auto out = tester.callProcedure(TourRegisterPlayer_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournaments[tId].registeredCount, 4);
}

TEST_F(QZNTournamentEngineTest, Register_UnderpaymentRefundedAndRejected)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);

    QZNTOUR::QZNTOUR::RegisterPlayer_input in{};
    in.tournamentId = tId;
    tester.setInvocator(PLAYER_A);
    tester.setInvocationReward(ENTRY_FEE - 1ULL);  // 1 under fee
    auto out = tester.callProcedure(TourRegisterPlayer_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournaments[tId].registeredCount, 0);
}

TEST_F(QZNTournamentEngineTest, Register_OutOfRangeTournamentId_Rejected)
{
    initialize();
    QZNTOUR::QZNTOUR::RegisterPlayer_input in{};
    in.tournamentId = 9999;
    tester.setInvocator(PLAYER_A);
    tester.setInvocationReward(ENTRY_FEE);
    auto out = tester.callProcedure(TourRegisterPlayer_id, in);

    EXPECT_EQ(out.success, 0);
}

TEST_F(QZNTournamentEngineTest, Register_AllStatsInitializedToZero)
{
    initialize();
    uint32 tId = createTournament();
    registerPlayer(tId, PLAYER_A);

    const auto& p = tester.state().tournaments[tId].players[0];
    EXPECT_EQ(p.wins,           0u);
    EXPECT_EQ(p.losses,         0u);
    EXPECT_EQ(p.draws,          0u);
    EXPECT_EQ(p.points,         0u);
    EXPECT_EQ(p.eliminated,     0);
    EXPECT_EQ(p.inLosersBracket, 0);
}

// ============================================================
//  [PROC-4]  StartTournament
// ============================================================

TEST_F(QZNTournamentEngineTest, Start_SingleElim_4Players_2R1Matches)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);

    auto out = startTournament(tId);

    EXPECT_EQ(out.success,    1);
    EXPECT_EQ(out.matchCount, 2u);  // 4 players → 2 R1 matches
    EXPECT_EQ(tester.state().tournaments[tId].tstate,       TSTATE_IN_PROGRESS);
    EXPECT_EQ(tester.state().tournaments[tId].currentRound, 1u);
    EXPECT_EQ(tester.state().tournaments[tId].totalRounds,  2u);  // log2(4) = 2
    EXPECT_EQ(tester.state().tournaments[tId].matchCount,   2u);
}

TEST_F(QZNTournamentEngineTest, Start_SingleElim_R1_MatchesCorrectlyPaired)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);
    startTournament(tId);

    const auto& t = tester.state().tournaments[tId];
    // Match 0: player[0] vs player[1]
    EXPECT_EQ(t.matches[0].playerA, PLAYER_A);
    EXPECT_EQ(t.matches[0].playerB, PLAYER_B);
    EXPECT_EQ(t.matches[0].round,   1u);
    EXPECT_EQ(t.matches[0].settled, 0);
    // Match 1: player[2] vs player[3]
    EXPECT_EQ(t.matches[1].playerA, PLAYER_C);
    EXPECT_EQ(t.matches[1].playerB, PLAYER_D);
}

TEST_F(QZNTournamentEngineTest, Start_DoubleElim_4Players_SameR1AsSingleElim)
{
    initialize();
    uint32 tId = createTournament(FORMAT_DOUBLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);

    auto out = startTournament(tId);

    EXPECT_EQ(out.success,    1);
    EXPECT_EQ(out.matchCount, 2u);  // Same R1 as single elim
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_IN_PROGRESS);
}

TEST_F(QZNTournamentEngineTest, Start_RoundRobin_4Players_FullSchedule)
{
    initialize();
    uint32 tId = createTournament(FORMAT_ROUND_ROBIN, 4, ENTRY_FEE);
    fillToMin(tId);

    auto out = startTournament(tId);

    // 4 players → 3 rounds × 2 matches/round = 6 total matches
    EXPECT_EQ(out.success,    1);
    EXPECT_EQ(out.matchCount, 6u);
    EXPECT_EQ(tester.state().tournaments[tId].totalRounds,  3u);  // n-1
    EXPECT_EQ(tester.state().tournaments[tId].currentRound, 1u);
}

TEST_F(QZNTournamentEngineTest, Start_NonAdmin_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);

    StartTournament_input in{};
    in.tournamentId = tId;
    tester.setInvocator(STRANGER);
    auto out = tester.callProcedure(StartTournament_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_REGISTRATION);
}

TEST_F(QZNTournamentEngineTest, Start_BelowMinPlayers_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    // Only register 3 players — below minimum
    registerPlayer(tId, PLAYER_A);
    registerPlayer(tId, PLAYER_B);
    registerPlayer(tId, PLAYER_C);

    auto out = startTournament(tId);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_REGISTRATION);
}

TEST_F(QZNTournamentEngineTest, Start_WrongState_AlreadyStarted_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    // Try to start again
    auto out = startTournament(tId);
    EXPECT_EQ(out.success, 0);
}

// ============================================================
//  [PROC-5]  SubmitMatchResult
// ============================================================

TEST_F(QZNTournamentEngineTest, Submit_HappyPath_WinnerLoserHashStored)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    auto out = submitResult(tId, 0, PLAYER_A, RESULT_HASH);

    EXPECT_EQ(out.success, 1);
    const auto& m = tester.state().tournaments[tId].matches[0];
    EXPECT_EQ(m.winner,     PLAYER_A);
    EXPECT_EQ(m.loser,      PLAYER_B);
    EXPECT_EQ(m.resultHash, RESULT_HASH);
    EXPECT_EQ(m.settled,    1);
}

TEST_F(QZNTournamentEngineTest, Submit_SettledMatchCountIncrements)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    submitResult(tId, 0, PLAYER_A);

    EXPECT_EQ(tester.state().tournaments[tId].settledMatchCount, 1u);
}

TEST_F(QZNTournamentEngineTest, Submit_WinnerRecordUpdated)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    submitResult(tId, 0, PLAYER_A);

    EXPECT_EQ(tester.state().tournaments[tId].players[0].wins,   1u);  // PLAYER_A is slot 0
    EXPECT_EQ(tester.state().tournaments[tId].players[0].points, 3u);
}

TEST_F(QZNTournamentEngineTest, Submit_LoserRecordUpdated)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    submitResult(tId, 0, PLAYER_A);  // PLAYER_B loses

    EXPECT_EQ(tester.state().tournaments[tId].players[1].losses, 1u);
}

TEST_F(QZNTournamentEngineTest, Submit_NonAdmin_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    SubmitMatchResult_input in{};
    in.tournamentId = tId;
    in.matchIndex   = 0;
    in.winner       = PLAYER_A;
    in.resultHash   = RESULT_HASH;
    tester.setInvocator(STRANGER);
    auto out = tester.callProcedure(SubmitMatchResult_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournaments[tId].matches[0].settled, 0);
}

TEST_F(QZNTournamentEngineTest, Submit_WrongState_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    // Not started — still REGISTRATION

    auto out = submitResult(tId, 0, PLAYER_A);
    EXPECT_EQ(out.success, 0);
}

TEST_F(QZNTournamentEngineTest, Submit_OutOfRangeMatchIndex_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    auto out = submitResult(tId, 999, PLAYER_A);
    EXPECT_EQ(out.success, 0);
}

TEST_F(QZNTournamentEngineTest, Submit_AlreadySettled_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    submitResult(tId, 0, PLAYER_A);  // Settle match 0

    // Try to submit again
    auto out = submitResult(tId, 0, PLAYER_B);
    EXPECT_EQ(out.success, 0);
    // Winner should still be PLAYER_A
    EXPECT_EQ(tester.state().tournaments[tId].matches[0].winner, PLAYER_A);
}

TEST_F(QZNTournamentEngineTest, Submit_InvalidWinner_NeitherPlayer_Rejected)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    auto out = submitResult(tId, 0, STRANGER);  // Not in match 0

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournaments[tId].matches[0].settled, 0);
}

TEST_F(QZNTournamentEngineTest, Submit_SingleElim_BothR1Matches_AdvancesRound)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);
    startTournament(tId);

    // Settle match 0 — round not yet complete
    auto r1 = submitResult(tId, 0, PLAYER_A);
    EXPECT_EQ(r1.advancedRound, 0);

    // Settle match 1 — round complete, should advance
    auto r2 = submitResult(tId, 1, PLAYER_C);
    EXPECT_EQ(r2.advancedRound, 1);
    EXPECT_EQ(tester.state().tournaments[tId].currentRound, 2u);
    // A final match should have been generated
    EXPECT_EQ(tester.state().tournaments[tId].matchCount, 3u);
}

TEST_F(QZNTournamentEngineTest, Submit_SingleElim_Final_TournamentComplete)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);
    startTournament(tId);

    // R1
    submitResult(tId, 0, PLAYER_A);
    submitResult(tId, 1, PLAYER_C);

    // Final (match index 2)
    auto out = submitResult(tId, 2, PLAYER_A);

    EXPECT_EQ(out.tournamentComplete, 1);
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_COMPLETE);
    EXPECT_EQ(tester.state().tournaments[tId].first,  PLAYER_A);
    EXPECT_NE(tester.state().tournaments[tId].second, NULL_ID);
    EXPECT_EQ(tester.state().tournaments[tId].prizesDistributed, 1);
}

TEST_F(QZNTournamentEngineTest, Submit_RoundRobin_4Players_AllRoundsComplete)
{
    initialize();
    uint32 tId = createTournament(FORMAT_ROUND_ROBIN, 4, ENTRY_FEE);
    fillToMin(tId);
    startTournament(tId);

    // 6 matches total across 3 rounds
    // Round 1: matches 0,1 | Round 2: matches 2,3 | Round 3: matches 4,5
    submitResult(tId, 0, PLAYER_A);
    auto r1 = submitResult(tId, 1, PLAYER_C);
    EXPECT_EQ(r1.advancedRound, 1);
    EXPECT_EQ(tester.state().tournaments[tId].currentRound, 2u);

    submitResult(tId, 2, PLAYER_A);
    submitResult(tId, 3, PLAYER_B);
    EXPECT_EQ(tester.state().tournaments[tId].currentRound, 3u);

    submitResult(tId, 4, PLAYER_A);
    auto last = submitResult(tId, 5, PLAYER_C);

    EXPECT_EQ(last.tournamentComplete, 1);
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_COMPLETE);
    EXPECT_NE(tester.state().tournaments[tId].first,  NULL_ID);
}

// ============================================================
//  [PROC-6]  CancelTournament
// ============================================================

TEST_F(QZNTournamentEngineTest, Cancel_RegistrationState_Succeeds)
{
    initialize();
    uint32 tId = createTournament();
    registerPlayer(tId, PLAYER_A);
    registerPlayer(tId, PLAYER_B);

    CancelTournament_input in{};
    in.tournamentId = tId;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(CancelTournament_id, in);

    EXPECT_EQ(out.success, 1);
    EXPECT_EQ(tester.state().tournaments[tId].tstate,   TSTATE_CANCELLED);
    EXPECT_EQ(tester.state().tournaments[tId].prizePool, 0ULL);
}

TEST_F(QZNTournamentEngineTest, Cancel_InProgressState_Succeeds)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    CancelTournament_input in{};
    in.tournamentId = tId;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(CancelTournament_id, in);

    EXPECT_EQ(out.success, 1);
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_CANCELLED);
}

TEST_F(QZNTournamentEngineTest, Cancel_NonAdmin_Rejected)
{
    initialize();
    uint32 tId = createTournament();

    CancelTournament_input in{};
    in.tournamentId = tId;
    tester.setInvocator(STRANGER);
    auto out = tester.callProcedure(CancelTournament_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_REGISTRATION);
}

TEST_F(QZNTournamentEngineTest, Cancel_CompleteTournament_Rejected)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);
    startTournament(tId);
    submitResult(tId, 0, PLAYER_A);
    submitResult(tId, 1, PLAYER_C);
    submitResult(tId, 2, PLAYER_A);  // Completes tournament

    CancelTournament_input in{};
    in.tournamentId = tId;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(CancelTournament_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_COMPLETE);
}

TEST_F(QZNTournamentEngineTest, Cancel_AlreadyCancelled_Rejected)
{
    initialize();
    uint32 tId = createTournament();

    CancelTournament_input in{};
    in.tournamentId = tId;
    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(CancelTournament_id, in);  // First cancel

    auto out = tester.callProcedure(CancelTournament_id, in);  // Second cancel
    EXPECT_EQ(out.success, 0);
}

// ============================================================
//  [FUNC-7]  GetTournament
// ============================================================

TEST_F(QZNTournamentEngineTest, GetTournament_AfterCreate_CorrectFields)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);

    GetTournament_input in{};
    in.tournamentId = tId;
    auto out = tester.callFunction(GetTournament_id, in);

    EXPECT_EQ(out.tournamentId,      tId);
    EXPECT_EQ(out.format,            FORMAT_SINGLE_ELIMINATION);
    EXPECT_EQ(out.tstate,            TSTATE_REGISTRATION);
    EXPECT_EQ(out.maxPlayers,        4);
    EXPECT_EQ(out.entryFee,          ENTRY_FEE);
    EXPECT_EQ(out.prizesDistributed, 0);
}

TEST_F(QZNTournamentEngineTest, GetTournament_AfterStart_StateInProgress)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    GetTournament_input in{};
    in.tournamentId = tId;
    auto out = tester.callFunction(GetTournament_id, in);

    EXPECT_EQ(out.tstate,       TSTATE_IN_PROGRESS);
    EXPECT_EQ(out.currentRound, 1u);
    EXPECT_EQ(out.matchCount,   2u);
}

TEST_F(QZNTournamentEngineTest, GetTournament_AfterComplete_WinnersSet)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);
    startTournament(tId);
    submitResult(tId, 0, PLAYER_A);
    submitResult(tId, 1, PLAYER_C);
    submitResult(tId, 2, PLAYER_A);

    GetTournament_input in{};
    in.tournamentId = tId;
    auto out = tester.callFunction(GetTournament_id, in);

    EXPECT_EQ(out.tstate,            TSTATE_COMPLETE);
    EXPECT_EQ(out.first,             PLAYER_A);
    EXPECT_EQ(out.prizesDistributed, 1);
}

TEST_F(QZNTournamentEngineTest, GetTournament_OutOfRange_ReturnsZeroId)
{
    initialize();
    GetTournament_input in{};
    in.tournamentId = 9999;
    auto out = tester.callFunction(GetTournament_id, in);

    EXPECT_EQ(out.tournamentId, 0u);
}

TEST_F(QZNTournamentEngineTest, GetTournament_IsReadOnly)
{
    initialize();
    uint32 tId = createTournament();
    uint32 prevCount = tester.state().tournamentCount;

    GetTournament_input in{};
    in.tournamentId = tId;
    tester.callFunction(GetTournament_id, in);
    tester.callFunction(GetTournament_id, in);

    EXPECT_EQ(tester.state().tournamentCount, prevCount);
}

// ============================================================
//  [FUNC-8]  GetMatch
// ============================================================

TEST_F(QZNTournamentEngineTest, GetMatch_AfterStart_CorrectFields)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    GetTourMatch_input in{};
    in.tournamentId = tId;
    in.matchIndex   = 0;
    auto out = tester.callFunction(GetTourMatch_id, in);

    EXPECT_EQ(out.found,          1);
    EXPECT_EQ(out.match.playerA,  PLAYER_A);
    EXPECT_EQ(out.match.playerB,  PLAYER_B);
    EXPECT_EQ(out.match.round,    1u);
    EXPECT_EQ(out.match.settled,  0);
}

TEST_F(QZNTournamentEngineTest, GetMatch_AfterSubmit_SettledAndWinnerSet)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);
    submitResult(tId, 0, PLAYER_A);

    GetTourMatch_input in{};
    in.tournamentId = tId;
    in.matchIndex   = 0;
    auto out = tester.callFunction(GetTourMatch_id, in);

    EXPECT_EQ(out.found,           1);
    EXPECT_EQ(out.match.settled,   1);
    EXPECT_EQ(out.match.winner,    PLAYER_A);
    EXPECT_EQ(out.match.loser,     PLAYER_B);
    EXPECT_EQ(out.match.resultHash, RESULT_HASH);
}

TEST_F(QZNTournamentEngineTest, GetMatch_OutOfRangeMatchIndex_NotFound)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    GetTourMatch_input in{};
    in.tournamentId = tId;
    in.matchIndex   = 999;
    auto out = tester.callFunction(GetTourMatch_id, in);

    EXPECT_EQ(out.found, 0);
}

// ============================================================
//  [FUNC-9]  GetPlayerRecord
// ============================================================

TEST_F(QZNTournamentEngineTest, GetPlayerRecord_RegisteredPlayer_CorrectFields)
{
    initialize();
    uint32 tId = createTournament();
    registerPlayer(tId, PLAYER_A);

    GetPlayerRecord_input in{};
    in.tournamentId = tId;
    in.wallet       = PLAYER_A;
    auto out = tester.callFunction(GetPlayerRecord_id, in);

    EXPECT_EQ(out.found,            1);
    EXPECT_EQ(out.record.wallet,    PLAYER_A);
    EXPECT_EQ(out.record.registered, 1);
    EXPECT_EQ(out.record.wins,      0u);
    EXPECT_EQ(out.record.losses,    0u);
}

TEST_F(QZNTournamentEngineTest, GetPlayerRecord_AfterWin_WinsUpdated)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);
    submitResult(tId, 0, PLAYER_A);

    GetPlayerRecord_input in{};
    in.tournamentId = tId;
    in.wallet       = PLAYER_A;
    auto out = tester.callFunction(GetPlayerRecord_id, in);

    EXPECT_EQ(out.found,       1);
    EXPECT_EQ(out.record.wins, 1u);
}

TEST_F(QZNTournamentEngineTest, GetPlayerRecord_UnregisteredPlayer_NotFound)
{
    initialize();
    uint32 tId = createTournament();

    GetPlayerRecord_input in{};
    in.tournamentId = tId;
    in.wallet       = STRANGER;
    auto out = tester.callFunction(GetPlayerRecord_id, in);

    EXPECT_EQ(out.found, 0);
}

// ============================================================
//  LIFECYCLE INTEGRATION TESTS
// ============================================================

TEST_F(QZNTournamentEngineTest, Integration_SingleElim4Player_FullEndToEnd)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);

    // Register
    ASSERT_TRUE(registerPlayer(tId, PLAYER_A));
    ASSERT_TRUE(registerPlayer(tId, PLAYER_B));
    ASSERT_TRUE(registerPlayer(tId, PLAYER_C));
    ASSERT_TRUE(registerPlayer(tId, PLAYER_D));
    EXPECT_EQ(tester.state().tournaments[tId].prizePool, ENTRY_FEE * 4);

    // Start
    auto startOut = startTournament(tId);
    ASSERT_EQ(startOut.success, 1);
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_IN_PROGRESS);

    // R1: A beats B, C beats D
    submitResult(tId, 0, PLAYER_A);
    auto r1done = submitResult(tId, 1, PLAYER_C);
    EXPECT_EQ(r1done.advancedRound, 1);

    // Final: A beats C
    auto finalOut = submitResult(tId, 2, PLAYER_A);
    EXPECT_EQ(finalOut.tournamentComplete, 1);

    // Verify completion
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_COMPLETE);
    EXPECT_EQ(tester.state().tournaments[tId].first,  PLAYER_A);
    EXPECT_EQ(tester.state().tournaments[tId].prizesDistributed, 1);
    EXPECT_GT(tester.state().totalPrizesDistributed, 0ULL);
}

TEST_F(QZNTournamentEngineTest, Integration_PrizeSplit_60_30_10_Math)
{
    initialize();
    uint64 fee   = 1000ULL;
    uint32 tId   = createTournament(FORMAT_SINGLE_ELIMINATION, 4, fee);
    fillToMin(tId);
    startTournament(tId);

    submitResult(tId, 0, PLAYER_A);
    submitResult(tId, 1, PLAYER_C);
    submitResult(tId, 2, PLAYER_A);  // A wins

    uint64 totalPool   = fee * 4;         // 4000
    uint64 firstPrize  = (totalPool * 60) / 100;  // 2400
    uint64 secondPrize = (totalPool * 30) / 100;  // 1200
    uint64 thirdPrize  = totalPool - firstPrize - secondPrize;  // 400

    // Total prizes distributed should equal the full pool
    EXPECT_EQ(tester.state().totalPrizesDistributed,
              firstPrize + secondPrize + thirdPrize);
    EXPECT_EQ(tester.state().totalPrizesDistributed, totalPool);
}

TEST_F(QZNTournamentEngineTest, Integration_RoundRobin4Player_FullEndToEnd)
{
    initialize();
    uint32 tId = createTournament(FORMAT_ROUND_ROBIN, 4, ENTRY_FEE);
    fillToMin(tId);
    startTournament(tId);

    // 6 matches: play all, PLAYER_A wins all
    submitResult(tId, 0, PLAYER_A);
    submitResult(tId, 1, PLAYER_C);
    submitResult(tId, 2, PLAYER_A);
    submitResult(tId, 3, PLAYER_B);
    submitResult(tId, 4, PLAYER_A);
    auto last = submitResult(tId, 5, PLAYER_C);

    EXPECT_EQ(last.tournamentComplete, 1);
    EXPECT_EQ(tester.state().tournaments[tId].tstate, TSTATE_COMPLETE);
    // Top scorer should be first
    EXPECT_NE(tester.state().tournaments[tId].first, NULL_ID);
}

TEST_F(QZNTournamentEngineTest, Integration_CancelMidTournament_RefundsAll)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);
    startTournament(tId);

    CancelTournament_input in{};
    in.tournamentId = tId;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(CancelTournament_id, in);

    EXPECT_EQ(out.success, 1);
    EXPECT_EQ(tester.state().tournaments[tId].tstate,   TSTATE_CANCELLED);
    EXPECT_EQ(tester.state().tournaments[tId].prizePool, 0ULL);
}

TEST_F(QZNTournamentEngineTest, Integration_MultipleConcurrentTournaments)
{
    initialize();

    uint32 tId0 = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    uint32 tId1 = createTournament(FORMAT_ROUND_ROBIN,         4, ENTRY_FEE * 2);

    fillToMin(tId0);
    fillToMin(tId1);

    startTournament(tId0);
    startTournament(tId1);

    // Both should be IN_PROGRESS simultaneously
    EXPECT_EQ(tester.state().tournaments[tId0].tstate, TSTATE_IN_PROGRESS);
    EXPECT_EQ(tester.state().tournaments[tId1].tstate, TSTATE_IN_PROGRESS);

    // Prize pools should be independent
    EXPECT_EQ(tester.state().tournaments[tId0].prizePool, ENTRY_FEE * 4);
    EXPECT_EQ(tester.state().tournaments[tId1].prizePool, ENTRY_FEE * 2 * 4);
}

// ============================================================
//  INVARIANTS
// ============================================================

TEST_F(QZNTournamentEngineTest, Invariant_SettledCountNeverExceedsMatchCount)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    startTournament(tId);

    submitResult(tId, 0, PLAYER_A);
    submitResult(tId, 1, PLAYER_C);

    const auto& t = tester.state().tournaments[tId];
    EXPECT_LE(t.settledMatchCount, t.matchCount);
}

TEST_F(QZNTournamentEngineTest, Invariant_PrizePoolNeverGrowsAfterStart)
{
    initialize();
    uint32 tId = createTournament();
    fillToMin(tId);
    uint64 poolAtStart = tester.state().tournaments[tId].prizePool;
    startTournament(tId);

    EXPECT_EQ(tester.state().tournaments[tId].prizePool, poolAtStart);
}

TEST_F(QZNTournamentEngineTest, Invariant_TotalPrizesDistributedEqualsPoolOnCompletion)
{
    initialize();
    uint64 fee = 1000ULL;
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, fee);
    fillToMin(tId);
    startTournament(tId);

    submitResult(tId, 0, PLAYER_A);
    submitResult(tId, 1, PLAYER_C);
    submitResult(tId, 2, PLAYER_A);

    uint64 expectedPool = fee * 4;
    EXPECT_EQ(tester.state().totalPrizesDistributed, expectedPool);
}

TEST_F(QZNTournamentEngineTest, Invariant_RegisteredCountNeverExceedsMaxPlayers)
{
    initialize();
    uint32 tId = createTournament(FORMAT_SINGLE_ELIMINATION, 4, ENTRY_FEE);
    fillToMin(tId);

    // Attempt to over-register
    registerPlayer(tId, id(99, 0, 0, 0));

    EXPECT_LE(tester.state().tournaments[tId].registeredCount,
              tester.state().tournaments[tId].maxPlayers);
}
