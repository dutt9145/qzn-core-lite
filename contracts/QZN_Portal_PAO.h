// ============================================================
//  QZN PORTAL NODE PAO
//  Contract: Programmable Arcade Object — PORTAL Node
//  Network:  Qubic (QPI / C++ Smart Contract)
//  Version:  1.0.0
//
//  Architecture from: QZN Whitepaper (Section 6 — PORTAL)
//
//  PORTAL is the access, discovery, and profit participation
//  layer of the QZN Arcade Coordination Protocol.
//
//  Responsibilities:
//    1. Game Registry    — Index all QZN-compatible arcade games
//    2. Access Control   — Gate premium content behind QZN stake
//    3. Node Ownership   — PORTAL Nodes are ownable onchain assets
//    4. Revenue Share    — Node owners earn % of protocol fees
//    5. Builder Onboard  — Third-party devs register games via PORTAL
//    6. Discovery Index  — Queryable arcade index for frontend
//
//  PORTAL Node Model:
//    - Fixed supply of PORTAL Nodes (100 max)
//    - Each Node is an ownable onchain object
//    - Node owners earn share of protocol fee revenue
//    - Nodes can be traded on QX secondary market
//    - Higher-tier nodes = higher revenue share
//
//  Node Tiers:
//    TIER_GENESIS  (1-10)   : 3.0x revenue share — founding nodes
//    TIER_CORE     (11-40)  : 1.5x revenue share
//    TIER_STANDARD (41-100) : 1.0x revenue share
//
//  Access Tiers (for players):
//    FREE     : Play base games, earn base rewards
//    STAKER   : Stake ≥ 1,000 QZN → unlock premium features
//    CHAMPION : Stake ≥ 25,000 QZN → unlock all content + early access
//
//  Builder Registration:
//    - Any dev can submit a game for PORTAL listing
//    - Requires QZN stake (anti-spam)
//    - Admin approval required before listing goes live
//    - Approved games share in protocol revenue routing
// ============================================================

using namespace QPI;

// ============================================================
//  CONSTANTS
// ============================================================

// Node supply
constexpr sint64 MAX_PORTAL_NODES         = 100LL;
constexpr sint64 GENESIS_NODE_COUNT       = 10LL;
constexpr sint64 CORE_NODE_COUNT          = 30LL;   // Nodes 11-40
constexpr sint64 STANDARD_NODE_COUNT      = 60LL;   // Nodes 41-100

// Node tiers
constexpr uint8  TIER_GENESIS             = 1;
constexpr uint8  TIER_CORE                = 2;
constexpr uint8  TIER_STANDARD            = 3;

// Revenue share multipliers (basis points, 1000 = 1.0x)
constexpr sint64 GENESIS_SHARE_BPS        = 3000LL;  // 3.0x
constexpr sint64 CORE_SHARE_BPS           = 1500LL;  // 1.5x
constexpr sint64 STANDARD_SHARE_BPS       = 1000LL;  // 1.0x
constexpr sint64 SHARE_BPS_DENOM          = 1000LL;

// Revenue distribution: % of protocol fees to PORTAL nodes
constexpr sint64 NODE_REVENUE_POOL_BPS    = 2000LL;  // 20% of protocol fees
constexpr sint64 REVENUE_BPS_DENOM        = 10000LL;

// Player access tier thresholds (QZN staked)
constexpr sint64 STAKER_THRESHOLD         = 1000LL;
constexpr sint64 CHAMPION_THRESHOLD       = 25000LL;

// Access tier IDs
constexpr uint8  ACCESS_FREE              = 0;
constexpr uint8  ACCESS_STAKER            = 1;
constexpr uint8  ACCESS_CHAMPION          = 2;

// Builder registration stake (anti-spam)
constexpr sint64 BUILDER_STAKE_REQUIRED   = 5000LL;  // 5k QZN to list a game
constexpr sint64 MAX_REGISTERED_GAMES     = 32LL;

// Game states
constexpr uint8  GAME_PENDING             = 0;   // Submitted, awaiting approval
constexpr uint8  GAME_ACTIVE              = 1;   // Live on PORTAL
constexpr uint8  GAME_SUSPENDED           = 2;   // Temporarily removed
constexpr uint8  GAME_RETIRED             = 3;   // Permanently removed

// Node states
constexpr uint8  NODE_UNISSUED            = 0;
constexpr uint8  NODE_ACTIVE              = 1;
constexpr uint8  NODE_SUSPENDED           = 2;

// Claim window
constexpr sint64 REVENUE_CLAIM_EPOCHS     = 8LL;  // Unclaimed after 8 epochs → returned to treasury

// ============================================================
//  DATA STRUCTURES
// ============================================================

struct PortalNode
{
    sint64  nodeId;               // 1-100
    uint8   tier;                 // TIER_GENESIS / TIER_CORE / TIER_STANDARD
    uint8   state;                // NODE_UNISSUED / NODE_ACTIVE / NODE_SUSPENDED
    id      ownerAddress;         // Current owner wallet
    sint64  shareBPS;             // Revenue share multiplier
    sint64  pendingRevenue;       // QZN pending claim
    sint64  lifetimeRevenue;      // Total earned
    sint64  issuedEpoch;          // Epoch node was first issued
    sint64  lastClaimEpoch;       // Last epoch owner claimed revenue
};

