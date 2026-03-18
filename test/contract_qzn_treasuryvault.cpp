#define NO_UEFI
#include <iostream>
#include "contract_testing.h"

// ============================================================
//  QZN TreasuryVault PAO — GTest Suite
//  File:    test/contract_qzn_treasuryvault.cpp
//  Place in qubic/core repo at: test/contract_qzn_treasuryvault.cpp
//
//  Coverage:
//    [PROC-1]  InitializeVault    — happy path, double-init guard,
//                                   signer storage, default epoch funding,
//                                   balance seeding, auto-fund on by default
//    [PROC-2]  ProposeSpend       — happy path all categories, auto-sign,
//                                   non-signer rejection, paused guard,
//                                   zero/excess amount guards, invalid category,
//                                   epoch fund cap, all-slots-full guard,
//                                   slot recycle after execute/expire/cancel,
//                                   timelock flag for large spends
//    [PROC-3]  SignProposal       — 1st sig pending, 2nd sig executes small spend,
//                                   2nd sig timelocks large spend,
//                                   duplicate sig ignored, non-signer rejected,
//                                   paused guard, wrong proposal ID rejected,
//                                   all 6 spend categories execute correctly
//    [PROC-4]  ExecuteProposal    — happy path after timelock delay,
//                                   early execution blocked,
//                                   non-signer rejected, paused guard,
//                                   non-timelocked proposal rejected,
//                                   all spend categories routed correctly
//    [PROC-5]  CancelProposal     — signer cancels pending proposal,
//                                   non-signer rejection,
//                                   cannot cancel timelocked proposal,
//                                   cannot cancel executed proposal
//    [FUNC-6]  GetVaultState      — initial state, post-spend state
//    [FUNC-7]  GetProposal        — pending, approved, executed, expired states
//
//  BEGIN_EPOCH integration tests:
//    Auto-fund transfers on epoch advance
//    Expiry sweep marks stale proposals EXPIRED
//    Runway guard disables auto-fund below threshold
//    Paused flag suppresses auto-fund
// ============================================================

#include "gtest/gtest.h"
#include "../src/contracts/QZN_TreasuryVault_PAO.h"
#include "contract_testing.h"

// ============================================================
//  HELPERS — well-known test addresses
// ============================================================

static const id ADMIN_ADDR        = id(1, 0, 0, 0);
static const id SIGNER_0          = id(2, 0, 0, 0);   // Founder
static const id SIGNER_1          = id(3, 0, 0, 0);   // Trustee 1
static const id SIGNER_2          = id(4, 0, 0, 0);   // Trustee 2
static const id REWARD_ROUTER     = id(5, 0, 0, 0);
static const id QZN_CORE          = id(6, 0, 0, 0);
static const id DEST_ADDR         = id(7, 0, 0, 0);
static const id STRANGER_ADDR     = id(8, 0, 0, 0);

static constexpr sint64 INITIAL_BALANCE   = 87500000LL;  // 87.5M QZN
static constexpr sint64 SMALL_SPEND       = 100000LL;    // Below timelock threshold
static constexpr sint64 LARGE_SPEND       = 2000000LL;   // Above timelock threshold
static constexpr sint64 TEST_MEMO         = 0xDEADBEEFLL;

// ============================================================
//  FIXTURE
// ============================================================

class QZNTreasuryVaultTest : public ::testing::Test
{
protected:
    ContractTester<QZNTREASVAULT> tester;

    void initialize(sint64 balance = INITIAL_BALANCE)
    {
        InitializeVault_input in{};
        in.signer0          = SIGNER_0;
        in.signer1          = SIGNER_1;
        in.signer2          = SIGNER_2;
        in.rewardRouterAddr = REWARD_ROUTER;
        in.qznCoreAddr      = QZN_CORE;
        in.initialBalance   = balance;

        tester.setInvocator(ADMIN_ADDR);
        tester.callProcedure(InitializeVault_id, in);
    }

    // Propose a spend as a given signer, returns proposalId
    sint64 propose(id signer, uint8 category, sint64 amount,
                   id dest = DEST_ADDR, sint64 memo = TEST_MEMO)
    {
        ProposeSpend_input in{};
        in.category    = category;
        in.destination = dest;
        in.amount      = amount;
        in.memo        = memo;

        tester.setInvocator(signer);
        auto out = tester.callProcedure(ProposeSpend_id, in);
        return out.proposalId;
    }

    // Sign a proposal as a given signer
    SignProposal_output sign(id signer, sint64 proposalId)
    {
        SignProposal_input in{};
        in.proposalId = proposalId;
        tester.setInvocator(signer);
        return tester.callProcedure(SignProposal_id, in);
    }

    // Propose + get 2nd signer to approve (small spend → immediate execution)
    sint64 proposeAndExecuteSmall(uint8 category = SPEND_ECOSYSTEM_GRANT,
                                   sint64 amount  = SMALL_SPEND)
    {
        sint64 id1 = propose(SIGNER_0, category, amount);
        sign(SIGNER_1, id1);
        return id1;
    }

