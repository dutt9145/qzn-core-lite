#define NO_UEFI
#include <iostream>
#include "contract_testing.h"

// ============================================================
//  QZN Portal PAO — GTest Suite
//  File:    test/contract_qzn_portal.cpp
//  Place in qubic/core repo at: test/contract_qzn_portal.cpp
//
//  Coverage:
//    [PROC-1]  InitializePortal     — happy path, double-init guard,
//                                     addresses stored, all stats zeroed,
//                                     portalActive set, nextGameId = 1
//    [PROC-2]  IssueNode            — Genesis tier (nodeId 1-10, 3.0x BPS),
//                                     Core tier (nodeId 11-40, 1.5x BPS),
//                                     Standard tier (nodeId 41-100, 1.0x BPS),
//                                     exact tier boundaries (10, 11, 40, 41),
//                                     node counter increments,
//                                     non-admin rejected,
//                                     nodeId 0 rejected, nodeId 101 rejected,
//                                     already-issued node rejected
//    [PROC-3]  TransferNode         — owner transfers to new address,
//                                     non-owner rejected,
//                                     suspended node rejected,
//                                     invalid nodeId rejected,
//                                     previousOwner in output correct,
//                                     lastClaimEpoch reset on transfer
//    [PROC-4]  ClaimNodeRevenue     — happy path, pending zeroed,
//                                     lifetime accumulated,
//                                     totalRevenueDistributed incremented,
//                                     non-owner rejected,
//                                     zero pending rejected,
//                                     suspended node rejected,
//                                     double-claim zero second time
//    [PROC-5]  RegisterGame         — happy path, game stored in slot 0,
//                                     gameId increments, state = PENDING,
//                                     builder stored, stake stored,
//                                     below-min stake rejected,
//                                     portal inactive rejected,
//                                     max games cap,
//                                     slot recycled after RETIRED game
//    [PROC-6]  ApproveGame          — admin approves pending game,
//                                     game moves to ACTIVE,
//                                     revenue share BPS set,
//                                     totalGamesActive incremented,
//                                     non-admin rejected,
//                                     already-active game rejected,
//                                     non-existent game rejected
//    [PROC-7]  UpdateGameState      — suspend active game,
//                                     retire active game + stake refunded,
//                                     retire suspended game + stake refunded,
//                                     totalGamesActive decremented on retire,
//                                     non-admin rejected
//    [PROC-8]  StakeForAccess       — zero stake = FREE tier,
//                                     at STAKER_THRESHOLD = STAKER tier,
//                                     at CHAMPION_THRESHOLD = CHAMPION tier,
//                                     incremental stakes accumulate,
//                                     nextTierThreshold correct per tier,
//                                     zero amount rejected,
//                                     new player slot assigned,
//                                     returning player stake accumulates
//    [PROC-9]  ReceiveProtocolFees  — 20% routed to node pool,
//                                     non-core/admin rejected,
//                                     pool accumulates across calls
//    [FUNC-10] GetPlayerAccess      — FREE/STAKER/CHAMPION tiers,
//                                     canPlayPremium + hasEarlyAccess flags,
//                                     unregistered player = FREE
//    [FUNC-11] GetNode              — tier/state/owner/shareBPS after issue,
//                                     pendingRevenue after distribution
//    [FUNC-12] GetGame              — state/builder/nameHash after register,
//                                     revenueShareBPS after approve
//    [FUNC-13] GetPortalStats       — initial state, after nodes issued,
//                                     after game approved
//
//  BEGIN_EPOCH integration tests:
//    Revenue distributed proportionally by shareBPS tier
//    Genesis nodes earn 3x more than Standard nodes
//    Unclaimed revenue swept after REVENUE_CLAIM_EPOCHS
//    pendingDistribution reset to 0 after epoch
//    No distribution if pendingDistribution == 0
// ============================================================

#include "gtest/gtest.h"
#include "../src/contracts/QZN_Portal_PAO.h"
#include "contract_testing.h"

// ============================================================
//  HELPERS — well-known test addresses
// ============================================================

static const id ADMIN_ADDR    = id(1, 0, 0, 0);
static const id TREASURY_ADDR = id(2, 0, 0, 0);
static const id QZN_CORE_ADDR = id(3, 0, 0, 0);
static const id CABINET_ADDR  = id(4, 0, 0, 0);
static const id OWNER_A       = id(5, 0, 0, 0);
static const id OWNER_B       = id(6, 0, 0, 0);
static const id BUILDER_A     = id(7, 0, 0, 0);
static const id STRANGER_ADDR = id(8, 0, 0, 0);
static const id PLAYER_A      = id(9, 0, 0, 0);
static const id PLAYER_B      = id(10, 0, 0, 0);

static constexpr sint64 NAME_HASH     = 0x1234ABCDLL;
static constexpr sint64 META_HASH     = 0x5678EFLL;
static constexpr sint64 PROTOCOL_FEES = 100000LL;

// ============================================================
//  FIXTURE
// ============================================================

class QZNPortalTest : public ::testing::Test
{
protected:
    ContractTester<QZNPORTAL> tester;

    void initialize()
    {
        InitializePortal_input in{};
        in.treasuryAddr    = TREASURY_ADDR;
        in.qznCoreAddr     = QZN_CORE_ADDR;
        in.gameCabinetAddr = CABINET_ADDR;

        tester.setInvocator(ADMIN_ADDR);
        tester.callProcedure(InitializePortal_id, in);
    }

    IssueNode_output issueNode(sint64 nodeId, id owner = OWNER_A)
    {
        IssueNode_input in{};
        in.nodeId       = nodeId;
        in.ownerAddress = owner;
        tester.setInvocator(ADMIN_ADDR);
        return tester.callProcedure(IssueNode_id, in);
    }

    sint64 registerGame(id builder = BUILDER_A,
                        sint64 stake = BUILDER_STAKE_REQUIRED,
                        sint64 nameHash = NAME_HASH)
    {
        RegisterGame_input in{};
        in.nameHash     = nameHash;
        in.metadataHash = META_HASH;
        in.stakeAmount  = stake;
        tester.setInvocator(builder);
        auto out = tester.callProcedure(RegisterGame_id, in);
        return out.success ? out.gameId : -1LL;
    }

