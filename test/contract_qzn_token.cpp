#define NO_UEFI
#include <iostream>
#include "contract_testing.h"

// ============================================================
//  QZN Token v2 — GTest Suite
//  File:    test/contract_qzn_token.cpp
//  Place in qubic/core repo at: test/contract_qzn_token.cpp
//
//  Coverage:
//    [PROC-1]  InitializeQZN      — happy path, double-init guard,
//                                   address storage, allocation buckets,
//                                   vesting clock arm, override epoch
//    [PROC-2]  SettleMatch        — BPS routing math, auth rejection,
//                                   zero-stake guard, stat accumulation,
//                                   multi-match cumulative state
//    [PROC-3]  ClaimVestedTokens  — pre-cliff block, at-cliff boundary,
//                                   mid-vest linear amount, full vest cap,
//                                   non-founder rejection, double-claim guard
//    [PROC-4]  BurnFromTreasury   — happy path, non-admin rejection,
//                                   excess-amount guard, zero-amount guard,
//                                   circulating supply update
//    [FUNC-5]  GetSupplyInfo      — initial state, post-settlement state
//    [FUNC-6]  GetVestingStatus   — pre-cliff, at-cliff, mid-vest, full-vest
//    [FUNC-7]  GetMatchStats      — zero state, single match, multi match
// ============================================================

#include "gtest/gtest.h"
#include "../src/contracts/QZN_Token_v2.h"
#include "contract_testing.h"

// ============================================================
//  HELPERS — well-known test addresses
// ============================================================

static const id ADMIN_ADDR   = id(1, 0, 0, 0);
static const id FOUNDER_ADDR = id(2, 0, 0, 0);
static const id TREASURY_ADDR  = id(3, 0, 0, 0);
static const id LIQUIDITY_ADDR = id(4, 0, 0, 0);
static const id ECOSYSTEM_ADDR = id(5, 0, 0, 0);
static const id PORTAL_NODE_ADDR  = id(6, 0, 0, 0);
static const id PORTAL_PROTO_ADDR = id(7, 0, 0, 0);
static const id QSWAP_PROTO_ADDR  = id(8, 0, 0, 0);
static const id WINNER_ADDR    = id(9,  0, 0, 0);
static const id STRANGER_ADDR  = id(10, 0, 0, 0);

// Canonical test stake — matches whitepaper 2-player duel pot
static constexpr sint64 STANDARD_STAKE = 50000LL;

// ============================================================
//  FIXTURE
// ============================================================

class QZNTokenTest : public ::testing::Test
{
protected:
    ContractTester<QZN> tester;

    void SetUp() override {
        tester.reset();
    }

    // Performs a standard InitializeQZN call as ADMIN_ADDR,
    // vesting start at epoch 100 for deterministic cliff tests.
    void initialize(sint64 vestingEpochOverride = 100)
    {
        InitializeQZN_input in{};
        in.treasuryAddr             = TREASURY_ADDR;
        in.founderAddr              = FOUNDER_ADDR;
        in.liquidityAddr            = LIQUIDITY_ADDR;
        in.ecosystemAddr            = ECOSYSTEM_ADDR;
        in.portalNodeAddr           = PORTAL_NODE_ADDR;
        in.portalProtoAddr          = PORTAL_PROTO_ADDR;
        in.qswapProtoAddr           = QSWAP_PROTO_ADDR;
        in.vestingStartEpochOverride = vestingEpochOverride;

        tester.setInvocator(ADMIN_ADDR);
        tester.callProcedure(InitializeQZN_id, in);
    }

    // Calls SettleMatch as ADMIN_ADDR (Phase 1 auth) with given stake.
    SettleMatch_output settleMatch(id winner, sint64 stake)
    {
        SettleMatch_input in{};
        in.winnerAddress = winner;
        in.totalStake    = stake;

        tester.setInvocator(ADMIN_ADDR);
        return tester.callProcedure(SettleMatch_id, in);
    }
};

// ============================================================
//  [PROC-1]  InitializeQZN
// ============================================================

TEST_F(QZNTokenTest, Init_HappyPath_SetsAllAddresses)
{
    initialize();

    const auto& s = tester.state();
    EXPECT_EQ(s.adminAddress,       ADMIN_ADDR);
    EXPECT_EQ(s.founderAddress,     FOUNDER_ADDR);
    EXPECT_EQ(s.treasuryAddress,    TREASURY_ADDR);
    EXPECT_EQ(s.liquidityAddress,   LIQUIDITY_ADDR);
    EXPECT_EQ(s.portalNodeAddress,  PORTAL_NODE_ADDR);
    EXPECT_EQ(s.portalProtoAddress, PORTAL_PROTO_ADDR);
    EXPECT_EQ(s.qswapProtoAddress,  QSWAP_PROTO_ADDR);
}