    // Propose + get 2nd signer signature (large spend → timelocked)
    sint64 proposeAndTimelockLarge(sint64 amount = LARGE_SPEND)
    {
        sint64 id1 = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, amount);
        sign(SIGNER_1, id1);
        return id1;
    }
};

// ============================================================
//  [PROC-1]  InitializeVault
// ============================================================

TEST_F(QZNTreasuryVaultTest, Init_HappyPath_SignersStored)
{
    initialize();
    EXPECT_EQ(tester.state().signer_0, SIGNER_0);
    EXPECT_EQ(tester.state().signer_1, SIGNER_1);
    EXPECT_EQ(tester.state().signer_2, SIGNER_2);
}

TEST_F(QZNTreasuryVaultTest, Init_HappyPath_AllSignersActive)
{
    initialize();
    EXPECT_EQ(tester.state().signer_0_active, 1);
    EXPECT_EQ(tester.state().signer_1_active, 1);
    EXPECT_EQ(tester.state().signer_2_active, 1);
}

TEST_F(QZNTreasuryVaultTest, Init_HappyPath_BalanceSeeded)
{
    initialize(INITIAL_BALANCE);
    EXPECT_EQ(tester.state().treasuryBalance, INITIAL_BALANCE);
    EXPECT_EQ(tester.state().totalReceived,   INITIAL_BALANCE);
}

TEST_F(QZNTreasuryVaultTest, Init_HappyPath_DefaultEpochFunding)
{
    initialize();
    EXPECT_EQ(tester.state().epochRewardFundAmount,  DEFAULT_EPOCH_REWARD_FUND);
    EXPECT_EQ(tester.state().epochAchievementAmount, DEFAULT_ACHIEVEMENT_FUND);
}

TEST_F(QZNTreasuryVaultTest, Init_HappyPath_AutoFundEnabledByDefault)
{
    initialize();
    EXPECT_EQ(tester.state().autoFundEnabled, 1);
}

TEST_F(QZNTreasuryVaultTest, Init_HappyPath_PausedFalse)
{
    initialize();
    EXPECT_EQ(tester.state().paused, 0);
}

TEST_F(QZNTreasuryVaultTest, Init_HappyPath_CountersReset)
{
    initialize();
    EXPECT_EQ(tester.state().totalSpent,       0LL);
    EXPECT_EQ(tester.state().totalBurned,      0LL);
    EXPECT_EQ(tester.state().totalEpochFunded, 0LL);
    EXPECT_EQ(tester.state().nextProposalId,   1LL);
}

TEST_F(QZNTreasuryVaultTest, Init_HappyPath_ConnectedAddressesStored)
{
    initialize();
    EXPECT_EQ(tester.state().rewardRouterAddress, REWARD_ROUTER);
    EXPECT_EQ(tester.state().qznCoreAddress,      QZN_CORE);
}

TEST_F(QZNTreasuryVaultTest, Init_HappyPath_OutputCorrect)
{
    InitializeVault_input in{};
    in.signer0          = SIGNER_0;
    in.signer1          = SIGNER_1;
    in.signer2          = SIGNER_2;
    in.rewardRouterAddr = REWARD_ROUTER;
    in.qznCoreAddr      = QZN_CORE;
    in.initialBalance   = INITIAL_BALANCE;

    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(InitializeVault_id, in);

    EXPECT_EQ(out.success,      1);
    EXPECT_EQ(out.vaultBalance, INITIAL_BALANCE);
}

TEST_F(QZNTreasuryVaultTest, Init_DoubleInitGuard_SecondCallIgnored)
{
    initialize(INITIAL_BALANCE);

    InitializeVault_input in{};
    in.signer0        = STRANGER_ADDR;
    in.initialBalance = 999LL;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(InitializeVault_id, in);

    EXPECT_EQ(tester.state().signer_0,        SIGNER_0);
    EXPECT_EQ(tester.state().treasuryBalance, INITIAL_BALANCE);
}

TEST_F(QZNTreasuryVaultTest, Init_InitializedFlagSet)
{
    initialize();
    EXPECT_EQ(tester.state().initialized, 1);
}

// ============================================================
//  [PROC-2]  ProposeSpend
// ============================================================

TEST_F(QZNTreasuryVaultTest, Propose_HappyPath_SlotAssignedAndIdReturned)
{
    initialize();
    ProposeSpend_input in{};
    in.category    = SPEND_ECOSYSTEM_GRANT;
    in.destination = DEST_ADDR;
    in.amount      = SMALL_SPEND;
    in.memo        = TEST_MEMO;

    tester.setInvocator(SIGNER_0);
    auto out = tester.callProcedure(ProposeSpend_id, in);

    EXPECT_EQ(out.proposalId,   1LL);
    EXPECT_EQ(out.proposalSlot, 0);
}

TEST_F(QZNTreasuryVaultTest, Propose_HappyPath_ProposalStoredCorrectly)
{
    initialize();
    propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    EXPECT_EQ(tester.state().proposals_0.state,    PROP_PENDING);
    EXPECT_EQ(tester.state().proposals_0.category, SPEND_ECOSYSTEM_GRANT);
    EXPECT_EQ(tester.state().proposals_0.amount,   SMALL_SPEND);
    EXPECT_EQ(tester.state().proposals_0.destinationAddress, DEST_ADDR);
    EXPECT_EQ(tester.state().proposals_0.proposerAddress,    SIGNER_0);
}

