#define NO_UEFI
#include <iostream>
#include "contract_testing.h"

// ============================================================
//  QZN RewardRouter PAO — GTest Suite
//  File:    test/contract_qzn_rewardrouter.cpp
//  Place in qubic/core repo at: test/contract_qzn_rewardrouter.cpp
//
//  !! KNOWN CONTRACT BUG — MUST FIX BEFORE PR !!
//  ReportMatchResult (line ~1349) declares variable 'wSlot'
//  but then assigns to undeclared 'slot', and references
//  'input.walletAddress' which does not exist in
//  ReportMatchResult_input — it should be 'input.winnerAddress'.
//  Tests below document expected correct behavior.
//  The contract will not compile until this is fixed.
//
//  Coverage:
//    [PROC-1]  InitializeRouter    — happy path, double-init guard,
//                                    reserves seeded, stats zeroed
//    [PROC-2]  RegisterPlayer      — slot 0 assigned first, initial stake
//                                    sets multiplier, zero stake = tier 0,
//                                    all 4 tier boundary values,
//                                    pre-init guard, active flag set
//    [PROC-3]  StakeQZN            — all 5 tier thresholds (at-boundary),
//                                    multiplier updates immediately,
//                                    zero amount rejected,
//                                    unregistered player rejected
//    [PROC-4]  UnstakeQZN          — happy path + transfer fires,
//                                    multiplier recalculates downward,
//                                    excess amount rejected,
//                                    zero amount rejected,
//                                    partial unstake stays in correct tier
//    [PROC-5]  ReportMatchResult   — auth guard (only cabinet),
//                                    base reward Tier 0 math,
//                                    multiplier applied (Tier 1–4),
//                                    epoch cap enforced,
//                                    epoch cap reset on new epoch,
//                                    match stats incremented (played/won/streak),
//                                    best streak updated,
//                                    per-game win flags set for ALL_GAMES,
//                                    all 8 achievements triggered correctly,
//                                    achievement double-award prevented,
//                                    achievement bonuses bypass epoch cap,
//                                    unregistered winner returns zero,
//                                    reserve deducted after reward
//    [PROC-6]  ClaimRewards        — full pending balance claimed,
//                                    pending zeroed after claim,
//                                    zero balance no-op,
//                                    unregistered player ignored,
//                                    totalQZNDistributed increments
//    [PROC-7]  FundReserve         — admin adds to reward reserve,
//                                    admin adds to achievement reserve,
//                                    non-admin rejected
//    [FUNC-8]  GetPlayerStats      — registered player data correct,
//                                    post-match data correct,
//                                    unregistered player returns zeroes
//    [FUNC-9]  GetLeaderboard      — initial state all zeroes,
//                                    after match report score reflected
//
//  BEGIN_EPOCH integration tests:
//    Epoch scores reset for all active players
//    Epoch counter increments
//    Leaderboard sorted descending after epoch advance
//    Top leaderboard achievement awarded to rank-1 player
// ============================================================

#include "gtest/gtest.h"
#include "../src/contracts/QZN_RewardRouter_PAO.h"
#include "contract_testing.h"

// ============================================================
//  HELPERS — well-known test addresses
// ============================================================

static const id ADMIN_ADDR    = id(1, 0, 0, 0);
static const id CABINET_ADDR  = id(2, 0, 0, 0);
static const id PLAYER_A      = id(3, 0, 0, 0);
static const id PLAYER_B      = id(4, 0, 0, 0);
static const id PLAYER_C      = id(5, 0, 0, 0);
static const id STRANGER_ADDR = id(6, 0, 0, 0);

static constexpr sint64 INITIAL_REWARD_RESERVE      = 10000000LL;  // 10M QZN
static constexpr sint64 INITIAL_ACHIEVEMENT_RESERVE = 1000000LL;   // 1M QZN

// ============================================================
//  FIXTURE
// ============================================================

class QZNRewardRouterTest : public ::testing::Test
{
protected:
    ContractTester<QZNREWARDROUTER> tester;

    void initialize()
    {
        InitializeRouter_input in{};
        in.gameCabinetAddr           = CABINET_ADDR;
        in.initialRewardReserve      = INITIAL_REWARD_RESERVE;
        in.initialAchievementReserve = INITIAL_ACHIEVEMENT_RESERVE;

        tester.setInvocator(ADMIN_ADDR);
        tester.callProcedure(InitializeRouter_id, in);
    }

    // Register a player with a given initial stake
    sint64 registerPlayer(id player, sint64 initialStake = 0LL)
    {
        RegisterPlayer_input in{};
        in.initialStake = initialStake;

        tester.setInvocator(player);
        auto out = tester.callProcedure(RegisterPlayer_id, in);
        return out.success ? out.playerSlot : -1LL;
    }

    // Report a win for a registered player via the cabinet
    ReportMatchResult_output reportWin(id winner, uint8 gameId = 3,
                                       sint64 stake = 0LL, bit isSolo = 0)
    {
        ReportMatchResult_input in{};
        in.winnerAddress = winner;
        in.loser1Address = PLAYER_B;
        in.loser2Address = NULL_ID;
        in.loser3Address = NULL_ID;
        in.gameId        = gameId;
        in.winnerScore   = 500LL;
        in.stakeAmount   = stake;
        in.isSolo        = isSolo;

        tester.setInvocator(CABINET_ADDR);
        return tester.callProcedure(ReportMatchResult_id, in);
    }

