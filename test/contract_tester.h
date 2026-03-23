#ifndef CONTRACT_TESTER_H
#define CONTRACT_TESTER_H

using InitializeQZN_input        = QZN::InitializeQZN_input;
using InitializeQZN_output       = QZN::InitializeQZN_output;
using SettleMatch_input          = QZN::SettleMatch_input;
using SettleMatch_output         = QZN::SettleMatch_output;
using ClaimVestedTokens_input    = QZN::ClaimVestedTokens_input;
using ClaimVestedTokens_output   = QZN::ClaimVestedTokens_output;
using BurnFromTreasury_input     = QZN::BurnFromTreasury_input;
using BurnFromTreasury_output    = QZN::BurnFromTreasury_output;
using GetSupplyInfo_input        = QZN::GetSupplyInfo_input;
using GetSupplyInfo_output       = QZN::GetSupplyInfo_output;
using GetVestingStatus_input     = QZN::GetVestingStatus_input;
using GetVestingStatus_output    = QZN::GetVestingStatus_output;
using GetMatchStats_input        = QZN::GetMatchStats_input;
using GetMatchStats_output       = QZN::GetMatchStats_output;
using InitializeCabinet_input    = QZNCABINET::InitializeCabinet_input;
using InitializeCabinet_output   = QZNCABINET::InitializeCabinet_output;
using RegisterMatch_input        = QZNCABINET::RegisterMatch_input;
using RegisterMatch_output       = QZNCABINET::RegisterMatch_output;
using SubmitResult_input         = QZNCABINET::SubmitResult_input;
using SubmitResult_output        = QZNCABINET::SubmitResult_output;
using ConfirmResult_input        = QZNCABINET::ConfirmResult_input;
using ConfirmResult_output       = QZNCABINET::ConfirmResult_output;
using DisputeResult_input        = QZNCABINET::DisputeResult_input;
using DisputeResult_output       = QZNCABINET::DisputeResult_output;
using InitializeRouter_input     = QZNREWARDROUTER::InitializeRouter_input;
using InitializeRouter_output    = QZNREWARDROUTER::InitializeRouter_output;
using RegisterPlayer_input       = QZNREWARDROUTER::RegisterPlayer_input;
using RegisterPlayer_output      = QZNREWARDROUTER::RegisterPlayer_output;
using StakeQZN_input             = QZNREWARDROUTER::StakeQZN_input;
using StakeQZN_output            = QZNREWARDROUTER::StakeQZN_output;
using UnstakeQZN_input           = QZNREWARDROUTER::UnstakeQZN_input;
using UnstakeQZN_output          = QZNREWARDROUTER::UnstakeQZN_output;
using ReportMatchResult_input    = QZNREWARDROUTER::ReportMatchResult_input;
using ReportMatchResult_output   = QZNREWARDROUTER::ReportMatchResult_output;
using ClaimRewards_input         = QZNREWARDROUTER::ClaimRewards_input;
using ClaimRewards_output        = QZNREWARDROUTER::ClaimRewards_output;
using FundReserve_input          = QZNREWARDROUTER::FundReserve_input;
using FundReserve_output         = QZNREWARDROUTER::FundReserve_output;
using InitializePortal_input     = QZNPORTAL::InitializePortal_input;
using InitializePortal_output    = QZNPORTAL::InitializePortal_output;
using IssueNode_input            = QZNPORTAL::IssueNode_input;
using IssueNode_output           = QZNPORTAL::IssueNode_output;
using TransferNode_input         = QZNPORTAL::TransferNode_input;
using TransferNode_output        = QZNPORTAL::TransferNode_output;
using ClaimNodeRevenue_input     = QZNPORTAL::ClaimNodeRevenue_input;
using ClaimNodeRevenue_output    = QZNPORTAL::ClaimNodeRevenue_output;
using RegisterGame_input         = QZNPORTAL::RegisterGame_input;
using RegisterGame_output        = QZNPORTAL::RegisterGame_output;
using ApproveGame_input          = QZNPORTAL::ApproveGame_input;
using ApproveGame_output         = QZNPORTAL::ApproveGame_output;
using UpdateGameState_input      = QZNPORTAL::UpdateGameState_input;
using UpdateGameState_output     = QZNPORTAL::UpdateGameState_output;
using StakeForAccess_input       = QZNPORTAL::StakeForAccess_input;
using StakeForAccess_output      = QZNPORTAL::StakeForAccess_output;
using ReceiveProtocolFees_input  = QZNPORTAL::ReceiveProtocolFees_input;
using ReceiveProtocolFees_output = QZNPORTAL::ReceiveProtocolFees_output;
using GetPlayerAccess_input      = QZNPORTAL::GetPlayerAccess_input;
using GetPlayerAccess_output     = QZNPORTAL::GetPlayerAccess_output;
using GetNode_input              = QZNPORTAL::GetNode_input;
using GetNode_output             = QZNPORTAL::GetNode_output;
using GetGame_input              = QZNPORTAL::GetGame_input;
using GetGame_output             = QZNPORTAL::GetGame_output;
using GetMatch_input             = QZNCABINET::GetMatch_input;
using GetMatch_output            = QZNCABINET::GetMatch_output;
using GetCabinetStats_input      = QZNCABINET::GetCabinetStats_input;
using GetCabinetStats_output     = QZNCABINET::GetCabinetStats_output;
using GetPlayerStats_input       = QZNREWARDROUTER::GetPlayerStats_input;
using GetPlayerStats_output      = QZNREWARDROUTER::GetPlayerStats_output;
using GetLeaderboard_input       = QZNREWARDROUTER::GetLeaderboard_input;
using GetLeaderboard_output      = QZNREWARDROUTER::GetLeaderboard_output;
using GetPortalStats_input  = QZNPORTAL::GetPortalStats_input;
using GetPortalStats_output = QZNPORTAL::GetPortalStats_output;
using InitializeVault_input      = QZNTREASVAULT::InitializeVault_input;
using InitializeVault_output     = QZNTREASVAULT::InitializeVault_output;
using ProposeSpend_input         = QZNTREASVAULT::ProposeSpend_input;
using ProposeSpend_output        = QZNTREASVAULT::ProposeSpend_output;
using SignProposal_input         = QZNTREASVAULT::SignProposal_input;
using SignProposal_output        = QZNTREASVAULT::SignProposal_output;
using ExecuteProposal_input      = QZNTREASVAULT::ExecuteProposal_input;
using ExecuteProposal_output     = QZNTREASVAULT::ExecuteProposal_output;
using CancelProposal_input       = QZNTREASVAULT::CancelProposal_input;
using CancelProposal_output      = QZNTREASVAULT::CancelProposal_output;
using GetVaultState_input        = QZNTREASVAULT::GetVaultState_input;
using GetVaultState_output       = QZNTREASVAULT::GetVaultState_output;
using GetProposal_input          = QZNTREASVAULT::GetProposal_input;
using GetProposal_output         = QZNTREASVAULT::GetProposal_output;
using InitializeTournamentEngine_input  = QZNTOUR::InitializeTournamentEngine_input;
using InitializeTournamentEngine_output = QZNTOUR::InitializeTournamentEngine_output;
using CreateTournament_input     = QZNTOUR::CreateTournament_input;
using CreateTournament_output    = QZNTOUR::CreateTournament_output;
using StartTournament_input      = QZNTOUR::StartTournament_input;
using StartTournament_output     = QZNTOUR::StartTournament_output;
using SubmitMatchResult_input    = QZNTOUR::SubmitMatchResult_input;
using SubmitMatchResult_output   = QZNTOUR::SubmitMatchResult_output;
using CancelTournament_input     = QZNTOUR::CancelTournament_input;
using CancelTournament_output    = QZNTOUR::CancelTournament_output;
using GetTournament_input        = QZNTOUR::GetTournament_input;
using GetTournament_output       = QZNTOUR::GetTournament_output;
using GetTourMatch_input  = QZNTOUR::GetMatch_input;
using GetTourMatch_output = QZNTOUR::GetMatch_output;
using GetPlayerRecord_input      = QZNTOUR::GetPlayerRecord_input;
using GetPlayerRecord_output     = QZNTOUR::GetPlayerRecord_output;