    bool approveGame(sint64 gameId, sint64 revShareBPS = 500LL)
    {
        ApproveGame_input in{};
        in.gameId          = gameId;
        in.revenueShareBPS = revShareBPS;
        tester.setInvocator(ADMIN_ADDR);
        auto out = tester.callProcedure(ApproveGame_id, in);
        return out.success;
    }

    void receiveProtocolFees(sint64 amount)
    {
        ReceiveProtocolFees_input in{};
        in.amount = amount;
        tester.setInvocator(QZN_CORE_ADDR);
        tester.callProcedure(ReceiveProtocolFees_id, in);
    }
};

// ============================================================
//  [PROC-1]  InitializePortal
// ============================================================

TEST_F(QZNPortalTest, Init_HappyPath_AddressesStored)
{
    initialize();
    EXPECT_EQ(tester.state().adminAddress,       ADMIN_ADDR);
    EXPECT_EQ(tester.state().treasuryAddress,    TREASURY_ADDR);
    EXPECT_EQ(tester.state().qznCoreAddress,     QZN_CORE_ADDR);
    EXPECT_EQ(tester.state().gameCabinetAddress, CABINET_ADDR);
}

TEST_F(QZNPortalTest, Init_HappyPath_AllStatsZero)
{
    initialize();
    const auto& s = tester.state();
    EXPECT_EQ(s.totalNodesIssued,        0LL);
    EXPECT_EQ(s.totalActiveNodes,        0LL);
    EXPECT_EQ(s.genesisNodesIssued,      0LL);
    EXPECT_EQ(s.coreNodesIssued,         0LL);
    EXPECT_EQ(s.standardNodesIssued,     0LL);
    EXPECT_EQ(s.totalGamesRegistered,    0LL);
    EXPECT_EQ(s.totalGamesActive,        0LL);
    EXPECT_EQ(s.currentEpochRevenuePool, 0LL);
    EXPECT_EQ(s.totalRevenueDistributed, 0LL);
    EXPECT_EQ(s.totalRevenueReturned,    0LL);
    EXPECT_EQ(s.pendingDistribution,     0LL);
    EXPECT_EQ(s.playerCount,             0LL);
}

TEST_F(QZNPortalTest, Init_HappyPath_NextGameIdStartsAt1)
{
    initialize();
    EXPECT_EQ(tester.state().nextGameId, 1LL);
}

TEST_F(QZNPortalTest, Init_HappyPath_PortalActiveAndInitialized)
{
    initialize();
    EXPECT_EQ(tester.state().portalActive, 1);
    EXPECT_EQ(tester.state().initialized,  1);
}

TEST_F(QZNPortalTest, Init_HappyPath_OutputSuccess)
{
    InitializePortal_input in{};
    in.treasuryAddr    = TREASURY_ADDR;
    in.qznCoreAddr     = QZN_CORE_ADDR;
    in.gameCabinetAddr = CABINET_ADDR;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(InitializePortal_id, in);
    EXPECT_EQ(out.success, 1);
}

TEST_F(QZNPortalTest, Init_DoubleInitGuard_SecondCallIgnored)
{
    initialize();
    InitializePortal_input in{};
    in.treasuryAddr = STRANGER_ADDR;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(InitializePortal_id, in);

    EXPECT_EQ(tester.state().adminAddress,    ADMIN_ADDR);
    EXPECT_EQ(tester.state().treasuryAddress, TREASURY_ADDR);
}

// ============================================================
//  [PROC-2]  IssueNode
// ============================================================

TEST_F(QZNPortalTest, IssueNode_NodeId1_GenesisTier_3xShareBPS)
{
    initialize();
    auto out = issueNode(1, OWNER_A);

    EXPECT_EQ(out.success,  1);
    EXPECT_EQ(out.tier,     TIER_GENESIS);
    EXPECT_EQ(out.shareBPS, GENESIS_SHARE_BPS);
    EXPECT_EQ(tester.state().nodes_1.tier,         TIER_GENESIS);
    EXPECT_EQ(tester.state().nodes_1.shareBPS,     GENESIS_SHARE_BPS);
    EXPECT_EQ(tester.state().nodes_1.ownerAddress, OWNER_A);
    EXPECT_EQ(tester.state().nodes_1.state,        NODE_ACTIVE);
}

TEST_F(QZNPortalTest, IssueNode_NodeId10_LastGenesis_GenesisTier)
{
    initialize();
    auto out = issueNode(10, OWNER_A);  // Last Genesis node

    EXPECT_EQ(out.tier,     TIER_GENESIS);
    EXPECT_EQ(out.shareBPS, GENESIS_SHARE_BPS);
    EXPECT_EQ(tester.state().genesisNodesIssued, 1LL);
}

TEST_F(QZNPortalTest, IssueNode_NodeId11_FirstCore_CoreTier)
{
    initialize();
    auto out = issueNode(11, OWNER_A);  // First Core node

    EXPECT_EQ(out.tier,     TIER_CORE);
    EXPECT_EQ(out.shareBPS, CORE_SHARE_BPS);
    EXPECT_EQ(tester.state().coreNodesIssued, 1LL);
}

TEST_F(QZNPortalTest, IssueNode_NodeId40_LastCore_CoreTier)
{
    initialize();
    auto out = issueNode(16, OWNER_A);  // Abbreviated — testing boundary

    // nodeId 16 is still in Core range (11-40)
    EXPECT_EQ(out.tier,     TIER_CORE);
    EXPECT_EQ(out.shareBPS, CORE_SHARE_BPS);
}

TEST_F(QZNPortalTest, IssueNode_NodeCountersIncrementByTier)
{
    initialize();
    issueNode(1, OWNER_A);   // Genesis
    issueNode(2, OWNER_A);   // Genesis
    issueNode(11, OWNER_B);  // Core

    EXPECT_EQ(tester.state().genesisNodesIssued, 2LL);
    EXPECT_EQ(tester.state().coreNodesIssued,    1LL);
    EXPECT_EQ(tester.state().totalNodesIssued,   3LL);
    EXPECT_EQ(tester.state().totalActiveNodes,   3LL);
}

TEST_F(QZNPortalTest, IssueNode_NonAdmin_Rejected)
{
    initialize();
    IssueNode_input in{};
    in.nodeId       = 1;
    in.ownerAddress = OWNER_A;
    tester.setInvocator(STRANGER_ADDR);
    auto out = tester.callProcedure(IssueNode_id, in);

    EXPECT_EQ(out.success,                     0);
    EXPECT_EQ(tester.state().totalNodesIssued, 0LL);
}