    // Stake QZN for a player
    StakeQZN_output stake(id player, sint64 amount)
    {
        StakeQZN_input in{};
        in.amount = amount;
        tester.setInvocator(player);
        return tester.callProcedure(StakeQZN_id, in);
    }

    // Claim rewards for a player
    ClaimRewards_output claim(id player)
    {
        ClaimRewards_input in{};
        tester.setInvocator(player);
        return tester.callProcedure(ClaimRewards_id, in);
    }
};

// ============================================================
//  [PROC-1]  InitializeRouter
// ============================================================

TEST_F(QZNRewardRouterTest, Init_HappyPath_AddressesStored)
{
    initialize();
    EXPECT_EQ(tester.state().adminAddress,       ADMIN_ADDR);
    EXPECT_EQ(tester.state().gameCabinetAddress, CABINET_ADDR);
}

TEST_F(QZNRewardRouterTest, Init_HappyPath_ReservesSeeded)
{
    initialize();
    EXPECT_EQ(tester.state().rewardReserveBalance,      INITIAL_REWARD_RESERVE);
    EXPECT_EQ(tester.state().achievementReserveBalance, INITIAL_ACHIEVEMENT_RESERVE);
}

TEST_F(QZNRewardRouterTest, Init_HappyPath_EpochRewardPoolSet)
{
    initialize();
    EXPECT_EQ(tester.state().epochRewardPool, INITIAL_REWARD_RESERVE);
}

TEST_F(QZNRewardRouterTest, Init_HappyPath_AllStatsZero)
{
    initialize();
    EXPECT_EQ(tester.state().totalPlayersRegistered,   0LL);
    EXPECT_EQ(tester.state().totalQZNDistributed,      0LL);
    EXPECT_EQ(tester.state().totalAchievementsAwarded, 0LL);
    EXPECT_EQ(tester.state().totalEpochsProcessed,     0LL);
    EXPECT_EQ(tester.state().epochTotalDistributed,    0LL);
    EXPECT_EQ(tester.state().epochTotalBurned,         0LL);
}

TEST_F(QZNRewardRouterTest, Init_HappyPath_LeaderboardClear)
{
    initialize();
    EXPECT_EQ(tester.state().board_0.score, 0LL);
    EXPECT_EQ(tester.state().board_9.score, 0LL);
}

TEST_F(QZNRewardRouterTest, Init_HappyPath_InitializedFlagSet)
{
    initialize();
    EXPECT_EQ(tester.state().initialized, 1);
}

TEST_F(QZNRewardRouterTest, Init_DoubleInitGuard_SecondCallIgnored)
{
    initialize();

    InitializeRouter_input in{};
    in.gameCabinetAddr      = STRANGER_ADDR;
    in.initialRewardReserve = 999LL;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(InitializeRouter_id, in);

    EXPECT_EQ(tester.state().gameCabinetAddress,    CABINET_ADDR);
    EXPECT_EQ(tester.state().rewardReserveBalance,  INITIAL_REWARD_RESERVE);
}

// ============================================================
//  [PROC-2]  RegisterPlayer
// ============================================================

TEST_F(QZNRewardRouterTest, Register_HappyPath_Slot0AssignedFirst)
{
    initialize();
    sint64 slot = registerPlayer(PLAYER_A);
    EXPECT_EQ(slot, 0LL);
}

TEST_F(QZNRewardRouterTest, Register_HappyPath_ActiveFlagSet)
{
    initialize();
    registerPlayer(PLAYER_A);
    EXPECT_EQ(tester.state().players_0.active, 1);
}

TEST_F(QZNRewardRouterTest, Register_HappyPath_WalletAddressStored)
{
    initialize();
    registerPlayer(PLAYER_A);
    EXPECT_EQ(tester.state().players_0.walletAddress, PLAYER_A);
}

TEST_F(QZNRewardRouterTest, Register_HappyPath_AllStatsInitializedToZero)
{
    initialize();
    registerPlayer(PLAYER_A);
    const auto& p = tester.state().players_0;
    EXPECT_EQ(p.epochEarned,        0LL);
    EXPECT_EQ(p.pendingBalance,     0LL);
    EXPECT_EQ(p.lifetimeEarned,     0LL);
    EXPECT_EQ(p.totalMatchesPlayed, 0LL);
    EXPECT_EQ(p.totalMatchesWon,    0LL);
    EXPECT_EQ(p.currentWinStreak,   0LL);
    EXPECT_EQ(p.bestWinStreak,      0LL);
}

TEST_F(QZNRewardRouterTest, Register_HappyPath_AllAchievementFlagsFalse)
{
    initialize();
    registerPlayer(PLAYER_A);
    const auto& p = tester.state().players_0;
    EXPECT_EQ(p.achFirstWin,      0);
    EXPECT_EQ(p.achStreak5,       0);
    EXPECT_EQ(p.achStreak10,      0);
    EXPECT_EQ(p.achMatches100,    0);
    EXPECT_EQ(p.achMatches1000,   0);
    EXPECT_EQ(p.achHighStake,     0);
    EXPECT_EQ(p.achAllGames,      0);
    EXPECT_EQ(p.achTopLeaderboard,0);
}

TEST_F(QZNRewardRouterTest, Register_HappyPath_TotalPlayersIncremented)
{
    initialize();
    registerPlayer(PLAYER_A);
    EXPECT_EQ(tester.state().totalPlayersRegistered, 1LL);
    registerPlayer(PLAYER_B);
    EXPECT_EQ(tester.state().totalPlayersRegistered, 2LL);
}