template<typename T> struct ProcOutput { struct type {}; };

template<> struct ProcOutput<InitializeQZN_input>     { using type = InitializeQZN_output; };
template<> struct ProcOutput<SettleMatch_input>        { using type = SettleMatch_output; };
template<> struct ProcOutput<ClaimVestedTokens_input>  { using type = ClaimVestedTokens_output; };
template<> struct ProcOutput<BurnFromTreasury_input>   { using type = BurnFromTreasury_output; };
template<> struct ProcOutput<InitializeCabinet_input>  { using type = InitializeCabinet_output; };
template<> struct ProcOutput<RegisterMatch_input>      { using type = RegisterMatch_output; };
template<> struct ProcOutput<SubmitResult_input>       { using type = SubmitResult_output; };
template<> struct ProcOutput<ConfirmResult_input>      { using type = ConfirmResult_output; };
template<> struct ProcOutput<DisputeResult_input>      { using type = DisputeResult_output; };
template<> struct ProcOutput<InitializeRouter_input>   { using type = InitializeRouter_output; };
template<> struct ProcOutput<QZNTOUR::RegisterPlayer_input>  { using type = QZNTOUR::RegisterPlayer_output; };
template<> struct ProcOutput<RegisterPlayer_input>     { using type = RegisterPlayer_output; };
template<> struct ProcOutput<StakeQZN_input>           { using type = StakeQZN_output; };
template<> struct ProcOutput<UnstakeQZN_input>         { using type = UnstakeQZN_output; };
template<> struct ProcOutput<ReportMatchResult_input>  { using type = ReportMatchResult_output; };
template<> struct ProcOutput<ClaimRewards_input>       { using type = ClaimRewards_output; };
template<> struct ProcOutput<FundReserve_input>        { using type = FundReserve_output; };
template<> struct ProcOutput<InitializePortal_input>   { using type = InitializePortal_output; };
template<> struct ProcOutput<IssueNode_input>          { using type = IssueNode_output; };
template<> struct ProcOutput<TransferNode_input>       { using type = TransferNode_output; };
template<> struct ProcOutput<ClaimNodeRevenue_input>   { using type = ClaimNodeRevenue_output; };
template<> struct ProcOutput<RegisterGame_input>       { using type = RegisterGame_output; };
template<> struct ProcOutput<ApproveGame_input>        { using type = ApproveGame_output; };
template<> struct ProcOutput<UpdateGameState_input>    { using type = UpdateGameState_output; };
template<> struct ProcOutput<StakeForAccess_input>     { using type = StakeForAccess_output; };
template<> struct ProcOutput<ReceiveProtocolFees_input>{ using type = ReceiveProtocolFees_output; };
template<typename T> struct FuncOutput { struct type {}; };
template<> struct FuncOutput<GetSupplyInfo_input>      { using type = GetSupplyInfo_output; };
template<> struct FuncOutput<GetVestingStatus_input>   { using type = GetVestingStatus_output; };
template<> struct FuncOutput<GetMatchStats_input>      { using type = GetMatchStats_output; };
template<> struct FuncOutput<GetPlayerAccess_input>    { using type = GetPlayerAccess_output; };
template<> struct FuncOutput<GetNode_input>            { using type = GetNode_output; };
template<> struct FuncOutput<GetGame_input>            { using type = GetGame_output; };
template<> struct FuncOutput<GetCabinetStats_input>    { using type = GetCabinetStats_output; };
template<> struct FuncOutput<GetPlayerStats_input>     { using type = GetPlayerStats_output; };
template<> struct FuncOutput<GetLeaderboard_input>     { using type = GetLeaderboard_output; };
template<> struct ProcOutput<InitializeVault_input>     { using type = InitializeVault_output; };
template<> struct ProcOutput<ProposeSpend_input>        { using type = ProposeSpend_output; };
template<> struct ProcOutput<SignProposal_input>        { using type = SignProposal_output; };
template<> struct ProcOutput<ExecuteProposal_input>     { using type = ExecuteProposal_output; };
template<> struct ProcOutput<CancelProposal_input>      { using type = CancelProposal_output; };
template<> struct FuncOutput<GetVaultState_input>       { using type = GetVaultState_output; };
template<> struct FuncOutput<GetProposal_input>         { using type = GetProposal_output; };
template<> struct ProcOutput<InitializeTournamentEngine_input> { using type = InitializeTournamentEngine_output; };
template<> struct ProcOutput<CreateTournament_input>    { using type = CreateTournament_output; };
template<> struct ProcOutput<StartTournament_input>     { using type = StartTournament_output; };
template<> struct ProcOutput<SubmitMatchResult_input>   { using type = SubmitMatchResult_output; };
template<> struct ProcOutput<CancelTournament_input>    { using type = CancelTournament_output; };
template<> struct FuncOutput<GetTournament_input>       { using type = GetTournament_output; };
template<> struct FuncOutput<QZNCABINET::GetMatch_input>    { using type = QZNCABINET::GetMatch_output; };
template<> struct FuncOutput<GetTourMatch_input>        { using type = GetTourMatch_output; };
template<> struct FuncOutput<GetPlayerRecord_input>     { using type = GetPlayerRecord_output; };
template<> struct FuncOutput<QZNPORTAL::GetPortalStats_input> { using type = QZNPORTAL::GetPortalStats_output; };