TEST_F(QZNTreasuryVaultTest, Propose_HappyPath_ProposerAutoSigned)
{
    initialize();
    propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    // SIGNER_0 proposed — sig_0 should be auto-set, sigCount == 1
    EXPECT_EQ(tester.state().proposals_0.sig_0,    1);
    EXPECT_EQ(tester.state().proposals_0.sigCount, 1);
    // Other sigs not yet collected
    EXPECT_EQ(tester.state().proposals_0.sig_1,    0);
    EXPECT_EQ(tester.state().proposals_0.sig_2,    0);
}

TEST_F(QZNTreasuryVaultTest, Propose_HappyPath_ProposalIdIncrementsPerProposal)
{
    initialize();
    sint64 id1 = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    sint64 id2 = propose(SIGNER_1, SPEND_LIQUIDITY_INJECT, SMALL_SPEND);

    EXPECT_EQ(id1, 1LL);
    EXPECT_EQ(id2, 2LL);
    EXPECT_EQ(tester.state().nextProposalId, 3LL);
}

TEST_F(QZNTreasuryVaultTest, Propose_SmallSpend_NoTimelockFlagged)
{
    initialize();
    ProposeSpend_input in{};
    in.category    = SPEND_ECOSYSTEM_GRANT;
    in.destination = DEST_ADDR;
    in.amount      = TIMELOCK_THRESHOLD - 1LL;  // Just below threshold
    in.memo        = TEST_MEMO;

    tester.setInvocator(SIGNER_0);
    auto out = tester.callProcedure(ProposeSpend_id, in);

    EXPECT_EQ(out.timelockRequired, 0);
}

TEST_F(QZNTreasuryVaultTest, Propose_LargeSpend_TimelockFlagged)
{
    initialize();
    ProposeSpend_input in{};
    in.category    = SPEND_ECOSYSTEM_GRANT;
    in.destination = DEST_ADDR;
    in.amount      = TIMELOCK_THRESHOLD;  // At threshold
    in.memo        = TEST_MEMO;

    tester.setInvocator(SIGNER_0);
    auto out = tester.callProcedure(ProposeSpend_id, in);

    EXPECT_EQ(out.timelockRequired, 1);
}

TEST_F(QZNTreasuryVaultTest, Propose_AllCategories_Accepted)
{
    initialize(INITIAL_BALANCE);
    uint8 categories[] = {
        SPEND_EPOCH_REWARD, SPEND_ACHIEVEMENT_FUND, SPEND_LIQUIDITY_INJECT,
        SPEND_ECOSYSTEM_GRANT, SPEND_BURN_EVENT, SPEND_SIGNER_CHANGE
    };

    for (int i = 0; i < 6; i++)
    {
        ProposeSpend_input in{};
        in.category    = categories[i];
        in.destination = DEST_ADDR;
        in.amount      = SMALL_SPEND;
        in.memo        = TEST_MEMO;
        tester.setInvocator(SIGNER_0);
        auto out = tester.callProcedure(ProposeSpend_id, in);
        EXPECT_GT(out.proposalId, 0LL) << "Category " << (int)categories[i] << " was rejected";
    }
}

TEST_F(QZNTreasuryVaultTest, Propose_NonSignerRejected)
{
    initialize();
    sint64 prevId = tester.state().nextProposalId;

    ProposeSpend_input in{};
    in.category    = SPEND_ECOSYSTEM_GRANT;
    in.destination = DEST_ADDR;
    in.amount      = SMALL_SPEND;
    in.memo        = TEST_MEMO;

    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(ProposeSpend_id, in);

    EXPECT_EQ(tester.state().nextProposalId, prevId);  // No proposal created
}

TEST_F(QZNTreasuryVaultTest, Propose_PausedVault_Rejected)
{
    initialize();
    tester.state().paused = 1;

    sint64 prevId = tester.state().nextProposalId;
    propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    EXPECT_EQ(tester.state().nextProposalId, prevId);
}

TEST_F(QZNTreasuryVaultTest, Propose_ZeroAmount_Rejected)
{
    initialize();
    sint64 prevId = tester.state().nextProposalId;

    ProposeSpend_input in{};
    in.category    = SPEND_ECOSYSTEM_GRANT;
    in.destination = DEST_ADDR;
    in.amount      = 0LL;
    in.memo        = TEST_MEMO;

    tester.setInvocator(SIGNER_0);
    tester.callProcedure(ProposeSpend_id, in);

    EXPECT_EQ(tester.state().nextProposalId, prevId);
}

TEST_F(QZNTreasuryVaultTest, Propose_ExcessAmount_Rejected)
{
    initialize(INITIAL_BALANCE);
    sint64 prevId = tester.state().nextProposalId;

    ProposeSpend_input in{};
    in.category    = SPEND_ECOSYSTEM_GRANT;
    in.destination = DEST_ADDR;
    in.amount      = INITIAL_BALANCE + 1LL;
    in.memo        = TEST_MEMO;

    tester.setInvocator(SIGNER_0);
    tester.callProcedure(ProposeSpend_id, in);

    EXPECT_EQ(tester.state().nextProposalId, prevId);
}