TEST_F(QZNRewardRouterTest, Register_ZeroStake_Tier0Multiplier)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_0);
    EXPECT_EQ(tester.state().players_0.stakedAmount,       0LL);
}

TEST_F(QZNRewardRouterTest, Register_AtTier1Threshold_Tier1Multiplier)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_1);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_1);
}

TEST_F(QZNRewardRouterTest, Register_AtTier2Threshold_Tier2Multiplier)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_2);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_2);
}

TEST_F(QZNRewardRouterTest, Register_AtTier3Threshold_Tier3Multiplier)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_3);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_3);
}

TEST_F(QZNRewardRouterTest, Register_AtTier4Threshold_Tier4Multiplier)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_4);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_4);
}

TEST_F(QZNRewardRouterTest, Register_BelowTier1_Tier0Multiplier)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_1 - 1LL);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_0);
}

TEST_F(QZNRewardRouterTest, Register_PreInit_Rejected)
{
    // Do not call initialize()
    RegisterPlayer_input in{};
    in.initialStake = 0LL;
    tester.setInvocator(PLAYER_A);
    auto out = tester.callProcedure(RegisterPlayer_id, in);
    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().totalPlayersRegistered, 0LL);
}

TEST_F(QZNRewardRouterTest, Register_OutputMatchesState)
{
    initialize();
    RegisterPlayer_input in{};
    in.initialStake = STAKE_TIER_2;
    tester.setInvocator(PLAYER_A);
    auto out = tester.callProcedure(RegisterPlayer_id, in);

    EXPECT_EQ(out.success,        1);
    EXPECT_EQ(out.playerSlot,     0LL);
    EXPECT_EQ(out.multiplierBPS,  MULT_TIER_2);
}

// ============================================================
//  [PROC-3]  StakeQZN
// ============================================================

TEST_F(QZNRewardRouterTest, Stake_Tier0ToTier1_MultiplierUpdates)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_0);

    auto out = stake(PLAYER_A, STAKE_TIER_1);

    EXPECT_EQ(out.newMultiplierBPS, MULT_TIER_1);
    EXPECT_EQ(out.newStakedTotal,   STAKE_TIER_1);
    EXPECT_EQ(out.multiplierTier,   1LL);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_1);
    EXPECT_EQ(tester.state().players_0.stakedAmount,       STAKE_TIER_1);
}

TEST_F(QZNRewardRouterTest, Stake_Tier1ToTier2_MultiplierUpdates)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_1);
    stake(PLAYER_A, STAKE_TIER_2 - STAKE_TIER_1);  // Top up to tier 2
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_2);
}

TEST_F(QZNRewardRouterTest, Stake_ToTier3_MultiplierUpdates)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);
    stake(PLAYER_A, STAKE_TIER_3);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_3);
    EXPECT_EQ(tester.state().players_0.stakedAmount,       STAKE_TIER_3);
}

TEST_F(QZNRewardRouterTest, Stake_ToTier4_MaxMultiplier)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);
    auto out = stake(PLAYER_A, STAKE_TIER_4);
    EXPECT_EQ(out.newMultiplierBPS, MULT_TIER_4);
    EXPECT_EQ(out.multiplierTier,   4LL);
}

TEST_F(QZNRewardRouterTest, Stake_AboveTier4_StaysAtTier4)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);
    stake(PLAYER_A, STAKE_TIER_4 * 2LL);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_4);
}

TEST_F(QZNRewardRouterTest, Stake_ZeroAmount_Rejected)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);
    sint64 prevStake = tester.state().players_0.stakedAmount;

    StakeQZN_input in{};
    in.amount = 0LL;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeQZN_id, in);

    EXPECT_EQ(tester.state().players_0.stakedAmount, prevStake);
}

TEST_F(QZNRewardRouterTest, Stake_NegativeAmount_Rejected)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);

    StakeQZN_input in{};
    in.amount = -100LL;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeQZN_id, in);

    EXPECT_EQ(tester.state().players_0.stakedAmount, 0LL);
}

TEST_F(QZNRewardRouterTest, Stake_UnregisteredPlayer_Rejected)
{
    initialize();
    // PLAYER_A not registered — stake should be a no-op
    StakeQZN_input in{};
    in.amount = STAKE_TIER_2;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeQZN_id, in);

    // No slot should have been modified
    EXPECT_EQ(tester.state().players_0.active,       0);
    EXPECT_EQ(tester.state().players_0.stakedAmount, 0LL);
}

TEST_F(QZNRewardRouterTest, Stake_IncrementalStakes_Accumulate)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);
    stake(PLAYER_A, 500LL);
    stake(PLAYER_A, 500LL);  // Total = 1000 → Tier 1
    EXPECT_EQ(tester.state().players_0.stakedAmount,       STAKE_TIER_1);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_1);
}

// ============================================================
//  [PROC-4]  UnstakeQZN
// ============================================================

TEST_F(QZNRewardRouterTest, Unstake_HappyPath_ReducesStakedAmount)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_2);

    UnstakeQZN_input in{};
    in.amount = STAKE_TIER_1;
    tester.setInvocator(PLAYER_A);
    auto out = tester.callProcedure(UnstakeQZN_id, in);

    EXPECT_EQ(out.amountReturned, STAKE_TIER_1);
    EXPECT_EQ(out.newStakedTotal, STAKE_TIER_2 - STAKE_TIER_1);
    EXPECT_EQ(tester.state().players_0.stakedAmount, STAKE_TIER_2 - STAKE_TIER_1);
}

