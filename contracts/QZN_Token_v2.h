// ============================================================
//  QZN TOKEN — Arcade Coordination Protocol on Qubic
//  Contract: QZN Core Token — Post-ICO Mechanics Vault
//  Network:  Qubic (QPI / C++ Smart Contract)
//  Version:  3.0.0
//
//  Architecture from: QZN Whitepaper
//  Total Fixed Supply: 250,000,000 QZN
//  Issuance:           Via QX asset issuance + QIP ICO portal
//
//  Token Allocation:
//    - ICO (QIP — 2 phases):    20%  (50,000,000 QZN)
//    - Strategic Treasury:      35%  (87,500,000 QZN)
//    - Founder & Team:          20%  (50,000,000 QZN) — 1yr cliff, 4yr vest
//    - Liquidity (QSWAP):       15%  (37,500,000 QZN)
//    - Ecosystem Expansion:     10%  (25,000,000 QZN)
//
//  ICO Flow (handled externally — NOT in this contract):
//    - All 250M QZN issued on QX, land in issuer wallet
//    - ICO portion transferred to QIP (qubicico.com) for sale
//    - Phase 1: 10M QZN @ 150 QU/token  (~1 epoch)
//    - Phase 2: 40M QZN @ 200 QU/token  (~1 epoch)
//    - QIP takes 5% of raise; 95% distributed to payout addresses
//    - Unsold tokens returned to issuer wallet after ICO closes
//
//  This contract handles ONLY post-ICO on-chain mechanics:
//    - Match settlement + BPS routing (called by GameCabinet PAO)
//    - Founder/team vesting with cliff enforcement
//    - Governance treasury burn
//    - Supply + stats tracking
//    - transferShareOwnershipAndPossession for all token moves
//
//  Match Pot Routing (per 50,000 QU pot / 2 players):
//    45% → Winner wallet
//    20% → Staker yield pool    (treasury accumulation)
//    20% → QSWAP liquidity
//    10% → Permanent burn       (deflation — NULL_ID)
//     2% → PORTAL node holders
//     1% → PORTAL protocol treasury
//     1% → QSWAP protocol treasury
//     1% → QZN protocol fee reserve
//   ----
//   100% total
// ============================================================

using namespace QPI;

// ============================================================
//  CONSTANTS — SUPPLY
// ============================================================

constexpr sint64 QZN_TOTAL_SUPPLY        = 250000000LL;   // 250M hard cap — fixed forever
constexpr uint64 QZN_ASSET_NAME          = 0x4E5A51ULL;   // "QZN" as little-endian uint64

// Allocation buckets
constexpr sint64 QZN_ICO_ALLOCATION      = 50000000LL;    // 20% — QIP ICO (external)
constexpr sint64 QZN_TREASURY_ALLOC      = 87500000LL;    // 35% — Strategic Treasury
constexpr sint64 QZN_TEAM_ALLOC          = 50000000LL;    // 20% — Founders (vested)
constexpr sint64 QZN_LIQUIDITY_ALLOC     = 37500000LL;    // 15% — QSWAP Liquidity
constexpr sint64 QZN_ECOSYSTEM_ALLOC     = 25000000LL;    // 10% — Ecosystem

// ============================================================
//  CONSTANTS — VESTING
// ============================================================
//  Qubic epochs are ~1 week
//  52 epochs  = ~1 year  (cliff)
//  208 epochs = ~4 years (linear vest after cliff)
//  260 epochs = ~5 years (total from vesting start)

constexpr sint64 QZN_VEST_CLIFF_EPOCHS   = 52LL;          // 1-year cliff
constexpr sint64 QZN_VEST_TOTAL_EPOCHS   = 260LL;         // 5 years total

// ============================================================
//  CONSTANTS — MATCH BPS ROUTING
// ============================================================
//  Basis points out of 10,000.
//  Called on every match settlement from GameCabinet PAO.