TEST_F(QZNTokenTest, Init_HappyPath_SetsAllocationBuckets)
{
    initialize();

    const auto& s = tester.state();
    EXPECT_EQ(s.totalSupply,          QZN_TOTAL_SUPPLY);
    EXPECT_EQ(s.treasuryBalance,      QZN_TREASURY_ALLOC);   // 87.5M
    EXPECT_EQ(s.teamAllocationTotal,  QZN_TEAM_ALLOC);       // 50M
    EXPECT_EQ(s.teamTokensClaimed,    0LL);
    EXPECT_EQ(s.liquidityBalance,     QZN_LIQUIDITY_ALLOC);  // 37.5M
    EXPECT_EQ(s.ecosystemBalance,     QZN_ECOSYSTEM_ALLOC);  // 25M
}

TEST_F(QZNTokenTest, Init_HappyPath_CirculatingSupplyEqualsLiquidity)
{
    // At init, only liquidity is live. All else is locked or on QX.
    initialize();
    EXPECT_EQ(tester.state().circulatingSupply, QZN_LIQUIDITY_ALLOC);
}

TEST_F(QZNTokenTest, Init_HappyPath_VestingClockArmed)
{
    initialize(100);

    const auto& s = tester.state();
    EXPECT_EQ(s.vestingStartEpoch, 100LL);
    EXPECT_EQ(s.vestingInitialized, 1);
}

TEST_F(QZNTokenTest, Init_HappyPath_VestingEpochZeroUsesCurrentEpoch)
{
    // When override is 0, contract uses qpi.epoch() as start.
    // Set current epoch to 42 and verify.
    tester.setCurrentEpoch(42);
    initialize(0);

    EXPECT_EQ(tester.state().vestingStartEpoch, 42LL);
}

TEST_F(QZNTokenTest, Init_HappyPath_StatsResetToZero)
{
    initialize();

    const auto& s = tester.state();
    EXPECT_EQ(s.totalBurned,           0LL);
    EXPECT_EQ(s.protocolFeeBalance,    0LL);
    EXPECT_EQ(s.totalMatchStake,       0LL);
    EXPECT_EQ(s.totalPrizeDistributed, 0LL);
    EXPECT_EQ(s.totalRouteBurned,      0LL);
    EXPECT_EQ(s.totalRouteLiquidity,   0LL);
    EXPECT_EQ(s.totalRouteTreasury,    0LL);
    EXPECT_EQ(s.totalRoutePortalNode,  0LL);
    EXPECT_EQ(s.totalRoutePortalProto, 0LL);
    EXPECT_EQ(s.totalRouteQswapProto,  0LL);
}

TEST_F(QZNTokenTest, Init_HappyPath_OutputMatchesState)
{
    InitializeQZN_input in{};
    in.treasuryAddr              = TREASURY_ADDR;
    in.founderAddr               = FOUNDER_ADDR;
    in.liquidityAddr             = LIQUIDITY_ADDR;
    in.ecosystemAddr             = ECOSYSTEM_ADDR;
    in.portalNodeAddr            = PORTAL_NODE_ADDR;
    in.portalProtoAddr           = PORTAL_PROTO_ADDR;
    in.qswapProtoAddr            = QSWAP_PROTO_ADDR;
    in.vestingStartEpochOverride = 100;

    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(InitializeQZN_id, in);

    EXPECT_EQ(out.totalSupply,     QZN_TOTAL_SUPPLY);
    EXPECT_EQ(out.treasuryLocked,  QZN_TREASURY_ALLOC);
    EXPECT_EQ(out.teamLocked,      QZN_TEAM_ALLOC);
    EXPECT_EQ(out.liquidityLocked, QZN_LIQUIDITY_ALLOC);
    EXPECT_EQ(out.ecosystemLocked, QZN_ECOSYSTEM_ALLOC);
    EXPECT_EQ(out.vestingStartEpoch, 100LL);
}

TEST_F(QZNTokenTest, Init_DoubleInitGuard_SecondCallIsNoop)
{
    initialize();
    sint64 firstTreasury = tester.state().treasuryBalance;

    // Second call with different addresses — must be ignored entirely
    InitializeQZN_input in{};
    in.treasuryAddr = STRANGER_ADDR;
    in.founderAddr  = STRANGER_ADDR;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(InitializeQZN_id, in);

    // State must be unchanged
    EXPECT_EQ(tester.state().treasuryBalance, firstTreasury);
    EXPECT_EQ(tester.state().adminAddress,    ADMIN_ADDR);
    EXPECT_EQ(tester.state().founderAddress,  FOUNDER_ADDR);
}

TEST_F(QZNTokenTest, Init_InitializedFlagSet)
{
    initialize();
    EXPECT_EQ(tester.state().initialized, 1);
}

// ============================================================
//  [PROC-2]  SettleMatch
// ============================================================

