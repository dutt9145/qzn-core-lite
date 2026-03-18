// ============================================================
//  QZN TOKEN — Arcade Coordination Protocol on Qubic
//  Contract: QZN Core Token — Post-ICO Mechanics Vault
//  Network:  Qubic (QPI / C++ Smart Contract)
//  Version:  3.0.0
// ============================================================

// ============================================================
//  CONSTANTS
// ============================================================

constexpr sint64 QZN_TOTAL_SUPPLY        = 250000000LL;
constexpr uint64 QZN_ASSET_NAME          = 0x4E5A51ULL;

constexpr sint64 QZN_ICO_ALLOCATION      = 50000000LL;
constexpr sint64 QZN_TREASURY_ALLOC      = 87500000LL;
constexpr sint64 QZN_TEAM_ALLOC          = 50000000LL;
constexpr sint64 QZN_LIQUIDITY_ALLOC     = 37500000LL;
constexpr sint64 QZN_ECOSYSTEM_ALLOC     = 25000000LL;

constexpr sint64 QZN_VEST_CLIFF_EPOCHS   = 52LL;
constexpr sint64 QZN_VEST_TOTAL_EPOCHS   = 260LL;

constexpr sint64 QZN_ROUTE_PRIZE_BPS         = 4500LL;
constexpr sint64 QZN_ROUTE_TREASURY_BPS      = 2000LL;
constexpr sint64 QZN_ROUTE_LIQUIDITY_BPS     = 2000LL;
constexpr sint64 QZN_ROUTE_BURN_BPS          = 1000LL;
constexpr sint64 QZN_ROUTE_NODE_BPS          = 200LL;
constexpr sint64 QZN_ROUTE_PORTAL_PROTO_BPS  = 100LL;
constexpr sint64 QZN_ROUTE_QSWAP_PROTO_BPS   = 100LL;
constexpr sint64 QZN_ROUTE_PROTOCOL_BPS      = 100LL;
constexpr sint64 QZN_BPS_DENOMINATOR         = 10000LL;

constexpr uint32 QZN_CABINET_CONTRACT_INDEX  = 0;

// ============================================================
//  STATE2
// ============================================================

struct QZN2
{
};

// ============================================================
//  CONTRACT
// ============================================================

struct QZN : public ContractBase
{
    struct StateData
    {
        sint64 totalSupply;
        sint64 totalBurned;
        sint64 circulatingSupply;

        sint64 treasuryBalance;
        sint64 liquidityBalance;
        sint64 ecosystemBalance;

        id     founderAddress;
        sint64 teamAllocationTotal;
        sint64 teamTokensClaimed;
        sint64 vestingStartEpoch;
        bit    vestingInitialized;

        id     treasuryAddress;
        id     liquidityAddress;
        id     portalNodeAddress;
        id     portalProtoAddress;
        id     qswapProtoAddress;

        sint64 protocolFeeBalance;

        sint64 totalMatchStake;
        sint64 totalPrizeDistributed;
        sint64 totalRouteBurned;
        sint64 totalRouteLiquidity;
        sint64 totalRouteTreasury;
        sint64 totalRoutePortalNode;
        sint64 totalRoutePortalProto;
        sint64 totalRouteQswapProto;

        id     adminAddress;
        bit    initialized;
    };

public:

    struct InitializeQZN_input
    {
        id     treasuryAddr;
        id     founderAddr;
        id     liquidityAddr;
        id     ecosystemAddr;
        id     portalNodeAddr;
        id     portalProtoAddr;
        id     qswapProtoAddr;
        sint64 vestingStartEpochOverride;
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

    struct SettleMatch_input
    {
        id     winnerAddress;
        sint64 totalStake;
    };
    struct SettleMatch_output
    {
        sint64 prizeAwarded;
        sint64 treasuryShare;
        sint64 liquidityShare;
        sint64 burnedAmount;
        sint64 portalNodeShare;
        sint64 portalProtoShare;
        sint64 qswapProtoShare;
        sint64 protocolFee;
    };

    struct ClaimVestedTokens_input {};
    struct ClaimVestedTokens_output
    {
        sint64 tokensClaimed;
        sint64 tokensRemaining;
        sint64 epochsUntilCliff;
        sint64 epochsUntilFullVest;
    };