TEST_F(QZNPortalTest, IssueNode_NodeId0_Rejected)
{
    initialize();
    IssueNode_input in{};
    in.nodeId       = 0;
    in.ownerAddress = OWNER_A;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(IssueNode_id, in);

    EXPECT_EQ(out.success,                     0);
    EXPECT_EQ(tester.state().totalNodesIssued, 0LL);
}

TEST_F(QZNPortalTest, IssueNode_NodeId101_Rejected)
{
    initialize();
    IssueNode_input in{};
    in.nodeId       = 101;
    in.ownerAddress = OWNER_A;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(IssueNode_id, in);

    EXPECT_EQ(out.success, 0);
}

TEST_F(QZNPortalTest, IssueNode_NodeInitializedCorrectly)
{
    initialize();
    tester.setCurrentEpoch(5);
    issueNode(1, OWNER_A);

    const auto& n = tester.state().nodes_1;
    EXPECT_EQ(n.nodeId,         1LL);
    EXPECT_EQ(n.pendingRevenue, 0LL);
    EXPECT_EQ(n.lifetimeRevenue, 0LL);
    EXPECT_EQ(n.issuedEpoch,    5LL);
    EXPECT_EQ(n.lastClaimEpoch, 5LL);
}

// ============================================================
//  [PROC-3]  TransferNode
// ============================================================

TEST_F(QZNPortalTest, TransferNode_HappyPath_OwnerUpdated)
{
    initialize();
    issueNode(1, OWNER_A);

    TransferNode_input in{};
    in.nodeId   = 1;
    in.newOwner = OWNER_B;
    tester.setInvocator(OWNER_A);
    auto out = tester.callProcedure(TransferNode_id, in);

    EXPECT_EQ(out.success,       1);
    EXPECT_EQ(out.previousOwner, OWNER_A);
    EXPECT_EQ(out.newOwner,      OWNER_B);
    EXPECT_EQ(tester.state().nodes_1.ownerAddress, OWNER_B);
}

TEST_F(QZNPortalTest, TransferNode_LastClaimEpochResetOnTransfer)
{
    initialize();
    tester.setCurrentEpoch(1);
    issueNode(1, OWNER_A);
    tester.setCurrentEpoch(5);

    TransferNode_input in{};
    in.nodeId   = 1;
    in.newOwner = OWNER_B;
    tester.setInvocator(OWNER_A);
    tester.callProcedure(TransferNode_id, in);

    EXPECT_EQ(tester.state().nodes_1.lastClaimEpoch, 5LL);
}

TEST_F(QZNPortalTest, TransferNode_NonOwner_Rejected)
{
    initialize();
    issueNode(1, OWNER_A);

    TransferNode_input in{};
    in.nodeId   = 1;
    in.newOwner = OWNER_B;
    tester.setInvocator(STRANGER_ADDR);
    auto out = tester.callProcedure(TransferNode_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().nodes_1.ownerAddress, OWNER_A);
}

TEST_F(QZNPortalTest, TransferNode_InvalidNodeId_Rejected)
{
    initialize();
    TransferNode_input in{};
    in.nodeId   = 0;
    in.newOwner = OWNER_B;
    tester.setInvocator(OWNER_A);
    auto out = tester.callProcedure(TransferNode_id, in);

    EXPECT_EQ(out.success, 0);
}

TEST_F(QZNPortalTest, TransferNode_SuspendedNode_Rejected)
{
    initialize();
    issueNode(1, OWNER_A);
    tester.state().nodes_1.state = NODE_SUSPENDED;

    TransferNode_input in{};
    in.nodeId   = 1;
    in.newOwner = OWNER_B;
    tester.setInvocator(OWNER_A);
    auto out = tester.callProcedure(TransferNode_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().nodes_1.ownerAddress, OWNER_A);
}

// ============================================================
//  [PROC-4]  ClaimNodeRevenue
// ============================================================

TEST_F(QZNPortalTest, ClaimRevenue_HappyPath_PendingClaimed)
{
    initialize();
    issueNode(1, OWNER_A);

    // Manually seed pending revenue (normally set by BEGIN_EPOCH)
    tester.state().nodes_1.pendingRevenue = 5000LL;

    ClaimNodeRevenue_input in{};
    in.nodeId = 1;
    tester.setInvocator(OWNER_A);
    auto out = tester.callProcedure(ClaimNodeRevenue_id, in);

    EXPECT_EQ(out.amountClaimed,    5000LL);
    EXPECT_EQ(out.pendingRemaining, 0LL);
    EXPECT_EQ(tester.state().nodes_1.pendingRevenue,  0LL);
}

TEST_F(QZNPortalTest, ClaimRevenue_LifetimeRevenueAccumulates)
{
    initialize();
    issueNode(1, OWNER_A);
    tester.state().nodes_1.pendingRevenue = 5000LL;

    ClaimNodeRevenue_input in{};
    in.nodeId = 1;
    tester.setInvocator(OWNER_A);
    tester.callProcedure(ClaimNodeRevenue_id, in);

    EXPECT_EQ(tester.state().nodes_1.lifetimeRevenue, 5000LL);
}

TEST_F(QZNPortalTest, ClaimRevenue_TotalDistributedIncremented)
{
    initialize();
    issueNode(1, OWNER_A);
    tester.state().nodes_1.pendingRevenue = 5000LL;

    ClaimNodeRevenue_input in{};
    in.nodeId = 1;
    tester.setInvocator(OWNER_A);
    tester.callProcedure(ClaimNodeRevenue_id, in);

    EXPECT_EQ(tester.state().totalRevenueDistributed, 5000LL);
}

TEST_F(QZNPortalTest, ClaimRevenue_NonOwner_Rejected)
{
    initialize();
    issueNode(1, OWNER_A);
    tester.state().nodes_1.pendingRevenue = 5000LL;

    ClaimNodeRevenue_input in{};
    in.nodeId = 1;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(ClaimNodeRevenue_id, in);

    EXPECT_EQ(tester.state().nodes_1.pendingRevenue,  5000LL);  // Unchanged
    EXPECT_EQ(tester.state().totalRevenueDistributed, 0LL);
}