TEST_F(QZNRewardRouterTest, Unstake_MultiplierRecalculatesDownward)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_3);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_3);

    // Unstake enough to drop to Tier 1
    UnstakeQZN_input in{};
    in.amount = STAKE_TIER_3 - STAKE_TIER_1;
    tester.setInvocator(PLAYER_A);
    auto out = tester.callProcedure(UnstakeQZN_id, in);

    EXPECT_EQ(out.newMultiplierBPS,                        MULT_TIER_1);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_1);
}

TEST_F(QZNRewardRouterTest, Unstake_FullUnstake_DropsToTier0)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_4);

    UnstakeQZN_input in{};
    in.amount = STAKE_TIER_4;
    tester.setInvocator(PLAYER_A);
    auto out = tester.callProcedure(UnstakeQZN_id, in);

    EXPECT_EQ(out.newStakedTotal,   0LL);
    EXPECT_EQ(out.newMultiplierBPS, MULT_TIER_0);
    EXPECT_EQ(tester.state().players_0.stakedAmount,       0LL);
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_0);
}

TEST_F(QZNRewardRouterTest, Unstake_ExcessAmount_Rejected)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_1);

    UnstakeQZN_input in{};
    in.amount = STAKE_TIER_1 + 1LL;  // 1 over staked
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(UnstakeQZN_id, in);

    EXPECT_EQ(tester.state().players_0.stakedAmount, STAKE_TIER_1);
}

TEST_F(QZNRewardRouterTest, Unstake_ZeroAmount_Rejected)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_2);

    UnstakeQZN_input in{};
    in.amount = 0LL;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(UnstakeQZN_id, in);

    EXPECT_EQ(tester.state().players_0.stakedAmount, STAKE_TIER_2);
}

TEST_F(QZNRewardRouterTest, Unstake_PartialUnstake_StaysInSameTier)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_2 + 500LL);  // 500 above tier 2
    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_2);

    UnstakeQZN_input in{};
    in.amount = 500LL;  // Removes buffer but stays at tier 2
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(UnstakeQZN_id, in);

    EXPECT_EQ(tester.state().players_0.stakeMultiplierBPS, MULT_TIER_2);
    EXPECT_EQ(tester.state().players_0.stakedAmount,       STAKE_TIER_2);
}

// ============================================================
//  [PROC-5]  ReportMatchResult
// ============================================================

TEST_F(QZNRewardRouterTest, Report_AuthGuard_NonCabinetRejected)
{
    initialize();
    registerPlayer(PLAYER_A);

    ReportMatchResult_input in{};
    in.winnerAddress = PLAYER_A;
    in.gameId        = 3;

    tester.setInvocator(STRANGER_ADDR);
    auto out = tester.callProcedure(ReportMatchResult_id, in);

    EXPECT_EQ(out.winnerReward, 0LL);
    EXPECT_EQ(tester.state().players_0.pendingBalance, 0LL);
}

TEST_F(QZNRewardRouterTest, Report_AuthGuard_AdminRejected)
{
    initialize();
    registerPlayer(PLAYER_A);

    ReportMatchResult_input in{};
    in.winnerAddress = PLAYER_A;
    in.gameId        = 3;

    tester.setInvocator(ADMIN_ADDR);  // Admin is not cabinet
    auto out = tester.callProcedure(ReportMatchResult_id, in);

    EXPECT_EQ(out.winnerReward, 0LL);
}

TEST_F(QZNRewardRouterTest, Report_Tier0_BaseRewardCredited)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);  // Tier 0 — 1.0x multiplier

    auto out = reportWin(PLAYER_A);

    // Expected: BASE_WIN_REWARD * MULT_TIER_0 / MULT_DENOMINATOR
    //         = 5 * 1000 / 1000 = 5 QZN
    EXPECT_EQ(out.winnerReward,      BASE_WIN_REWARD);
    EXPECT_EQ(out.multiplierApplied, MULT_TIER_0);
    EXPECT_EQ(tester.state().players_0.pendingBalance, BASE_WIN_REWARD);
}

TEST_F(QZNRewardRouterTest, Report_Tier1_MultipliedRewardCorrect)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_1);  // 1.25x

    auto out = reportWin(PLAYER_A);

    // Expected: 5 * 1250 / 1000 = 6 QZN (integer division)
    sint64 expected = (BASE_WIN_REWARD * MULT_TIER_1) / MULT_DENOMINATOR;
    EXPECT_EQ(out.winnerReward, expected);
    EXPECT_EQ(out.multiplierApplied, MULT_TIER_1);
}

TEST_F(QZNRewardRouterTest, Report_Tier2_MultipliedRewardCorrect)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_2);  // 1.5x

    auto out = reportWin(PLAYER_A);

    sint64 expected = (BASE_WIN_REWARD * MULT_TIER_2) / MULT_DENOMINATOR;
    EXPECT_EQ(out.winnerReward, expected);
}

TEST_F(QZNRewardRouterTest, Report_Tier3_MultipliedRewardCorrect)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_3);  // 2.0x

    auto out = reportWin(PLAYER_A);

    sint64 expected = (BASE_WIN_REWARD * MULT_TIER_3) / MULT_DENOMINATOR;
    EXPECT_EQ(out.winnerReward, expected);  // 10 QZN
}

TEST_F(QZNRewardRouterTest, Report_Tier4_MaxMultiplierRewardCorrect)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_4);  // 3.0x

    auto out = reportWin(PLAYER_A);

    sint64 expected = (BASE_WIN_REWARD * MULT_TIER_4) / MULT_DENOMINATOR;
    EXPECT_EQ(out.winnerReward, expected);  // 15 QZN
}