static constexpr unsigned short InitializeQZN_id      = 1;
static constexpr unsigned short SettleMatch_id         = 2;
static constexpr unsigned short ClaimVestedTokens_id   = 3;
static constexpr unsigned short BurnFromTreasury_id    = 4;
static constexpr unsigned short GetSupplyInfo_id       = 5;
static constexpr unsigned short GetVestingStatus_id    = 6;
static constexpr unsigned short GetMatchStats_id       = 7;
static constexpr unsigned short InitializeCabinet_id   = 1;
static constexpr unsigned short RegisterMatch_id       = 2;
static constexpr unsigned short SubmitResult_id        = 3;
static constexpr unsigned short ConfirmResult_id       = 4;
static constexpr unsigned short DisputeResult_id       = 5;
static constexpr unsigned short InitializeRouter_id    = 1;
static constexpr unsigned short RegisterPlayer_id      = 2;
static constexpr unsigned short StakeQZN_id            = 3;
static constexpr unsigned short UnstakeQZN_id          = 4;
static constexpr unsigned short ReportMatchResult_id   = 5;
static constexpr unsigned short ClaimRewards_id        = 6;
static constexpr unsigned short FundReserve_id         = 7;
static constexpr unsigned short InitializePortal_id    = 1;
static constexpr unsigned short IssueNode_id           = 2;
static constexpr unsigned short TransferNode_id        = 3;
static constexpr unsigned short ClaimNodeRevenue_id    = 4;
static constexpr unsigned short RegisterGame_id        = 5;
static constexpr unsigned short ApproveGame_id         = 6;
static constexpr unsigned short UpdateGameState_id     = 7;
static constexpr unsigned short StakeForAccess_id      = 8;
static constexpr unsigned short ReceiveProtocolFees_id = 9;
static constexpr unsigned short GetPlayerAccess_id     = 10;
static constexpr unsigned short GetNode_id             = 11;
static constexpr unsigned short GetGame_id             = 12;
static constexpr unsigned short GetMatch_id            = 6;
static constexpr unsigned short GetCabinetStats_id     = 7;
static constexpr unsigned short GetPlayerStats_id      = 8;
static constexpr unsigned short GetLeaderboard_id      = 9;
static constexpr unsigned short InitializeVault_id      = 1;
static constexpr unsigned short ProposeSpend_id         = 2;
static constexpr unsigned short SignProposal_id         = 3;
static constexpr unsigned short ExecuteProposal_id      = 4;
static constexpr unsigned short CancelProposal_id       = 5;
static constexpr unsigned short GetVaultState_id        = 6;
static constexpr unsigned short GetProposal_id          = 7;
static constexpr unsigned short InitializeTournamentEngine_id = 1;
static constexpr unsigned short CreateTournament_id     = 2;
static constexpr unsigned short StartTournament_id      = 4;
static constexpr unsigned short SubmitMatchResult_id    = 5;
static constexpr unsigned short CancelTournament_id     = 6;
static constexpr unsigned short GetTournament_id        = 7;
static constexpr unsigned short GetTourMatch_id         = 8;
static constexpr unsigned short GetPlayerRecord_id      = 9;
static constexpr unsigned short GetPortalStats_id      = 13;
static constexpr sint64 STAKE_TIER_0 = 0LL;