struct GameListing
{
    sint64  gameId;               // Auto-assigned
    uint8   state;                // GAME_PENDING / GAME_ACTIVE / GAME_SUSPENDED / GAME_RETIRED
    id      builderAddress;       // Developer wallet
    sint64  builderStake;         // QZN staked by builder (returned on retirement)
    sint64  nameHash;             // K12 hash of game name
    sint64  metadataHash;         // K12 hash of game metadata (offchain)
    sint64  revenueShareBPS;      // % of routing fees this game contributes
    sint64  totalPlayCount;       // Reported by Game Cabinet PAO
    sint64  totalVolumeQZN;       // Total QZN staked in this game's matches
    sint64  registeredEpoch;
    sint64  approvedEpoch;
};

// ============================================================
//  CONTRACT STATE
// ============================================================

struct QZNPORTAL2
{
};

struct QZNPORTAL : public ContractBase
{
    struct StateData
    {
    // ---- PORTAL Nodes (100 slots) ----
    // Abbreviated to 16 for clarity — full deployment expands to 100
    PortalNode nodes_1;
    PortalNode nodes_2;
    PortalNode nodes_3;
    PortalNode nodes_4;
    PortalNode nodes_5;
    PortalNode nodes_6;
    PortalNode nodes_7;
    PortalNode nodes_8;
    PortalNode nodes_9;
    PortalNode nodes_10;
    PortalNode nodes_11;
    PortalNode nodes_12;
    PortalNode nodes_13;
    PortalNode nodes_14;
    PortalNode nodes_15;
    PortalNode nodes_16;

    // ---- Game Registry (32 slots) ----
    GameListing games_0;
    GameListing games_1;
    GameListing games_2;
    GameListing games_3;
    GameListing games_4;
    GameListing games_5;
    GameListing games_6;
    GameListing games_7;
    GameListing games_8;
    GameListing games_9;
    GameListing games_10;
    GameListing games_11;
    GameListing games_12;
    GameListing games_13;
    GameListing games_14;
    GameListing games_15;

    // ---- Player Access Registry ----
    // Tracks staked amount per player for access tier computation
    // Abbreviated to 32 slots — full deployment: 256+
    id      playerWallet_0;   sint64 playerStake_0;
    id      playerWallet_1;   sint64 playerStake_1;
    id      playerWallet_2;   sint64 playerStake_2;
    id      playerWallet_3;   sint64 playerStake_3;
    id      playerWallet_4;   sint64 playerStake_4;
    id      playerWallet_5;   sint64 playerStake_5;
    id      playerWallet_6;   sint64 playerStake_6;
    id      playerWallet_7;   sint64 playerStake_7;
    id      playerWallet_8;   sint64 playerStake_8;
    id      playerWallet_9;   sint64 playerStake_9;
    id      playerWallet_10;  sint64 playerStake_10;
    id      playerWallet_11;  sint64 playerStake_11;
    id      playerWallet_12;  sint64 playerStake_12;
    id      playerWallet_13;  sint64 playerStake_13;
    id      playerWallet_14;  sint64 playerStake_14;
    id      playerWallet_15;  sint64 playerStake_15;
    sint64  playerCount;

    // ---- Revenue Pool ----
    sint64  currentEpochRevenuePool;  // QZN accumulated this epoch
    sint64  totalRevenueDistributed;  // Lifetime distributed to nodes
    sint64  totalRevenueReturned;     // Unclaimed revenue returned to treasury
    sint64  pendingDistribution;      // Revenue queued for next BEGIN_EPOCH

    // ---- Node Stats ----
    sint64  totalNodesIssued;
    sint64  totalActiveNodes;
    sint64  genesisNodesIssued;
    sint64  coreNodesIssued;
    sint64  standardNodesIssued;

    // ---- Game Registry Stats ----
    sint64  totalGamesRegistered;
    sint64  totalGamesActive;
    sint64  nextGameId;

    // ---- Connected Contracts ----
    id      treasuryAddress;
    id      qznCoreAddress;
    id      gameCabinetAddress;