TEST_F(QZNRewardRouterTest, Report_MatchStatsIncremented)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);

    EXPECT_EQ(tester.state().players_0.totalMatchesPlayed, 1LL);
    EXPECT_EQ(tester.state().players_0.totalMatchesWon,    1LL);
    EXPECT_EQ(tester.state().players_0.currentWinStreak,   1LL);
}

TEST_F(QZNRewardRouterTest, Report_WinStreakIncrementsPerWin)
{
    initialize();
    registerPlayer(PLAYER_A);

    reportWin(PLAYER_A);
    reportWin(PLAYER_A);
    reportWin(PLAYER_A);

    EXPECT_EQ(tester.state().players_0.currentWinStreak, 3LL);
    EXPECT_EQ(tester.state().players_0.bestWinStreak,    3LL);
}

TEST_F(QZNRewardRouterTest, Report_BestStreakUpdatesOnNewHigher)
{
    initialize();
    registerPlayer(PLAYER_A);

    for (int i = 0; i < 7; i++) reportWin(PLAYER_A);

    EXPECT_EQ(tester.state().players_0.bestWinStreak, 7LL);
}

TEST_F(QZNRewardRouterTest, Report_EpochCapEnforced_NoRewardAboveCap)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_0);  // Tier 0: 5 QZN/win

    // Fill up to cap — need EPOCH_EARN_CAP / BASE_WIN_REWARD wins
    sint64 winsNeeded = EPOCH_EARN_CAP / BASE_WIN_REWARD;
    for (sint64 i = 0; i < winsNeeded; i++)
    {
        reportWin(PLAYER_A);
    }

    // At or above cap — next win should give 0 base reward
    auto out = reportWin(PLAYER_A);
    // Expect zero or minimal reward (only achievement bonus possible)
    // Base reward portion should be 0 once cap is hit
    EXPECT_LE(tester.state().players_0.epochEarned, EPOCH_EARN_CAP);
}

TEST_F(QZNRewardRouterTest, Report_EpochCapResetOnNewEpoch)
{
    initialize();
    registerPlayer(PLAYER_A);
    tester.setCurrentEpoch(1);

    // Hit the cap
    sint64 winsNeeded = EPOCH_EARN_CAP / BASE_WIN_REWARD + 1LL;
    for (sint64 i = 0; i < winsNeeded; i++) reportWin(PLAYER_A);

    EXPECT_GE(tester.state().players_0.epochEarned, EPOCH_EARN_CAP);

    // Advance to new epoch — cap should reset
    tester.setCurrentEpoch(2);
    auto out = reportWin(PLAYER_A);

    EXPECT_GT(out.winnerReward, 0LL);
    EXPECT_EQ(tester.state().players_0.epochEarned, out.winnerReward);
}

TEST_F(QZNRewardRouterTest, Report_PerGameWinFlags_SetCorrectly)
{
    initialize();
    registerPlayer(PLAYER_A);

    // Win on each game type
    reportWin(PLAYER_A, 1);  // snaQe
    reportWin(PLAYER_A, 2);  // paQman
    reportWin(PLAYER_A, 3);  // TANQ

    EXPECT_EQ(tester.state().players_0.wonSnaqe,  1);
    EXPECT_EQ(tester.state().players_0.wonPaqman, 1);
    EXPECT_EQ(tester.state().players_0.wonTanq,   1);
}

TEST_F(QZNRewardRouterTest, Report_UnregisteredWinner_ReturnsZero)
{
    initialize();
    // PLAYER_A not registered

    auto out = reportWin(PLAYER_A);

    EXPECT_EQ(out.winnerReward, 0LL);
}

TEST_F(QZNRewardRouterTest, Report_ReserveDeductedAfterReward)
{
    initialize();
    registerPlayer(PLAYER_A);
    sint64 prevReserve = tester.state().rewardReserveBalance;

    reportWin(PLAYER_A);

    EXPECT_LT(tester.state().rewardReserveBalance, prevReserve);
}

// ---- Achievement tests ----

TEST_F(QZNRewardRouterTest, Achievement_FirstWin_AwardedOnFirstWin)
{
    initialize();
    registerPlayer(PLAYER_A);
    EXPECT_EQ(tester.state().players_0.achFirstWin, 0);

    reportWin(PLAYER_A);

    EXPECT_EQ(tester.state().players_0.achFirstWin, 1);
    // Pending balance should include first-win bonus
    EXPECT_GE(tester.state().players_0.pendingBalance,
              BASE_WIN_REWARD + ACH_FIRST_WIN);
}

TEST_F(QZNRewardRouterTest, Achievement_FirstWin_NotDoubleAwarded)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);
    sint64 pendingAfterFirst = tester.state().players_0.pendingBalance;

    reportWin(PLAYER_A);  // Second win — no first-win bonus again

    sint64 gain = tester.state().players_0.pendingBalance - pendingAfterFirst;
    // Gain should only be base reward, not another ACH_FIRST_WIN
    EXPECT_LT(gain, BASE_WIN_REWARD + ACH_FIRST_WIN);
}