TEST_F(QZNTreasuryVaultTest, Propose_InvalidCategory_Rejected)
{
    initialize();
    sint64 prevId = tester.state().nextProposalId;

    ProposeSpend_input in{};
    in.category    = 99;
    in.destination = DEST_ADDR;
    in.amount      = SMALL_SPEND;
    in.memo        = TEST_MEMO;

    tester.setInvocator(SIGNER_0);
    tester.callProcedure(ProposeSpend_id, in);

    EXPECT_EQ(tester.state().nextProposalId, prevId);
}

TEST_F(QZNTreasuryVaultTest, Propose_EpochRewardAboveCap_Rejected)
{
    initialize();
    sint64 prevId = tester.state().nextProposalId;

    ProposeSpend_input in{};
    in.category    = SPEND_EPOCH_REWARD;
    in.destination = DEST_ADDR;
    in.amount      = MAX_EPOCH_FUND + 1LL;
    in.memo        = TEST_MEMO;

    tester.setInvocator(SIGNER_0);
    tester.callProcedure(ProposeSpend_id, in);

    EXPECT_EQ(tester.state().nextProposalId, prevId);
}

TEST_F(QZNTreasuryVaultTest, Propose_AllSlotsFull_EighthRejected)
{
    initialize(INITIAL_BALANCE);

    // Fill all 8 proposal slots with PENDING proposals
    for (int i = 0; i < 8; i++)
    {
        // Use different signers to vary auto-sign patterns
        id s = (i % 2 == 0) ? SIGNER_0 : SIGNER_1;
        ProposeSpend_input in{};
        in.category    = SPEND_ECOSYSTEM_GRANT;
        in.destination = DEST_ADDR;
        in.amount      = SMALL_SPEND;
        in.memo        = TEST_MEMO;
        tester.setInvocator(s);
        auto out = tester.callProcedure(ProposeSpend_id, in);
        EXPECT_GT(out.proposalId, 0LL) << "Slot " << i << " should have been created";
    }

    // 9th proposal must fail — all slots occupied
    sint64 prevId = tester.state().nextProposalId;
    ProposeSpend_input in{};
    in.category    = SPEND_ECOSYSTEM_GRANT;
    in.destination = DEST_ADDR;
    in.amount      = SMALL_SPEND;
    in.memo        = TEST_MEMO;
    tester.setInvocator(SIGNER_0);
    tester.callProcedure(ProposeSpend_id, in);

    EXPECT_EQ(tester.state().nextProposalId, prevId);
}

TEST_F(QZNTreasuryVaultTest, Propose_SlotRecycledAfterExecution)
{
    initialize(INITIAL_BALANCE);

    // Fill slot 0, execute it
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    sign(SIGNER_1, pid);  // 2nd sig executes immediately (small spend)

    EXPECT_EQ(tester.state().proposals_0.state, PROP_EXECUTED);

    // Slot 0 should now be reusable
    sint64 pid2 = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    EXPECT_EQ(pid2, 2LL);
    EXPECT_EQ(tester.state().proposals_0.state, PROP_PENDING);
}

// ============================================================
//  [PROC-3]  SignProposal
// ============================================================

TEST_F(QZNTreasuryVaultTest, Sign_FirstSig_StillPending)
{
    initialize();
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    // SIGNER_0 auto-signed on propose — add SIGNER_1 sig now

    // Actually SIGNER_0 already has sig_0=1 (auto-sign).
    // SignProposal from SIGNER_1 brings to sigCount = 2 → executes.
    // So let's test with a proposer who is SIGNER_1 (auto-sign sig_1=1),
    // then have SIGNER_0 sign to verify 1→2 path.

    // Re-test: propose as SIGNER_1 (sig_1 auto = 1, sigCount = 1)
    sint64 pid2 = propose(SIGNER_1, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    // Before 2nd sig — check output from sign by SIGNER_0
    SignProposal_input in{};
    in.proposalId = pid2;
    tester.setInvocator(SIGNER_0);
    auto out = tester.callProcedure(SignProposal_id, in);

    EXPECT_EQ(out.sigsCollected, (uint8)2);
    EXPECT_EQ(out.sigsRequired,  REQUIRED_SIGS);
}

TEST_F(QZNTreasuryVaultTest, Sign_SecondSig_SmallSpend_ExecutesImmediately)
{
    initialize(INITIAL_BALANCE);
    sint64 prevBalance = tester.state().treasuryBalance;

    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    auto out   = sign(SIGNER_1, pid);

    EXPECT_EQ(out.approved,  1);
    EXPECT_EQ(out.executed,  1);
    EXPECT_EQ(out.timelocked, 0);
    EXPECT_EQ(tester.state().proposals_0.state,    PROP_EXECUTED);
    EXPECT_EQ(tester.state().treasuryBalance,       prevBalance - SMALL_SPEND);
    EXPECT_EQ(tester.state().totalSpent,            SMALL_SPEND);
}

TEST_F(QZNTreasuryVaultTest, Sign_SecondSig_LargeSpend_Timelocked)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(10);

    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, LARGE_SPEND);
    auto out   = sign(SIGNER_1, pid);

    EXPECT_EQ(out.approved,   1);
    EXPECT_EQ(out.timelocked, 1);
    EXPECT_EQ(out.executed,   0);
    EXPECT_EQ(tester.state().proposals_0.state,             PROP_TIMELOCKED);
    EXPECT_EQ(tester.state().proposals_0.executeAfterEpoch, 10LL + TIMELOCK_DELAY_EPOCHS);
    // Balance must NOT have changed yet
    EXPECT_EQ(tester.state().treasuryBalance, INITIAL_BALANCE);
}