constexpr sint64 QZN_ROUTE_PRIZE_BPS         = 4500LL;   // 45% → Winner
constexpr sint64 QZN_ROUTE_TREASURY_BPS      = 2000LL;   // 20% → Staker yield pool
constexpr sint64 QZN_ROUTE_LIQUIDITY_BPS     = 2000LL;   // 20% → QSWAP liquidity
constexpr sint64 QZN_ROUTE_BURN_BPS          = 1000LL;   // 10% → Permanent burn
constexpr sint64 QZN_ROUTE_NODE_BPS          = 200LL;    //  2% → PORTAL node holders
constexpr sint64 QZN_ROUTE_PORTAL_PROTO_BPS  = 100LL;   //  1% → PORTAL protocol treasury
constexpr sint64 QZN_ROUTE_QSWAP_PROTO_BPS   = 100LL;   //  1% → QSWAP protocol treasury
constexpr sint64 QZN_ROUTE_PROTOCOL_BPS      = 100LL;    //  1% → QZN protocol fee
constexpr sint64 QZN_BPS_DENOMINATOR         = 10000LL;  // Basis points base

// ============================================================
//  CONSTANTS — CROSS-CONTRACT INDICES
// ============================================================
//  Filled in after IPO — deployment order must be:
//  Token_v2 index < GameCabinet index  (lower = callable by higher)

constexpr uint32 QZN_CABINET_CONTRACT_INDEX  = 0;  // TODO: set to GameCabinet index after IPO

// ============================================================
//  CONTRACT STATE
// ============================================================

struct QZN
{
    // ---- Supply Tracking ----
    sint64 totalSupply;               // Fixed: 250,000,000 QZN
    sint64 totalBurned;               // Lifetime burn (match + governance)
    sint64 circulatingSupply;         // Live: totalSupply - totalBurned - locked

    // ---- Allocation Vaults ----
    sint64 treasuryBalance;           // QZN in protocol treasury
    sint64 liquidityBalance;          // QZN allocated to QSWAP LP
    sint64 ecosystemBalance;          // Remaining ecosystem allocation

    // ---- Team Vesting ----
    id     founderAddress;            // Founder wallet — only claimer
    sint64 teamAllocationTotal;       // Total team QZN (locked)
    sint64 teamTokensClaimed;         // Already vested + claimed
    sint64 vestingStartEpoch;         // Epoch vesting clock starts (set on initialize)
    bit    vestingInitialized;        // Vesting clock armed

    // ---- Routing Destination Addresses ----
    id     treasuryAddress;           // Protocol treasury wallet
    id     liquidityAddress;          // QSWAP LP address
    id     portalNodeAddress;         // PORTAL node holder pool address (2%)
    id     portalProtoAddress;        // PORTAL protocol treasury address (1%)
    id     qswapProtoAddress;         // QSWAP protocol treasury address (1%)

    // ---- Protocol Fee Reserve ----
    sint64 protocolFeeBalance;        // QU protocol fees accumulated (1% per match)

    // ---- Lifetime Match Stats ----
    sint64 totalMatchStake;           // Lifetime QU staked in all matches
    sint64 totalPrizeDistributed;     // Lifetime prizes awarded (QZN)
    sint64 totalRouteBurned;          // Lifetime QZN burned via match routing
    sint64 totalRouteLiquidity;       // Lifetime QZN routed to liquidity
    sint64 totalRouteTreasury;        // Lifetime QZN routed to treasury
    sint64 totalRoutePortalNode;      // Lifetime sent to PORTAL node holders
    sint64 totalRoutePortalProto;     // Lifetime sent to PORTAL protocol
    sint64 totalRouteQswapProto;      // Lifetime sent to QSWAP protocol

    // ---- Admin ----
    id     adminAddress;              // Deployer / admin (for governance burns)
    bit    initialized;               // One-time init guard
};

// ============================================================
//  INPUT / OUTPUT STRUCTS
// ============================================================

// --- Initialize Contract ---
struct InitializeQZN_input
{
    id     treasuryAddr;          // Protocol treasury wallet
    id     founderAddr;           // Founder vesting wallet
    id     liquidityAddr;         // QSWAP LP address
    id     ecosystemAddr;         // Ecosystem fund address
    id     portalNodeAddr;        // PORTAL node holder pool (2% per match)
    id     portalProtoAddr;       // PORTAL protocol treasury (1% per match)
    id     qswapProtoAddr;        // QSWAP protocol treasury (1% per match)
    sint64 vestingStartEpochOverride; // Set to 0 to use current epoch
};
struct InitializeQZN_output
{
    sint64 totalSupply;
    sint64 treasuryLocked;
    sint64 teamLocked;
    sint64 liquidityLocked;
    sint64 ecosystemLocked;
    sint64 vestingStartEpoch;
};