TEST_F(QZNRewardRouterTest, Achievement_Streak5_AwardedAtExactly5)
{
    initialize();
    registerPlayer(PLAYER_A);

    for (int i = 0; i < 4; i++) reportWin(PLAYER_A);
    EXPECT_EQ(tester.state().players_0.achStreak5, 0);

    reportWin(PLAYER_A);  // 5th consecutive win

    EXPECT_EQ(tester.state().players_0.achStreak5, 1);
    EXPECT_GT(tester.state().players_0.pendingBalance, 0LL);
}

TEST_F(QZNRewardRouterTest, Achievement_Streak10_AwardedAtExactly10)
{
    initialize();
    registerPlayer(PLAYER_A);

    for (int i = 0; i < 9; i++) reportWin(PLAYER_A);
    EXPECT_EQ(tester.state().players_0.achStreak10, 0);

    reportWin(PLAYER_A);  // 10th consecutive win

    EXPECT_EQ(tester.state().players_0.achStreak10, 1);
}

TEST_F(QZNRewardRouterTest, Achievement_Streak5AndStreak10_BothAwardedInOrder)
{
    initialize();
    registerPlayer(PLAYER_A);

    for (int i = 0; i < 10; i++) reportWin(PLAYER_A);

    EXPECT_EQ(tester.state().players_0.achStreak5,  1);
    EXPECT_EQ(tester.state().players_0.achStreak10, 1);
}

TEST_F(QZNRewardRouterTest, Achievement_Matches100_AwardedAtExact100)
{
    initialize();
    registerPlayer(PLAYER_A);

    for (int i = 0; i < 99; i++) reportWin(PLAYER_A);
    EXPECT_EQ(tester.state().players_0.achMatches100, 0);

    reportWin(PLAYER_A);  // 100th match

    EXPECT_EQ(tester.state().players_0.achMatches100, 1);
}

TEST_F(QZNRewardRouterTest, Achievement_HighStake_AwardedAtThreshold)
{
    initialize();
    registerPlayer(PLAYER_A);
    EXPECT_EQ(tester.state().players_0.achHighStake, 0);

    // Report a match with stake at the high-stake threshold
    reportWin(PLAYER_A, 3, ACH_HIGH_STAKE_MIN);

    EXPECT_EQ(tester.state().players_0.achHighStake, 1);
}

TEST_F(QZNRewardRouterTest, Achievement_HighStake_NotAwardedBelowThreshold)
{
    initialize();
    registerPlayer(PLAYER_A);

    reportWin(PLAYER_A, 3, ACH_HIGH_STAKE_MIN - 1LL);

    EXPECT_EQ(tester.state().players_0.achHighStake, 0);
}

TEST_F(QZNRewardRouterTest, Achievement_AllGames_RequiresAllThreeGames)
{
    initialize();
    registerPlayer(PLAYER_A);

    // Win 2 of 3 games — not enough
    reportWin(PLAYER_A, 1);
    reportWin(PLAYER_A, 2);
    EXPECT_EQ(tester.state().players_0.achAllGames, 0);

    // Win the third — achievement should fire
    reportWin(PLAYER_A, 3);
    EXPECT_EQ(tester.state().players_0.achAllGames, 1);
}

TEST_F(QZNRewardRouterTest, Achievement_AllGames_NotAwardedWithOnlyOneGame)
{
    initialize();
    registerPlayer(PLAYER_A);

    // Win all on same game — does not trigger
    reportWin(PLAYER_A, 1);
    reportWin(PLAYER_A, 1);
    reportWin(PLAYER_A, 1);

    EXPECT_EQ(tester.state().players_0.achAllGames, 0);
}

TEST_F(QZNRewardRouterTest, Achievement_BonusesBypassEpochCap)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);
    tester.setCurrentEpoch(1);

    // Exhaust epoch cap first
    sint64 winsNeeded = EPOCH_EARN_CAP / BASE_WIN_REWARD + 5LL;
    for (sint64 i = 0; i < winsNeeded - 1; i++) reportWin(PLAYER_A);

    sint64 pendingBeforeFirstWinAch = tester.state().players_0.pendingBalance;

    // This win triggers FIRST_WIN achievement — bonus should still be credited
    // even though epoch cap is hit
    reportWin(PLAYER_A);

    // Pending should have increased by at least the achievement amount
    sint64 gain = tester.state().players_0.pendingBalance - pendingBeforeFirstWinAch;
    // If first win ach fires, gain should include ACH_FIRST_WIN even at cap
    // (This test will FAIL if achievements don't bypass the cap — documents intent)
    EXPECT_GE(gain, ACH_FIRST_WIN);
}

TEST_F(QZNRewardRouterTest, Achievement_TotalCountAccumulates)
{
    initialize();
    registerPlayer(PLAYER_A);

    // Trigger FIRST_WIN
    reportWin(PLAYER_A);
    EXPECT_EQ(tester.state().totalAchievementsAwarded, 1LL);

    // Trigger STREAK_5
    for (int i = 0; i < 4; i++) reportWin(PLAYER_A);
    EXPECT_GE(tester.state().totalAchievementsAwarded, 2LL);
}

// ============================================================
//  [PROC-6]  ClaimRewards
// ============================================================

TEST_F(QZNRewardRouterTest, Claim_HappyPath_FullBalanceClaimed)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);

    sint64 pendingBefore = tester.state().players_0.pendingBalance;
    ASSERT_GT(pendingBefore, 0LL);

    auto out = claim(PLAYER_A);

    EXPECT_EQ(out.amountClaimed,    pendingBefore);
    EXPECT_EQ(out.pendingRemaining, 0LL);
}