// Map contract types to their indices
template<typename T> struct ContractIndexOf { static constexpr unsigned int value = 0; };
template<> struct ContractIndexOf<QZN> { static constexpr unsigned int value = 25; };
template<> struct ContractIndexOf<QZNCABINET> { static constexpr unsigned int value = 26; };
template<> struct ContractIndexOf<QZNREWARDROUTER> { static constexpr unsigned int value = 27; };
template<> struct ContractIndexOf<QZNTREASVAULT> { static constexpr unsigned int value = 28; };
template<> struct ContractIndexOf<QZNPORTAL> { static constexpr unsigned int value = 29; };
template<> struct ContractIndexOf<QZNTOUR> { static constexpr unsigned int value = 30; };

template<typename ContractType>
class ContractTester : public ContractTesting {
public:
    unsigned int contractIdx = 0;
    id currentInvocator = {};
    long long currentInvocationReward = 0;

    ContractTester() : ContractTesting() {
        contractIdx = ContractIndexOf<ContractType>::value;
        if (!contractStates[contractIdx]) {
            contractStates[contractIdx] = (unsigned char*)malloc(contractDescriptions[contractIdx].stateSize);
            setMem(contractStates[contractIdx], contractDescriptions[contractIdx].stateSize, 0);
        }
        // Register functions and procedures for this contract
        contractSystemProcedures[contractIdx][INITIALIZE] = (SYSTEM_PROCEDURE)ContractType::__initialize;
        contractSystemProcedureLocalsSizes[contractIdx][INITIALIZE] = ContractType::__initializeLocalsSize;
        contractSystemProcedures[contractIdx][BEGIN_EPOCH] = (SYSTEM_PROCEDURE)ContractType::__beginEpoch;
        contractSystemProcedureLocalsSizes[contractIdx][BEGIN_EPOCH] = ContractType::__beginEpochLocalsSize;
        contractSystemProcedures[contractIdx][END_EPOCH] = (SYSTEM_PROCEDURE)ContractType::__endEpoch;
        contractSystemProcedureLocalsSizes[contractIdx][END_EPOCH] = ContractType::__endEpochLocalsSize;
        contractSystemProcedures[contractIdx][BEGIN_TICK] = (SYSTEM_PROCEDURE)ContractType::__beginTick;
        contractSystemProcedureLocalsSizes[contractIdx][BEGIN_TICK] = ContractType::__beginTickLocalsSize;
        contractSystemProcedures[contractIdx][END_TICK] = (SYSTEM_PROCEDURE)ContractType::__endTick;
        contractSystemProcedureLocalsSizes[contractIdx][END_TICK] = ContractType::__endTickLocalsSize;
        QpiContextForInit qpi(contractIdx);
        ContractType::__registerUserFunctionsAndProcedures(qpi);
        setContractFeeReserve(contractIdx, 10000000);
        initEmptySpectrum();
        initEmptyUniverse();
    }

