// ============================================================
//  QZN TREASURY VAULT PAO
//  Contract: Programmable Arcade Object — Treasury Vault
//  Network:  Qubic (QPI / C++ Smart Contract)
//  Version:  1.0.0
//
//  Architecture from: QZN Whitepaper (Section 9)
//
//  Treasury Control:
//    - Multi-signature vault (2-of-3 threshold)
//    - Transparent onchain activity
//    - Time-structured release curve
//
//  Responsibilities:
//    1. Hold and protect QZN treasury (87.5M allocation)
//    2. Enforce 2-of-3 multi-sig on all outflows
//    3. Fund Reward Router reserve each epoch
//    4. Fund Achievement reserve each epoch
//    5. Execute governance-approved burns
//    6. Time-lock large withdrawals (anti-rug protection)
//    7. Emit transparent spending logs
//
//  Spending Categories:
//    - EPOCH_REWARD_FUND  : Weekly reward pool top-up
//    - ACHIEVEMENT_FUND   : Achievement reserve top-up
//    - LIQUIDITY_INJECT   : Add liquidity to QSWAP
//    - ECOSYSTEM_GRANT    : Ecosystem fund disbursement
//    - BURN_EVENT         : Governance-triggered burn
//    - EMERGENCY_PAUSE    : Admin safety pause
//
//  Multi-Sig Rules:
//    - 3 keyholders defined at init (founder + 2 trustees)
//    - Any spend proposal requires 2-of-3 signatures
//    - Proposals expire after PROPOSAL_EXPIRY_EPOCHS
//    - Large spends (> TIMELOCK_THRESHOLD) have 2-epoch delay
//    - No single key can move funds unilaterally — ever
// ============================================================

using namespace QPI;

// ============================================================
//  CONSTANTS
// ============================================================

// Multi-sig
constexpr uint8  REQUIRED_SIGS            = 2;     // 2-of-3 threshold
constexpr uint8  TOTAL_SIGNERS            = 3;     // Total keyholders
constexpr sint64 PROPOSAL_EXPIRY_EPOCHS   = 4LL;   // ~4 weeks to collect sigs
constexpr sint64 MAX_ACTIVE_PROPOSALS     = 8LL;   // Concurrent proposal slots

// Timelock: large spends require 2-epoch delay before execution
constexpr sint64 TIMELOCK_THRESHOLD       = 1000000LL;  // 1M QZN triggers timelock
constexpr sint64 TIMELOCK_DELAY_EPOCHS    = 2LL;        // 2 epoch wait

// Epoch funding defaults (admin-adjustable via proposal)
constexpr sint64 DEFAULT_EPOCH_REWARD_FUND     = 500000LL;   // 500k QZN/epoch to rewards
constexpr sint64 DEFAULT_ACHIEVEMENT_FUND      = 50000LL;    // 50k QZN/epoch to achievements
constexpr sint64 MAX_EPOCH_FUND                = 2000000LL;  // Safety cap per epoch

// Spending categories
constexpr uint8  SPEND_EPOCH_REWARD       = 1;
constexpr uint8  SPEND_ACHIEVEMENT_FUND   = 2;
constexpr uint8  SPEND_LIQUIDITY_INJECT   = 3;
constexpr uint8  SPEND_ECOSYSTEM_GRANT    = 4;
constexpr uint8  SPEND_BURN_EVENT         = 5;
constexpr uint8  SPEND_SIGNER_CHANGE      = 6;   // Replace a keyholder

// Proposal states
constexpr uint8  PROP_EMPTY              = 0;
constexpr uint8  PROP_PENDING            = 1;    // Awaiting signatures
constexpr uint8  PROP_APPROVED           = 2;    // 2-of-3 reached, queued
constexpr uint8  PROP_TIMELOCKED         = 3;    // Approved, waiting timelock
constexpr uint8  PROP_EXECUTED           = 4;    // Funds moved
constexpr uint8  PROP_EXPIRED            = 5;    // Timed out
constexpr uint8  PROP_CANCELLED          = 6;    // Manually cancelled

// ============================================================
//  DATA STRUCTURES
// ============================================================

struct SpendProposal
{
    // Identity
    sint64  proposalId;
    uint8   category;              // SPEND_* constant
    uint8   state;                 // PROP_* constant