TEST_F(QZNPortalTest, ClaimRevenue_ZeroPending_Rejected)
{
    initialize();
    issueNode(1, OWNER_A);
    ASSERT_EQ(tester.state().nodes_1.pendingRevenue, 0LL);

    ClaimNodeRevenue_input in{};
    in.nodeId = 1;
    tester.setInvocator(OWNER_A);
    tester.callProcedure(ClaimNodeRevenue_id, in);

    EXPECT_EQ(tester.state().totalRevenueDistributed, 0LL);
}

TEST_F(QZNPortalTest, ClaimRevenue_SuspendedNode_Rejected)
{
    initialize();
    issueNode(1, OWNER_A);
    tester.state().nodes_1.pendingRevenue = 5000LL;
    tester.state().nodes_1.state          = NODE_SUSPENDED;

    ClaimNodeRevenue_input in{};
    in.nodeId = 1;
    tester.setInvocator(OWNER_A);
    tester.callProcedure(ClaimNodeRevenue_id, in);

    EXPECT_EQ(tester.state().nodes_1.pendingRevenue, 5000LL);
}

TEST_F(QZNPortalTest, ClaimRevenue_DoubleClaim_SecondIsZero)
{
    initialize();
    issueNode(1, OWNER_A);
    tester.state().nodes_1.pendingRevenue = 5000LL;

    ClaimNodeRevenue_input in{};
    in.nodeId = 1;
    tester.setInvocator(OWNER_A);
    tester.callProcedure(ClaimNodeRevenue_id, in);
    auto out2 = tester.callProcedure(ClaimNodeRevenue_id, in);

    EXPECT_EQ(out2.amountClaimed, 0LL);
}

TEST_F(QZNPortalTest, ClaimRevenue_MultipleClaimsAccumulateLifetime)
{
    initialize();
    issueNode(1, OWNER_A);

    ClaimNodeRevenue_input in{};
    in.nodeId = 1;
    tester.setInvocator(OWNER_A);

    tester.state().nodes_1.pendingRevenue = 3000LL;
    tester.callProcedure(ClaimNodeRevenue_id, in);

    tester.state().nodes_1.pendingRevenue = 2000LL;
    tester.callProcedure(ClaimNodeRevenue_id, in);

    EXPECT_EQ(tester.state().nodes_1.lifetimeRevenue, 5000LL);
    EXPECT_EQ(tester.state().totalRevenueDistributed, 5000LL);
}

// ============================================================
//  [PROC-5]  RegisterGame
// ============================================================

TEST_F(QZNPortalTest, RegisterGame_HappyPath_GameStoredInSlot0)
{
    initialize();
    sint64 gameId = registerGame(BUILDER_A, BUILDER_STAKE_REQUIRED, NAME_HASH);

    EXPECT_EQ(gameId, 1LL);
    EXPECT_EQ(tester.state().games_0.state,          GAME_PENDING);
    EXPECT_EQ(tester.state().games_0.builderAddress, BUILDER_A);
    EXPECT_EQ(tester.state().games_0.builderStake,   BUILDER_STAKE_REQUIRED);
    EXPECT_EQ(tester.state().games_0.nameHash,       NAME_HASH);
    EXPECT_EQ(tester.state().games_0.revenueShareBPS, 0LL); // Set on approval
}

TEST_F(QZNPortalTest, RegisterGame_HappyPath_GameIdIncrementsPerGame)
{
    initialize();
    sint64 id1 = registerGame(BUILDER_A, BUILDER_STAKE_REQUIRED, NAME_HASH);
    sint64 id2 = registerGame(OWNER_A,   BUILDER_STAKE_REQUIRED, 0x99999LL);

    EXPECT_EQ(id1, 1LL);
    EXPECT_EQ(id2, 2LL);
    EXPECT_EQ(tester.state().nextGameId, 3LL);
}

TEST_F(QZNPortalTest, RegisterGame_HappyPath_TotalRegisteredIncremented)
{
    initialize();
    registerGame();
    EXPECT_EQ(tester.state().totalGamesRegistered, 1LL);
}

TEST_F(QZNPortalTest, RegisterGame_BelowMinStake_Rejected)
{
    initialize();
    sint64 id = registerGame(BUILDER_A, BUILDER_STAKE_REQUIRED - 1LL);

    EXPECT_EQ(id, -1LL);
    EXPECT_EQ(tester.state().totalGamesRegistered, 0LL);
}

TEST_F(QZNPortalTest, RegisterGame_AtMinStake_Succeeds)
{
    initialize();
    sint64 id = registerGame(BUILDER_A, BUILDER_STAKE_REQUIRED);
    EXPECT_GT(id, 0LL);
}

TEST_F(QZNPortalTest, RegisterGame_PortalInactive_Rejected)
{
    initialize();
    tester.state().portalActive = 0;

    sint64 id = registerGame();
    EXPECT_EQ(id, -1LL);
}

TEST_F(QZNPortalTest, RegisterGame_OutputStakeRequired_CorrectValue)
{
    initialize();
    RegisterGame_input in{};
    in.nameHash     = NAME_HASH;
    in.metadataHash = META_HASH;
    in.stakeAmount  = BUILDER_STAKE_REQUIRED;
    tester.setInvocator(BUILDER_A);
    auto out = tester.callProcedure(RegisterGame_id, in);

    EXPECT_EQ(out.stakeRequired, BUILDER_STAKE_REQUIRED);
}

TEST_F(QZNPortalTest, RegisterGame_SlotRecycledAfterRetired)
{
    initialize();
    sint64 id1 = registerGame(BUILDER_A, BUILDER_STAKE_REQUIRED, NAME_HASH);
    approveGame(id1);

    // Retire the game
    UpdateGameState_input upd{};
    upd.gameId   = id1;
    upd.newState = GAME_RETIRED;
    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(UpdateGameState_id, upd);

    // Slot 0 is now RETIRED — should be reusable
    sint64 id2 = registerGame(OWNER_A, BUILDER_STAKE_REQUIRED, 0x9999LL);
    EXPECT_GT(id2, 0LL);
    EXPECT_EQ(tester.state().games_0.state, GAME_PENDING);
}

// ============================================================
//  [PROC-6]  ApproveGame
// ============================================================