// --- Match Settlement ---
// Called by GameCabinet PAO cross-contract after onchain match validation.
// Caller must be QZN_CABINET_CONTRACT_INDEX (enforced in procedure).
struct SettleMatch_input
{
    id     winnerAddress;         // Match winner — receives 45%
    sint64 totalStake;            // Total QU in match pot (both players combined)
};
struct SettleMatch_output
{
    sint64 prizeAwarded;          // QZN sent to winner
    sint64 treasuryShare;         // QZN added to treasury pool
    sint64 liquidityShare;        // QZN added to liquidity pool
    sint64 burnedAmount;          // QZN permanently burned
    sint64 portalNodeShare;       // QZN sent to PORTAL node holders
    sint64 portalProtoShare;      // QZN sent to PORTAL protocol treasury
    sint64 qswapProtoShare;       // QZN sent to QSWAP protocol treasury
    sint64 protocolFee;           // QU added to QZN protocol reserve
};

// --- Claim Vested Team Tokens ---
// Callable only by founderAddress.
struct ClaimVestedTokens_input {};
struct ClaimVestedTokens_output
{
    sint64 tokensClaimed;
    sint64 tokensRemaining;
    sint64 epochsUntilCliff;      // 0 if cliff already passed
    sint64 epochsUntilFullVest;
};

// --- Admin Governance Burn ---
// Called by adminAddress (future: PAO vote).
struct BurnFromTreasury_input
{
    sint64 amount;
};
struct BurnFromTreasury_output
{
    sint64 amountBurned;
    sint64 newCirculatingSupply;
    sint64 lifetimeBurned;
};

// --- Query: Full Supply Snapshot ---
struct GetSupplyInfo_input {};
struct GetSupplyInfo_output
{
    sint64 totalSupply;
    sint64 totalBurned;
    sint64 circulatingSupply;
    sint64 treasuryBalance;
    sint64 liquidityBalance;
    sint64 ecosystemBalance;
    sint64 teamLocked;            // teamAllocationTotal - teamTokensClaimed
    sint64 protocolFeeBalance;
};

// --- Query: Vesting Status ---
struct GetVestingStatus_input {};
struct GetVestingStatus_output
{
    sint64 totalAllocation;
    sint64 totalClaimed;
    sint64 claimableNow;
    sint64 vestingStartEpoch;
    sint64 epochsElapsed;
    sint64 epochsUntilCliff;
    sint64 epochsUntilFullVest;
    bit    cliffPassed;
};

// --- Query: Match Routing Stats ---
struct GetMatchStats_input {};
struct GetMatchStats_output
{
    sint64 totalMatchStake;
    sint64 totalPrizeDistributed;
    sint64 totalRouteBurned;
    sint64 totalRouteLiquidity;
    sint64 totalRouteTreasury;
    sint64 totalRoutePortalNode;
    sint64 totalRoutePortalProto;
    sint64 totalRouteQswapProto;
};

// ============================================================
//  CONTRACT PROCEDURES
// ============================================================