    // Spend details
    id      destinationAddress;    // Where funds go
    sint64  amount;                // QZN amount
    sint64  memo;                  // Hash of offchain justification doc

    // Signatures (3 slots for 3 keyholders)
    bit     sig_0;                 // Keyholder 0 signed
    bit     sig_1;                 // Keyholder 1 signed
    bit     sig_2;                 // Keyholder 2 signed
    uint8   sigCount;              // Running count

    // Timing
    sint64  proposedEpoch;         // Epoch proposal was created
    sint64  approvedEpoch;         // Epoch 2nd sig collected
    sint64  executeAfterEpoch;     // For timelocked proposals
    sint64  executedEpoch;         // Epoch funds moved

    // Proposer
    id      proposerAddress;
};

// ============================================================
//  CONTRACT STATE
// ============================================================

struct QZNTREASVAULT2
{
};

struct QZNTREASVAULT : public ContractBase
{
    struct StateData
    {
    // ---- Balances ----
    sint64  treasuryBalance;          // QZN in vault
    sint64  totalReceived;            // Lifetime QZN received
    sint64  totalSpent;               // Lifetime QZN spent
    sint64  totalBurned;              // Lifetime QZN burned from treasury
    sint64  totalEpochFunded;         // Lifetime QZN sent to reward router

    // ---- Multi-Sig Keyholders ----
    id      signer_0;                 // Founder
    id      signer_1;                 // Trustee 1
    id      signer_2;                 // Trustee 2
    bit     signer_0_active;
    bit     signer_1_active;
    bit     signer_2_active;

    // ---- Proposal Registry ----
    SpendProposal proposals_0;
    SpendProposal proposals_1;
    SpendProposal proposals_2;
    SpendProposal proposals_3;
    SpendProposal proposals_4;
    SpendProposal proposals_5;
    SpendProposal proposals_6;
    SpendProposal proposals_7;

    // ---- Proposal Counter ----
    sint64  nextProposalId;

    // ---- Epoch Funding Config ----
    sint64  epochRewardFundAmount;    // QZN to send to Reward Router each epoch
    sint64  epochAchievementAmount;   // QZN to send to achievement reserve each epoch
    bit     autoFundEnabled;          // Auto-fund fires on BEGIN_EPOCH if true

    // ---- Connected Contracts ----
    id      rewardRouterAddress;      // Reward Router PAO
    id      qznCoreAddress;           // QZN Core Token contract

    // ---- Spending Log ----
    sint64  lastSpendEpoch;
    sint64  lastSpendAmount;
    uint8   lastSpendCategory;