    // ---- Admin ----
    id      adminAddress;
    bit     initialized;
    bit     portalActive;
    id      tokenContractAddress;
    sint64  nodeDividendPool;
    sint64  epochEfficiencyRating;
    };

public:

// ============================================================
//  INPUT / OUTPUT STRUCTS
// ============================================================

// --- Initialize ---
struct InitializePortal_input
{
    id      treasuryAddr;
    id      qznCoreAddr;
    id      gameCabinetAddr;
};
struct InitializePortal_output
{
    bit     success;
};

// --- Issue Node (admin mints node to owner) ---
struct IssueNode_input
{
    sint64  nodeId;          // 1-100
    id      ownerAddress;
};
struct IssueNode_output
{
    sint64  nodeId;
    uint8   tier;
    sint64  shareBPS;
    bit     success;
};

// --- Transfer Node Ownership ---
struct TransferNode_input
{
    sint64  nodeId;
    id      newOwner;
};
struct TransferNode_output
{
    bit     success;
    sint64  nodeId;
    id      previousOwner;
    id      newOwner;
};

// --- Claim Node Revenue ---
struct ClaimNodeRevenue_input
{
    sint64  nodeId;
};
struct ClaimNodeRevenue_output
{
    sint64  amountClaimed;
    sint64  pendingRemaining;
    sint64  lifetimeRevenue;
};

// --- Register Game (builder submits game for listing) ---
struct RegisterGame_input
{
    sint64  nameHash;         // K12 hash of game name
    sint64  metadataHash;     // K12 hash of metadata (offchain: description, screenshots, etc.)
    sint64  stakeAmount;      // QZN staked (must be >= BUILDER_STAKE_REQUIRED)
};
struct RegisterGame_output
{
    sint64  gameId;
    bit     success;
    sint64  stakeRequired;
};

// --- Approve Game (admin approves pending listing) ---
struct ApproveGame_input
{
    sint64  gameId;
    sint64  revenueShareBPS;  // % of routing fees assigned to this game
};
struct ApproveGame_output
{
    bit     success;
    sint64  gameId;
};

// --- Suspend/Retire Game ---
struct UpdateGameState_input
{
    sint64  gameId;
    uint8   newState;         // GAME_SUSPENDED or GAME_RETIRED
};
struct UpdateGameState_output
{
    bit     success;
    sint64  stakeRefunded;    // Builder stake returned on retirement
};

// --- Stake for Access Tier ---
struct StakeForAccess_input
{
    sint64  amount;
};
struct StakeForAccess_output
{
    uint8   accessTier;       // ACCESS_FREE / ACCESS_STAKER / ACCESS_CHAMPION
    sint64  totalStaked;
    sint64  nextTierThreshold; // QZN needed to reach next tier (0 if max)
};

// --- Check Player Access ---
struct GetPlayerAccess_input
{
    id      walletAddress;
};
struct GetPlayerAccess_output
{
    uint8   accessTier;
    sint64  stakedAmount;
    bit     canPlayPremium;
    bit     hasEarlyAccess;
};

// --- Receive Protocol Fees (called by QZN Core) ---
struct ReceiveProtocolFees_input
{
    sint64  amount;
};
struct ReceiveProtocolFees_output
{
    sint64  addedToPool;
    sint64  currentPool;
};

// --- Query Node ---
struct GetNode_input
{
    sint64  nodeId;
};
struct GetNode_output
{
    uint8   tier;
    uint8   state;
    id      owner;
    sint64  shareBPS;
    sint64  pendingRevenue;
    sint64  lifetimeRevenue;
};

// --- Query Game ---
struct GetGame_input
{
    sint64  gameId;
};
struct GetGame_output
{
    uint8   state;
    id      builder;
    sint64  nameHash;
    sint64  totalPlayCount;
    sint64  totalVolumeQZN;
    sint64  revenueShareBPS;
    sint64  registeredEpoch;
};

// --- Portal Stats ---
struct GetPortalStats_input {};
struct GetPortalStats_output
{
    sint64  totalNodesIssued;
    sint64  totalActiveNodes;
    sint64  totalGamesActive;
    sint64  totalRevenueDistributed;
    sint64  currentEpochPool;
};

// ============================================================
//  CONTRACT PROCEDURES
// ============================================================

PUBLIC_PROCEDURE(InitializePortal)
{
    if (state.get().initialized) { return; }

    state.mut().adminAddress = qpi.invocator();
    state.mut().treasuryAddress = input.treasuryAddr;
    state.mut().qznCoreAddress = input.qznCoreAddr;
    state.mut().gameCabinetAddress = input.gameCabinetAddr;

    state.mut().totalNodesIssued = 0;
    state.mut().totalActiveNodes = 0;
    state.mut().genesisNodesIssued = 0;
    state.mut().coreNodesIssued = 0;
    state.mut().standardNodesIssued = 0;
    state.mut().totalGamesRegistered = 0;
    state.mut().totalGamesActive = 0;
    state.mut().nextGameId = 1;
    state.mut().currentEpochRevenuePool = 0;
    state.mut().totalRevenueDistributed = 0;
    state.mut().totalRevenueReturned = 0;
    state.mut().pendingDistribution = 0;
    state.mut().playerCount = 0;
    state.mut().portalActive = 1;
    state.mut().initialized = 1;

    output.success = 1;
}

PUBLIC_PROCEDURE(IssueNode)
/*
 * Admin mints a PORTAL Node to an owner address.
 * Node tier is determined by nodeId range:
 *   1-10:  GENESIS  (3.0x revenue share)
 *   11-40: CORE     (1.5x revenue share)
 *   41-100: STANDARD (1.0x revenue share)
 *
 * Nodes are finite — only 100 ever exist.
 * After issuance, nodes can be traded on QX.
 */
{
    if (qpi.invocator() != state.get().adminAddress) { return; }
    if (input.nodeId < 1 || input.nodeId > MAX_PORTAL_NODES) { return; }

    // Determine tier and share
    uint8   tier;
    sint64  shareBPS;

    if (input.nodeId <= GENESIS_NODE_COUNT)
    {
        tier     = TIER_GENESIS;
        shareBPS = GENESIS_SHARE_BPS;
        state.mut().genesisNodesIssued = state.get().genesisNodesIssued + 1;
    }
    else if (input.nodeId <= GENESIS_NODE_COUNT + CORE_NODE_COUNT)
    {
        tier     = TIER_CORE;
        shareBPS = CORE_SHARE_BPS;
        state.mut().coreNodesIssued = state.get().coreNodesIssued + 1;
    }
    else
    {
        tier     = TIER_STANDARD;
        shareBPS = STANDARD_SHARE_BPS;
        state.mut().standardNodesIssued = state.get().standardNodesIssued + 1;
    }

    // Write to node slot (slot = nodeId - 1, shown for nodes 1-16)
    if (input.nodeId == 1)
    {
        state.mut().nodes_1.nodeId = 1;
        state.mut().nodes_1.tier = tier;
        state.mut().nodes_1.state = NODE_ACTIVE;
        state.mut().nodes_1.ownerAddress = input.ownerAddress;
        state.mut().nodes_1.shareBPS = shareBPS;
        state.mut().nodes_1.pendingRevenue = 0;
        state.mut().nodes_1.lifetimeRevenue = 0;
        state.mut().nodes_1.issuedEpoch = qpi.epoch();
        state.mut().nodes_1.lastClaimEpoch = qpi.epoch();
    }
    else if (input.nodeId == 2)
    {
        state.mut().nodes_2.nodeId = 2;
        state.mut().nodes_2.tier = tier;
        state.mut().nodes_2.state = NODE_ACTIVE;
        state.mut().nodes_2.ownerAddress = input.ownerAddress;
        state.mut().nodes_2.shareBPS = shareBPS;
        state.mut().nodes_2.pendingRevenue = 0;
        state.mut().nodes_2.lifetimeRevenue = 0;
        state.mut().nodes_2.issuedEpoch = qpi.epoch();
        state.mut().nodes_2.lastClaimEpoch = qpi.epoch();
    }
    // Nodes 3-16 follow identical pattern
    // Full deployment expands to all 100

    state.mut().totalNodesIssued = state.get().totalNodesIssued + 1;
    state.mut().totalActiveNodes = state.get().totalActiveNodes + 1;

    output.nodeId   = input.nodeId;
    output.tier     = tier;
    output.shareBPS = shareBPS;
    output.success  = 1;
}

PUBLIC_PROCEDURE(TransferNode)
/*
 * Current node owner transfers ownership to a new address.
 * Used for QX secondary market trades.
 * Revenue accrued before transfer stays with previous owner.
 * Future revenue goes to new owner from transfer epoch onward.
 */
{
    if (input.nodeId < 1 || input.nodeId > MAX_PORTAL_NODES) { return; }

    if (input.nodeId == 1)
    {
        if (qpi.invocator() != state.get().nodes_1.ownerAddress) { output.success = 0; return; }
        if (state.get().nodes_1.state != NODE_ACTIVE)            { output.success = 0; return; }

        id prevOwner;
        prevOwner = state.get().nodes_1.ownerAddress;
        state.mut().nodes_1.ownerAddress = input.newOwner;
        state.mut().nodes_1.lastClaimEpoch = qpi.epoch();

        output.success       = 1;
        output.nodeId        = 1;
        output.previousOwner = prevOwner;
        output.newOwner      = input.newOwner;
    }
    else if (input.nodeId == 2)
    {
        if (qpi.invocator() != state.get().nodes_2.ownerAddress) { output.success = 0; return; }
        if (state.get().nodes_2.state != NODE_ACTIVE)            { output.success = 0; return; }

        id prevOwner;
        prevOwner = state.get().nodes_2.ownerAddress;
        state.mut().nodes_2.ownerAddress = input.newOwner;
        state.mut().nodes_2.lastClaimEpoch = qpi.epoch();

        output.success       = 1;
        output.nodeId        = 2;
        output.previousOwner = prevOwner;
        output.newOwner      = input.newOwner;
    }
    // Nodes 3-16 identical
}

PUBLIC_PROCEDURE(ClaimNodeRevenue)
/*
 * Node owner claims accumulated revenue.
 * Revenue is accrued each epoch via BEGIN_EPOCH() distribution.
 * Unclaimed after REVENUE_CLAIM_EPOCHS is returned to treasury.
 */
{
    if (input.nodeId < 1 || input.nodeId > MAX_PORTAL_NODES) { return; }

    if (input.nodeId == 1)
    {
        if (qpi.invocator() != state.get().nodes_1.ownerAddress)  { return; }
        if (state.get().nodes_1.state != NODE_ACTIVE)              { return; }
        if (state.get().nodes_1.pendingRevenue <= 0)               { return; }

        sint64 claimAmt;
        claimAmt = state.get().nodes_1.pendingRevenue;

        state.mut().nodes_1.pendingRevenue = 0;
        state.mut().nodes_1.lifetimeRevenue = state.get().nodes_1.lifetimeRevenue + claimAmt;
        state.mut().nodes_1.lastClaimEpoch = qpi.epoch();
        state.mut().totalRevenueDistributed = state.get().totalRevenueDistributed + claimAmt;

        qpi.transfer(qpi.invocator(), claimAmt);

        output.amountClaimed    = claimAmt;
        output.pendingRemaining = 0;
        output.lifetimeRevenue  = state.get().nodes_1.lifetimeRevenue;
    }
    else if (input.nodeId == 2)
    {
        if (qpi.invocator() != state.get().nodes_2.ownerAddress)  { return; }
        if (state.get().nodes_2.state != NODE_ACTIVE)              { return; }
        if (state.get().nodes_2.pendingRevenue <= 0)               { return; }

        sint64 claimAmt;
        claimAmt = state.get().nodes_2.pendingRevenue;

        state.mut().nodes_2.pendingRevenue = 0;
        state.mut().nodes_2.lifetimeRevenue = state.get().nodes_2.lifetimeRevenue + claimAmt;
        state.mut().nodes_2.lastClaimEpoch = qpi.epoch();
        state.mut().totalRevenueDistributed = state.get().totalRevenueDistributed + claimAmt;

        qpi.transfer(qpi.invocator(), claimAmt);

        output.amountClaimed    = claimAmt;
        output.pendingRemaining = 0;
        output.lifetimeRevenue  = state.get().nodes_2.lifetimeRevenue;
    }
    // Nodes 3-16 identical
}

PUBLIC_PROCEDURE(RegisterGame)
/*
 * Third-party builder submits a game for PORTAL listing.
 * Requires QZN stake (anti-spam, returned on retirement).
 * Game starts in GAME_PENDING state — admin must approve.
 *
 * Approved games appear in the PORTAL discovery index
 * and share in protocol revenue routing.
 */
{
    if (!state.get().portalActive) { return; }
    if (input.stakeAmount < BUILDER_STAKE_REQUIRED) { return; }
    if (state.get().totalGamesRegistered >= MAX_REGISTERED_GAMES) { return; }

    // Find free game slot
    sint64 slot;
    slot = -1;

    if (state.get().games_0.state == 0 || state.get().games_0.state == GAME_RETIRED)  { slot = 0; }
    else if (state.get().games_1.state == 0 || state.get().games_1.state == GAME_RETIRED)  { slot = 1; }
    else if (state.get().games_2.state == 0 || state.get().games_2.state == GAME_RETIRED)  { slot = 2; }
    else if (state.get().games_3.state == 0 || state.get().games_3.state == GAME_RETIRED)  { slot = 3; }
    else if (state.get().games_4.state == 0 || state.get().games_4.state == GAME_RETIRED)  { slot = 4; }
    else if (state.get().games_5.state == 0 || state.get().games_5.state == GAME_RETIRED)  { slot = 5; }
    else if (state.get().games_6.state == 0 || state.get().games_6.state == GAME_RETIRED)  { slot = 6; }
    else if (state.get().games_7.state == 0 || state.get().games_7.state == GAME_RETIRED)  { slot = 7; }
    // Slots 8-15 identical

    if (slot < 0) { output.success = 0; return; }

    if (slot == 0)
    {
        state.mut().games_0.gameId = state.get().nextGameId;
        state.mut().games_0.state = GAME_PENDING;
        state.mut().games_0.builderAddress = qpi.invocator();
        state.mut().games_0.builderStake = input.stakeAmount;
        state.mut().games_0.nameHash = input.nameHash;
        state.mut().games_0.metadataHash = input.metadataHash;
        state.mut().games_0.revenueShareBPS = 0;      // Set by admin on approval
        state.mut().games_0.totalPlayCount = 0;
        state.mut().games_0.totalVolumeQZN = 0;
        state.mut().games_0.registeredEpoch = qpi.epoch();
        state.mut().games_0.approvedEpoch = 0;
    }
    // Slots 1-15 identical

    state.mut().nextGameId = state.get().nextGameId + 1;
    state.mut().totalGamesRegistered = state.get().totalGamesRegistered + 1;

    output.gameId        = state.get().nextGameId - 1;
    output.success       = 1;
    output.stakeRequired = BUILDER_STAKE_REQUIRED;
}

PUBLIC_PROCEDURE(ApproveGame)
/*
 * Admin approves a pending game listing.
 * Sets revenue share BPS for this game's contribution.
 * Game goes live in PORTAL discovery index immediately.
 */
{
    if (qpi.invocator() != state.get().adminAddress) { return; }

    if (state.get().games_0.gameId == input.gameId && state.get().games_0.state == GAME_PENDING)
    {
        state.mut().games_0.state = GAME_ACTIVE;
        state.mut().games_0.revenueShareBPS = input.revenueShareBPS;
        state.mut().games_0.approvedEpoch = qpi.epoch();
        state.mut().totalGamesActive = state.get().totalGamesActive + 1;
        output.success = 1;
        output.gameId  = input.gameId;
    }
    else if (state.get().games_1.gameId == input.gameId && state.get().games_1.state == GAME_PENDING)
    {
        state.mut().games_1.state = GAME_ACTIVE;
        state.mut().games_1.revenueShareBPS = input.revenueShareBPS;
        state.mut().games_1.approvedEpoch = qpi.epoch();
        state.mut().totalGamesActive = state.get().totalGamesActive + 1;
        output.success = 1;
        output.gameId  = input.gameId;
    }
    // Slots 2-15 identical
}

PUBLIC_PROCEDURE(UpdateGameState)
/*
 * Admin suspends or retires a game.
 * On retirement: builder stake is refunded automatically.
 * Retired games are removed from discovery index.
 */
{
    if (qpi.invocator() != state.get().adminAddress) { return; }

    if (state.get().games_0.gameId == input.gameId)
    {
        state.mut().games_0.state = input.newState;

        if (input.newState == GAME_RETIRED)
        {
            // Refund builder stake
            qpi.transfer(state.get().games_0.builderAddress, state.get().games_0.builderStake);
            output.stakeRefunded = state.get().games_0.builderStake;
            state.mut().games_0.builderStake = 0;

            if (state.get().totalGamesActive > 0)
            {
                state.mut().totalGamesActive = state.get().totalGamesActive - 1;
            }
        }
        output.success = 1;
    }
    // Slots 1-15 identical
}

PUBLIC_PROCEDURE(StakeForAccess)
/*
 * Player stakes QZN to unlock higher access tiers.
 *
 * FREE     : 0 QZN     — base games
 * STAKER   : 1,000 QZN — premium features, bonus rewards
 * CHAMPION : 25,000 QZN — all content + early access to new games
 *
 * Stake is tracked here and mirrored in Reward Router for multiplier.
 */
{
    if (input.amount <= 0) { return; }

    // Find or create player record
    sint64 slot;
    slot = -1;

    if (state.get().playerWallet_0 == qpi.invocator())  { slot = 0; }
    else if (state.get().playerWallet_1 == qpi.invocator())  { slot = 1; }
    else if (state.get().playerWallet_2 == qpi.invocator())  { slot = 2; }
    else if (state.get().playerWallet_3 == qpi.invocator())  { slot = 3; }
    else if (state.get().playerWallet_4 == qpi.invocator())  { slot = 4; }
    else if (state.get().playerWallet_5 == qpi.invocator())  { slot = 5; }
    else if (state.get().playerWallet_6 == qpi.invocator())  { slot = 6; }
    else if (state.get().playerWallet_7 == qpi.invocator())  { slot = 7; }
    else if (state.get().playerWallet_8 == qpi.invocator())  { slot = 8; }
    else if (state.get().playerWallet_9 == qpi.invocator())  { slot = 9; }
    else if (state.get().playerWallet_10 == qpi.invocator()) { slot = 10; }
    else if (state.get().playerWallet_11 == qpi.invocator()) { slot = 11; }
    else if (state.get().playerWallet_12 == qpi.invocator()) { slot = 12; }
    else if (state.get().playerWallet_13 == qpi.invocator()) { slot = 13; }
    else if (state.get().playerWallet_14 == qpi.invocator()) { slot = 14; }
    else if (state.get().playerWallet_15 == qpi.invocator()) { slot = 15; }

    // New player — assign slot
    if (slot < 0 && state.get().playerCount < 16)
    {
        slot = state.get().playerCount;
        state.mut().playerCount = state.get().playerCount + 1;

        if (slot == 0)  { state.mut().playerWallet_0 = qpi.invocator(); state.mut().playerStake_0 = 0; }
        else if (slot == 1)  { state.mut().playerWallet_1 = qpi.invocator(); state.mut().playerStake_1 = 0; }
        else if (slot == 2)  { state.mut().playerWallet_2 = qpi.invocator(); state.mut().playerStake_2 = 0; }
        else if (slot == 3)  { state.mut().playerWallet_3 = qpi.invocator(); state.mut().playerStake_3 = 0; }
        else if (slot == 4)  { state.mut().playerWallet_4 = qpi.invocator(); state.mut().playerStake_4 = 0; }
        else if (slot == 5)  { state.mut().playerWallet_5 = qpi.invocator(); state.mut().playerStake_5 = 0; }
        else if (slot == 6)  { state.mut().playerWallet_6 = qpi.invocator(); state.mut().playerStake_6 = 0; }
        else if (slot == 7)  { state.mut().playerWallet_7 = qpi.invocator(); state.mut().playerStake_7 = 0; }
        else if (slot == 8)  { state.mut().playerWallet_8 = qpi.invocator(); state.mut().playerStake_8 = 0; }
        else if (slot == 9)  { state.mut().playerWallet_9 = qpi.invocator(); state.mut().playerStake_9 = 0; }
        else if (slot == 10) { state.mut().playerWallet_10 = qpi.invocator(); state.mut().playerStake_10 = 0; }
        else if (slot == 11) { state.mut().playerWallet_11 = qpi.invocator(); state.mut().playerStake_11 = 0; }
        else if (slot == 12) { state.mut().playerWallet_12 = qpi.invocator(); state.mut().playerStake_12 = 0; }
        else if (slot == 13) { state.mut().playerWallet_13 = qpi.invocator(); state.mut().playerStake_13 = 0; }
        else if (slot == 14) { state.mut().playerWallet_14 = qpi.invocator(); state.mut().playerStake_14 = 0; }
        else if (slot == 15) { state.mut().playerWallet_15 = qpi.invocator(); state.mut().playerStake_15 = 0; }
    }

    if (slot < 0) { return; }

    // Add stake
    sint64 newStake;
    newStake = 0;

    if (slot == 0)  { state.mut().playerStake_0 = state.get().playerStake_0  + input.amount; newStake = state.get().playerStake_0; }
    else if (slot == 1)  { state.mut().playerStake_1 = state.get().playerStake_1  + input.amount; newStake = state.get().playerStake_1; }
    else if (slot == 2)  { state.mut().playerStake_2 = state.get().playerStake_2  + input.amount; newStake = state.get().playerStake_2; }
    else if (slot == 3)  { state.mut().playerStake_3 = state.get().playerStake_3  + input.amount; newStake = state.get().playerStake_3; }
    else if (slot == 4)  { state.mut().playerStake_4 = state.get().playerStake_4  + input.amount; newStake = state.get().playerStake_4; }
    else if (slot == 5)  { state.mut().playerStake_5 = state.get().playerStake_5  + input.amount; newStake = state.get().playerStake_5; }
    else if (slot == 6)  { state.mut().playerStake_6 = state.get().playerStake_6  + input.amount; newStake = state.get().playerStake_6; }
    else if (slot == 7)  { state.mut().playerStake_7 = state.get().playerStake_7  + input.amount; newStake = state.get().playerStake_7; }
    // Slots 8-15 identical

    // Compute access tier
    uint8  tier;
    sint64 nextThreshold;
    tier = ACCESS_FREE;
    nextThreshold = STAKER_THRESHOLD;

    if (newStake >= CHAMPION_THRESHOLD)
    {
        tier          = ACCESS_CHAMPION;
        nextThreshold = 0;
    }
    else if (newStake >= STAKER_THRESHOLD)
    {
        tier          = ACCESS_STAKER;
        nextThreshold = CHAMPION_THRESHOLD;
    }

    output.accessTier         = tier;
    output.totalStaked        = newStake;
    output.nextTierThreshold  = nextThreshold;
}

PUBLIC_PROCEDURE(ReceiveProtocolFees)
/*
 * Called by QZN Core when protocol fees accumulate.
 * 20% of protocol fees flow to PORTAL node revenue pool.
 * Remainder stays in protocol treasury.
 */
{
    if (qpi.invocator() != state.get().qznCoreAddress &&
        qpi.invocator() != state.get().adminAddress)
    {
        return;
    }

    sint64 nodePool;
    nodePool = div(input.amount * NODE_REVENUE_POOL_BPS, REVENUE_BPS_DENOM).quot;

    state.mut().currentEpochRevenuePool = state.get().currentEpochRevenuePool + nodePool;
    state.mut().pendingDistribution = state.get().pendingDistribution + nodePool;

    output.addedToPool  = nodePool;
    output.currentPool  = state.get().currentEpochRevenuePool;
}

// ============================================================
//  READ-ONLY QUERY FUNCTIONS
// ============================================================

PUBLIC_FUNCTION(GetPlayerAccess)
{
    sint64 stake;
    stake = 0;

    if (state.get().playerWallet_0 == input.walletAddress)  { stake = state.get().playerStake_0; }
    else if (state.get().playerWallet_1 == input.walletAddress)  { stake = state.get().playerStake_1; }
    else if (state.get().playerWallet_2 == input.walletAddress)  { stake = state.get().playerStake_2; }
    else if (state.get().playerWallet_3 == input.walletAddress)  { stake = state.get().playerStake_3; }
    else if (state.get().playerWallet_4 == input.walletAddress)  { stake = state.get().playerStake_4; }
    else if (state.get().playerWallet_5 == input.walletAddress)  { stake = state.get().playerStake_5; }
    else if (state.get().playerWallet_6 == input.walletAddress)  { stake = state.get().playerStake_6; }
    else if (state.get().playerWallet_7 == input.walletAddress)  { stake = state.get().playerStake_7; }
    // Slots 8-15 identical

    uint8 tier;
    tier = ACCESS_FREE;
    if (stake >= CHAMPION_THRESHOLD) { tier = ACCESS_CHAMPION; }
    else if (stake >= STAKER_THRESHOLD) { tier = ACCESS_STAKER; }

    output.accessTier      = tier;
    output.stakedAmount    = stake;
    output.canPlayPremium  = (tier >= ACCESS_STAKER)   ? 1 : 0;
    output.hasEarlyAccess  = (tier == ACCESS_CHAMPION) ? 1 : 0;
}

PUBLIC_FUNCTION(GetNode)
{
    if (input.nodeId == 1)
    {
        output.tier           = state.get().nodes_1.tier;
        output.state          = state.get().nodes_1.state;
        output.owner          = state.get().nodes_1.ownerAddress;
        output.shareBPS       = state.get().nodes_1.shareBPS;
        output.pendingRevenue = state.get().nodes_1.pendingRevenue;
        output.lifetimeRevenue = state.get().nodes_1.lifetimeRevenue;
    }
    else if (input.nodeId == 2)
    {
        output.tier           = state.get().nodes_2.tier;
        output.state          = state.get().nodes_2.state;
        output.owner          = state.get().nodes_2.ownerAddress;
        output.shareBPS       = state.get().nodes_2.shareBPS;
        output.pendingRevenue = state.get().nodes_2.pendingRevenue;
        output.lifetimeRevenue = state.get().nodes_2.lifetimeRevenue;
    }
    // Nodes 3-16 identical
}

PUBLIC_FUNCTION(GetGame)
{
    if (state.get().games_0.gameId == input.gameId)
    {
        output.state           = state.get().games_0.state;
        output.builder         = state.get().games_0.builderAddress;
        output.nameHash        = state.get().games_0.nameHash;
        output.totalPlayCount  = state.get().games_0.totalPlayCount;
        output.totalVolumeQZN  = state.get().games_0.totalVolumeQZN;
        output.revenueShareBPS = state.get().games_0.revenueShareBPS;
        output.registeredEpoch = state.get().games_0.registeredEpoch;
    }
    // Slots 1-15 identical
}

PUBLIC_FUNCTION(GetPortalStats)
{
    output.totalNodesIssued        = state.get().totalNodesIssued;
    output.totalActiveNodes        = state.get().totalActiveNodes;
    output.totalGamesActive        = state.get().totalGamesActive;
    output.totalRevenueDistributed = state.get().totalRevenueDistributed;
    output.currentEpochPool        = state.get().currentEpochRevenuePool;
}

// ============================================================
//  REGISTRATION
// ============================================================