PUBLIC_PROCEDURE(InitializeQZN)
/*
 * One-time setup. Called by admin immediately after deployment.
 *
 * Responsibilities:
 *  - Store all routing destination addresses
 *  - Lock allocation buckets into tracked vaults
 *  - Arm the team vesting clock
 *  - Set admin address to invoker
 *
 * NOTE: This contract does NOT handle ICO token distribution.
 *       ICO tokens (50M QZN) are issued on QX and sold via QIP.
 *       This contract tracks post-ICO token mechanics only.
 */
{
    if (state.initialized)
    {
        return;
    }

    // ---- STORE ADDRESSES ----
    state.treasuryAddress       = input.treasuryAddr;
    state.founderAddress        = input.founderAddr;
    state.liquidityAddress      = input.liquidityAddr;
    state.portalNodeAddress     = input.portalNodeAddr;
    state.portalProtoAddress    = input.portalProtoAddr;
    state.qswapProtoAddress     = input.qswapProtoAddr;
    state.adminAddress          = qpi.invocator();

    // ---- LOCK ALLOCATION VAULTS ----
    state.totalSupply           = QZN_TOTAL_SUPPLY;
    state.treasuryBalance       = QZN_TREASURY_ALLOC;     // 87.5M
    state.teamAllocationTotal   = QZN_TEAM_ALLOC;         // 50M — vested
    state.teamTokensClaimed     = 0;
    state.liquidityBalance      = QZN_LIQUIDITY_ALLOC;    // 37.5M
    state.ecosystemBalance      = QZN_ECOSYSTEM_ALLOC;    // 25M
    // ICO allocation (50M) lives in issuer wallet on QX — not tracked here

    // ---- CIRCULATING SUPPLY ----
    // At init: only liquidity is live. All else locked or on QX.
    state.circulatingSupply     = QZN_LIQUIDITY_ALLOC;

    // ---- ARM VESTING CLOCK ----
    if (input.vestingStartEpochOverride > 0)
    {
        state.vestingStartEpoch = input.vestingStartEpochOverride;
    }
    else
    {
        state.vestingStartEpoch = qpi.epoch();
    }
    state.vestingInitialized    = 1;

    // ---- RESET STATS ----
    state.totalBurned           = 0;
    state.protocolFeeBalance    = 0;
    state.totalMatchStake       = 0;
    state.totalPrizeDistributed = 0;
    state.totalRouteBurned      = 0;
    state.totalRouteLiquidity   = 0;
    state.totalRouteTreasury    = 0;
    state.totalRoutePortalNode  = 0;
    state.totalRoutePortalProto = 0;
    state.totalRouteQswapProto  = 0;

    // ---- MARK INITIALIZED ----
    state.initialized           = 1;

    output.totalSupply          = state.totalSupply;
    output.treasuryLocked       = state.treasuryBalance;
    output.teamLocked           = state.teamAllocationTotal;
    output.liquidityLocked      = state.liquidityBalance;
    output.ecosystemLocked      = state.ecosystemBalance;
    output.vestingStartEpoch    = state.vestingStartEpoch;
}
_