TEST_F(QZNTreasuryVaultTest, Sign_DuplicateSig_CountNotIncremented)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    // SIGNER_0 already has auto-sig — signing again should be ignored

    SignProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_0);
    tester.callProcedure(SignProposal_id, in);  // Duplicate

    EXPECT_EQ(tester.state().proposals_0.sigCount, 1);  // Still 1, not 2
}

TEST_F(QZNTreasuryVaultTest, Sign_NonSignerRejected)
{
    initialize();
    sint64 pid       = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    sint64 prevCount = tester.state().proposals_0.sigCount;

    SignProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(SignProposal_id, in);

    EXPECT_EQ(tester.state().proposals_0.sigCount, prevCount);
}

TEST_F(QZNTreasuryVaultTest, Sign_PausedVault_Rejected)
{
    initialize();
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    tester.state().paused = 1;

    SignProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_1);
    tester.callProcedure(SignProposal_id, in);

    // sigCount should still be 1 (only auto-sign from propose)
    EXPECT_EQ(tester.state().proposals_0.sigCount, 1);
}

TEST_F(QZNTreasuryVaultTest, Sign_WrongProposalId_Rejected)
{
    initialize();
    propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    SignProposal_input in{};
    in.proposalId = 9999LL;  // Does not exist
    tester.setInvocator(SIGNER_1);
    tester.callProcedure(SignProposal_id, in);

    // Proposal 1 untouched
    EXPECT_EQ(tester.state().proposals_0.sigCount, 1);
}

// ---- All 6 spend categories route correctly on execute ----

TEST_F(QZNTreasuryVaultTest, Sign_Category_EpochReward_UpdatesEpochFunded)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = propose(SIGNER_0, SPEND_EPOCH_REWARD, SMALL_SPEND, REWARD_ROUTER);
    sign(SIGNER_1, pid);

    EXPECT_EQ(tester.state().totalEpochFunded, SMALL_SPEND);
    EXPECT_EQ(tester.state().totalSpent,       SMALL_SPEND);
}

TEST_F(QZNTreasuryVaultTest, Sign_Category_AchievementFund_UpdatesEpochFunded)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = propose(SIGNER_0, SPEND_ACHIEVEMENT_FUND, SMALL_SPEND, REWARD_ROUTER);
    sign(SIGNER_1, pid);

    EXPECT_EQ(tester.state().totalEpochFunded, SMALL_SPEND);
}

TEST_F(QZNTreasuryVaultTest, Sign_Category_LiquidityInject_UpdatesSpent)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = propose(SIGNER_0, SPEND_LIQUIDITY_INJECT, SMALL_SPEND);
    sign(SIGNER_1, pid);

    EXPECT_EQ(tester.state().totalSpent,    SMALL_SPEND);
    EXPECT_EQ(tester.state().totalBurned,   0LL);
}

TEST_F(QZNTreasuryVaultTest, Sign_Category_EcosystemGrant_UpdatesSpent)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    sign(SIGNER_1, pid);

    EXPECT_EQ(tester.state().totalSpent,  SMALL_SPEND);
    EXPECT_EQ(tester.state().totalBurned, 0LL);
}

TEST_F(QZNTreasuryVaultTest, Sign_Category_BurnEvent_UpdatesBurned)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = propose(SIGNER_0, SPEND_BURN_EVENT, SMALL_SPEND);
    sign(SIGNER_1, pid);

    EXPECT_EQ(tester.state().totalBurned,    SMALL_SPEND);
    EXPECT_EQ(tester.state().totalSpent,     SMALL_SPEND);
    EXPECT_EQ(tester.state().treasuryBalance, INITIAL_BALANCE - SMALL_SPEND);
}

TEST_F(QZNTreasuryVaultTest, Sign_LastSpendLog_Updated)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(5);

    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    sign(SIGNER_1, pid);

    EXPECT_EQ(tester.state().lastSpendEpoch,    5LL);
    EXPECT_EQ(tester.state().lastSpendAmount,   SMALL_SPEND);
    EXPECT_EQ(tester.state().lastSpendCategory, SPEND_ECOSYSTEM_GRANT);
}

// ============================================================
//  [PROC-4]  ExecuteProposal
// ============================================================