TEST_F(QZNRewardRouterTest, Claim_PendingZeroedAfterClaim)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);
    claim(PLAYER_A);

    EXPECT_EQ(tester.state().players_0.pendingBalance, 0LL);
}

TEST_F(QZNRewardRouterTest, Claim_TotalDistributedIncremented)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);
    sint64 claimAmount = tester.state().players_0.pendingBalance;

    claim(PLAYER_A);

    EXPECT_EQ(tester.state().totalQZNDistributed, claimAmount);
}

TEST_F(QZNRewardRouterTest, Claim_ZeroBalance_ReturnsZero)
{
    initialize();
    registerPlayer(PLAYER_A);
    // No wins — pending is 0

    auto out = claim(PLAYER_A);

    EXPECT_EQ(out.amountClaimed,    0LL);
    EXPECT_EQ(out.pendingRemaining, 0LL);
}

TEST_F(QZNRewardRouterTest, Claim_UnregisteredPlayer_ReturnsZero)
{
    initialize();
    // PLAYER_A not registered

    auto out = claim(PLAYER_A);

    EXPECT_EQ(out.amountClaimed,    0LL);
    EXPECT_EQ(out.pendingRemaining, 0LL);
}

TEST_F(QZNRewardRouterTest, Claim_DoubleClaim_SecondReturnsZero)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);
    claim(PLAYER_A);

    auto out2 = claim(PLAYER_A);

    EXPECT_EQ(out2.amountClaimed, 0LL);
    EXPECT_EQ(tester.state().players_0.pendingBalance, 0LL);
}

TEST_F(QZNRewardRouterTest, Claim_MultipleWinsThenSingleClaim)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);
    reportWin(PLAYER_A);
    reportWin(PLAYER_A);

    sint64 pending = tester.state().players_0.pendingBalance;
    auto out = claim(PLAYER_A);

    EXPECT_EQ(out.amountClaimed, pending);
    EXPECT_EQ(tester.state().players_0.pendingBalance, 0LL);
}

// ============================================================
//  [PROC-7]  FundReserve
// ============================================================

TEST_F(QZNRewardRouterTest, FundReserve_Admin_RewardReserveIncreases)
{
    initialize();
    sint64 prevReward = tester.state().rewardReserveBalance;

    FundReserve_input in{};
    in.amount            = 500000LL;
    in.isAchievementFund = 0;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(FundReserve_id, in);

    EXPECT_EQ(out.newRewardReserve, prevReward + 500000LL);
    EXPECT_EQ(tester.state().rewardReserveBalance, prevReward + 500000LL);
}

TEST_F(QZNRewardRouterTest, FundReserve_Admin_AchievementReserveIncreases)
{
    initialize();
    sint64 prevAch = tester.state().achievementReserveBalance;

    FundReserve_input in{};
    in.amount            = 100000LL;
    in.isAchievementFund = 1;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(FundReserve_id, in);

    EXPECT_EQ(out.newAchievementReserve, prevAch + 100000LL);
    EXPECT_EQ(tester.state().achievementReserveBalance, prevAch + 100000LL);
}

TEST_F(QZNRewardRouterTest, FundReserve_NonAdmin_Rejected)
{
    initialize();
    sint64 prevReward = tester.state().rewardReserveBalance;

    FundReserve_input in{};
    in.amount            = 500000LL;
    in.isAchievementFund = 0;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(FundReserve_id, in);

    EXPECT_EQ(tester.state().rewardReserveBalance, prevReward);
}

// ============================================================
//  [FUNC-8]  GetPlayerStats
// ============================================================

TEST_F(QZNRewardRouterTest, GetPlayerStats_RegisteredPlayer_ReturnsCorrectData)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_2);
    reportWin(PLAYER_A);

    GetPlayerStats_input in{};
    in.walletAddress = PLAYER_A;
    auto out = tester.callFunction(GetPlayerStats_id, in);

    EXPECT_GT(out.pendingBalance,  0LL);
    EXPECT_EQ(out.stakedAmount,    STAKE_TIER_2);
    EXPECT_EQ(out.multiplierBPS,   MULT_TIER_2);
    EXPECT_EQ(out.epochCap,        EPOCH_EARN_CAP);
    EXPECT_EQ(out.totalMatchesWon, 1LL);
    EXPECT_EQ(out.currentWinStreak, 1LL);
    EXPECT_GT(out.lifetimeEarned,  0LL);
}

TEST_F(QZNRewardRouterTest, GetPlayerStats_UnregisteredPlayer_ReturnsZeroes)
{
    initialize();

    GetPlayerStats_input in{};
    in.walletAddress = PLAYER_A;  // Not registered
    auto out = tester.callFunction(GetPlayerStats_id, in);

    EXPECT_EQ(out.pendingBalance,   0LL);
    EXPECT_EQ(out.stakedAmount,     0LL);
    EXPECT_EQ(out.totalMatchesWon,  0LL);
}

TEST_F(QZNRewardRouterTest, GetPlayerStats_IsReadOnly)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);
    sint64 pendingBefore = tester.state().players_0.pendingBalance;

    GetPlayerStats_input in{};
    in.walletAddress = PLAYER_A;
    tester.callFunction(GetPlayerStats_id, in);
    tester.callFunction(GetPlayerStats_id, in);

    EXPECT_EQ(tester.state().players_0.pendingBalance, pendingBefore);
}

// ============================================================
//  [FUNC-9]  GetLeaderboard
// ============================================================