PUBLIC_PROCEDURE(SettleMatch)
/*
 * Deterministic match pot routing — called by GameCabinet PAO
 * via cross-contract call after onchain match hash validation.
 *
 * Auth: caller MUST be the GameCabinet PAO contract.
 *       Use qpi.invocator() to check contract identity.
 *       In Phase 1 (before cross-contract wiring): admin address allowed.
 *
 * Routing per 50,000 QU pot (2 players @ 25,000 QU each):
 *   45% → Winner               (22,500 QU)
 *   20% → Staker yield pool    (10,000 QU — treasury accumulation)
 *   20% → QSWAP liquidity      (10,000 QU)
 *   10% → Permanent burn       ( 5,000 QU — NULL_ID)
 *    2% → PORTAL node holders  ( 1,000 QU)
 *    1% → PORTAL protocol      (   500 QU)
 *    1% → QSWAP protocol       (   500 QU)
 *    1% → QZN protocol fee     (   500 QU)
 *
 * All routing is fixed and immutable.
 * Every match compresses circulating supply by 10%.
 */
{
    // ---- AUTH: Only GameCabinet PAO may call this ----
    // Phase 1: allow admin for testing
    // Phase 2: replace with contract index check below
    if (qpi.invocator() != state.adminAddress)
    {
        // TODO Phase 2: uncomment and remove admin check above
        // if (qpi.invocator() != id(QZN_CABINET_CONTRACT_INDEX))
        // {
        //     return;
        // }
        return;
    }

    if (input.totalStake <= 0)
    {
        return;
    }

    // ---- DETERMINISTIC BPS ROUTING ----
    sint64 prizeShare;
    sint64 treasuryShare;
    sint64 liquidityShare;
    sint64 burnShare;
    sint64 nodeShare;
    sint64 portalProtoShare;
    sint64 qswapProtoShare;
    sint64 protocolShare;

    prizeShare       = div(input.totalStake * QZN_ROUTE_PRIZE_BPS,        QZN_BPS_DENOMINATOR);
    treasuryShare    = div(input.totalStake * QZN_ROUTE_TREASURY_BPS,     QZN_BPS_DENOMINATOR);
    liquidityShare   = div(input.totalStake * QZN_ROUTE_LIQUIDITY_BPS,    QZN_BPS_DENOMINATOR);
    burnShare        = div(input.totalStake * QZN_ROUTE_BURN_BPS,         QZN_BPS_DENOMINATOR);
    nodeShare        = div(input.totalStake * QZN_ROUTE_NODE_BPS,         QZN_BPS_DENOMINATOR);
    portalProtoShare = div(input.totalStake * QZN_ROUTE_PORTAL_PROTO_BPS, QZN_BPS_DENOMINATOR);
    qswapProtoShare  = div(input.totalStake * QZN_ROUTE_QSWAP_PROTO_BPS,  QZN_BPS_DENOMINATOR);
    protocolShare    = div(input.totalStake * QZN_ROUTE_PROTOCOL_BPS,     QZN_BPS_DENOMINATOR);

    // 1. Prize → Winner wallet
    qpi.transferShareOwnershipAndPossession(
        QZN_ASSET_NAME,
        SELF,
        SELF,
        SELF,
        prizeShare,
        input.winnerAddress
    );

    // 2. Staker yield pool → treasury accumulation
    state.treasuryBalance     = state.treasuryBalance + treasuryShare;
    state.totalRouteTreasury  = state.totalRouteTreasury + treasuryShare;

    // 3. QSWAP liquidity reinforcement
    state.liquidityBalance    = state.liquidityBalance + liquidityShare;
    state.totalRouteLiquidity = state.totalRouteLiquidity + liquidityShare;

    // 4. Permanent burn → NULL_ID (supply permanently destroyed)
    qpi.transferShareOwnershipAndPossession(
        QZN_ASSET_NAME,
        SELF,
        SELF,
        SELF,
        burnShare,
        NULL_ID
    );
    state.totalBurned         = state.totalBurned + burnShare;
    state.circulatingSupply   = state.circulatingSupply - burnShare;
    state.totalRouteBurned    = state.totalRouteBurned + burnShare;

    // 5. PORTAL node holders (2%)
    qpi.transferShareOwnershipAndPossession(
        QZN_ASSET_NAME,
        SELF,
        SELF,
        SELF,
        nodeShare,
        state.portalNodeAddress
    );
    state.totalRoutePortalNode = state.totalRoutePortalNode + nodeShare;

    // 6. PORTAL protocol treasury (1%)
    qpi.transferShareOwnershipAndPossession(
        QZN_ASSET_NAME,
        SELF,
        SELF,
        SELF,
        portalProtoShare,
        state.portalProtoAddress
    );
    state.totalRoutePortalProto = state.totalRoutePortalProto + portalProtoShare;

    // 7. QSWAP protocol treasury (1%)
    qpi.transferShareOwnershipAndPossession(
        QZN_ASSET_NAME,
        SELF,
        SELF,
        SELF,
        qswapProtoShare,
        state.qswapProtoAddress
    );
    state.totalRouteQswapProto = state.totalRouteQswapProto + qswapProtoShare;

    // 8. QZN protocol fee reserve (1% — held in contract, not transferred)
    state.protocolFeeBalance   = state.protocolFeeBalance + protocolShare;

    // ---- UPDATE LIFETIME STATS ----
    state.totalMatchStake       = state.totalMatchStake + input.totalStake;
    state.totalPrizeDistributed = state.totalPrizeDistributed + prizeShare;

    output.prizeAwarded      = prizeShare;
    output.treasuryShare     = treasuryShare;
    output.liquidityShare    = liquidityShare;
    output.burnedAmount      = burnShare;
    output.portalNodeShare   = nodeShare;
    output.portalProtoShare  = portalProtoShare;
    output.qswapProtoShare   = qswapProtoShare;
    output.protocolFee       = protocolShare;
}
_