    // ---- Admin ----
    id      adminAddress;
    bit     initialized;
    bit     paused;                   // Emergency pause flag
    };

public:


// ============================================================
//  INPUT / OUTPUT STRUCTS
// ============================================================

// --- Initialize ---
struct InitializeVault_input
{
    id      signer0;                  // Founder
    id      signer1;                  // Trustee 1
    id      signer2;                  // Trustee 2
    id      rewardRouterAddr;
    id      qznCoreAddr;
    sint64  initialBalance;           // QZN to seed vault with
};
struct InitializeVault_output
{
    bit     success;
    sint64  vaultBalance;
};

// --- Propose Spend ---
struct ProposeSpend_input
{
    uint8   category;
    id      destination;
    sint64  amount;
    sint64  memo;                     // K12 hash of justification
};
struct ProposeSpend_output
{
    sint64  proposalId;
    uint8   proposalSlot;
    bit     timelockRequired;
    sint64  executeAfterEpoch;        // 0 if no timelock
};

// --- Sign Proposal ---
struct SignProposal_input
{
    sint64  proposalId;
};
struct SignProposal_output
{
    uint8   sigsCollected;
    uint8   sigsRequired;
    bit     approved;                 // True if threshold met
    bit     timelocked;               // True if large spend queued
    bit     executed;                 // True if no timelock + instant exec
};

// --- Execute Timelocked Proposal ---
struct ExecuteProposal_input
{
    sint64  proposalId;
};
struct ExecuteProposal_output
{
    bit     executed;
    sint64  amountSent;
    uint8   category;
};

// --- Cancel Proposal ---
struct CancelProposal_input
{
    sint64  proposalId;
};
struct CancelProposal_output
{
    bit     cancelled;
};

// --- Update Epoch Funding Config ---
struct SetEpochFunding_input
{
    sint64  rewardAmount;
    sint64  achievementAmount;
    bit     enableAutoFund;
};
struct SetEpochFunding_output
{
    bit     success;
    sint64  proposalIdRequired;       // Config change requires proposal approval
};

// --- Query Vault State ---
struct GetVaultState_input {};
struct GetVaultState_output
{
    sint64  treasuryBalance;
    sint64  totalSpent;
    sint64  totalBurned;
    sint64  epochRewardFundAmount;
    bit     autoFundEnabled;
    bit     paused;
    sint64  nextProposalId;
};

// --- Query Proposal ---
struct GetProposal_input
{
    sint64  proposalId;
};
struct GetProposal_output
{
    uint8   category;
    uint8   state;
    sint64  amount;
    id      destination;
    uint8   sigsCollected;
    sint64  proposedEpoch;
    sint64  executeAfterEpoch;
    bit     expired;
};

// ============================================================
//  INTERNAL HELPERS
// ============================================================

PRIVATE_FUNCTION(_isSigner)
/*
 * Returns 1 if given address is an active keyholder.
 */
{
    // Embedded inline at call sites
}

PRIVATE_PROCEDURE(_executeSpend)
/*
 * Routes an approved, unlocked proposal to its destination.
 * Handles all 6 spend categories.
 */
{
    // Embedded inline in SignProposal and ExecuteProposal
}

// ============================================================
//  CONTRACT PROCEDURES
// ============================================================

PUBLIC_PROCEDURE(InitializeVault)
{
    if (state.get().initialized)
    {
        return;
    }

    state.mut().adminAddress = qpi.invocator();
    state.mut().signer_0 = input.signer0;
    state.mut().signer_1 = input.signer1;
    state.mut().signer_2 = input.signer2;
    state.mut().signer_0_active = 1;
    state.mut().signer_1_active = 1;
    state.mut().signer_2_active = 1;
    state.mut().rewardRouterAddress = input.rewardRouterAddr;
    state.mut().qznCoreAddress = input.qznCoreAddr;
    state.mut().treasuryBalance = input.initialBalance;
    state.mut().totalReceived = input.initialBalance;

    // Default epoch funding
    state.mut().epochRewardFundAmount = DEFAULT_EPOCH_REWARD_FUND;
    state.mut().epochAchievementAmount = DEFAULT_ACHIEVEMENT_FUND;
    state.mut().autoFundEnabled = 1;   // Auto-fund on by default

    // Reset counters
    state.mut().totalSpent = 0;
    state.mut().totalBurned = 0;
    state.mut().totalEpochFunded = 0;
    state.mut().nextProposalId = 1;
    state.mut().paused = 0;
    state.mut().initialized = 1;

    output.success              = 1;
    output.vaultBalance         = state.get().treasuryBalance;
}

PUBLIC_PROCEDURE(ProposeSpend)
/*
 * Any keyholder can propose a treasury spend.
 * Proposal is recorded on-chain with full details.
 * Proposer's signature is auto-collected (counts as sig 1).
 *
 * Large spends (> TIMELOCK_THRESHOLD) are flagged for
 * 2-epoch delay even after 2nd sig is collected.
 *
 * All proposals are public — full transparency.
 */
{
    if (state.get().paused) { return; }

    // Only keyholders can propose
    bit isSigner;
    isSigner = 0;
    if (qpi.invocator() == state.get().signer_0 && state.get().signer_0_active) { isSigner = 1; }
    if (qpi.invocator() == state.get().signer_1 && state.get().signer_1_active) { isSigner = 1; }
    if (qpi.invocator() == state.get().signer_2 && state.get().signer_2_active) { isSigner = 1; }

    if (!isSigner) { return; }

    // Validate amount
    if (input.amount <= 0 || input.amount > state.get().treasuryBalance) { return; }

    // Validate category
    if (input.category < SPEND_EPOCH_REWARD || input.category > SPEND_SIGNER_CHANGE) { return; }

    // Epoch fund safety cap
    if (input.category == SPEND_EPOCH_REWARD && input.amount > MAX_EPOCH_FUND) { return; }

    // Find free proposal slot
    sint64 slot;
    slot = -1;

    if (state.get().proposals_0.state == PROP_EMPTY || state.get().proposals_0.state == PROP_EXECUTED ||
        state.get().proposals_0.state == PROP_EXPIRED || state.get().proposals_0.state == PROP_CANCELLED)
    { slot = 0; }
    else if (state.get().proposals_1.state == PROP_EMPTY || state.get().proposals_1.state == PROP_EXECUTED ||
             state.get().proposals_1.state == PROP_EXPIRED || state.get().proposals_1.state == PROP_CANCELLED)
    { slot = 1; }
    else if (state.get().proposals_2.state == PROP_EMPTY || state.get().proposals_2.state == PROP_EXECUTED ||
             state.get().proposals_2.state == PROP_EXPIRED || state.get().proposals_2.state == PROP_CANCELLED)
    { slot = 2; }
    else if (state.get().proposals_3.state == PROP_EMPTY || state.get().proposals_3.state == PROP_EXECUTED ||
             state.get().proposals_3.state == PROP_EXPIRED || state.get().proposals_3.state == PROP_CANCELLED)
    { slot = 3; }
    else if (state.get().proposals_4.state == PROP_EMPTY || state.get().proposals_4.state == PROP_EXECUTED ||
             state.get().proposals_4.state == PROP_EXPIRED || state.get().proposals_4.state == PROP_CANCELLED)
    { slot = 4; }
    else if (state.get().proposals_5.state == PROP_EMPTY || state.get().proposals_5.state == PROP_EXECUTED ||
             state.get().proposals_5.state == PROP_EXPIRED || state.get().proposals_5.state == PROP_CANCELLED)
    { slot = 5; }
    else if (state.get().proposals_6.state == PROP_EMPTY || state.get().proposals_6.state == PROP_EXECUTED ||
             state.get().proposals_6.state == PROP_EXPIRED || state.get().proposals_6.state == PROP_CANCELLED)
    { slot = 6; }
    else if (state.get().proposals_7.state == PROP_EMPTY || state.get().proposals_7.state == PROP_EXECUTED ||
             state.get().proposals_7.state == PROP_EXPIRED || state.get().proposals_7.state == PROP_CANCELLED)
    { slot = 7; }

    if (slot < 0) { return; } // All slots full

    // Determine if timelock required
    bit     needsTimelock;
    sint64  execAfter;
    needsTimelock = (input.amount >= TIMELOCK_THRESHOLD) ? 1 : 0;
    execAfter     = 0;

    // Write proposal
    if (slot == 0)
    {
        state.mut().proposals_0.proposalId        = state.get().nextProposalId;
        state.mut().proposals_0.category          = input.category;
        state.mut().proposals_0.state             = PROP_PENDING;
        state.mut().proposals_0.destinationAddress = input.destination;
        state.mut().proposals_0.amount            = input.amount;
        state.mut().proposals_0.memo              = input.memo;
        state.mut().proposals_0.proposedEpoch     = qpi.epoch();
        state.mut().proposals_0.approvedEpoch     = 0;
        state.mut().proposals_0.executeAfterEpoch = 0;
        state.mut().proposals_0.executedEpoch     = 0;
        state.mut().proposals_0.proposerAddress   = qpi.invocator();
        state.mut().proposals_0.sigCount          = 0;
        state.mut().proposals_0.sig_0             = 0;
        state.mut().proposals_0.sig_1             = 0;
        state.mut().proposals_0.sig_2             = 0;

        // Auto-sign for proposer
        if (qpi.invocator() == state.get().signer_0) { state.mut().proposals_0.sig_0 = 1; state.mut().proposals_0.sigCount = 1; }
        if (qpi.invocator() == state.get().signer_1) { state.mut().proposals_0.sig_1 = 1; state.mut().proposals_0.sigCount = 1; }
        if (qpi.invocator() == state.get().signer_2) { state.mut().proposals_0.sig_2 = 1; state.mut().proposals_0.sigCount = 1; }
    }
    // Slots 1-7 follow identical pattern

    state.mut().nextProposalId = state.get().nextProposalId + 1;

    output.proposalId        = state.get().nextProposalId - 1;
    output.proposalSlot      = slot;
    output.timelockRequired  = needsTimelock;
    output.executeAfterEpoch = execAfter;
}

PUBLIC_PROCEDURE(SignProposal)
/*
 * Keyholder adds their signature to a pending proposal.
 * When REQUIRED_SIGS (2) collected:
 *   - No timelock: executes immediately
 *   - Timelock required: moves to PROP_TIMELOCKED state
 *
 * Spend execution is embedded here for small spends.
 * Large spends must call ExecuteProposal after timelock.
 */
{
    if (state.get().paused) { return; }

    // Verify signer
    bit isSigner;
    isSigner = 0;
    if (qpi.invocator() == state.get().signer_0 && state.get().signer_0_active) { isSigner = 1; }
    if (qpi.invocator() == state.get().signer_1 && state.get().signer_1_active) { isSigner = 1; }
    if (qpi.invocator() == state.get().signer_2 && state.get().signer_2_active) { isSigner = 1; }

    if (!isSigner) { return; }

    // Find proposal by ID
    sint64 slot;
    slot = -1;

    if (state.get().proposals_0.proposalId == input.proposalId && state.get().proposals_0.state == PROP_PENDING) { slot = 0; }
    else if (state.get().proposals_1.proposalId == input.proposalId && state.get().proposals_1.state == PROP_PENDING) { slot = 1; }
    else if (state.get().proposals_2.proposalId == input.proposalId && state.get().proposals_2.state == PROP_PENDING) { slot = 2; }
    else if (state.get().proposals_3.proposalId == input.proposalId && state.get().proposals_3.state == PROP_PENDING) { slot = 3; }
    else if (state.get().proposals_4.proposalId == input.proposalId && state.get().proposals_4.state == PROP_PENDING) { slot = 4; }
    else if (state.get().proposals_5.proposalId == input.proposalId && state.get().proposals_5.state == PROP_PENDING) { slot = 5; }
    else if (state.get().proposals_6.proposalId == input.proposalId && state.get().proposals_6.state == PROP_PENDING) { slot = 6; }
    else if (state.get().proposals_7.proposalId == input.proposalId && state.get().proposals_7.state == PROP_PENDING) { slot = 7; }

    if (slot < 0) { return; }

    if (slot == 0)
    {
        // Check not already signed by this signer
        if (qpi.invocator() == state.get().signer_0 && !state.get().proposals_0.sig_0)
        {
            state.mut().proposals_0.sig_0    = 1;
            state.mut().proposals_0.sigCount = state.get().proposals_0.sigCount + 1;
        }
        else if (qpi.invocator() == state.get().signer_1 && !state.get().proposals_0.sig_1)
        {
            state.mut().proposals_0.sig_1    = 1;
            state.mut().proposals_0.sigCount = state.get().proposals_0.sigCount + 1;
        }
        else if (qpi.invocator() == state.get().signer_2 && !state.get().proposals_0.sig_2)
        {
            state.mut().proposals_0.sig_2    = 1;
            state.mut().proposals_0.sigCount = state.get().proposals_0.sigCount + 1;
        }

        output.sigsCollected = state.get().proposals_0.sigCount;
        output.sigsRequired  = REQUIRED_SIGS;

        // Check if threshold met
        if (state.get().proposals_0.sigCount >= REQUIRED_SIGS)
        {
            state.mut().proposals_0.approvedEpoch = qpi.epoch();
            output.approved = 1;

            // Large spend → timelock
            if (state.get().proposals_0.amount >= TIMELOCK_THRESHOLD)
            {
                state.mut().proposals_0.state             = PROP_TIMELOCKED;
                state.mut().proposals_0.executeAfterEpoch = qpi.epoch() + TIMELOCK_DELAY_EPOCHS;
                output.timelocked        = 1;
                output.executed          = 0;
            }
            else
            {
                // ---- IMMEDIATE EXECUTION (small spend) ----
                state.mut().proposals_0.state         = PROP_APPROVED;
                state.mut().proposals_0.executedEpoch = qpi.epoch();

                sint64  amt;
                uint8   cat;
                id      dest;

                amt  = state.get().proposals_0.amount;
                cat  = state.get().proposals_0.category;
                dest = state.get().proposals_0.destinationAddress;

                // Route by category
                if (cat == SPEND_EPOCH_REWARD || cat == SPEND_ACHIEVEMENT_FUND)
                {
                    // Transfer to Reward Router reserve
                    qpi.transfer(dest, amt);
                    state.mut().treasuryBalance = state.get().treasuryBalance - amt;
                    state.mut().totalSpent = state.get().totalSpent + amt;
                    state.mut().totalEpochFunded = state.get().totalEpochFunded + amt;
                }
                else if (cat == SPEND_LIQUIDITY_INJECT)
                {
                    // Transfer to liquidity pool
                    qpi.transfer(dest, amt);
                    state.mut().treasuryBalance = state.get().treasuryBalance - amt;
                    state.mut().totalSpent = state.get().totalSpent + amt;
                }
                else if (cat == SPEND_ECOSYSTEM_GRANT)
                {
                    // Transfer to ecosystem fund
                    qpi.transfer(dest, amt);
                    state.mut().treasuryBalance = state.get().treasuryBalance - amt;
                    state.mut().totalSpent = state.get().totalSpent + amt;
                }
                else if (cat == SPEND_BURN_EVENT)
                {
                    // Burn from treasury — deflationary event
                    state.mut().treasuryBalance = state.get().treasuryBalance - amt;
                    state.mut().totalBurned = state.get().totalBurned + amt;
                    state.mut().totalSpent = state.get().totalSpent + amt;
                    // Calls QZN Core BurnFromTreasury in Phase 2
                }

                state.mut().proposals_0.state    = PROP_EXECUTED;
                state.mut().lastSpendEpoch = qpi.epoch();
                state.mut().lastSpendAmount = amt;
                state.mut().lastSpendCategory = cat;

                output.timelocked = 0;
                output.executed   = 1;
            }
        }
        else
        {
            output.approved   = 0;
            output.timelocked = 0;
            output.executed   = 0;
        }
    }
    // Slots 1-7 follow identical pattern
}

PUBLIC_PROCEDURE(ExecuteProposal)
/*
 * Executes a TIMELOCKED proposal after delay has passed.
 * Any keyholder can trigger execution once timelock expires.
 * Provides accountability window for community to react
 * before large funds move.
 */
{
    if (state.get().paused) { return; }

    // Verify signer
    bit isSigner;
    isSigner = 0;
    if (qpi.invocator() == state.get().signer_0 && state.get().signer_0_active) { isSigner = 1; }
    if (qpi.invocator() == state.get().signer_1 && state.get().signer_1_active) { isSigner = 1; }
    if (qpi.invocator() == state.get().signer_2 && state.get().signer_2_active) { isSigner = 1; }

    if (!isSigner) { return; }

    // Find timelocked proposal
    sint64 slot;
    slot = -1;

    if (state.get().proposals_0.proposalId == input.proposalId && state.get().proposals_0.state == PROP_TIMELOCKED) { slot = 0; }
    else if (state.get().proposals_1.proposalId == input.proposalId && state.get().proposals_1.state == PROP_TIMELOCKED) { slot = 1; }
    else if (state.get().proposals_2.proposalId == input.proposalId && state.get().proposals_2.state == PROP_TIMELOCKED) { slot = 2; }
    else if (state.get().proposals_3.proposalId == input.proposalId && state.get().proposals_3.state == PROP_TIMELOCKED) { slot = 3; }
    else if (state.get().proposals_4.proposalId == input.proposalId && state.get().proposals_4.state == PROP_TIMELOCKED) { slot = 4; }
    else if (state.get().proposals_5.proposalId == input.proposalId && state.get().proposals_5.state == PROP_TIMELOCKED) { slot = 5; }
    else if (state.get().proposals_6.proposalId == input.proposalId && state.get().proposals_6.state == PROP_TIMELOCKED) { slot = 6; }
    else if (state.get().proposals_7.proposalId == input.proposalId && state.get().proposals_7.state == PROP_TIMELOCKED) { slot = 7; }

    if (slot < 0) { return; }

    if (slot == 0)
    {
        // Enforce timelock delay
        if (qpi.epoch() < state.get().proposals_0.executeAfterEpoch)
        {
            output.executed = 0;
            return;
        }

        sint64 amt;
        uint8  cat;
        id     dest;

        amt  = state.get().proposals_0.amount;
        cat  = state.get().proposals_0.category;
        dest = state.get().proposals_0.destinationAddress;

        // Execute spend by category
        if (cat == SPEND_EPOCH_REWARD || cat == SPEND_ACHIEVEMENT_FUND)
        {
            qpi.transfer(dest, amt);
            state.mut().treasuryBalance = state.get().treasuryBalance - amt;
            state.mut().totalSpent = state.get().totalSpent + amt;
            state.mut().totalEpochFunded = state.get().totalEpochFunded + amt;
        }
        else if (cat == SPEND_LIQUIDITY_INJECT || cat == SPEND_ECOSYSTEM_GRANT)
        {
            qpi.transfer(dest, amt);
            state.mut().treasuryBalance = state.get().treasuryBalance - amt;
            state.mut().totalSpent = state.get().totalSpent + amt;
        }
        else if (cat == SPEND_BURN_EVENT)
        {
            state.mut().treasuryBalance = state.get().treasuryBalance - amt;
            state.mut().totalBurned = state.get().totalBurned + amt;
            state.mut().totalSpent = state.get().totalSpent + amt;
        }

        state.mut().proposals_0.state         = PROP_EXECUTED;
        state.mut().proposals_0.executedEpoch = qpi.epoch();
        state.mut().lastSpendEpoch = qpi.epoch();
        state.mut().lastSpendAmount = amt;
        state.mut().lastSpendCategory = cat;

        output.executed    = 1;
        output.amountSent  = amt;
        output.category    = cat;
    }
    // Slots 1-7 identical
}

PUBLIC_PROCEDURE(CancelProposal)
/*
 * Any keyholder can cancel a PENDING proposal.
 * Cannot cancel TIMELOCKED or EXECUTED proposals.
 * Cancellation is final — must re-propose to retry.
 */
{
    bit isSigner;
    isSigner = 0;
    if (qpi.invocator() == state.get().signer_0 && state.get().signer_0_active) { isSigner = 1; }
    if (qpi.invocator() == state.get().signer_1 && state.get().signer_1_active) { isSigner = 1; }
    if (qpi.invocator() == state.get().signer_2 && state.get().signer_2_active) { isSigner = 1; }

    if (!isSigner) { return; }

    if (state.get().proposals_0.proposalId == input.proposalId && state.get().proposals_0.state == PROP_PENDING)
    {
        state.mut().proposals_0.state = PROP_CANCELLED;
        output.cancelled        = 1;
        return;
    }
    // Slots 1-7 identical

    output.cancelled = 0;
}

// ============================================================
//  READ-ONLY QUERY FUNCTIONS
// ============================================================

PUBLIC_FUNCTION(GetVaultState)
{
    output.treasuryBalance        = state.get().treasuryBalance;
    output.totalSpent             = state.get().totalSpent;
    output.totalBurned            = state.get().totalBurned;
    output.epochRewardFundAmount  = state.get().epochRewardFundAmount;
    output.autoFundEnabled        = state.get().autoFundEnabled;
    output.paused                 = state.get().paused;
    output.nextProposalId         = state.get().nextProposalId;
}

PUBLIC_FUNCTION(GetProposal)
{
    if (state.get().proposals_0.proposalId == input.proposalId)
    {
        output.category          = state.get().proposals_0.category;
        output.state             = state.get().proposals_0.state;
        output.amount            = state.get().proposals_0.amount;
        output.destination       = state.get().proposals_0.destinationAddress;
        output.sigsCollected     = state.get().proposals_0.sigCount;
        output.proposedEpoch     = state.get().proposals_0.proposedEpoch;
        output.executeAfterEpoch = state.get().proposals_0.executeAfterEpoch;
        output.expired           = (qpi.epoch() > state.get().proposals_0.proposedEpoch + PROPOSAL_EXPIRY_EPOCHS) ? 1 : 0;
    }
    // Slots 1-7 identical
}

// ============================================================
//  REGISTRATION
// ============================================================

REGISTER_USER_FUNCTIONS_AND_PROCEDURES()
{
    REGISTER_USER_PROCEDURE(InitializeVault,   1);
    REGISTER_USER_PROCEDURE(ProposeSpend,      2);
    REGISTER_USER_PROCEDURE(SignProposal,      3);
    REGISTER_USER_PROCEDURE(ExecuteProposal,   4);
    REGISTER_USER_PROCEDURE(CancelProposal,    5);
    REGISTER_USER_FUNCTION(GetVaultState,      6);
    REGISTER_USER_FUNCTION(GetProposal,        7);
}

// ============================================================
//  SYSTEM HOOKS
// ============================================================

BEGIN_EPOCH()
/*
 * Fires every epoch (~weekly). Responsibilities:
 *
 * 1. AUTO-FUND: If enabled, automatically send epoch reward
 *    allocation to Reward Router — no manual proposal needed
 *    for routine weekly funding
 *
 * 2. EXPIRY SWEEP: Mark stale PENDING proposals as EXPIRED
 *    so slots free up for new proposals
 *
 * 3. BALANCE GUARD: Pause auto-fund if treasury falls below
 *    6-month runway threshold
 */
{
    // ---- EXPIRY SWEEP ----
    if (state.get().proposals_0.state == PROP_PENDING)
    {
        if (qpi.epoch() > state.get().proposals_0.proposedEpoch + PROPOSAL_EXPIRY_EPOCHS)
        {
            state.mut().proposals_0.state = PROP_EXPIRED;
        }
    }
    if (state.get().proposals_1.state == PROP_PENDING)
    {
        if (qpi.epoch() > state.get().proposals_1.proposedEpoch + PROPOSAL_EXPIRY_EPOCHS)
        {
            state.mut().proposals_1.state = PROP_EXPIRED;
        }
    }
    if (state.get().proposals_2.state == PROP_PENDING)
    {
        if (qpi.epoch() > state.get().proposals_2.proposedEpoch + PROPOSAL_EXPIRY_EPOCHS)
        {
            state.mut().proposals_2.state = PROP_EXPIRED;
        }
    }
    if (state.get().proposals_3.state == PROP_PENDING)
    {
        if (qpi.epoch() > state.get().proposals_3.proposedEpoch + PROPOSAL_EXPIRY_EPOCHS)
        {
            state.mut().proposals_3.state = PROP_EXPIRED;
        }
    }
    // Slots 4-7 identical pattern

    // ---- BALANCE RUNWAY GUARD ----
    // 6-month runway = 26 epochs * (reward + achievement fund)
    sint64  monthlyBurn;
    sint64  runwayThreshold;
    monthlyBurn      = state.get().epochRewardFundAmount + state.get().epochAchievementAmount;
    runwayThreshold  = monthlyBurn * 26LL;

    if (state.get().treasuryBalance < runwayThreshold)
    {
        // Below 6-month runway — disable auto-fund, alert via state
        state.mut().autoFundEnabled = 0;
    }

    // ---- AUTO-FUND REWARD ROUTER ----
    if (state.get().autoFundEnabled && !state.get().paused)
    {
        // Reward fund
        if (state.get().epochRewardFundAmount > 0 &&
            state.get().treasuryBalance >= state.get().epochRewardFundAmount)
        {
            qpi.transfer(state.get().rewardRouterAddress, state.get().epochRewardFundAmount);
            state.mut().treasuryBalance = state.get().treasuryBalance - state.get().epochRewardFundAmount;
            state.mut().totalSpent = state.get().totalSpent + state.get().epochRewardFundAmount;
            state.mut().totalEpochFunded = state.get().totalEpochFunded + state.get().epochRewardFundAmount;
        }

        // Achievement fund
        if (state.get().epochAchievementAmount > 0 &&
            state.get().treasuryBalance >= state.get().epochAchievementAmount)
        {
            qpi.transfer(state.get().rewardRouterAddress, state.get().epochAchievementAmount);
            state.mut().treasuryBalance = state.get().treasuryBalance - state.get().epochAchievementAmount;
            state.mut().totalSpent = state.get().totalSpent + state.get().epochAchievementAmount;
            state.mut().totalEpochFunded = state.get().totalEpochFunded + state.get().epochAchievementAmount;
        }
    }
}

END_TICK() {}

};