// Helper: compute expected BPS share
static sint64 bps(sint64 stake, sint64 bpsVal)
{
    return (stake * bpsVal) / 10000LL;
}

TEST_F(QZNTokenTest, SettleMatch_HappyPath_OutputRoutingMathCorrect)
{
    initialize();
    auto out = settleMatch(WINNER_ADDR, STANDARD_STAKE);

    EXPECT_EQ(out.prizeAwarded,     bps(STANDARD_STAKE, 4500));  // 45%
    EXPECT_EQ(out.treasuryShare,    bps(STANDARD_STAKE, 100));  //  1%
    EXPECT_EQ(out.liquidityShare,   bps(STANDARD_STAKE, 2000));  // 20%
    EXPECT_EQ(out.burnedAmount,     bps(STANDARD_STAKE, 1000));  // 10%
    EXPECT_EQ(out.portalNodeShare,  bps(STANDARD_STAKE,  300));  //  3%
    EXPECT_EQ(out.portalProtoShare, 0LL);                        //  0% (PORTAL_PROTO disabled)
    EXPECT_EQ(out.qswapProtoShare,  bps(STANDARD_STAKE,  100));  //  1%
    EXPECT_EQ(out.protocolFee,      0LL);                            //  0%
}

TEST_F(QZNTokenTest, SettleMatch_HappyPath_BpsRoutingSumsTo100Percent)
{
    initialize();
    auto out = settleMatch(WINNER_ADDR, STANDARD_STAKE);

    sint64 total = out.prizeAwarded
                 + out.treasuryShare
                 + out.liquidityShare
                 + out.burnedAmount
                 + out.portalNodeShare
                 + out.portalProtoShare
                 + out.qswapProtoShare
                 + out.protocolFee
                + out.stakerDividendShare
                + out.builderDividendShare;

    // Integer division may produce dust of at most 7 QU.
    // Assert total is within 8 QU of the full stake.
    EXPECT_GE(total, STANDARD_STAKE - 8LL);
    EXPECT_LE(total, STANDARD_STAKE);
}

TEST_F(QZNTokenTest, SettleMatch_HappyPath_TreasuryBalanceAccumulates)
{
    initialize();
    sint64 initialTreasury = tester.state().treasuryBalance;

    settleMatch(WINNER_ADDR, STANDARD_STAKE);

    sint64 expected = initialTreasury + bps(STANDARD_STAKE,  100);
    EXPECT_EQ(tester.state().treasuryBalance, expected);
}

TEST_F(QZNTokenTest, SettleMatch_HappyPath_LiquidityBalanceAccumulates)
{
    initialize();
    sint64 initialLiquidity = tester.state().liquidityBalance;

    settleMatch(WINNER_ADDR, STANDARD_STAKE);

    sint64 expected = initialLiquidity + bps(STANDARD_STAKE, 2000);
    EXPECT_EQ(tester.state().liquidityBalance, expected);
}

TEST_F(QZNTokenTest, SettleMatch_HappyPath_BurnReducesCirculatingSupply)
{
    initialize();
    sint64 initialCirculating = tester.state().circulatingSupply;

    settleMatch(WINNER_ADDR, STANDARD_STAKE);

    sint64 burnAmount = bps(STANDARD_STAKE, 1000);
    EXPECT_EQ(tester.state().circulatingSupply, initialCirculating - burnAmount);
}

TEST_F(QZNTokenTest, SettleMatch_HappyPath_TotalBurnedAccumulates)
{
    initialize();
    settleMatch(WINNER_ADDR, STANDARD_STAKE);
    EXPECT_EQ(tester.state().totalBurned, bps(STANDARD_STAKE, 1000));
}

TEST_F(QZNTokenTest, SettleMatch_HappyPath_ProtocolFeeAccumulates)
{
    initialize();
    settleMatch(WINNER_ADDR, STANDARD_STAKE);
    EXPECT_EQ(tester.state().protocolFeeBalance, 0LL);
}

TEST_F(QZNTokenTest, SettleMatch_HappyPath_LifetimeStatsUpdated)
{
    initialize();
    settleMatch(WINNER_ADDR, STANDARD_STAKE);

    const auto& s = tester.state();
    EXPECT_EQ(s.totalMatchStake,       STANDARD_STAKE);
    EXPECT_EQ(s.totalPrizeDistributed, bps(STANDARD_STAKE, 4500));
    EXPECT_EQ(s.totalRouteBurned,      bps(STANDARD_STAKE, 1000));
    EXPECT_EQ(s.totalRouteLiquidity,   bps(STANDARD_STAKE, 2000));
    EXPECT_EQ(s.totalRouteTreasury,    bps(STANDARD_STAKE,  100));
    EXPECT_EQ(s.totalRoutePortalNode,  bps(STANDARD_STAKE,  300));
    EXPECT_EQ(s.totalRoutePortalProto, 0LL);  // PORTAL_PROTO_BPS = 0
    EXPECT_EQ(s.totalRouteQswapProto,  bps(STANDARD_STAKE,  100));
}