PUBLIC_PROCEDURE(ClaimVestedTokens)
/*
 * Founder claims linearly vested team allocation.
 *
 * Vesting schedule:
 *  - Clock starts at vestingStartEpoch (set on InitializeQZN)
 *  - Nothing claimable for first 52 epochs (1-year cliff)
 *  - After cliff: linear release over 208 epochs (4 years)
 *  - Fully vested at epoch 260 from start
 *
 * Contract-enforced — no admin override possible.
 * Uses transferShareOwnershipAndPossession for all token moves.
 */
{
    if (qpi.invocator() != state.founderAddress)
    {
        return;
    }
    if (!state.vestingInitialized)
    {
        return;
    }

    sint64 currentEpoch;
    sint64 epochsElapsed;
    sint64 vestEpochsElapsed;
    sint64 vestDuration;
    sint64 totalVestable;
    sint64 claimable;

    currentEpoch  = qpi.epoch();
    epochsElapsed = currentEpoch - state.vestingStartEpoch;

    // Enforce cliff — nothing before 52 epochs
    if (epochsElapsed < QZN_VEST_CLIFF_EPOCHS)
    {
        output.tokensClaimed       = 0;
        output.tokensRemaining     = state.teamAllocationTotal - state.teamTokensClaimed;
        output.epochsUntilCliff    = QZN_VEST_CLIFF_EPOCHS - epochsElapsed;
        output.epochsUntilFullVest = QZN_VEST_TOTAL_EPOCHS - epochsElapsed;
        return;
    }

    // Linear vest after cliff
    vestEpochsElapsed = epochsElapsed - QZN_VEST_CLIFF_EPOCHS;
    vestDuration      = QZN_VEST_TOTAL_EPOCHS - QZN_VEST_CLIFF_EPOCHS;

    // Cap at full vest duration
    if (vestEpochsElapsed > vestDuration)
    {
        vestEpochsElapsed = vestDuration;
    }

    // Tokens vested = allocation * elapsed / duration
    totalVestable = div(state.teamAllocationTotal * vestEpochsElapsed, vestDuration);
    claimable     = totalVestable - state.teamTokensClaimed;

    if (claimable <= 0)
    {
        output.tokensClaimed       = 0;
        output.tokensRemaining     = state.teamAllocationTotal - state.teamTokensClaimed;
        output.epochsUntilCliff    = 0;
        output.epochsUntilFullVest = QZN_VEST_TOTAL_EPOCHS - epochsElapsed;
        return;
    }

    // ---- EXECUTE VEST CLAIM ----
    state.teamTokensClaimed  = state.teamTokensClaimed + claimable;
    state.circulatingSupply  = state.circulatingSupply + claimable;

    qpi.transferShareOwnershipAndPossession(
        QZN_ASSET_NAME,
        SELF,
        SELF,
        SELF,
        claimable,
        state.founderAddress
    );

    sint64 epochsLeft;
    epochsLeft = QZN_VEST_TOTAL_EPOCHS - epochsElapsed;
    if (epochsLeft < 0) { epochsLeft = 0; }

    output.tokensClaimed       = claimable;
    output.tokensRemaining     = state.teamAllocationTotal - state.teamTokensClaimed;
    output.epochsUntilCliff    = 0;
    output.epochsUntilFullVest = epochsLeft;
}
_

PUBLIC_PROCEDURE(BurnFromTreasury)
/*
 * Admin (future: PAO governance vote) burns QZN from treasury.
 * Used for: scheduled deflationary events, governance decisions.
 *
 * Burns via NULL_ID — supply permanently destroyed.
 * No re-mint possible — fixed supply is immutable.
 */
{
    if (qpi.invocator() != state.adminAddress)
    {
        return;
    }
    if (input.amount <= 0 || input.amount > state.treasuryBalance)
    {
        return;
    }

    state.treasuryBalance    = state.treasuryBalance - input.amount;
    state.totalBurned        = state.totalBurned + input.amount;
    state.circulatingSupply  = state.circulatingSupply - input.amount;

    qpi.transferShareOwnershipAndPossession(
        QZN_ASSET_NAME,
        SELF,
        SELF,
        SELF,
        input.amount,
        NULL_ID
    );

    output.amountBurned         = input.amount;
    output.newCirculatingSupply = state.circulatingSupply;
    output.lifetimeBurned       = state.totalBurned;
}
_

// ============================================================
//  READ-ONLY QUERY FUNCTIONS
// ============================================================