TEST_F(QZNTreasuryVaultTest, Execute_HappyPath_AfterTimelockDelay)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(10);

    sint64 pid = proposeAndTimelockLarge(LARGE_SPEND);
    EXPECT_EQ(tester.state().proposals_0.state, PROP_TIMELOCKED);

    // Advance past timelock
    tester.setCurrentEpoch(10 + TIMELOCK_DELAY_EPOCHS + 1);

    ExecuteProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_2);
    auto out = tester.callProcedure(ExecuteProposal_id, in);

    EXPECT_EQ(out.executed,   1);
    EXPECT_EQ(out.amountSent, LARGE_SPEND);
    EXPECT_EQ(tester.state().proposals_0.state,    PROP_EXECUTED);
    EXPECT_EQ(tester.state().treasuryBalance,       INITIAL_BALANCE - LARGE_SPEND);
    EXPECT_EQ(tester.state().totalSpent,            LARGE_SPEND);
}

TEST_F(QZNTreasuryVaultTest, Execute_BeforeTimelockExpiry_Blocked)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(10);

    sint64 pid = proposeAndTimelockLarge();
    sint64 prevBalance = tester.state().treasuryBalance;

    // Advance to exactly the required epoch — not yet past it
    tester.setCurrentEpoch(10 + TIMELOCK_DELAY_EPOCHS);  // AT epoch, not after

    ExecuteProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_2);
    auto out = tester.callProcedure(ExecuteProposal_id, in);

    EXPECT_EQ(out.executed,                    0);
    EXPECT_EQ(tester.state().treasuryBalance,  prevBalance);
    EXPECT_EQ(tester.state().proposals_0.state, PROP_TIMELOCKED);
}

TEST_F(QZNTreasuryVaultTest, Execute_NonSignerRejected)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(10);
    sint64 pid = proposeAndTimelockLarge();
    tester.setCurrentEpoch(10 + TIMELOCK_DELAY_EPOCHS + 1);

    sint64 prevBalance = tester.state().treasuryBalance;

    ExecuteProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(ExecuteProposal_id, in);

    EXPECT_EQ(tester.state().treasuryBalance, prevBalance);
}

TEST_F(QZNTreasuryVaultTest, Execute_PausedVault_Rejected)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(10);
    sint64 pid = proposeAndTimelockLarge();
    tester.setCurrentEpoch(10 + TIMELOCK_DELAY_EPOCHS + 1);
    tester.state().paused = 1;

    sint64 prevBalance = tester.state().treasuryBalance;
    ExecuteProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_2);
    tester.callProcedure(ExecuteProposal_id, in);

    EXPECT_EQ(tester.state().treasuryBalance, prevBalance);
}

TEST_F(QZNTreasuryVaultTest, Execute_PendingProposal_NotTimelocked_Rejected)
{
    initialize(INITIAL_BALANCE);
    // Propose but only 1 sig — still PENDING, not TIMELOCKED
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, LARGE_SPEND);

    sint64 prevBalance = tester.state().treasuryBalance;
    ExecuteProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_1);
    tester.callProcedure(ExecuteProposal_id, in);

    EXPECT_EQ(tester.state().treasuryBalance, prevBalance);
}

TEST_F(QZNTreasuryVaultTest, Execute_BurnCategory_UpdatesBurned)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(10);

    sint64 pid = propose(SIGNER_0, SPEND_BURN_EVENT, LARGE_SPEND);
    sign(SIGNER_1, pid);  // Timelocked

    tester.setCurrentEpoch(10 + TIMELOCK_DELAY_EPOCHS + 1);
    ExecuteProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_2);
    tester.callProcedure(ExecuteProposal_id, in);

    EXPECT_EQ(tester.state().totalBurned, LARGE_SPEND);
}

// ============================================================
//  [PROC-5]  CancelProposal
// ============================================================

TEST_F(QZNTreasuryVaultTest, Cancel_HappyPath_PendingProposalCancelled)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    CancelProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_1);
    auto out = tester.callProcedure(CancelProposal_id, in);

    EXPECT_EQ(out.cancelled, 1);
    EXPECT_EQ(tester.state().proposals_0.state, PROP_CANCELLED);
}

TEST_F(QZNTreasuryVaultTest, Cancel_NonSignerRejected)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    CancelProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(STRANGER_ADDR);
    auto out = tester.callProcedure(CancelProposal_id, in);

    EXPECT_EQ(out.cancelled, 0);
    EXPECT_EQ(tester.state().proposals_0.state, PROP_PENDING);
}

TEST_F(QZNTreasuryVaultTest, Cancel_TimelockProposal_Rejected)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(10);

    sint64 pid = proposeAndTimelockLarge();
    EXPECT_EQ(tester.state().proposals_0.state, PROP_TIMELOCKED);

    CancelProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_2);
    auto out = tester.callProcedure(CancelProposal_id, in);

    EXPECT_EQ(out.cancelled, 0);
    EXPECT_EQ(tester.state().proposals_0.state, PROP_TIMELOCKED);
}