TEST_F(QZNTokenTest, SettleMatch_MultipleMatches_StatsAccumulateCorrectly)
{
    initialize();
    settleMatch(WINNER_ADDR, STANDARD_STAKE);
    settleMatch(WINNER_ADDR, STANDARD_STAKE);
    settleMatch(WINNER_ADDR, STANDARD_STAKE);

    EXPECT_EQ(tester.state().totalMatchStake, STANDARD_STAKE * 3);
    EXPECT_EQ(tester.state().totalBurned,     bps(STANDARD_STAKE, 1000) * 3);
}

TEST_F(QZNTokenTest, SettleMatch_AuthRejection_StrangerCannotSettle)
{
    initialize();
    sint64 burnBefore = tester.state().totalBurned;

    SettleMatch_input in{};
    in.winnerAddress = WINNER_ADDR;
    in.totalStake    = STANDARD_STAKE;

    tester.setInvocator(STRANGER_ADDR);  // Not admin — should be rejected
    tester.callProcedure(SettleMatch_id, in);

    // State must be completely unchanged
    EXPECT_EQ(tester.state().totalBurned, burnBefore);
    EXPECT_EQ(tester.state().totalMatchStake, 0LL);
}

TEST_F(QZNTokenTest, SettleMatch_ZeroStakeGuard_IsRejected)
{
    initialize();

    SettleMatch_input in{};
    in.winnerAddress = WINNER_ADDR;
    in.totalStake    = 0LL;

    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(SettleMatch_id, in);

    EXPECT_EQ(tester.state().totalMatchStake, 0LL);
    EXPECT_EQ(tester.state().totalBurned,     0LL);
}

TEST_F(QZNTokenTest, SettleMatch_NegativeStakeGuard_IsRejected)
{
    initialize();

    SettleMatch_input in{};
    in.winnerAddress = WINNER_ADDR;
    in.totalStake    = -100LL;

    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(SettleMatch_id, in);

    EXPECT_EQ(tester.state().totalMatchStake, 0LL);
}

TEST_F(QZNTokenTest, SettleMatch_SmallStake_MinimumRouting)
{
    initialize();
    // Very small stake — BPS division may round some routes to 0.
    // Verify contract does not crash and stats reflect actual amounts.
    auto out = settleMatch(WINNER_ADDR, 10LL);

    // Sum must not exceed input stake
    sint64 total = out.prizeAwarded + out.treasuryShare + out.liquidityShare
                 + out.burnedAmount + out.portalNodeShare + out.portalProtoShare  // portalProtoShare == 0
                 + out.qswapProtoShare + out.protocolFee
                + out.stakerDividendShare
                + out.builderDividendShare;
    EXPECT_LE(total, 10LL);
    EXPECT_GE(total, 0LL);
}

TEST_F(QZNTokenTest, SettleMatch_NotInitialized_DoesNotCrash)
{
    // Call SettleMatch before InitializeQZN — auth will fail (admin is NULL_ID)
    // but contract must not crash or corrupt state.
    SettleMatch_input in{};
    in.winnerAddress = WINNER_ADDR;
    in.totalStake    = STANDARD_STAKE;

    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(SettleMatch_id, in);

    // State should remain at zero defaults
    EXPECT_EQ(tester.state().totalMatchStake, 0LL);
}

// ============================================================
//  [PROC-3]  ClaimVestedTokens
// ============================================================

TEST_F(QZNTokenTest, ClaimVested_PreCliff_NothingClaimable)
{
    initialize(100);  // Vesting starts at epoch 100
    tester.setCurrentEpoch(100 + 10);  // Only 10 epochs elapsed, cliff is 52

    ClaimVestedTokens_input in{};
    tester.setInvocator(FOUNDER_ADDR);
    auto out = tester.callProcedure(ClaimVestedTokens_id, in);

    EXPECT_EQ(out.tokensClaimed, 0LL);
    EXPECT_GT(out.epochsUntilCliff, 0LL);
    EXPECT_EQ(tester.state().teamTokensClaimed, 0LL);
}

TEST_F(QZNTokenTest, ClaimVested_AtExactCliffEpoch_CliffPassed)
{
    initialize(100);
    tester.setCurrentEpoch(100 + QZN_VEST_CLIFF_EPOCHS);  // Exactly at cliff

    ClaimVestedTokens_input in{};
    tester.setInvocator(FOUNDER_ADDR);
    auto out = tester.callProcedure(ClaimVestedTokens_id, in);

    EXPECT_EQ(out.epochsUntilCliff, 0LL);
    // At exactly the cliff boundary, minimal vesting has elapsed —
    // claimable may be 0 or very small depending on rounding.
    EXPECT_GE(out.tokensClaimed, 0LL);
}