PUBLIC_FUNCTION(GetSupplyInfo)
/*
 * Full supply snapshot.
 * Safe to call at any time — read-only, does not alter state.
 */
{
    output.totalSupply       = state.totalSupply;
    output.totalBurned       = state.totalBurned;
    output.circulatingSupply = state.circulatingSupply;
    output.treasuryBalance   = state.treasuryBalance;
    output.liquidityBalance  = state.liquidityBalance;
    output.ecosystemBalance  = state.ecosystemBalance;
    output.teamLocked        = state.teamAllocationTotal - state.teamTokensClaimed;
    output.protocolFeeBalance = state.protocolFeeBalance;
}
_

PUBLIC_FUNCTION(GetVestingStatus)
/*
 * Returns current vesting status for the founder allocation.
 * Frontend should call this to show cliff + linear vest progress.
 */
{
    sint64 epochsElapsed;
    sint64 vestEpochsElapsed;
    sint64 vestDuration;
    sint64 totalVestable;
    sint64 claimableNow;

    epochsElapsed = qpi.epoch() - state.vestingStartEpoch;

    output.totalAllocation   = state.teamAllocationTotal;
    output.totalClaimed      = state.teamTokensClaimed;
    output.vestingStartEpoch = state.vestingStartEpoch;
    output.epochsElapsed     = epochsElapsed;

    if (epochsElapsed < QZN_VEST_CLIFF_EPOCHS)
    {
        output.cliffPassed         = 0;
        output.epochsUntilCliff    = QZN_VEST_CLIFF_EPOCHS - epochsElapsed;
        output.epochsUntilFullVest = QZN_VEST_TOTAL_EPOCHS - epochsElapsed;
        output.claimableNow        = 0;
        return;
    }

    output.cliffPassed      = 1;
    output.epochsUntilCliff = 0;

    vestEpochsElapsed = epochsElapsed - QZN_VEST_CLIFF_EPOCHS;
    vestDuration      = QZN_VEST_TOTAL_EPOCHS - QZN_VEST_CLIFF_EPOCHS;

    if (vestEpochsElapsed > vestDuration) { vestEpochsElapsed = vestDuration; }

    totalVestable = div(state.teamAllocationTotal * vestEpochsElapsed, vestDuration);
    claimableNow  = totalVestable - state.teamTokensClaimed;
    if (claimableNow < 0) { claimableNow = 0; }

    sint64 epochsLeft;
    epochsLeft = QZN_VEST_TOTAL_EPOCHS - epochsElapsed;
    if (epochsLeft < 0) { epochsLeft = 0; }

    output.claimableNow        = claimableNow;
    output.epochsUntilFullVest = epochsLeft;
}
_

PUBLIC_FUNCTION(GetMatchStats)
/*
 * Lifetime match routing statistics.
 * Useful for protocol dashboards and frontend analytics.
 */
{
    output.totalMatchStake      = state.totalMatchStake;
    output.totalPrizeDistributed = state.totalPrizeDistributed;
    output.totalRouteBurned     = state.totalRouteBurned;
    output.totalRouteLiquidity  = state.totalRouteLiquidity;
    output.totalRouteTreasury   = state.totalRouteTreasury;
    output.totalRoutePortalNode = state.totalRoutePortalNode;
    output.totalRoutePortalProto = state.totalRoutePortalProto;
    output.totalRouteQswapProto = state.totalRouteQswapProto;
}
_

// ============================================================
//  REGISTRATION
// ============================================================

REGISTER_USER_FUNCTIONS_AND_PROCEDURES
{
    REGISTER_USER_PROCEDURE(InitializeQZN,        1);
    REGISTER_USER_PROCEDURE(SettleMatch,          2);
    REGISTER_USER_PROCEDURE(ClaimVestedTokens,    3);
    REGISTER_USER_PROCEDURE(BurnFromTreasury,     4);
    REGISTER_USER_FUNCTION(GetSupplyInfo,         5);
    REGISTER_USER_FUNCTION(GetVestingStatus,      6);
    REGISTER_USER_FUNCTION(GetMatchStats,         7);
}
_

// ============================================================
//  SYSTEM HOOKS
// ============================================================

BEGIN_EPOCH
/*
 * Fires at the start of every Qubic epoch (~weekly).
 * Reserved for future: staker yield distribution heartbeat,
 * liquidity rebalancing signals.
 */
{
}
_

END_TICK
/*
 * Reserved for Phase 2: oracle sync, reward router heartbeat.
 */
{
}
_