    void reset() {
        unsigned int idx = contractIdx;
        this->~ContractTester<ContractType>();
        contractStates[idx] = nullptr;
        new (this) ContractTester<ContractType>();
    }

    void setInvocator(id addr) {
        currentInvocator = addr;
        increaseEnergy(addr, 1000000000LL);
    }
    void setInvocationReward(long long amount) { currentInvocationReward = amount; }
    void setCurrentEpoch(unsigned short epoch) { system.epoch = epoch; }
    void setCurrentTick(unsigned int tick) { system.tick = tick; }
    void advanceBeginEpoch() {
        system.epoch++;
        callSystemProcedure(contractIdx, SystemProcedureID::BEGIN_EPOCH);
    }
    void advanceEndTick() {
        callSystemProcedure(contractIdx, SystemProcedureID::END_TICK);
    }

    typename ContractType::StateData& state() {
        return *(typename ContractType::StateData*)contractStates[contractIdx];
    }

    template<typename InputType>
    typename ProcOutput<InputType>::type callProcedure(unsigned short procId, const InputType& input) {
        using OutputType = typename ProcOutput<InputType>::type;
        OutputType output{};
        QpiContextUserProcedureCall qpiContext(contractIdx, currentInvocator, currentInvocationReward);
        currentInvocationReward = 0;
        qpiContext.call(procId, &input, sizeof(input));
        if (qpiContext.outputSize >= sizeof(output))
            copyMem(&output, qpiContext.outputBuffer, sizeof(output));
        return output;
    }

    template<typename InputType>
    typename FuncOutput<InputType>::type callFunction(unsigned short funcId, const InputType& input) {
        using OutputType = typename FuncOutput<InputType>::type;
        OutputType output{};
        ContractTesting::callFunction(contractIdx, funcId, input, output);
        return output;
    }
};


#endif // CONTRACT_TESTER_H
// This file was intentionally left blank - patch applied below