    struct BurnFromTreasury_input { sint64 amount; };
    struct BurnFromTreasury_output
    {
        sint64 amountBurned;
        sint64 newCirculatingSupply;
        sint64 lifetimeBurned;
    };

    struct GetSupplyInfo_input {};
    struct GetSupplyInfo_output
    {
        sint64 totalSupply;
        sint64 totalBurned;
        sint64 circulatingSupply;
        sint64 treasuryBalance;
        sint64 liquidityBalance;
        sint64 ecosystemBalance;
        sint64 teamLocked;
        sint64 protocolFeeBalance;
    };

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

    PUBLIC_PROCEDURE(InitializeQZN)
    {
        if (state.get().initialized) { return; }

        state.mut().treasuryAddress = input.treasuryAddr;
        state.mut().founderAddress = input.founderAddr;
        state.mut().liquidityAddress = input.liquidityAddr;
        state.mut().portalNodeAddress = input.portalNodeAddr;
        state.mut().portalProtoAddress = input.portalProtoAddr;
        state.mut().qswapProtoAddress = input.qswapProtoAddr;
        state.mut().adminAddress = qpi.invocator();

        state.mut().totalSupply = QZN_TOTAL_SUPPLY;
        state.mut().treasuryBalance = QZN_TREASURY_ALLOC;
        state.mut().teamAllocationTotal = QZN_TEAM_ALLOC;
        state.mut().teamTokensClaimed = 0;
        state.mut().liquidityBalance = QZN_LIQUIDITY_ALLOC;
        state.mut().ecosystemBalance = QZN_ECOSYSTEM_ALLOC;
        state.mut().circulatingSupply = QZN_LIQUIDITY_ALLOC;

        if (input.vestingStartEpochOverride > 0)
            state.mut().vestingStartEpoch = input.vestingStartEpochOverride;
        else
            state.mut().vestingStartEpoch = qpi.epoch();

        state.mut().vestingInitialized = 1;
        state.mut().totalBurned = 0;
        state.mut().protocolFeeBalance = 0;
        state.mut().totalMatchStake = 0;
        state.mut().totalPrizeDistributed = 0;
        state.mut().totalRouteBurned = 0;
        state.mut().totalRouteLiquidity = 0;
        state.mut().totalRouteTreasury = 0;
        state.mut().totalRoutePortalNode = 0;
        state.mut().totalRoutePortalProto = 0;
        state.mut().totalRouteQswapProto = 0;
        state.mut().initialized = 1;

        output.totalSupply       = state.get().totalSupply;
        output.treasuryLocked    = state.get().treasuryBalance;
        output.teamLocked        = state.get().teamAllocationTotal;
        output.liquidityLocked   = state.get().liquidityBalance;
        output.ecosystemLocked   = state.get().ecosystemBalance;
        output.vestingStartEpoch = state.get().vestingStartEpoch;
    }

    PUBLIC_PROCEDURE(SettleMatch)
    {
        if (qpi.invocator() != state.get().adminAddress) { return; }
        if (input.totalStake <= 0) { return; }

        sint64 prizeShare       = div(input.totalStake * QZN_ROUTE_PRIZE_BPS,        QZN_BPS_DENOMINATOR).quot;
        sint64 treasuryShare    = div(input.totalStake * QZN_ROUTE_TREASURY_BPS,     QZN_BPS_DENOMINATOR).quot;
        sint64 liquidityShare   = div(input.totalStake * QZN_ROUTE_LIQUIDITY_BPS,    QZN_BPS_DENOMINATOR).quot;
        sint64 burnShare        = div(input.totalStake * QZN_ROUTE_BURN_BPS,         QZN_BPS_DENOMINATOR).quot;
        sint64 nodeShare        = div(input.totalStake * QZN_ROUTE_NODE_BPS,         QZN_BPS_DENOMINATOR).quot;
        sint64 portalProtoShare = div(input.totalStake * QZN_ROUTE_PORTAL_PROTO_BPS, QZN_BPS_DENOMINATOR).quot;
        sint64 qswapProtoShare  = div(input.totalStake * QZN_ROUTE_QSWAP_PROTO_BPS,  QZN_BPS_DENOMINATOR).quot;
        sint64 protocolShare    = div(input.totalStake * QZN_ROUTE_PROTOCOL_BPS,     QZN_BPS_DENOMINATOR).quot;

        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, prizeShare, input.winnerAddress);