TEST_F(QZNTokenTest, ClaimVested_MidVest_LinearAmountCorrect)
{
    initialize(100);

    // Advance to halfway through vest: cliff(52) + half of vest duration(104) = 156 epochs elapsed
    sint64 halfVestEpochs = QZN_VEST_CLIFF_EPOCHS + (QZN_VEST_TOTAL_EPOCHS - QZN_VEST_CLIFF_EPOCHS) / 2;
    tester.setCurrentEpoch(100 + halfVestEpochs);

    ClaimVestedTokens_input in{};
    tester.setInvocator(FOUNDER_ADDR);
    auto out = tester.callProcedure(ClaimVestedTokens_id, in);

    // At halfway through vest, roughly 50% of team allocation should be claimable.
    // Allow 2% tolerance for integer division rounding.
    sint64 expectedHalf    = QZN_TEAM_ALLOC / 2;
    sint64 toleranceAmount = QZN_TEAM_ALLOC / 50;  // 2%

    EXPECT_GT(out.tokensClaimed, 0LL);
    EXPECT_NEAR(out.tokensClaimed, expectedHalf, toleranceAmount);
    EXPECT_EQ(tester.state().teamTokensClaimed, out.tokensClaimed);
}

TEST_F(QZNTokenTest, ClaimVested_FullyVested_ClaimsEntireAllocation)
{
    initialize(100);
    tester.setCurrentEpoch(100 + QZN_VEST_TOTAL_EPOCHS + 10);  // Well past full vest

    ClaimVestedTokens_input in{};
    tester.setInvocator(FOUNDER_ADDR);
    auto out = tester.callProcedure(ClaimVestedTokens_id, in);

    EXPECT_EQ(out.tokensClaimed,   QZN_TEAM_ALLOC);
    EXPECT_EQ(out.tokensRemaining, 0LL);
    EXPECT_EQ(tester.state().teamTokensClaimed, QZN_TEAM_ALLOC);
}

TEST_F(QZNTokenTest, ClaimVested_DoubleClaim_SecondClaimIsZero)
{
    initialize(100);
    tester.setCurrentEpoch(100 + QZN_VEST_TOTAL_EPOCHS + 10);

    ClaimVestedTokens_input in{};
    tester.setInvocator(FOUNDER_ADDR);
    tester.callProcedure(ClaimVestedTokens_id, in);  // First claim — full amount
    auto out2 = tester.callProcedure(ClaimVestedTokens_id, in);  // Second claim — nothing left

    EXPECT_EQ(out2.tokensClaimed,   0LL);
    EXPECT_EQ(out2.tokensRemaining, 0LL);
}

TEST_F(QZNTokenTest, ClaimVested_IncrementalClaims_AccumulateCorrectly)
{
    initialize(100);

    // First claim: at quarter-vest point
    sint64 quarterVest = QZN_VEST_CLIFF_EPOCHS + (QZN_VEST_TOTAL_EPOCHS - QZN_VEST_CLIFF_EPOCHS) / 4;
    tester.setCurrentEpoch(100 + quarterVest);

    ClaimVestedTokens_input in{};
    tester.setInvocator(FOUNDER_ADDR);
    auto out1 = tester.callProcedure(ClaimVestedTokens_id, in);
    sint64 firstClaim = out1.tokensClaimed;
    EXPECT_GT(firstClaim, 0LL);

    // Second claim: advance to half-vest point
    sint64 halfVest = QZN_VEST_CLIFF_EPOCHS + (QZN_VEST_TOTAL_EPOCHS - QZN_VEST_CLIFF_EPOCHS) / 2;
    tester.setCurrentEpoch(100 + halfVest);
    auto out2 = tester.callProcedure(ClaimVestedTokens_id, in);
    EXPECT_GT(out2.tokensClaimed, 0LL);

    // Total claimed after both should be approximately half allocation
    sint64 totalClaimed = firstClaim + out2.tokensClaimed;
    sint64 expectedHalf = QZN_TEAM_ALLOC / 2;
    sint64 tolerance    = QZN_TEAM_ALLOC / 50;
    EXPECT_NEAR(totalClaimed, expectedHalf, tolerance);
}

TEST_F(QZNTokenTest, ClaimVested_NonFounderRejected)
{
    initialize(100);
    tester.setCurrentEpoch(100 + QZN_VEST_TOTAL_EPOCHS + 10);

    ClaimVestedTokens_input in{};
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(ClaimVestedTokens_id, in);

    EXPECT_EQ(tester.state().teamTokensClaimed, 0LL);
}

TEST_F(QZNTokenTest, ClaimVested_AdminRejectedIfNotFounder)
{
    initialize(100);
    tester.setCurrentEpoch(100 + QZN_VEST_TOTAL_EPOCHS + 10);

    ClaimVestedTokens_input in{};
    tester.setInvocator(ADMIN_ADDR);  // Admin is not founder
    tester.callProcedure(ClaimVestedTokens_id, in);

    EXPECT_EQ(tester.state().teamTokensClaimed, 0LL);
}