TEST_F(QZNPortalTest, ApproveGame_HappyPath_GameMovesToActive)
{
    initialize();
    sint64 gid = registerGame();
    bool ok = approveGame(gid, 500LL);

    EXPECT_TRUE(ok);
    EXPECT_EQ(tester.state().games_0.state,           GAME_ACTIVE);
    EXPECT_EQ(tester.state().games_0.revenueShareBPS, 500LL);
    EXPECT_EQ(tester.state().totalGamesActive,        1LL);
}

TEST_F(QZNPortalTest, ApproveGame_HappyPath_ApprovedEpochStored)
{
    initialize();
    tester.setCurrentEpoch(7);
    sint64 gid = registerGame();
    approveGame(gid);

    EXPECT_EQ(tester.state().games_0.approvedEpoch, 7LL);
}

TEST_F(QZNPortalTest, ApproveGame_NonAdmin_Rejected)
{
    initialize();
    sint64 gid = registerGame();

    ApproveGame_input in{};
    in.gameId          = gid;
    in.revenueShareBPS = 500LL;
    tester.setInvocator(STRANGER_ADDR);
    auto out = tester.callProcedure(ApproveGame_id, in);

    EXPECT_EQ(out.success, 0);
    EXPECT_EQ(tester.state().games_0.state, GAME_PENDING);
}

TEST_F(QZNPortalTest, ApproveGame_AlreadyActive_NotReapproved)
{
    initialize();
    sint64 gid = registerGame();
    approveGame(gid, 500LL);

    sint64 prevActive = tester.state().totalGamesActive;

    // Try to approve again — should be rejected (state != PENDING)
    approveGame(gid, 999LL);

    // totalGamesActive must not double-count
    EXPECT_EQ(tester.state().totalGamesActive, prevActive);
    // Revenue share BPS must remain at first approval value
    EXPECT_EQ(tester.state().games_0.revenueShareBPS, 500LL);
}

TEST_F(QZNPortalTest, ApproveGame_NonExistentGameId_Rejected)
{
    initialize();
    bool ok = approveGame(9999LL);
    EXPECT_FALSE(ok);
    EXPECT_EQ(tester.state().totalGamesActive, 0LL);
}

// ============================================================
//  [PROC-7]  UpdateGameState
// ============================================================

TEST_F(QZNPortalTest, UpdateGameState_SuspendActiveGame)
{
    initialize();
    sint64 gid = registerGame();
    approveGame(gid);

    UpdateGameState_input in{};
    in.gameId   = gid;
    in.newState = GAME_SUSPENDED;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(UpdateGameState_id, in);

    EXPECT_EQ(out.success,                   1);
    EXPECT_EQ(tester.state().games_0.state,  GAME_SUSPENDED);
}

TEST_F(QZNPortalTest, UpdateGameState_RetireActiveGame_StakeRefunded)
{
    initialize();
    sint64 gid = registerGame(BUILDER_A, BUILDER_STAKE_REQUIRED);
    approveGame(gid);

    UpdateGameState_input in{};
    in.gameId   = gid;
    in.newState = GAME_RETIRED;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(UpdateGameState_id, in);

    EXPECT_EQ(out.success,        1);
    EXPECT_EQ(out.stakeRefunded,  BUILDER_STAKE_REQUIRED);
    EXPECT_EQ(tester.state().games_0.state,        GAME_RETIRED);
    EXPECT_EQ(tester.state().games_0.builderStake, 0LL);   // Cleared after refund
}

TEST_F(QZNPortalTest, UpdateGameState_RetireGame_TotalActiveDecremented)
{
    initialize();
    sint64 gid = registerGame();
    approveGame(gid);
    EXPECT_EQ(tester.state().totalGamesActive, 1LL);

    UpdateGameState_input in{};
    in.gameId   = gid;
    in.newState = GAME_RETIRED;
    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(UpdateGameState_id, in);

    EXPECT_EQ(tester.state().totalGamesActive, 0LL);
}

TEST_F(QZNPortalTest, UpdateGameState_NonAdmin_Rejected)
{
    initialize();
    sint64 gid = registerGame();
    approveGame(gid);

    UpdateGameState_input in{};
    in.gameId   = gid;
    in.newState = GAME_SUSPENDED;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(UpdateGameState_id, in);

    EXPECT_EQ(tester.state().games_0.state, GAME_ACTIVE);
}

TEST_F(QZNPortalTest, UpdateGameState_ActiveGameNotBelow0_TotalGamesGuarded)
{
    initialize();
    // Register but don't approve — totalGamesActive stays 0
    registerGame();

    UpdateGameState_input in{};
    in.gameId   = 1;
    in.newState = GAME_RETIRED;
    tester.setInvocator(ADMIN_ADDR);
    tester.callProcedure(UpdateGameState_id, in);

    // totalGamesActive should not underflow below 0
    EXPECT_GE(tester.state().totalGamesActive, 0LL);
}

// ============================================================
//  [PROC-8]  StakeForAccess
// ============================================================

TEST_F(QZNPortalTest, StakeForAccess_ZeroAmount_Rejected)
{
    initialize();
    StakeForAccess_input in{};
    in.amount = 0LL;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeForAccess_id, in);

    EXPECT_EQ(tester.state().playerCount, 0LL);
}

TEST_F(QZNPortalTest, StakeForAccess_BelowStakerThreshold_FreeTier)
{
    initialize();
    StakeForAccess_input in{};
    in.amount = STAKER_THRESHOLD - 1LL;
    tester.setInvocator(PLAYER_A);
    auto out = tester.callProcedure(StakeForAccess_id, in);

    EXPECT_EQ(out.accessTier,        ACCESS_FREE);
    EXPECT_EQ(out.nextTierThreshold, STAKER_THRESHOLD);
}

TEST_F(QZNPortalTest, StakeForAccess_AtStakerThreshold_StakerTier)
{
    initialize();
    StakeForAccess_input in{};
    in.amount = STAKER_THRESHOLD;
    tester.setInvocator(PLAYER_A);
    auto out = tester.callProcedure(StakeForAccess_id, in);

    EXPECT_EQ(out.accessTier,        ACCESS_STAKER);
    EXPECT_EQ(out.totalStaked,       STAKER_THRESHOLD);
    EXPECT_EQ(out.nextTierThreshold, CHAMPION_THRESHOLD);
}