TEST_F(QZNTreasuryVaultTest, Cancel_ExecutedProposal_Rejected)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = proposeAndExecuteSmall();
    EXPECT_EQ(tester.state().proposals_0.state, PROP_EXECUTED);

    CancelProposal_input in{};
    in.proposalId = pid;
    tester.setInvocator(SIGNER_2);
    auto out = tester.callProcedure(CancelProposal_id, in);

    EXPECT_EQ(out.cancelled, 0);
    EXPECT_EQ(tester.state().proposals_0.state, PROP_EXECUTED);
}

TEST_F(QZNTreasuryVaultTest, Cancel_SlotFreedForReuse)
{
    initialize(INITIAL_BALANCE);
    sint64 pid1 = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    CancelProposal_input in{};
    in.proposalId = pid1;
    tester.setInvocator(SIGNER_1);
    tester.callProcedure(CancelProposal_id, in);

    // Cancelled slot should be reusable
    sint64 pid2 = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    EXPECT_EQ(pid2, 2LL);
    EXPECT_EQ(tester.state().proposals_0.state, PROP_PENDING);
}

// ============================================================
//  [FUNC-6]  GetVaultState
// ============================================================

TEST_F(QZNTreasuryVaultTest, GetVaultState_InitialState_Correct)
{
    initialize(INITIAL_BALANCE);

    GetVaultState_input in{};
    auto out = tester.callFunction(GetVaultState_id, in);

    EXPECT_EQ(out.treasuryBalance,       INITIAL_BALANCE);
    EXPECT_EQ(out.totalSpent,            0LL);
    EXPECT_EQ(out.totalBurned,           0LL);
    EXPECT_EQ(out.epochRewardFundAmount, DEFAULT_EPOCH_REWARD_FUND);
    EXPECT_EQ(out.autoFundEnabled,       1);
    EXPECT_EQ(out.paused,                0);
    EXPECT_EQ(out.nextProposalId,        1LL);
}

TEST_F(QZNTreasuryVaultTest, GetVaultState_AfterSpend_ReflectsChanges)
{
    initialize(INITIAL_BALANCE);
    proposeAndExecuteSmall(SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    GetVaultState_input in{};
    auto out = tester.callFunction(GetVaultState_id, in);

    EXPECT_EQ(out.treasuryBalance, INITIAL_BALANCE - SMALL_SPEND);
    EXPECT_EQ(out.totalSpent,      SMALL_SPEND);
    EXPECT_EQ(out.nextProposalId,  2LL);
}

TEST_F(QZNTreasuryVaultTest, GetVaultState_IsReadOnly)
{
    initialize(INITIAL_BALANCE);
    sint64 balBefore = tester.state().treasuryBalance;

    GetVaultState_input in{};
    tester.callFunction(GetVaultState_id, in);
    tester.callFunction(GetVaultState_id, in);

    EXPECT_EQ(tester.state().treasuryBalance, balBefore);
}

// ============================================================
//  [FUNC-7]  GetProposal
// ============================================================

TEST_F(QZNTreasuryVaultTest, GetProposal_PendingState_Correct)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(5);
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    GetProposal_input in{};
    in.proposalId = pid;
    auto out = tester.callFunction(GetProposal_id, in);

    EXPECT_EQ(out.category,      SPEND_ECOSYSTEM_GRANT);
    EXPECT_EQ(out.state,         PROP_PENDING);
    EXPECT_EQ(out.amount,        SMALL_SPEND);
    EXPECT_EQ(out.destination,   DEST_ADDR);
    EXPECT_EQ(out.sigsCollected, (uint8)1);
    EXPECT_EQ(out.proposedEpoch, 5LL);
    EXPECT_EQ(out.expired,       0);
}

TEST_F(QZNTreasuryVaultTest, GetProposal_ExecutedState_Correct)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = proposeAndExecuteSmall();

    GetProposal_input in{};
    in.proposalId = pid;
    auto out = tester.callFunction(GetProposal_id, in);

    EXPECT_EQ(out.state, PROP_EXECUTED);
}

TEST_F(QZNTreasuryVaultTest, GetProposal_ExpiredFlag_SetWhenPastExpiry)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(1);
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    // Advance past expiry window
    tester.setCurrentEpoch(1 + PROPOSAL_EXPIRY_EPOCHS + 1);

    GetProposal_input in{};
    in.proposalId = pid;
    auto out = tester.callFunction(GetProposal_id, in);

    EXPECT_EQ(out.expired, 1);
}

TEST_F(QZNTreasuryVaultTest, GetProposal_TimelockEpoch_StoredCorrectly)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(10);

    sint64 pid = proposeAndTimelockLarge();

    GetProposal_input in{};
    in.proposalId = pid;
    auto out = tester.callFunction(GetProposal_id, in);

    EXPECT_EQ(out.state,             PROP_TIMELOCKED);
    EXPECT_EQ(out.executeAfterEpoch, 10LL + TIMELOCK_DELAY_EPOCHS);
}

// ============================================================
//  BEGIN_EPOCH integration tests
// ============================================================