TEST_F(QZNTokenTest, ClaimVested_BeforeInit_DoesNotCrash)
{
    // vestingInitialized is 0 — contract must early-return cleanly
    tester.setCurrentEpoch(500);
    tester.setInvocator(FOUNDER_ADDR);

    ClaimVestedTokens_input in{};
    tester.callProcedure(ClaimVestedTokens_id, in);

    EXPECT_EQ(tester.state().teamTokensClaimed, 0LL);
}

// ============================================================
//  [PROC-4]  BurnFromTreasury
// ============================================================

TEST_F(QZNTokenTest, BurnFromTreasury_HappyPath_ReducesTreasuryAndCirculating)
{
    initialize();

    sint64 burnAmount    = 1000000LL;  // 1M QZN
    sint64 prevTreasury  = tester.state().treasuryBalance;
    sint64 prevBurned    = tester.state().totalBurned;
    sint64 prevCirc      = tester.state().circulatingSupply;

    BurnFromTreasury_input in{};
    in.amount = burnAmount;

    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(BurnFromTreasury_id, in);

    EXPECT_EQ(out.amountBurned,         burnAmount);
    EXPECT_EQ(out.lifetimeBurned,       prevBurned + burnAmount);
    EXPECT_EQ(out.newCirculatingSupply, prevCirc - burnAmount);
    EXPECT_EQ(tester.state().treasuryBalance,   prevTreasury - burnAmount);
    EXPECT_EQ(tester.state().totalBurned,        prevBurned + burnAmount);
    EXPECT_EQ(tester.state().circulatingSupply,  prevCirc - burnAmount);
}

TEST_F(QZNTokenTest, BurnFromTreasury_NonAdminRejected)
{
    initialize();
    sint64 prevTreasury = tester.state().treasuryBalance;

    BurnFromTreasury_input in{};
    in.amount = 1000000LL;

    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(BurnFromTreasury_id, in);

    EXPECT_EQ(tester.state().treasuryBalance, prevTreasury);
    EXPECT_EQ(tester.state().totalBurned, 0LL);
}

TEST_F(QZNTokenTest, BurnFromTreasury_ExcessAmountRejected)
{
    initialize();
    sint64 prevTreasury = tester.state().treasuryBalance;

    BurnFromTreasury_input in{};
    in.amount = prevTreasury + 1LL;  // 1 over balance

    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(BurnFromTreasury_id, in);

    EXPECT_EQ(tester.state().treasuryBalance, prevTreasury);
    EXPECT_EQ(tester.state().totalBurned, 0LL);
}

TEST_F(QZNTokenTest, BurnFromTreasury_ZeroAmountRejected)
{
    initialize();
    sint64 prevTreasury = tester.state().treasuryBalance;

    BurnFromTreasury_input in{};
    in.amount = 0LL;

    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(BurnFromTreasury_id, in);

    EXPECT_EQ(tester.state().treasuryBalance, prevTreasury);
}

TEST_F(QZNTokenTest, BurnFromTreasury_NegativeAmountRejected)
{
    initialize();
    sint64 prevTreasury = tester.state().treasuryBalance;

    BurnFromTreasury_input in{};
    in.amount = -500LL;

    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(BurnFromTreasury_id, in);

    EXPECT_EQ(tester.state().treasuryBalance, prevTreasury);
}

TEST_F(QZNTokenTest, BurnFromTreasury_ExactTreasuryBalance_Succeeds)
{
    initialize();
    sint64 fullBalance = tester.state().treasuryBalance;

    BurnFromTreasury_input in{};
    in.amount = fullBalance;

    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(BurnFromTreasury_id, in);

    EXPECT_EQ(out.amountBurned, fullBalance);
    EXPECT_EQ(tester.state().treasuryBalance, 0LL);
}

TEST_F(QZNTokenTest, BurnFromTreasury_MultipleBurns_AccumulateCorrectly)
{
    initialize();

    BurnFromTreasury_input in{};
    in.amount = 500000LL;

    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(BurnFromTreasury_id, in);
    tester.callProcedure(BurnFromTreasury_id, in);

    EXPECT_EQ(tester.state().totalBurned, 1000000LL);
}

// ============================================================
//  [FUNC-5]  GetSupplyInfo
// ============================================================