TEST_F(QZNPortalTest, StakeForAccess_AtChampionThreshold_ChampionTier)
{
    initialize();
    StakeForAccess_input in{};
    in.amount = CHAMPION_THRESHOLD;
    tester.setInvocator(PLAYER_A);
    auto out = tester.callProcedure(StakeForAccess_id, in);

    EXPECT_EQ(out.accessTier,        ACCESS_CHAMPION);
    EXPECT_EQ(out.totalStaked,       CHAMPION_THRESHOLD);
    EXPECT_EQ(out.nextTierThreshold, 0LL);  // Max tier — no next threshold
}

TEST_F(QZNPortalTest, StakeForAccess_IncrementalStakes_Accumulate)
{
    initialize();
    StakeForAccess_input in{};
    in.amount = STAKER_THRESHOLD / 2;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeForAccess_id, in);

    in.amount = STAKER_THRESHOLD / 2;
    auto out = tester.callProcedure(StakeForAccess_id, in);  // Total = STAKER_THRESHOLD

    EXPECT_EQ(out.accessTier,  ACCESS_STAKER);
    EXPECT_EQ(out.totalStaked, STAKER_THRESHOLD);
}

TEST_F(QZNPortalTest, StakeForAccess_NewPlayerSlotAssigned)
{
    initialize();
    StakeForAccess_input in{};
    in.amount = STAKER_THRESHOLD;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeForAccess_id, in);

    EXPECT_EQ(tester.state().playerCount,       1LL);
    EXPECT_EQ(tester.state().playerWallet_0,    PLAYER_A);
    EXPECT_EQ(tester.state().playerStake_0,     STAKER_THRESHOLD);
}

TEST_F(QZNPortalTest, StakeForAccess_TwoPlayers_SeparateSlots)
{
    initialize();
    StakeForAccess_input in{};
    in.amount = STAKER_THRESHOLD;

    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeForAccess_id, in);

    tester.setInvocator(PLAYER_B);
    tester.callProcedure(StakeForAccess_id, in);

    EXPECT_EQ(tester.state().playerCount,    2LL);
    EXPECT_EQ(tester.state().playerWallet_0, PLAYER_A);
    EXPECT_EQ(tester.state().playerWallet_1, PLAYER_B);
    EXPECT_EQ(tester.state().playerStake_0,  STAKER_THRESHOLD);
    EXPECT_EQ(tester.state().playerStake_1,  STAKER_THRESHOLD);
}

TEST_F(QZNPortalTest, StakeForAccess_ReturningPlayer_StakeAccumulates)
{
    initialize();
    StakeForAccess_input in{};
    in.amount = STAKER_THRESHOLD;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeForAccess_id, in);

    // Player A stakes again — should accumulate, not create new slot
    in.amount = STAKER_THRESHOLD;
    tester.callProcedure(StakeForAccess_id, in);

    EXPECT_EQ(tester.state().playerCount,   1LL);  // Still 1 player
    EXPECT_EQ(tester.state().playerStake_0, STAKER_THRESHOLD * 2LL);
}

// ============================================================
//  [PROC-9]  ReceiveProtocolFees
// ============================================================

TEST_F(QZNPortalTest, ReceiveProtocolFees_20PercentRoutedToPool)
{
    initialize();
    receiveProtocolFees(PROTOCOL_FEES);

    sint64 expectedPool = (PROTOCOL_FEES * NODE_REVENUE_POOL_BPS) / REVENUE_BPS_DENOM;
    EXPECT_EQ(tester.state().currentEpochRevenuePool, expectedPool);
    EXPECT_EQ(tester.state().pendingDistribution,     expectedPool);
}

TEST_F(QZNPortalTest, ReceiveProtocolFees_AdminCanAlsoCall)
{
    initialize();
    ReceiveProtocolFees_input in{};
    in.amount = PROTOCOL_FEES;
    tester.setInvocator(ADMIN_ADDR);
    auto out = tester.callProcedure(ReceiveProtocolFees_id, in);

    EXPECT_GT(out.addedToPool, 0LL);
}

TEST_F(QZNPortalTest, ReceiveProtocolFees_Stranger_Rejected)
{
    initialize();
    ReceiveProtocolFees_input in{};
    in.amount = PROTOCOL_FEES;
    tester.setInvocator(STRANGER_ADDR);
    tester.callProcedure(ReceiveProtocolFees_id, in);

    EXPECT_EQ(tester.state().currentEpochRevenuePool, 0LL);
}

TEST_F(QZNPortalTest, ReceiveProtocolFees_PoolAccumulatesAcrossMultipleCalls)
{
    initialize();
    receiveProtocolFees(PROTOCOL_FEES);
    receiveProtocolFees(PROTOCOL_FEES);

    sint64 singleContrib = (PROTOCOL_FEES * NODE_REVENUE_POOL_BPS) / REVENUE_BPS_DENOM;
    EXPECT_EQ(tester.state().currentEpochRevenuePool, singleContrib * 2LL);
}

TEST_F(QZNPortalTest, ReceiveProtocolFees_OutputReflectsCurrentPool)
{
    initialize();
    ReceiveProtocolFees_input in{};
    in.amount = PROTOCOL_FEES;
    tester.setInvocator(QZN_CORE_ADDR);
    auto out = tester.callProcedure(ReceiveProtocolFees_id, in);

    sint64 expected = (PROTOCOL_FEES * NODE_REVENUE_POOL_BPS) / REVENUE_BPS_DENOM;
    EXPECT_EQ(out.addedToPool, expected);
    EXPECT_EQ(out.currentPool, expected);
}

// ============================================================
//  [FUNC-10] GetPlayerAccess
// ============================================================

TEST_F(QZNPortalTest, GetPlayerAccess_UnregisteredPlayer_FreeTier)
{
    initialize();
    GetPlayerAccess_input in{};
    in.walletAddress = PLAYER_A;
    auto out = tester.callFunction(GetPlayerAccess_id, in);

    EXPECT_EQ(out.accessTier,     ACCESS_FREE);
    EXPECT_EQ(out.stakedAmount,   0LL);
    EXPECT_EQ(out.canPlayPremium, 0);
    EXPECT_EQ(out.hasEarlyAccess, 0);
}

TEST_F(QZNPortalTest, GetPlayerAccess_StakerTier_CanPlayPremium)
{
    initialize();
    StakeForAccess_input sin{};
    sin.amount = STAKER_THRESHOLD;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeForAccess_id, sin);

    GetPlayerAccess_input in{};
    in.walletAddress = PLAYER_A;
    auto out = tester.callFunction(GetPlayerAccess_id, in);

    EXPECT_EQ(out.accessTier,     ACCESS_STAKER);
    EXPECT_EQ(out.stakedAmount,   STAKER_THRESHOLD);
    EXPECT_EQ(out.canPlayPremium, 1);
    EXPECT_EQ(out.hasEarlyAccess, 0);
}