        state.mut().treasuryBalance = state.get().treasuryBalance + treasuryShare;
        state.mut().totalRouteTreasury = state.get().totalRouteTreasury + treasuryShare;
        state.mut().liquidityBalance = state.get().liquidityBalance + liquidityShare;
        state.mut().totalRouteLiquidity = state.get().totalRouteLiquidity + liquidityShare;

        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, burnShare, NULL_ID);
        state.mut().totalBurned = state.get().totalBurned + burnShare;
        state.mut().circulatingSupply = state.get().circulatingSupply - burnShare;
        state.mut().totalRouteBurned = state.get().totalRouteBurned + burnShare;

        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, nodeShare, state.get().portalNodeAddress);
        state.mut().totalRoutePortalNode = state.get().totalRoutePortalNode + nodeShare;

        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, portalProtoShare, state.get().portalProtoAddress);
        state.mut().totalRoutePortalProto = state.get().totalRoutePortalProto + portalProtoShare;

        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, qswapProtoShare, state.get().qswapProtoAddress);
        state.mut().totalRouteQswapProto = state.get().totalRouteQswapProto + qswapProtoShare;

        state.mut().protocolFeeBalance = state.get().protocolFeeBalance + protocolShare;
        state.mut().totalMatchStake = state.get().totalMatchStake + input.totalStake;
        state.mut().totalPrizeDistributed = state.get().totalPrizeDistributed + prizeShare;

        output.prizeAwarded     = prizeShare;
        output.treasuryShare    = treasuryShare;
        output.liquidityShare   = liquidityShare;
        output.burnedAmount     = burnShare;
        output.portalNodeShare  = nodeShare;
        output.portalProtoShare = portalProtoShare;
        output.qswapProtoShare  = qswapProtoShare;
        output.protocolFee      = protocolShare;
    }

    PUBLIC_PROCEDURE(ClaimVestedTokens)
    {
        if (qpi.invocator() != state.get().founderAddress) { return; }
        if (!state.get().vestingInitialized) { return; }

        sint64 currentEpoch      = qpi.epoch();
        sint64 epochsElapsed     = currentEpoch - state.get().vestingStartEpoch;

        if (epochsElapsed < QZN_VEST_CLIFF_EPOCHS)
        {
            output.tokensClaimed       = 0;
            output.tokensRemaining     = state.get().teamAllocationTotal - state.get().teamTokensClaimed;
            output.epochsUntilCliff    = QZN_VEST_CLIFF_EPOCHS - epochsElapsed;
            output.epochsUntilFullVest = QZN_VEST_TOTAL_EPOCHS - epochsElapsed;
            return;
        }

        sint64 vestEpochsElapsed = epochsElapsed - QZN_VEST_CLIFF_EPOCHS;
        sint64 vestDuration      = QZN_VEST_TOTAL_EPOCHS - QZN_VEST_CLIFF_EPOCHS;
        if (vestEpochsElapsed > vestDuration) { vestEpochsElapsed = vestDuration; }

        sint64 totalVestable = div(state.get().teamAllocationTotal * vestEpochsElapsed, vestDuration).quot;
        sint64 claimable     = totalVestable - state.get().teamTokensClaimed;

        if (claimable <= 0)
        {
            output.tokensClaimed       = 0;
            output.tokensRemaining     = state.get().teamAllocationTotal - state.get().teamTokensClaimed;
            output.epochsUntilCliff    = 0;
            output.epochsUntilFullVest = QZN_VEST_TOTAL_EPOCHS - epochsElapsed;
            return;
        }

        state.mut().teamTokensClaimed = state.get().teamTokensClaimed + claimable;
        state.mut().circulatingSupply = state.get().circulatingSupply + claimable;
        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, claimable, state.get().founderAddress);

        sint64 epochsLeft = QZN_VEST_TOTAL_EPOCHS - epochsElapsed;
        if (epochsLeft < 0) { epochsLeft = 0; }

        output.tokensClaimed       = claimable;
        output.tokensRemaining     = state.get().teamAllocationTotal - state.get().teamTokensClaimed;
        output.epochsUntilCliff    = 0;
        output.epochsUntilFullVest = epochsLeft;
    }

    PUBLIC_PROCEDURE(BurnFromTreasury)
    {
        if (qpi.invocator() != state.get().adminAddress) { return; }
        if (input.amount <= 0 || input.amount > state.get().treasuryBalance) { return; }

        state.mut().treasuryBalance = state.get().treasuryBalance - input.amount;
        state.mut().totalBurned = state.get().totalBurned + input.amount;
        state.mut().circulatingSupply = state.get().circulatingSupply - input.amount;
        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, input.amount, NULL_ID);

        output.amountBurned         = input.amount;
        output.newCirculatingSupply = state.get().circulatingSupply;
        output.lifetimeBurned       = state.get().totalBurned;
    }

    PUBLIC_FUNCTION(GetSupplyInfo)
    {
        output.totalSupply        = state.get().totalSupply;
        output.totalBurned        = state.get().totalBurned;
        output.circulatingSupply  = state.get().circulatingSupply;
        output.treasuryBalance    = state.get().treasuryBalance;
        output.liquidityBalance   = state.get().liquidityBalance;
        output.ecosystemBalance   = state.get().ecosystemBalance;
        output.teamLocked         = state.get().teamAllocationTotal - state.get().teamTokensClaimed;
        output.protocolFeeBalance = state.get().protocolFeeBalance;
    }

    PUBLIC_FUNCTION(GetVestingStatus)
    {
        sint64 epochsElapsed = qpi.epoch() - state.get().vestingStartEpoch;
        output.totalAllocation   = state.get().teamAllocationTotal;
        output.totalClaimed      = state.get().teamTokensClaimed;
        output.vestingStartEpoch = state.get().vestingStartEpoch;
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

        sint64 vestEpochsElapsed = epochsElapsed - QZN_VEST_CLIFF_EPOCHS;
        sint64 vestDuration      = QZN_VEST_TOTAL_EPOCHS - QZN_VEST_CLIFF_EPOCHS;
        if (vestEpochsElapsed > vestDuration) { vestEpochsElapsed = vestDuration; }

        sint64 totalVestable = div(state.get().teamAllocationTotal * vestEpochsElapsed, vestDuration).quot;
        sint64 claimableNow  = totalVestable - state.get().teamTokensClaimed;
        if (claimableNow < 0) { claimableNow = 0; }

        sint64 epochsLeft = QZN_VEST_TOTAL_EPOCHS - epochsElapsed;
        if (epochsLeft < 0) { epochsLeft = 0; }

        output.claimableNow        = claimableNow;
        output.epochsUntilFullVest = epochsLeft;
    }

    PUBLIC_FUNCTION(GetMatchStats)
    {
        output.totalMatchStake        = state.get().totalMatchStake;
        output.totalPrizeDistributed  = state.get().totalPrizeDistributed;
        output.totalRouteBurned       = state.get().totalRouteBurned;
        output.totalRouteLiquidity    = state.get().totalRouteLiquidity;
        output.totalRouteTreasury     = state.get().totalRouteTreasury;
        output.totalRoutePortalNode   = state.get().totalRoutePortalNode;
        output.totalRoutePortalProto  = state.get().totalRoutePortalProto;
        output.totalRouteQswapProto   = state.get().totalRouteQswapProto;
    }

    REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
    {
        REGISTER_USER_PROCEDURE(InitializeQZN,     1);
        REGISTER_USER_PROCEDURE(SettleMatch,       2);
        REGISTER_USER_PROCEDURE(ClaimVestedTokens, 3);
        REGISTER_USER_PROCEDURE(BurnFromTreasury,  4);
        REGISTER_USER_FUNCTION(GetSupplyInfo,      5);
        REGISTER_USER_FUNCTION(GetVestingStatus,   6);
        REGISTER_USER_FUNCTION(GetMatchStats,      7);
    }

    INITIALIZE()
    {
    }

    BEGIN_EPOCH()
    {
    }

    END_TICK()
    {
    }
};