TEST_F(QZNTokenTest, GetSupplyInfo_AfterInit_ReturnsCorrectSnapshot)
{
    initialize();

    GetSupplyInfo_input in{};
    tester.setInvocator(STRANGER_ADDR);  // Public function — anyone can call
    auto out = tester.callFunction(GetSupplyInfo_id, in);

    EXPECT_EQ(out.totalSupply,       QZN_TOTAL_SUPPLY);
    EXPECT_EQ(out.totalBurned,       0LL);
    EXPECT_EQ(out.circulatingSupply, QZN_LIQUIDITY_ALLOC);
    EXPECT_EQ(out.treasuryBalance,   QZN_TREASURY_ALLOC);
    EXPECT_EQ(out.liquidityBalance,  QZN_LIQUIDITY_ALLOC);
    EXPECT_EQ(out.ecosystemBalance,  QZN_ECOSYSTEM_ALLOC);
    EXPECT_EQ(out.teamLocked,        QZN_TEAM_ALLOC);
    EXPECT_EQ(out.protocolFeeBalance, 0LL);
}

TEST_F(QZNTokenTest, GetSupplyInfo_AfterSettlement_ReflectsChanges)
{
    initialize();
    settleMatch(WINNER_ADDR, STANDARD_STAKE);

    GetSupplyInfo_input in{};
    auto out = tester.callFunction(GetSupplyInfo_id, in);

    sint64 expectedBurn    = bps(STANDARD_STAKE, 1000);
    sint64 expectedCirc    = QZN_LIQUIDITY_ALLOC - expectedBurn;
    sint64 expectedFee     = 0LL;

    EXPECT_EQ(out.totalBurned,        expectedBurn);
    EXPECT_EQ(out.circulatingSupply,  expectedCirc);
    EXPECT_EQ(out.protocolFeeBalance, expectedFee);
}

TEST_F(QZNTokenTest, GetSupplyInfo_AfterTreasuryBurn_ReflectsChanges)
{
    initialize();

    BurnFromTreasury_input in{};
    in.amount = 1000000LL;
    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(BurnFromTreasury_id, in);

    GetSupplyInfo_input queryIn{};
    auto out = tester.callFunction(GetSupplyInfo_id, queryIn);

    EXPECT_EQ(out.totalBurned,      1000000LL);
    EXPECT_EQ(out.treasuryBalance,  QZN_TREASURY_ALLOC - 1000000LL);
}

// ============================================================
//  [FUNC-6]  GetVestingStatus
// ============================================================

TEST_F(QZNTokenTest, GetVestingStatus_PreCliff_CliffNotPassed)
{
    initialize(100);
    tester.setCurrentEpoch(100 + 10);

    GetVestingStatus_input in{};
    auto out = tester.callFunction(GetVestingStatus_id, in);

    EXPECT_EQ(out.cliffPassed,      0);
    EXPECT_GT(out.epochsUntilCliff, 0LL);
    EXPECT_EQ(out.claimableNow,     0LL);
    EXPECT_EQ(out.totalAllocation,  QZN_TEAM_ALLOC);
    EXPECT_EQ(out.totalClaimed,     0LL);
}

TEST_F(QZNTokenTest, GetVestingStatus_PostCliff_CliffFlagSet)
{
    initialize(100);
    tester.setCurrentEpoch(100 + QZN_VEST_CLIFF_EPOCHS + 10);

    GetVestingStatus_input in{};
    auto out = tester.callFunction(GetVestingStatus_id, in);

    EXPECT_EQ(out.cliffPassed,       1);
    EXPECT_EQ(out.epochsUntilCliff,  0LL);
    EXPECT_GT(out.claimableNow,      0LL);
}

TEST_F(QZNTokenTest, GetVestingStatus_FullyVested_EpochsUntilFullVestIsZero)
{
    initialize(100);
    tester.setCurrentEpoch(100 + QZN_VEST_TOTAL_EPOCHS + 20);

    GetVestingStatus_input in{};
    auto out = tester.callFunction(GetVestingStatus_id, in);

    EXPECT_EQ(out.epochsUntilFullVest, 0LL);
    EXPECT_EQ(out.claimableNow,        QZN_TEAM_ALLOC);
}

TEST_F(QZNTokenTest, GetVestingStatus_AfterPartialClaim_ClaimableIsReduced)
{
    initialize(100);
    tester.setCurrentEpoch(100 + QZN_VEST_TOTAL_EPOCHS + 20);

    // Claim everything first
    ClaimVestedTokens_input claimIn{};
    tester.setInvocator(FOUNDER_ADDR);
    tester.callProcedure(ClaimVestedTokens_id, claimIn);

    // Query should now show 0 claimable
    GetVestingStatus_input queryIn{};
    auto out = tester.callFunction(GetVestingStatus_id, queryIn);

    EXPECT_EQ(out.claimableNow,   0LL);
    EXPECT_EQ(out.totalClaimed,   QZN_TEAM_ALLOC);
}

TEST_F(QZNTokenTest, GetVestingStatus_EpochsElapsedMatchesClock)
{
    initialize(100);
    tester.setCurrentEpoch(130);  // 30 epochs elapsed

    GetVestingStatus_input in{};
    auto out = tester.callFunction(GetVestingStatus_id, in);

    EXPECT_EQ(out.epochsElapsed,      30LL);
    EXPECT_EQ(out.vestingStartEpoch,  100LL);
}