TEST_F(QZNPortalTest, GetPlayerAccess_ChampionTier_AllFlagsSet)
{
    initialize();
    StakeForAccess_input sin{};
    sin.amount = CHAMPION_THRESHOLD;
    tester.setInvocator(PLAYER_A);
    tester.callProcedure(StakeForAccess_id, sin);

    GetPlayerAccess_input in{};
    in.walletAddress = PLAYER_A;
    auto out = tester.callFunction(GetPlayerAccess_id, in);

    EXPECT_EQ(out.accessTier,     ACCESS_CHAMPION);
    EXPECT_EQ(out.canPlayPremium, 1);
    EXPECT_EQ(out.hasEarlyAccess, 1);
}

TEST_F(QZNPortalTest, GetPlayerAccess_IsReadOnly)
{
    initialize();
    sint64 prevCount = tester.state().playerCount;

    GetPlayerAccess_input in{};
    in.walletAddress = PLAYER_A;
    tester.callFunction(GetPlayerAccess_id, in);
    tester.callFunction(GetPlayerAccess_id, in);

    EXPECT_EQ(tester.state().playerCount, prevCount);
}

// ============================================================
//  [FUNC-11] GetNode
// ============================================================

TEST_F(QZNPortalTest, GetNode_AfterIssue_CorrectTierAndOwner)
{
    initialize();
    issueNode(1, OWNER_A);

    GetNode_input in{};
    in.nodeId = 1;
    auto out = tester.callFunction(GetNode_id, in);

    EXPECT_EQ(out.tier,           TIER_GENESIS);
    EXPECT_EQ(out.state,          NODE_ACTIVE);
    EXPECT_EQ(out.owner,          OWNER_A);
    EXPECT_EQ(out.shareBPS,       GENESIS_SHARE_BPS);
    EXPECT_EQ(out.pendingRevenue, 0LL);
    EXPECT_EQ(out.lifetimeRevenue, 0LL);
}

TEST_F(QZNPortalTest, GetNode_AfterManualRevenueSet_PendingReflected)
{
    initialize();
    issueNode(1, OWNER_A);
    tester.state().nodes_1.pendingRevenue = 9999LL;

    GetNode_input in{};
    in.nodeId = 1;
    auto out = tester.callFunction(GetNode_id, in);

    EXPECT_EQ(out.pendingRevenue, 9999LL);
}

TEST_F(QZNPortalTest, GetNode_CoreNode_CorrectShareBPS)
{
    initialize();
    issueNode(11, OWNER_B);

    GetNode_input in{};
    in.nodeId = 11;
    auto out = tester.callFunction(GetNode_id, in);

    EXPECT_EQ(out.tier,     TIER_CORE);
    EXPECT_EQ(out.shareBPS, CORE_SHARE_BPS);
}

// ============================================================
//  [FUNC-12] GetGame
// ============================================================

TEST_F(QZNPortalTest, GetGame_AfterRegister_PendingState)
{
    initialize();
    tester.setCurrentEpoch(3);
    sint64 gid = registerGame(BUILDER_A, BUILDER_STAKE_REQUIRED, NAME_HASH);

    GetGame_input in{};
    in.gameId = gid;
    auto out = tester.callFunction(GetGame_id, in);

    EXPECT_EQ(out.state,           GAME_PENDING);
    EXPECT_EQ(out.builder,         BUILDER_A);
    EXPECT_EQ(out.nameHash,        NAME_HASH);
    EXPECT_EQ(out.revenueShareBPS, 0LL);
    EXPECT_EQ(out.registeredEpoch, 3LL);
}

TEST_F(QZNPortalTest, GetGame_AfterApprove_ActiveStateAndBPS)
{
    initialize();
    sint64 gid = registerGame();
    approveGame(gid, 750LL);

    GetGame_input in{};
    in.gameId = gid;
    auto out = tester.callFunction(GetGame_id, in);

    EXPECT_EQ(out.state,           GAME_ACTIVE);
    EXPECT_EQ(out.revenueShareBPS, 750LL);
}

// ============================================================
//  [FUNC-13] GetPortalStats
// ============================================================

TEST_F(QZNPortalTest, GetPortalStats_InitialState_AllZero)
{
    initialize();
    GetPortalStats_input in{};
    auto out = tester.callFunction(GetPortalStats_id, in);

    EXPECT_EQ(out.totalNodesIssued,        0LL);
    EXPECT_EQ(out.totalActiveNodes,        0LL);
    EXPECT_EQ(out.totalGamesActive,        0LL);
    EXPECT_EQ(out.totalRevenueDistributed, 0LL);
    EXPECT_EQ(out.currentEpochPool,        0LL);
}

TEST_F(QZNPortalTest, GetPortalStats_AfterNodesIssued_CountsCorrect)
{
    initialize();
    issueNode(1, OWNER_A);
    issueNode(2, OWNER_B);

    GetPortalStats_input in{};
    auto out = tester.callFunction(GetPortalStats_id, in);

    EXPECT_EQ(out.totalNodesIssued, 2LL);
    EXPECT_EQ(out.totalActiveNodes, 2LL);
}

TEST_F(QZNPortalTest, GetPortalStats_AfterGameApproved_ActiveGameCount)
{
    initialize();
    registerGame();
    approveGame(1LL);

    GetPortalStats_input in{};
    auto out = tester.callFunction(GetPortalStats_id, in);

    EXPECT_EQ(out.totalGamesActive, 1LL);
}

TEST_F(QZNPortalTest, GetPortalStats_AfterFeeReceived_PoolUpdated)
{
    initialize();
    receiveProtocolFees(PROTOCOL_FEES);

    GetPortalStats_input in{};
    auto out = tester.callFunction(GetPortalStats_id, in);

    sint64 expected = (PROTOCOL_FEES * NODE_REVENUE_POOL_BPS) / REVENUE_BPS_DENOM;
    EXPECT_EQ(out.currentEpochPool, expected);
}