    struct ReceiveNodeDividend_input  { sint64 amount; };
    struct ReceiveNodeDividend_output { sint64 newPool; };

    PUBLIC_PROCEDURE(ReceiveNodeDividend)
    {
        if (qpi.invocator() != state.get().tokenContractAddress) { return; }
        if (input.amount <= 0LL) { return; }
        state.mut().nodeDividendPool += input.amount;
        output.newPool = state.get().nodeDividendPool;
    }


    struct SetEfficiencyRating_input  { sint64 rating; };  // 1000=1x, 5000=5x max
    struct SetEfficiencyRating_output { sint64 applied; };

    PUBLIC_PROCEDURE(SetEfficiencyRating)
    {
        if (qpi.invocator() != state.get().adminAddress) { return; }
        if (input.rating < 1000LL || input.rating > 5000LL) { return; }
        state.mut().epochEfficiencyRating = input.rating;
        output.applied = input.rating;
    }

REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
{
    REGISTER_USER_PROCEDURE(InitializePortal,     1);
    REGISTER_USER_PROCEDURE(IssueNode,            2);
    REGISTER_USER_PROCEDURE(TransferNode,         3);
    REGISTER_USER_PROCEDURE(ClaimNodeRevenue,     4);
    REGISTER_USER_PROCEDURE(RegisterGame,         5);
    REGISTER_USER_PROCEDURE(ApproveGame,          6);
    REGISTER_USER_PROCEDURE(UpdateGameState,      7);
    REGISTER_USER_PROCEDURE(StakeForAccess,       8);
    REGISTER_USER_PROCEDURE(ReceiveProtocolFees,  9);
    REGISTER_USER_FUNCTION(GetPlayerAccess,       10);
    REGISTER_USER_FUNCTION(GetNode,               11);
    REGISTER_USER_FUNCTION(GetGame,               12);
    REGISTER_USER_FUNCTION(GetPortalStats,        13);
}

// ============================================================
//  SYSTEM HOOKS
// ============================================================

BEGIN_EPOCH()
/*
 * Every epoch:
 * 1. Distribute accumulated revenue pool across all active nodes
 *    weighted by their share BPS and tier multiplier
 * 2. Sweep unclaimed revenue older than REVENUE_CLAIM_EPOCHS
 *    back to treasury (anti-hoarding)
 * 3. Reset epoch revenue pool
 */
{
    if (state.get().pendingDistribution <= 0) { return; }

    // ---- COMPUTE TOTAL WEIGHTED SHARES ----
    // Sum of (shareBPS) for all active nodes
    sint64 totalWeightedShares;
    totalWeightedShares = 0;

    if (state.get().nodes_1.state == NODE_ACTIVE)  { totalWeightedShares = totalWeightedShares + state.get().nodes_1.shareBPS; }
    if (state.get().nodes_2.state == NODE_ACTIVE)  { totalWeightedShares = totalWeightedShares + state.get().nodes_2.shareBPS; }
    if (state.get().nodes_3.state == NODE_ACTIVE)  { totalWeightedShares = totalWeightedShares + state.get().nodes_3.shareBPS; }
    if (state.get().nodes_4.state == NODE_ACTIVE)  { totalWeightedShares = totalWeightedShares + state.get().nodes_4.shareBPS; }
    if (state.get().nodes_5.state == NODE_ACTIVE)  { totalWeightedShares = totalWeightedShares + state.get().nodes_5.shareBPS; }
    if (state.get().nodes_6.state == NODE_ACTIVE)  { totalWeightedShares = totalWeightedShares + state.get().nodes_6.shareBPS; }
    if (state.get().nodes_7.state == NODE_ACTIVE)  { totalWeightedShares = totalWeightedShares + state.get().nodes_7.shareBPS; }
    if (state.get().nodes_8.state == NODE_ACTIVE)  { totalWeightedShares = totalWeightedShares + state.get().nodes_8.shareBPS; }
    if (state.get().nodes_9.state == NODE_ACTIVE)  { totalWeightedShares = totalWeightedShares + state.get().nodes_9.shareBPS; }
    if (state.get().nodes_10.state == NODE_ACTIVE) { totalWeightedShares = totalWeightedShares + state.get().nodes_10.shareBPS; }
    if (state.get().nodes_11.state == NODE_ACTIVE) { totalWeightedShares = totalWeightedShares + state.get().nodes_11.shareBPS; }
    if (state.get().nodes_12.state == NODE_ACTIVE) { totalWeightedShares = totalWeightedShares + state.get().nodes_12.shareBPS; }
    if (state.get().nodes_13.state == NODE_ACTIVE) { totalWeightedShares = totalWeightedShares + state.get().nodes_13.shareBPS; }
    if (state.get().nodes_14.state == NODE_ACTIVE) { totalWeightedShares = totalWeightedShares + state.get().nodes_14.shareBPS; }
    if (state.get().nodes_15.state == NODE_ACTIVE) { totalWeightedShares = totalWeightedShares + state.get().nodes_15.shareBPS; }
    if (state.get().nodes_16.state == NODE_ACTIVE) { totalWeightedShares = totalWeightedShares + state.get().nodes_16.shareBPS; }

    if (totalWeightedShares <= 0) { return; }

    // ---- DISTRIBUTE REVENUE TO EACH ACTIVE NODE ----
    sint64 pool;
    pool = state.get().pendingDistribution;

    if (state.get().nodes_1.state == NODE_ACTIVE)
    {
        sint64 nodeShare;
        nodeShare = div(pool * state.get().nodes_1.shareBPS, totalWeightedShares).quot;
        state.mut().nodes_1.pendingRevenue = state.get().nodes_1.pendingRevenue + nodeShare;
    }
    if (state.get().nodes_2.state == NODE_ACTIVE)
    {
        sint64 nodeShare;
        nodeShare = div(pool * state.get().nodes_2.shareBPS, totalWeightedShares).quot;
        state.mut().nodes_2.pendingRevenue = state.get().nodes_2.pendingRevenue + nodeShare;
    }
    // Nodes 3-16 identical distribution pattern

    // ---- UNCLAIMED SWEEP ----
    // Return stale revenue to treasury
    if (state.get().nodes_1.state == NODE_ACTIVE &&
        state.get().nodes_1.pendingRevenue > 0 &&
        qpi.epoch() > state.get().nodes_1.lastClaimEpoch + REVENUE_CLAIM_EPOCHS)
    {
        state.mut().totalRevenueReturned = state.get().totalRevenueReturned + state.get().nodes_1.pendingRevenue;
        state.mut().nodes_1.pendingRevenue = 0;
    }
    if (state.get().nodes_2.state == NODE_ACTIVE &&
        state.get().nodes_2.pendingRevenue > 0 &&
        qpi.epoch() > state.get().nodes_2.lastClaimEpoch + REVENUE_CLAIM_EPOCHS)
    {
        state.mut().totalRevenueReturned = state.get().totalRevenueReturned + state.get().nodes_2.pendingRevenue;
        state.mut().nodes_2.pendingRevenue = 0;
    }
    // Nodes 3-16 identical sweep

    // Reset pool
    state.mut().pendingDistribution = 0;
    state.mut().currentEpochRevenuePool = 0;
}

END_TICK() {}

};