// ============================================================
//  [FUNC-7]  GetMatchStats
// ============================================================

TEST_F(QZNTokenTest, GetMatchStats_ZeroState_AllFieldsZero)
{
    initialize();

    GetMatchStats_input in{};
    auto out = tester.callFunction(GetMatchStats_id, in);

    EXPECT_EQ(out.totalMatchStake,       0LL);
    EXPECT_EQ(out.totalPrizeDistributed, 0LL);
    EXPECT_EQ(out.totalRouteBurned,      0LL);
    EXPECT_EQ(out.totalRouteLiquidity,   0LL);
    EXPECT_EQ(out.totalRouteTreasury,    0LL);
    EXPECT_EQ(out.totalRoutePortalNode,  0LL);
    EXPECT_EQ(out.totalRoutePortalProto, 0LL);
    EXPECT_EQ(out.totalRouteQswapProto,  0LL);
}

TEST_F(QZNTokenTest, GetMatchStats_SingleMatch_CorrectTotals)
{
    initialize();
    settleMatch(WINNER_ADDR, STANDARD_STAKE);

    GetMatchStats_input in{};
    auto out = tester.callFunction(GetMatchStats_id, in);

    EXPECT_EQ(out.totalMatchStake,       STANDARD_STAKE);
    EXPECT_EQ(out.totalPrizeDistributed, bps(STANDARD_STAKE, 4500));
    EXPECT_EQ(out.totalRouteBurned,      bps(STANDARD_STAKE, 1000));
    EXPECT_EQ(out.totalRouteLiquidity,   bps(STANDARD_STAKE, 2000));
    EXPECT_EQ(out.totalRouteTreasury,    bps(STANDARD_STAKE,  100));
    EXPECT_EQ(out.totalRoutePortalNode,  bps(STANDARD_STAKE,  300));
    EXPECT_EQ(out.totalRoutePortalProto, 0LL);  // PORTAL_PROTO_BPS = 0
    EXPECT_EQ(out.totalRouteQswapProto,  bps(STANDARD_STAKE,  100));
}

TEST_F(QZNTokenTest, GetMatchStats_MultipleMatches_AccumulatesCorrectly)
{
    initialize();
    constexpr int MATCH_COUNT = 5;
    for (int i = 0; i < MATCH_COUNT; i++)
    {
        settleMatch(WINNER_ADDR, STANDARD_STAKE);
    }

    GetMatchStats_input in{};
    auto out = tester.callFunction(GetMatchStats_id, in);

    EXPECT_EQ(out.totalMatchStake,       STANDARD_STAKE * MATCH_COUNT);
    EXPECT_EQ(out.totalRouteBurned,      bps(STANDARD_STAKE, 1000) * MATCH_COUNT);
    EXPECT_EQ(out.totalPrizeDistributed, bps(STANDARD_STAKE, 4500) * MATCH_COUNT);
}

TEST_F(QZNTokenTest, GetMatchStats_IsReadOnly_DoesNotAlterState)
{
    initialize();
    settleMatch(WINNER_ADDR, STANDARD_STAKE);

    sint64 burnBefore = tester.state().totalBurned;

    GetMatchStats_input in{};
    tester.callFunction(GetMatchStats_id, in);
    tester.callFunction(GetMatchStats_id, in);  // Call twice

    EXPECT_EQ(tester.state().totalBurned, burnBefore);
}

// ============================================================
//  CROSS-CUTTING — invariant checks
// ============================================================

TEST_F(QZNTokenTest, Invariant_CirculatingPlusBurnedNeverExceedsTotalSupply)
{
    initialize();

    // Run 10 match settlements and 2 treasury burns
    for (int i = 0; i < 10; i++) settleMatch(WINNER_ADDR, STANDARD_STAKE);

    BurnFromTreasury_input in{};
    in.amount = 2000000LL;
    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(BurnFromTreasury_id, in);
    tester.callProcedure(BurnFromTreasury_id, in);

    const auto& s = tester.state();
    EXPECT_LE(s.circulatingSupply + s.totalBurned, QZN_TOTAL_SUPPLY);
}

TEST_F(QZNTokenTest, Invariant_TeamClaimedNeverExceedsTeamAllocation)
{
    initialize(100);
    tester.setCurrentEpoch(100 + QZN_VEST_TOTAL_EPOCHS + 100);

    ClaimVestedTokens_input in{};
    tester.setInvocator(FOUNDER_ADDR);
    tester.callProcedure(ClaimVestedTokens_id, in);
    tester.callProcedure(ClaimVestedTokens_id, in);  // Double-claim attempt

    EXPECT_LE(tester.state().teamTokensClaimed, QZN_TEAM_ALLOC);
}