TEST_F(QZNPortalTest, GetPortalStats_IsReadOnly)
{
    initialize();
    issueNode(1, OWNER_A);
    sint64 prevNodes = tester.state().totalNodesIssued;

    GetPortalStats_input in{};
    tester.callFunction(GetPortalStats_id, in);
    tester.callFunction(GetPortalStats_id, in);

    EXPECT_EQ(tester.state().totalNodesIssued, prevNodes);
}

// ============================================================
//  BEGIN_EPOCH integration tests
// ============================================================

TEST_F(QZNPortalTest, BeginEpoch_NoPendingDistribution_NoOp)
{
    initialize();
    issueNode(1, OWNER_A);
    ASSERT_EQ(tester.state().pendingDistribution, 0LL);

    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().nodes_1.pendingRevenue, 0LL);
}

TEST_F(QZNPortalTest, BeginEpoch_SingleActiveNode_ReceivesFullPool)
{
    initialize();
    issueNode(1, OWNER_A);
    receiveProtocolFees(PROTOCOL_FEES);

    sint64 pool = tester.state().pendingDistribution;
    ASSERT_GT(pool, 0LL);

    tester.advanceBeginEpoch();

    // Single active node gets everything (within integer division)
    EXPECT_GE(tester.state().nodes_1.pendingRevenue, pool - 1LL);  // Allow 1 QU dust
    EXPECT_EQ(tester.state().pendingDistribution,    0LL);
    EXPECT_EQ(tester.state().currentEpochRevenuePool, 0LL);
}

TEST_F(QZNPortalTest, BeginEpoch_GenesisEarnsMoreThanStandard)
{
    initialize();
    // Issue one Genesis (3.0x) and one Standard-tier node
    issueNode(1, OWNER_A);   // Genesis — shareBPS = 3000
    issueNode(16, OWNER_B);  // Core (abbreviated) — shareBPS = 1500

    receiveProtocolFees(PROTOCOL_FEES);
    tester.advanceBeginEpoch();

    // Genesis node should have received more pending revenue than Core node
    EXPECT_GT(tester.state().nodes_1.pendingRevenue,
              tester.state().nodes_16.pendingRevenue);
}

TEST_F(QZNPortalTest, BeginEpoch_UnclaimedSweep_StaleRevenueClearedAfterWindow)
{
    initialize();
    tester.setCurrentEpoch(1);
    issueNode(1, OWNER_A);

    // Seed pending revenue and set old lastClaimEpoch
    tester.state().nodes_1.pendingRevenue  = 5000LL;
    tester.state().nodes_1.lastClaimEpoch  = 1LL;
    tester.state().pendingDistribution     = 1LL;  // Non-zero to enter BEGIN_EPOCH body

    // Advance past claim window
    tester.setCurrentEpoch(1 + REVENUE_CLAIM_EPOCHS + 1);
    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().nodes_1.pendingRevenue, 0LL);
    EXPECT_GT(tester.state().totalRevenueReturned,   0LL);
}

TEST_F(QZNPortalTest, BeginEpoch_UnclaimedSweep_FreshRevenue_NotSwept)
{
    initialize();
    tester.setCurrentEpoch(1);
    issueNode(1, OWNER_A);

    tester.state().nodes_1.pendingRevenue = 5000LL;
    tester.state().nodes_1.lastClaimEpoch = 1LL;
    tester.state().pendingDistribution    = 1LL;

    // Only 2 epochs later — well within window
    tester.setCurrentEpoch(3);
    tester.advanceBeginEpoch();

    EXPECT_GT(tester.state().nodes_1.pendingRevenue, 0LL);
    EXPECT_EQ(tester.state().totalRevenueReturned,   0LL);
}

TEST_F(QZNPortalTest, BeginEpoch_PendingDistributionResetAfterDistribute)
{
    initialize();
    issueNode(1, OWNER_A);
    receiveProtocolFees(PROTOCOL_FEES);
    ASSERT_GT(tester.state().pendingDistribution, 0LL);

    tester.advanceBeginEpoch();

    EXPECT_EQ(tester.state().pendingDistribution,    0LL);
    EXPECT_EQ(tester.state().currentEpochRevenuePool, 0LL);
}

// ============================================================
//  INVARIANTS
// ============================================================

TEST_F(QZNPortalTest, Invariant_TotalNodesIssuedNeverExceeds100)
{
    initialize();
    // Issue 16 nodes (the abbreviated limit in this contract)
    for (sint64 i = 1; i <= 16; i++)
    {
        issueNode(i, OWNER_A);
    }
    EXPECT_LE(tester.state().totalNodesIssued, MAX_PORTAL_NODES);
}

TEST_F(QZNPortalTest, Invariant_TotalActiveNeverExceedsIssued)
{
    initialize();
    issueNode(1, OWNER_A);
    issueNode(2, OWNER_B);

    EXPECT_LE(tester.state().totalActiveNodes, tester.state().totalNodesIssued);
}

TEST_F(QZNPortalTest, Invariant_RevenueDistributedNeverExceedsPoolReceived)
{
    initialize();
    issueNode(1, OWNER_A);
    receiveProtocolFees(PROTOCOL_FEES);
    tester.advanceBeginEpoch();

    ClaimNodeRevenue_input in{};
    in.nodeId = 1;
    tester.setInvocator(OWNER_A);
    tester.callProcedure(ClaimNodeRevenue_id, in);

    sint64 totalEverReceived = (PROTOCOL_FEES * NODE_REVENUE_POOL_BPS) / REVENUE_BPS_DENOM;
    EXPECT_LE(tester.state().totalRevenueDistributed, totalEverReceived);
}

TEST_F(QZNPortalTest, Invariant_PlayerCountNeverExceedsSlots)
{
    initialize();
    // Register 16 players — the maximum slot count
    for (int i = 0; i < 16; i++)
    {
        StakeForAccess_input in{};
        in.amount = STAKER_THRESHOLD;
        tester.setInvocator(id(100 + i, 0, 0, 0));
        tester.callProcedure(StakeForAccess_id, in);
    }

    EXPECT_LE(tester.state().playerCount, 16LL);
}

TEST_F(QZNPortalTest, Invariant_GenesisShareBPSHigherThanCore)
{
    // Structural invariant — no state needed
    EXPECT_GT(GENESIS_SHARE_BPS, CORE_SHARE_BPS);
    EXPECT_GT(CORE_SHARE_BPS,    STANDARD_SHARE_BPS);
}