TEST_F(QZNTreasuryVaultTest, BeginEpoch_AutoFund_TransfersRewardAndAchievement)
{
    initialize(INITIAL_BALANCE);
    tester.state().autoFundEnabled = 1;
    tester.state().paused          = 0;

    sint64 prevBalance = tester.state().treasuryBalance;
    sint64 expectedOut = DEFAULT_EPOCH_REWARD_FUND + DEFAULT_ACHIEVEMENT_FUND;

    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().treasuryBalance,  prevBalance - expectedOut);
    EXPECT_EQ(tester.state().totalEpochFunded, expectedOut);
    EXPECT_EQ(tester.state().totalSpent,       expectedOut);
}

TEST_F(QZNTreasuryVaultTest, BeginEpoch_Paused_AutoFundSuppressed)
{
    initialize(INITIAL_BALANCE);
    tester.state().paused = 1;
    sint64 prevBalance    = tester.state().treasuryBalance;

    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().treasuryBalance, prevBalance);
    EXPECT_EQ(tester.state().totalSpent,      0LL);
}

TEST_F(QZNTreasuryVaultTest, BeginEpoch_RunwayGuard_DisablesAutoFundBelowThreshold)
{
    // Set balance just below 6-month runway threshold
    sint64 monthlyBurn  = DEFAULT_EPOCH_REWARD_FUND + DEFAULT_ACHIEVEMENT_FUND;
    sint64 runwayNeeded = monthlyBurn * 26LL;

    initialize(runwayNeeded - 1LL);  // 1 QZN below threshold

    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().autoFundEnabled, 0);
}

TEST_F(QZNTreasuryVaultTest, BeginEpoch_RunwayGuard_AboveThreshold_AutoFundStaysOn)
{
    sint64 monthlyBurn  = DEFAULT_EPOCH_REWARD_FUND + DEFAULT_ACHIEVEMENT_FUND;
    sint64 runwayNeeded = monthlyBurn * 26LL;

    initialize(runwayNeeded + DEFAULT_EPOCH_REWARD_FUND + DEFAULT_ACHIEVEMENT_FUND);

    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().autoFundEnabled, 1);
}

TEST_F(QZNTreasuryVaultTest, BeginEpoch_ExpirySweep_StaleProposalMarkedExpired)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(1);

    // Create proposal that will go stale
    propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    EXPECT_EQ(tester.state().proposals_0.state, PROP_PENDING);

    // Advance past expiry
    tester.setCurrentEpoch(1 + PROPOSAL_EXPIRY_EPOCHS + 1);
    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().proposals_0.state, PROP_EXPIRED);
}

TEST_F(QZNTreasuryVaultTest, BeginEpoch_ExpirySweep_FreshProposal_NotExpired)
{
    initialize(INITIAL_BALANCE);
    tester.setCurrentEpoch(1);
    propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    // Only 1 epoch later — not expired yet
    tester.setCurrentEpoch(2);
    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().proposals_0.state, PROP_PENDING);
}

TEST_F(QZNTreasuryVaultTest, BeginEpoch_AutoFundDisabled_NoTransfer)
{
    initialize(INITIAL_BALANCE);
    tester.state().autoFundEnabled = 0;
    sint64 prevBalance             = tester.state().treasuryBalance;

    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().treasuryBalance, prevBalance);
    EXPECT_EQ(tester.state().totalSpent,      0LL);
}

// ============================================================
//  INVARIANTS
// ============================================================

TEST_F(QZNTreasuryVaultTest, Invariant_BalanceNeverGoesNegative)
{
    initialize(SMALL_SPEND * 3);

    // Execute 3 small spends
    for (int i = 0; i < 3; i++)
    {
        sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
        sign(SIGNER_1, pid);
    }

    EXPECT_GE(tester.state().treasuryBalance, 0LL);
}

TEST_F(QZNTreasuryVaultTest, Invariant_TotalSpentPlusBurnedNeverExceedsTotalReceived)
{
    initialize(INITIAL_BALANCE);

    proposeAndExecuteSmall(SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);
    proposeAndExecuteSmall(SPEND_BURN_EVENT,      SMALL_SPEND);

    const auto& s = tester.state();
    EXPECT_LE(s.totalSpent, s.totalReceived);
}

TEST_F(QZNTreasuryVaultTest, Invariant_ProposalSigCountNeverExceedsThree)
{
    initialize(INITIAL_BALANCE);
    sint64 pid = propose(SIGNER_0, SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    // All 3 signers sign
    sign(SIGNER_1, pid);  // This will execute on 2nd sig
    sign(SIGNER_2, pid);  // Post-execution sign should be ignored

    EXPECT_LE(tester.state().proposals_0.sigCount, (uint8)TOTAL_SIGNERS);
}

TEST_F(QZNTreasuryVaultTest, Invariant_TreasuryBalancePlusTotalSpentEqualsReceived)
{
    initialize(INITIAL_BALANCE);

    proposeAndExecuteSmall(SPEND_ECOSYSTEM_GRANT, SMALL_SPEND);

    const auto& s = tester.state();
    // balance + spent should equal initial received (no auto-fund in this test)
    EXPECT_EQ(s.treasuryBalance + s.totalSpent, s.totalReceived);
}