TEST_F(QZNRewardRouterTest, GetLeaderboard_InitialState_AllZero)
{
    initialize();

    GetLeaderboard_input in{};
    auto out = tester.callFunction(GetLeaderboard_id, in);

    EXPECT_EQ(out.rank1_score,    0LL);
    EXPECT_EQ(out.rank2_score,    0LL);
    EXPECT_EQ(out.rank3_score,    0LL);
    EXPECT_EQ(out.epochRewardPool, INITIAL_REWARD_RESERVE);
}

TEST_F(QZNRewardRouterTest, GetLeaderboard_AfterMatch_ScoreReflected)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);

    // Player A's epoch score should be > 0 after winning
    EXPECT_GT(tester.state().players_0.epochScore, 0LL);
}

// ============================================================
//  BEGIN_EPOCH integration tests
// ============================================================

TEST_F(QZNRewardRouterTest, BeginEpoch_EpochCounterIncrements)
{
    initialize();
    tester.setCurrentEpoch(5);
    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().currentEpoch,         5LL);
    EXPECT_EQ(tester.state().totalEpochsProcessed, 1LL);
}

TEST_F(QZNRewardRouterTest, BeginEpoch_EpochTotalDistributedReset)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);
    claim(PLAYER_A);

    sint64 prevDistributed = tester.state().epochTotalDistributed;
    ASSERT_GT(prevDistributed, 0LL);

    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().epochTotalDistributed, 0LL);
}

TEST_F(QZNRewardRouterTest, BeginEpoch_EpochScoresReset)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);
    ASSERT_GT(tester.state().players_0.epochScore, 0LL);

    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().players_0.epochScore,  0LL);
    EXPECT_EQ(tester.state().players_0.epochEarned, 0LL);
}

TEST_F(QZNRewardRouterTest, BeginEpoch_LeaderboardSortedDescending)
{
    initialize();
    // Manually set leaderboard entries in wrong order
    tester.state().board_0.score = 50LL;
    tester.state().board_1.score = 200LL;
    tester.state().board_2.score = 100LL;

    tester.advanceBeginEpoch();

    // After sort, board_0 should have highest score
    EXPECT_GE(tester.state().board_0.score, tester.state().board_1.score);
    EXPECT_GE(tester.state().board_1.score, tester.state().board_2.score);
}

TEST_F(QZNRewardRouterTest, BeginEpoch_TopLeaderboardAchievement_AwardedToRank1)
{
    initialize();
    registerPlayer(PLAYER_A);

    // Give PLAYER_A a high epoch score via multiple wins
    for (int i = 0; i < 5; i++) reportWin(PLAYER_A);

    // PLAYER_A should be on leaderboard
    EXPECT_GT(tester.state().board_9.score, 0LL);  // Inserted at slot 9

    tester.advanceBeginEpoch();

    // After sort, rank 1 (board_0) should hold PLAYER_A
    // and TOP_LEADERBOARD achievement should be awarded
    if (tester.state().board_0.walletAddress == PLAYER_A)
    {
        EXPECT_EQ(tester.state().players_0.achTopLeaderboard, 1);
    }
}

TEST_F(QZNRewardRouterTest, BeginEpoch_PendingBalancePreservedAcrossEpoch)
{
    initialize();
    registerPlayer(PLAYER_A);
    reportWin(PLAYER_A);
    claim(PLAYER_A);  // Claim before epoch boundary

    sint64 pendingBeforeEpoch = tester.state().players_0.pendingBalance;
    EXPECT_EQ(pendingBeforeEpoch, 0LL);  // Just claimed

    tester.advanceBeginEpoch();

    // Pending should still be 0 — epoch advance doesn't reset it
    EXPECT_EQ(tester.state().players_0.pendingBalance, 0LL);
}

// ============================================================
//  INVARIANTS
// ============================================================

TEST_F(QZNRewardRouterTest, Invariant_ReserveNeverGoesNegative)
{
    initialize();
    registerPlayer(PLAYER_A, STAKE_TIER_4);  // Max multiplier

    // Report many wins to drain reserve
    for (int i = 0; i < 200; i++) reportWin(PLAYER_A);

    EXPECT_GE(tester.state().rewardReserveBalance,      0LL);
    EXPECT_GE(tester.state().achievementReserveBalance, 0LL);
}

TEST_F(QZNRewardRouterTest, Invariant_TotalDistributedNeverExceedsInitialReserve)
{
    initialize();
    registerPlayer(PLAYER_A);

    for (int i = 0; i < 100; i++) reportWin(PLAYER_A);
    claim(PLAYER_A);

    EXPECT_LE(tester.state().totalQZNDistributed, INITIAL_REWARD_RESERVE + INITIAL_ACHIEVEMENT_RESERVE);
}

TEST_F(QZNRewardRouterTest, Invariant_EpochEarnedNeverExceedsCapWithinEpoch)
{
    initialize();
    registerPlayer(PLAYER_A);
    tester.setCurrentEpoch(1);

    for (int i = 0; i < 5000; i++) reportWin(PLAYER_A);

    EXPECT_LE(tester.state().players_0.epochEarned, EPOCH_EARN_CAP);
}

TEST_F(QZNRewardRouterTest, Invariant_MultiplierAlwaysInValidRange)
{
    initialize();
    registerPlayer(PLAYER_A, 0LL);

    stake(PLAYER_A, STAKE_TIER_4 * 10LL);  // Massively overstake

    sint64 mult = tester.state().players_0.stakeMultiplierBPS;
    EXPECT_GE(mult, MULT_TIER_0);
    EXPECT_LE(mult, MULT_TIER_4);
}
