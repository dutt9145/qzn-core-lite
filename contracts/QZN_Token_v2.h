// ============================================================
//  QZN TOKEN v4.0.0 — Inverted Dividend Architecture
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
constexpr sint64 QZN_ROUTE_STAKER_BPS     = 2000LL;
constexpr sint64 QZN_ROUTE_BUILDER_BPS    = 0LL;
constexpr sint64 QZN_ROUTE_PROTOCOL_BPS   = 0LL;
constexpr sint64 QZN_ROUTE_LIQUIDITY_BPS     = 2000LL;
constexpr sint64 QZN_ROUTE_BURN_BPS          = 1000LL;
constexpr sint64 QZN_ROUTE_NODE_BPS       = 300LL;
constexpr sint64 QZN_ROUTE_TREASURY_BPS   = 100LL;
constexpr sint64 QZN_ROUTE_NOVUM_INITIUM_PROTO_BPS  = 0LL;
constexpr sint64 QZN_ROUTE_QSWAP_PROTO_BPS   = 100LL;
constexpr sint64 QZN_BPS_DENOMINATOR         = 10000LL;
constexpr uint32 QZN_CABINET_CONTRACT_INDEX  = 0;

struct QZN2 {};

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
        id     rewardRouterAddress;
        id     cabinetAddress;
        id     treasuryVaultAddress;
        sint64 protocolFeeThreshold;
        sint64 protocolFeeBalance;
        sint64 stakerDividendAccumulator;
        sint64 builderDividendAccumulator;
        sint64 totalDividendsPaidToStakers;
        sint64 totalDividendsPaidToBuilders;
        uint32 lastDividendEpoch;
        sint64 totalMatchStake;
        sint64 totalPrizeDistributed;
        sint64 totalRouteBurned;
        sint64 totalRouteLiquidity;
        sint64 totalRouteTreasury;
        sint64 totalRoutePortalNode;
        sint64 totalRoutePortalProto;
        sint64 totalRouteQswapProto;
        sint64 totalRouteStakers;
        sint64 totalRouteBuilders;
        id     adminAddress;
        bit    initialized;

    // ── SC Shareholder Dividends ─────────────────────────────────────
    sint64 epochScDividendPool;      // Fills from protocol fees each epoch
    sint64 totalScDividendsPaid;
    sint64 epochEfficiencyRating;    // Hint: inverse revenue rank multiplier
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
        id     rewardRouterAddr;
        id     treasuryVaultAddr;
        id     cabinetAddr;
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
        sint64 stakerDividendShare;
        sint64 builderDividendShare;
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
        sint64 stakerDividendAccumulator;
        sint64 builderDividendAccumulator;
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
        sint64 totalRouteStakers;
        sint64 totalRouteBuilders;
    };

    struct GetDividendStats_input {};
    struct GetDividendStats_output
    {
        sint64 stakerDividendAccumulator;
        sint64 builderDividendAccumulator;
        sint64 totalDividendsPaidToStakers;
        sint64 totalDividendsPaidToBuilders;
        uint32 lastDividendEpoch;
    };

    PUBLIC_PROCEDURE(InitializeQZN)
    {
        if (state.get().initialized) { return; }
        state.mut().treasuryAddress      = input.treasuryAddr;
        state.mut().founderAddress       = input.founderAddr;
        state.mut().liquidityAddress     = input.liquidityAddr;
        state.mut().portalNodeAddress    = input.portalNodeAddr;
        state.mut().portalProtoAddress   = input.portalProtoAddr;
        state.mut().qswapProtoAddress    = input.qswapProtoAddr;
        state.mut().rewardRouterAddress  = input.rewardRouterAddr;
        state.mut().treasuryVaultAddress = input.treasuryVaultAddr;
        state.mut().protocolFeeThreshold = 10000000LL; // 10M QZN threshold
        state.mut().cabinetAddress       = input.cabinetAddr;
        state.mut().adminAddress         = qpi.invocator();
        state.mut().totalSupply          = QZN_TOTAL_SUPPLY;
        state.mut().treasuryBalance      = QZN_TREASURY_ALLOC;
        state.mut().teamAllocationTotal  = QZN_TEAM_ALLOC;
        state.mut().teamTokensClaimed    = 0;
        state.mut().liquidityBalance     = QZN_LIQUIDITY_ALLOC;
        state.mut().ecosystemBalance     = QZN_ECOSYSTEM_ALLOC;
        state.mut().circulatingSupply    = QZN_LIQUIDITY_ALLOC;
        if (input.vestingStartEpochOverride > 0)
            state.mut().vestingStartEpoch = input.vestingStartEpochOverride;
        else
            state.mut().vestingStartEpoch = qpi.epoch();
        state.mut().vestingInitialized           = 1;
        state.mut().totalBurned                  = 0;
        state.mut().protocolFeeBalance           = 0;
        state.mut().stakerDividendAccumulator    = 0;
        state.mut().builderDividendAccumulator   = 0;
        state.mut().totalDividendsPaidToStakers  = 0;
        state.mut().totalDividendsPaidToBuilders = 0;
        state.mut().lastDividendEpoch            = 0;
        state.mut().totalMatchStake              = 0;
        state.mut().totalPrizeDistributed        = 0;
        state.mut().totalRouteBurned             = 0;
        state.mut().totalRouteLiquidity          = 0;
        state.mut().totalRouteTreasury           = 0;
        state.mut().totalRoutePortalNode         = 0;
        state.mut().totalRoutePortalProto        = 0;
        state.mut().totalRouteQswapProto         = 0;
        state.mut().totalRouteStakers            = 0;
        state.mut().totalRouteBuilders           = 0;
        state.mut().initialized                  = 1;
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
        sint64 stakerShare      = div(input.totalStake * QZN_ROUTE_STAKER_BPS,       QZN_BPS_DENOMINATOR).quot;
        sint64 builderShare     = div(input.totalStake * QZN_ROUTE_BUILDER_BPS,      QZN_BPS_DENOMINATOR).quot;
        sint64 treasuryShare    = div(input.totalStake * QZN_ROUTE_TREASURY_BPS,     QZN_BPS_DENOMINATOR).quot;
        sint64 liquidityShare   = div(input.totalStake * QZN_ROUTE_LIQUIDITY_BPS,    QZN_BPS_DENOMINATOR).quot;
        sint64 burnShare        = div(input.totalStake * QZN_ROUTE_BURN_BPS,         QZN_BPS_DENOMINATOR).quot;
        sint64 nodeShare        = div(input.totalStake * QZN_ROUTE_NODE_BPS,         QZN_BPS_DENOMINATOR).quot;
        sint64 portalProtoShare = div(input.totalStake * QZN_ROUTE_NOVUM_INITIUM_PROTO_BPS, QZN_BPS_DENOMINATOR).quot;
        sint64 qswapProtoShare  = div(input.totalStake * QZN_ROUTE_QSWAP_PROTO_BPS,  QZN_BPS_DENOMINATOR).quot;
        sint64 protocolShare    = div(input.totalStake * QZN_ROUTE_PROTOCOL_BPS,     QZN_BPS_DENOMINATOR).quot;
        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, prizeShare, input.winnerAddress);
        state.mut().treasuryBalance       += treasuryShare;
        state.mut().totalRouteTreasury    += treasuryShare;
        state.mut().liquidityBalance      += liquidityShare;
        state.mut().totalRouteLiquidity   += liquidityShare;
        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, burnShare, NULL_ID);
        state.mut().totalBurned           += burnShare;
        state.mut().circulatingSupply     -= burnShare;
        state.mut().totalRouteBurned      += burnShare;
        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, nodeShare, state.get().portalNodeAddress);
        state.mut().totalRoutePortalNode  += nodeShare;
        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, portalProtoShare, state.get().portalProtoAddress);
        state.mut().totalRoutePortalProto += portalProtoShare;
        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, qswapProtoShare, state.get().qswapProtoAddress);
        state.mut().totalRouteQswapProto  += qswapProtoShare;
        state.mut().protocolFeeBalance    += protocolShare;
        state.mut().stakerDividendAccumulator  += stakerShare;
        state.mut().builderDividendAccumulator += builderShare;
        state.mut().totalRouteStakers     += stakerShare;
        state.mut().totalRouteBuilders    += builderShare;
        state.mut().totalMatchStake       += input.totalStake;
        state.mut().totalPrizeDistributed += prizeShare;
        output.prizeAwarded         = prizeShare;
        output.treasuryShare        = treasuryShare;
        output.liquidityShare       = liquidityShare;
        output.burnedAmount         = burnShare;
        output.portalNodeShare      = nodeShare;
        output.portalProtoShare     = portalProtoShare;
        output.qswapProtoShare      = qswapProtoShare;
        output.protocolFee          = protocolShare;
        output.stakerDividendShare  = stakerShare;
        output.builderDividendShare = builderShare;
    }

    PUBLIC_PROCEDURE(ClaimVestedTokens)
    {
        if (qpi.invocator() != state.get().founderAddress) { return; }
        if (!state.get().vestingInitialized) { return; }
        sint64 currentEpoch  = qpi.epoch();
        sint64 epochsElapsed = currentEpoch - state.get().vestingStartEpoch;
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
        state.mut().teamTokensClaimed += claimable;
        state.mut().circulatingSupply += claimable;
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
        state.mut().treasuryBalance   -= input.amount;
        state.mut().totalBurned       += input.amount;
        state.mut().circulatingSupply -= input.amount;
        qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF, input.amount, NULL_ID);
        output.amountBurned         = input.amount;
        output.newCirculatingSupply = state.get().circulatingSupply;
        output.lifetimeBurned       = state.get().totalBurned;
    }

    PUBLIC_FUNCTION(GetSupplyInfo)
    {
        output.totalSupply                = state.get().totalSupply;
        output.totalBurned                = state.get().totalBurned;
        output.circulatingSupply          = state.get().circulatingSupply;
        output.treasuryBalance            = state.get().treasuryBalance;
        output.liquidityBalance           = state.get().liquidityBalance;
        output.ecosystemBalance           = state.get().ecosystemBalance;
        output.teamLocked                 = state.get().teamAllocationTotal - state.get().teamTokensClaimed;
        output.protocolFeeBalance         = state.get().protocolFeeBalance;
        output.stakerDividendAccumulator  = state.get().stakerDividendAccumulator;
        output.builderDividendAccumulator = state.get().builderDividendAccumulator;
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
        output.totalMatchStake       = state.get().totalMatchStake;
        output.totalPrizeDistributed = state.get().totalPrizeDistributed;
        output.totalRouteBurned      = state.get().totalRouteBurned;
        output.totalRouteLiquidity   = state.get().totalRouteLiquidity;
        output.totalRouteTreasury    = state.get().totalRouteTreasury;
        output.totalRoutePortalNode  = state.get().totalRoutePortalNode;
        output.totalRoutePortalProto = state.get().totalRoutePortalProto;
        output.totalRouteQswapProto  = state.get().totalRouteQswapProto;
        output.totalRouteStakers     = state.get().totalRouteStakers;
        output.totalRouteBuilders    = state.get().totalRouteBuilders;
    }

    PUBLIC_FUNCTION(GetDividendStats)
    {
        output.stakerDividendAccumulator    = state.get().stakerDividendAccumulator;
        output.builderDividendAccumulator   = state.get().builderDividendAccumulator;
        output.totalDividendsPaidToStakers  = state.get().totalDividendsPaidToStakers;
        output.totalDividendsPaidToBuilders = state.get().totalDividendsPaidToBuilders;
        output.lastDividendEpoch            = state.get().lastDividendEpoch;
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
        REGISTER_USER_PROCEDURE(InitializeQZN,     1);
        REGISTER_USER_PROCEDURE(SettleMatch,       2);
        REGISTER_USER_PROCEDURE(ClaimVestedTokens, 3);
        REGISTER_USER_PROCEDURE(BurnFromTreasury,  4);
        REGISTER_USER_FUNCTION(GetSupplyInfo,      5);
        REGISTER_USER_FUNCTION(GetVestingStatus,   6);
        REGISTER_USER_FUNCTION(GetMatchStats,      7);
        REGISTER_USER_FUNCTION(GetDividendStats,   8);
    }

    INITIALIZE() {}

    BEGIN_EPOCH()
    {
        if (state.get().stakerDividendAccumulator > 0LL &&
            state.get().rewardRouterAddress != NULL_ID)
        {
            qpi.transferShareOwnershipAndPossession(
                QZN_ASSET_NAME, SELF, SELF, SELF,
                state.get().stakerDividendAccumulator,
                state.get().rewardRouterAddress);
            state.mut().totalDividendsPaidToStakers += state.get().stakerDividendAccumulator;
            state.mut().stakerDividendAccumulator    = 0LL;
        }
        if (state.get().builderDividendAccumulator > 0LL &&
            state.get().cabinetAddress != NULL_ID)
        {
            qpi.transferShareOwnershipAndPossession(
                QZN_ASSET_NAME, SELF, SELF, SELF,
                state.get().builderDividendAccumulator,
                state.get().cabinetAddress);
            state.mut().totalDividendsPaidToBuilders += state.get().builderDividendAccumulator;
            state.mut().builderDividendAccumulator    = 0LL;
        }
        state.mut().lastDividendEpoch = qpi.epoch();

    // ── Protocol Fee Distribution ────────────────────────────────────
    // Each epoch the protocol fee pool is split:
    //   20% → SC holders (676 shareholders via distributeDividends)
    //   10% → RewardRouter reserve
    //    6% → GameCabinet reserve
    //    6% → NOVUM INITIUM reserve
    //    5% → TournamentEngine reserve
    //    3% → TreasuryVault reserve
    //   50% → stays in protocolFeeBalance (runway)
    // If protocolFeeBalance exceeds threshold, excess moves to TreasuryVault
    // for governance spending proposals.
    sint64 feePool = state.get().protocolFeeBalance;
    if (feePool > 0LL)
    {
        sint64 scShare       = div(feePool * 2000LL, 10000LL).quot;
        sint64 rrShare       = div(feePool * 1000LL, 10000LL).quot;
        sint64 cabShare      = div(feePool *  600LL, 10000LL).quot;
        sint64 portalShare   = div(feePool *  600LL, 10000LL).quot;
        sint64 tourShare     = div(feePool *  500LL, 10000LL).quot;
        sint64 vaultShare    = div(feePool *  300LL, 10000LL).quot;
        sint64 retainShare   = feePool - scShare - rrShare - cabShare
                               - portalShare - tourShare - vaultShare;

        // SC holders
        scShare = scShare * state.get().epochEfficiencyRating / 1000LL;
        if (scShare > 0LL)
        {
            qpi.distributeDividends(scShare);
            state.mut().totalScDividendsPaid += scShare;
            state.mut().epochScDividendPool   = scShare;
        }

        // Cross-contract reserve transfers
        if (rrShare > 0LL && state.get().rewardRouterAddress != NULL_ID)
        {
            // Transfer QZN then notify RewardRouter to update its reserve
            qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF,
                rrShare, state.get().rewardRouterAddress);
            // RewardRouter.ReceiveDividend credited via incoming transfer
            state.mut().totalRouteStakers += rrShare;
        }

        if (cabShare > 0LL && state.get().cabinetAddress != NULL_ID)
            qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF,
                cabShare, state.get().cabinetAddress);

        if (portalShare > 0LL && state.get().portalProtoAddress != NULL_ID)
            qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF,
                portalShare, state.get().portalProtoAddress);

        if (tourShare > 0LL)
        {
            // TournamentEngine — no direct address stored yet, use ecosystemBalance as proxy
            state.mut().ecosystemBalance += tourShare;
        }

        if (vaultShare > 0LL && state.get().treasuryVaultAddress != NULL_ID)
            qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF,
                vaultShare, state.get().treasuryVaultAddress);

        // Retain remainder in protocol fee balance
        state.mut().protocolFeeBalance = retainShare;

        // ── Governance overflow to TreasuryVault ──────────────────────
        // If retained balance exceeds threshold, move excess to governance
        if (state.get().protocolFeeBalance > state.get().protocolFeeThreshold &&
            state.get().treasuryVaultAddress != NULL_ID)
        {
            sint64 overflow = state.get().protocolFeeBalance
                              - state.get().protocolFeeThreshold;
            qpi.transferShareOwnershipAndPossession(QZN_ASSET_NAME, SELF, SELF, SELF,
                overflow, state.get().treasuryVaultAddress);
            state.mut().protocolFeeBalance -= overflow;
        }
        // ─────────────────────────────────────────────────────────────
    }
    state.mut().epochEfficiencyRating = 1000LL;
}

    END_TICK() {}
};
