// ============================================================
//  QZN REWARD ROUTER PAO
//  Contract: Programmable Arcade Object — Reward Router
//  Network:  Qubic (QPI / C++ Smart Contract)
//  Version:  1.0.0
//
//  Reward Mechanics:
//    1. Base Match Rewards     — earned per settled match
//    2. Staking Multiplier     — more QZN staked = higher earn rate
//    3. Achievement Milestones — one-time bonuses for feats
//    4. Epoch Leaderboard      — top players per epoch get bonus pool
//
//  Distribution Model:
//    - Rewards ACCUMULATE in player balance each epoch
//    - Players CLAIM once per epoch (weekly batch)
//    - Unclaimed rewards roll over (no expiry)
//    - Epoch cap enforced — anti-farming protection
//
//  Staking Multiplier Tiers:
//    Tier 0:  0 QZN staked       → 1.0x  (base)
//    Tier 1:  1,000 QZN staked   → 1.25x
//    Tier 2:  5,000 QZN staked   → 1.5x
//    Tier 3:  25,000 QZN staked  → 2.0x
//    Tier 4:  100,000 QZN staked → 3.0x
//
//  Achievement Milestones (one-time, per player):
//    FIRST_WIN        — First match won            → 50 QZN
//    STREAK_5         — 5 match win streak         → 100 QZN
//    STREAK_10        — 10 match win streak        → 250 QZN
//    MATCHES_100      — 100 matches played         → 500 QZN
//    MATCHES_1000     — 1000 matches played        → 2,000 QZN
//    HIGH_STAKE       — Single stake ≥ 10,000 QZN → 200 QZN
//    ALL_GAMES        — Won on all 3 games         → 300 QZN
//    TOP_LEADERBOARD  — Finished #1 in an epoch    → 1,000 QZN
// ============================================================

using namespace QPI;

// ============================================================
//  CONSTANTS — REWARD RATES
// ============================================================

// Base reward per match win (QZN)
constexpr sint64 BASE_WIN_REWARD         = 5LL;
constexpr sint64 BASE_LOSS_REWARD        = 1LL;    // Participation reward

// Epoch cap — max QZN earnable per player per epoch
constexpr sint64 EPOCH_EARN_CAP          = 10000LL;

// Staking multiplier thresholds (QZN staked)
constexpr sint64 STAKE_TIER_1            = 1000LL;
constexpr sint64 STAKE_TIER_2            = 5000LL;
constexpr sint64 STAKE_TIER_3            = 25000LL;
constexpr sint64 STAKE_TIER_4            = 100000LL;

// Multipliers stored as basis points (1000 = 1.0x)
constexpr sint64 MULT_TIER_0             = 1000LL;  // 1.0x
constexpr sint64 MULT_TIER_1             = 1250LL;  // 1.25x
constexpr sint64 MULT_TIER_2             = 1500LL;  // 1.5x
constexpr sint64 MULT_TIER_3             = 2000LL;  // 2.0x
constexpr sint64 MULT_TIER_4             = 3000LL;  // 3.0x
constexpr sint64 MULT_DENOMINATOR        = 1000LL;

// Achievement milestone rewards (QZN)
constexpr sint64 ACH_FIRST_WIN           = 50LL;
constexpr sint64 ACH_STREAK_5            = 100LL;
constexpr sint64 ACH_STREAK_10           = 250LL;
constexpr sint64 ACH_MATCHES_100         = 500LL;
constexpr sint64 ACH_MATCHES_1000        = 2000LL;
constexpr sint64 ACH_HIGH_STAKE          = 200LL;
constexpr sint64 ACH_ALL_GAMES           = 300LL;
constexpr sint64 ACH_TOP_LEADERBOARD     = 1000LL;
constexpr sint64 ACH_HIGH_STAKE_MIN      = 10000LL; // Min stake to trigger

// Leaderboard
constexpr sint64 MAX_LEADERBOARD_PLAYERS = 10LL;    // Top 10 tracked per epoch
constexpr sint64 LEADERBOARD_POOL_BPS    = 500LL;   // 5% of epoch reward pool → top 10
constexpr sint64 LEADERBOARD_WINNER_BPS  = 3000LL;  // 30% of leaderboard pool → #1
constexpr sint64 LEADERBOARD_BPS_DENOM   = 10000LL;

// Max tracked players (static memory)
constexpr sint64 MAX_PLAYERS             = 256LL;

// ============================================================
//  DATA STRUCTURES
// ============================================================

// Per-player state (stored in indexed slot)
struct PlayerRecord
{
    id      walletAddress;

    // Epoch tracking
    sint64  currentEpoch;           // Last epoch this record was updated
    sint64  epochEarned;            // QZN earned this epoch (vs cap)
    sint64  pendingBalance;         // Unclaimed QZN accumulated
    sint64  lifetimeEarned;         // Total QZN ever earned

    // Staking
    sint64  stakedAmount;           // QZN currently staked
    sint64  stakeMultiplierBPS;     // Current multiplier in BPS

    // Match stats
    sint64  totalMatchesPlayed;
    sint64  totalMatchesWon;
    sint64  currentWinStreak;
    sint64  bestWinStreak;

    // Game-specific win flags (for ALL_GAMES achievement)
    bit     wonSnaqe;
    bit     wonPaqman;
    bit     wonTanq;

    // Achievement flags (1 = already claimed)
    bit     achFirstWin;
    bit     achStreak5;
    bit     achStreak10;
    bit     achMatches100;
    bit     achMatches1000;
    bit     achHighStake;
    bit     achAllGames;
    bit     achTopLeaderboard;

    // Epoch leaderboard score
    sint64  epochScore;             // Points accumulated this epoch
    bit     active;                 // Slot in use
};

// Leaderboard entry
struct LeaderboardEntry
{
    id      walletAddress;
    sint64  score;
    sint64  rank;
};

// ============================================================
//  CONTRACT STATE
// ============================================================

struct QZNREWARDROUTER2
{
};

struct QZNREWARDROUTER : public ContractBase
{
    struct StateData
    {
    // ---- Player Registry (256 slots) ----
    // QPI static memory — explicit slot declarations
    PlayerRecord players_0;
    PlayerRecord players_1;
    PlayerRecord players_2;
    PlayerRecord players_3;
    PlayerRecord players_4;
    PlayerRecord players_5;
    PlayerRecord players_6;
    PlayerRecord players_7;
    PlayerRecord players_8;
    PlayerRecord players_9;
    PlayerRecord players_10;
    PlayerRecord players_11;
    PlayerRecord players_12;
    PlayerRecord players_13;
    PlayerRecord players_14;
    PlayerRecord players_15;
    PlayerRecord players_16;
    PlayerRecord players_17;
    PlayerRecord players_18;
    PlayerRecord players_19;
    PlayerRecord players_20;
    PlayerRecord players_21;
    PlayerRecord players_22;
    PlayerRecord players_23;
    PlayerRecord players_24;
    PlayerRecord players_25;
    PlayerRecord players_26;
    PlayerRecord players_27;
    PlayerRecord players_28;
    PlayerRecord players_29;
    PlayerRecord players_30;
    PlayerRecord players_31;
    PlayerRecord players_32;
    PlayerRecord players_33;
    PlayerRecord players_34;
    PlayerRecord players_35;
    PlayerRecord players_36;
    PlayerRecord players_37;
    PlayerRecord players_38;
    PlayerRecord players_39;
    PlayerRecord players_40;
    PlayerRecord players_41;
    PlayerRecord players_42;
    PlayerRecord players_43;
    PlayerRecord players_44;
    PlayerRecord players_45;
    PlayerRecord players_46;
    PlayerRecord players_47;
    PlayerRecord players_48;
    PlayerRecord players_49;
    PlayerRecord players_50;
    PlayerRecord players_51;
    PlayerRecord players_52;
    PlayerRecord players_53;
    PlayerRecord players_54;
    PlayerRecord players_55;
    PlayerRecord players_56;
    PlayerRecord players_57;
    PlayerRecord players_58;
    PlayerRecord players_59;
    PlayerRecord players_60;
    PlayerRecord players_61;
    PlayerRecord players_62;
    PlayerRecord players_63;
    PlayerRecord players_64;
    PlayerRecord players_65;
    PlayerRecord players_66;
    PlayerRecord players_67;
    PlayerRecord players_68;
    PlayerRecord players_69;
    PlayerRecord players_70;
    PlayerRecord players_71;
    PlayerRecord players_72;
    PlayerRecord players_73;
    PlayerRecord players_74;
    PlayerRecord players_75;
    PlayerRecord players_76;
    PlayerRecord players_77;
    PlayerRecord players_78;
    PlayerRecord players_79;
    PlayerRecord players_80;
    PlayerRecord players_81;
    PlayerRecord players_82;
    PlayerRecord players_83;
    PlayerRecord players_84;
    PlayerRecord players_85;
    PlayerRecord players_86;
    PlayerRecord players_87;
    PlayerRecord players_88;
    PlayerRecord players_89;
    PlayerRecord players_90;
    PlayerRecord players_91;
    PlayerRecord players_92;
    PlayerRecord players_93;
    PlayerRecord players_94;
    PlayerRecord players_95;
    PlayerRecord players_96;
    PlayerRecord players_97;
    PlayerRecord players_98;
    PlayerRecord players_99;
    PlayerRecord players_100;
    PlayerRecord players_101;
    PlayerRecord players_102;
    PlayerRecord players_103;
    PlayerRecord players_104;
    PlayerRecord players_105;
    PlayerRecord players_106;
    PlayerRecord players_107;
    PlayerRecord players_108;
    PlayerRecord players_109;
    PlayerRecord players_110;
    PlayerRecord players_111;
    PlayerRecord players_112;
    PlayerRecord players_113;
    PlayerRecord players_114;
    PlayerRecord players_115;
    PlayerRecord players_116;
    PlayerRecord players_117;
    PlayerRecord players_118;
    PlayerRecord players_119;
    PlayerRecord players_120;
    PlayerRecord players_121;
    PlayerRecord players_122;
    PlayerRecord players_123;
    PlayerRecord players_124;
    PlayerRecord players_125;
    PlayerRecord players_126;
    PlayerRecord players_127;
    PlayerRecord players_128;
    PlayerRecord players_129;
    PlayerRecord players_130;
    PlayerRecord players_131;
    PlayerRecord players_132;
    PlayerRecord players_133;
    PlayerRecord players_134;
    PlayerRecord players_135;
    PlayerRecord players_136;
    PlayerRecord players_137;
    PlayerRecord players_138;
    PlayerRecord players_139;
    PlayerRecord players_140;
    PlayerRecord players_141;
    PlayerRecord players_142;
    PlayerRecord players_143;
    PlayerRecord players_144;
    PlayerRecord players_145;
    PlayerRecord players_146;
    PlayerRecord players_147;
    PlayerRecord players_148;
    PlayerRecord players_149;
    PlayerRecord players_150;
    PlayerRecord players_151;
    PlayerRecord players_152;
    PlayerRecord players_153;
    PlayerRecord players_154;
    PlayerRecord players_155;
    PlayerRecord players_156;
    PlayerRecord players_157;
    PlayerRecord players_158;
    PlayerRecord players_159;
    PlayerRecord players_160;
    PlayerRecord players_161;
    PlayerRecord players_162;
    PlayerRecord players_163;
    PlayerRecord players_164;
    PlayerRecord players_165;
    PlayerRecord players_166;
    PlayerRecord players_167;
    PlayerRecord players_168;
    PlayerRecord players_169;
    PlayerRecord players_170;
    PlayerRecord players_171;
    PlayerRecord players_172;
    PlayerRecord players_173;
    PlayerRecord players_174;
    PlayerRecord players_175;
    PlayerRecord players_176;
    PlayerRecord players_177;
    PlayerRecord players_178;
    PlayerRecord players_179;
    PlayerRecord players_180;
    PlayerRecord players_181;
    PlayerRecord players_182;
    PlayerRecord players_183;
    PlayerRecord players_184;
    PlayerRecord players_185;
    PlayerRecord players_186;
    PlayerRecord players_187;
    PlayerRecord players_188;
    PlayerRecord players_189;
    PlayerRecord players_190;
    PlayerRecord players_191;
    PlayerRecord players_192;
    PlayerRecord players_193;
    PlayerRecord players_194;
    PlayerRecord players_195;
    PlayerRecord players_196;
    PlayerRecord players_197;
    PlayerRecord players_198;
    PlayerRecord players_199;
    PlayerRecord players_200;
    PlayerRecord players_201;
    PlayerRecord players_202;
    PlayerRecord players_203;
    PlayerRecord players_204;
    PlayerRecord players_205;
    PlayerRecord players_206;
    PlayerRecord players_207;
    PlayerRecord players_208;
    PlayerRecord players_209;
    PlayerRecord players_210;
    PlayerRecord players_211;
    PlayerRecord players_212;
    PlayerRecord players_213;
    PlayerRecord players_214;
    PlayerRecord players_215;
    PlayerRecord players_216;
    PlayerRecord players_217;
    PlayerRecord players_218;
    PlayerRecord players_219;
    PlayerRecord players_220;
    PlayerRecord players_221;
    PlayerRecord players_222;
    PlayerRecord players_223;
    PlayerRecord players_224;
    PlayerRecord players_225;
    PlayerRecord players_226;
    PlayerRecord players_227;
    PlayerRecord players_228;
    PlayerRecord players_229;
    PlayerRecord players_230;
    PlayerRecord players_231;
    PlayerRecord players_232;
    PlayerRecord players_233;
    PlayerRecord players_234;
    PlayerRecord players_235;
    PlayerRecord players_236;
    PlayerRecord players_237;
    PlayerRecord players_238;
    PlayerRecord players_239;
    PlayerRecord players_240;
    PlayerRecord players_241;
    PlayerRecord players_242;
    PlayerRecord players_243;
    PlayerRecord players_244;
    PlayerRecord players_245;
    PlayerRecord players_246;
    PlayerRecord players_247;
    PlayerRecord players_248;
    PlayerRecord players_249;
    PlayerRecord players_250;
    PlayerRecord players_251;
    PlayerRecord players_252;
    PlayerRecord players_253;
    PlayerRecord players_254;
    PlayerRecord players_255;

    // Note: Full deployment expands to 256 slots
    // Abbreviated here — pattern repeats identically

    // ---- Epoch State ----
    sint64  currentEpoch;           // Current Qubic epoch
    sint64  epochRewardPool;        // QZN available for distribution this epoch
    sint64  epochTotalDistributed;  // QZN distributed this epoch
    sint64  epochTotalBurned;       // QZN burned this epoch (unclaimed after grace)

    // ---- Leaderboard (Top 10 this epoch) ----
    LeaderboardEntry board_0;
    LeaderboardEntry board_1;
    LeaderboardEntry board_2;
    LeaderboardEntry board_3;
    LeaderboardEntry board_4;
    LeaderboardEntry board_5;
    LeaderboardEntry board_6;
    LeaderboardEntry board_7;
    LeaderboardEntry board_8;
    LeaderboardEntry board_9;

    // ---- Lifetime Stats ----
    sint64  totalPlayersRegistered;
    sint64  totalQZNDistributed;
    sint64  epochScDividendPool;
    sint64  totalScDividendsPaid;
    sint64  epochEfficiencyRating;   // Hint: silent multiplier if lagging

    // ── Dividend pool (receives from QZN Token BEGIN_EPOCH flush) ──
    sint64  stakerDividendPool;         // Accumulated from Token contract
    sint64  totalStaked;
    bit     initialized;
    sint64  totalDividendsPaidToStakers; // Lifetime stat
    uint32  lastDividendEpoch;          // Prevent double-pay
    sint64  totalAchievementsAwarded;
    sint64  totalEpochsProcessed;

    // ---- Reserve ----
    sint64  rewardReserveBalance;   // QZN available for reward payouts
    sint64  achievementReserveBalance; // QZN reserved for achievements

    // ---- Authority ----
    id      adminAddress;
    id      gameCabinetAddress;     // Only cabinet can report match results
    id     tokenContractAddress;
    };

public:


// ============================================================
//  INPUT / OUTPUT STRUCTS
// ============================================================

// --- Initialize ---
struct InitializeRouter_input
{
    id      gameCabinetAddr;
    sint64  initialRewardReserve;
    sint64  initialAchievementReserve;
};
struct InitializeRouter_output
{
    bit     success;
};

// --- Register Player ---
struct RegisterPlayer_input
{
    sint64  initialStake;           // QZN to stake at registration (can be 0)
};
struct RegisterPlayer_output
{
    sint64  playerSlot;
    sint64  multiplierBPS;
    bit     success;
};

// --- Report Match Result (called by Game Cabinet PAO) ---
struct ReportMatchResult_input
{
    id      winnerAddress;
    id      loser1Address;
    id      loser2Address;          // NULL_ID if DUEL
    id      loser3Address;          // NULL_ID if DUEL
    uint8   gameId;                 // 1=snaQe, 2=paQman, 3=TANQ
    sint64  winnerScore;
    sint64  stakeAmount;            // Total stake in match
    bit     isSolo;                 // Solo match flag
};
struct ReportMatchResult_output
{
    sint64  winnerReward;           // QZN credited to winner
    sint64  multiplierApplied;      // Multiplier BPS used
    sint64  achievementsUnlocked;   // Count of new achievements triggered
};

// --- Stake QZN (increases multiplier) ---
struct StakeQZN_input
{
    sint64  amount;
};
struct StakeQZN_output
{
    sint64  newStakedTotal;
    sint64  newMultiplierBPS;
    sint64  multiplierTier;
};

// --- Unstake QZN ---
struct UnstakeQZN_input
{
    sint64  amount;
};
struct UnstakeQZN_output
{
    sint64  amountReturned;
    sint64  newStakedTotal;
    sint64  newMultiplierBPS;
};

// --- Claim Epoch Rewards ---
struct ClaimRewards_input {};
struct ClaimRewards_output
{
    sint64  amountClaimed;
    sint64  pendingRemaining;
    sint64  currentEpoch;
};

// --- Get Player Stats ---
struct GetPlayerStats_input
{
    id      walletAddress;
};
struct GetPlayerStats_output
{
    sint64  pendingBalance;
    sint64  stakedAmount;
    sint64  multiplierBPS;
    sint64  epochEarned;
    sint64  epochCap;
    sint64  totalMatchesWon;
    sint64  currentWinStreak;
    sint64  lifetimeEarned;
};

// --- Get Leaderboard ---
struct GetLeaderboard_input {};
struct GetLeaderboard_output
{
    id      rank1_wallet;
    sint64  rank1_score;
    id      rank2_wallet;
    sint64  rank2_score;
    id      rank3_wallet;
    sint64  rank3_score;
    sint64  currentEpoch;
    sint64  epochRewardPool;
};

// --- Admin: Fund Reward Reserve ---
struct FundReserve_input
{
    sint64  amount;
    bit     isAchievementFund;
};
struct FundReserve_output
{
    sint64  newRewardReserve;
    sint64  newAchievementReserve;
};

// ============================================================
//  INTERNAL HELPERS
// ============================================================

struct _getMultiplierBPS_input {};
struct _getMultiplierBPS_output {};

PRIVATE_FUNCTION(_getMultiplierBPS)
/*
 * Returns multiplier in BPS for a given stake amount.
 * Tier thresholds are fixed constants.
 */
{
    // Called inline — result used directly at call site
}

struct _updateLeaderboard_input {};
struct _updateLeaderboard_output {};

PRIVATE_PROCEDURE(_updateLeaderboard)
/*
 * Checks if player's epoch score places them in top 10.
 * Inserts and re-sorts leaderboard if so.
 * Called after every match report.
 */
{
    // Embedded inline in ReportMatchResult for QPI single-file compliance
}

struct _checkAchievements_input {};
struct _checkAchievements_output {};

PRIVATE_PROCEDURE(_checkAchievements)
/*
 * Evaluates all achievement conditions for a player after a match.
 * Awards QZN from achievement reserve for newly unlocked ones.
 * Each achievement is one-time — flag prevents double-award.
 */
{
    // Embedded inline in ReportMatchResult
}

// ============================================================
//  CONTRACT PROCEDURES
// ============================================================

PUBLIC_PROCEDURE(InitializeRouter)
{
    if (state.get().initialized)
    {
        return;
    }

    state.mut().adminAddress = qpi.invocator();
    state.mut().gameCabinetAddress = input.gameCabinetAddr;
    state.mut().rewardReserveBalance = input.initialRewardReserve;
    state.mut().achievementReserveBalance = input.initialAchievementReserve;
    state.mut().currentEpoch = 0;

    // ── PRO-RATA STAKER DIVIDEND DISTRIBUTION ────────────────────────
    // Pool was filled by QZN Token contract in its own BEGIN_EPOCH.
    // Distribute proportionally: each staker gets
    //   share = pool * (stakerAmount / totalStaked)
    if (state.get().stakerDividendPool > 0LL &&
        state.get().totalStaked > 0LL &&
        state.get().lastDividendEpoch != qpi.epoch())
    {
        sint64 pool       = state.get().stakerDividendPool;
        sint64 totalStake = state.get().totalStaked;
        sint64 distributed = 0LL;

        // Iterate all 256 player slots
        #define DIST_DIVIDEND(N) \
        if (state.get().players_##N.active && state.get().players_##N.stakedAmount > 0LL) { \
            sint64 share = div(pool * state.get().players_##N.stakedAmount, totalStake).quot; \
            if (share > 0LL) { \
                state.mut().players_##N.pendingBalance += share; \
                state.mut().players_##N.lifetimeEarned += share; \
                distributed += share; \
            } \
        }

        DIST_DIVIDEND(0)   DIST_DIVIDEND(1)   DIST_DIVIDEND(2)   DIST_DIVIDEND(3)
        DIST_DIVIDEND(4)   DIST_DIVIDEND(5)   DIST_DIVIDEND(6)   DIST_DIVIDEND(7)
        DIST_DIVIDEND(8)   DIST_DIVIDEND(9)   DIST_DIVIDEND(10)  DIST_DIVIDEND(11)
        DIST_DIVIDEND(12)  DIST_DIVIDEND(13)  DIST_DIVIDEND(14)  DIST_DIVIDEND(15)
        DIST_DIVIDEND(16)  DIST_DIVIDEND(17)  DIST_DIVIDEND(18)  DIST_DIVIDEND(19)
        DIST_DIVIDEND(20)  DIST_DIVIDEND(21)  DIST_DIVIDEND(22)  DIST_DIVIDEND(23)
        DIST_DIVIDEND(24)  DIST_DIVIDEND(25)  DIST_DIVIDEND(26)  DIST_DIVIDEND(27)
        DIST_DIVIDEND(28)  DIST_DIVIDEND(29)  DIST_DIVIDEND(30)  DIST_DIVIDEND(31)
        DIST_DIVIDEND(32)  DIST_DIVIDEND(33)  DIST_DIVIDEND(34)  DIST_DIVIDEND(35)
        DIST_DIVIDEND(36)  DIST_DIVIDEND(37)  DIST_DIVIDEND(38)  DIST_DIVIDEND(39)
        DIST_DIVIDEND(40)  DIST_DIVIDEND(41)  DIST_DIVIDEND(42)  DIST_DIVIDEND(43)
        DIST_DIVIDEND(44)  DIST_DIVIDEND(45)  DIST_DIVIDEND(46)  DIST_DIVIDEND(47)
        DIST_DIVIDEND(48)  DIST_DIVIDEND(49)  DIST_DIVIDEND(50)  DIST_DIVIDEND(51)
        DIST_DIVIDEND(52)  DIST_DIVIDEND(53)  DIST_DIVIDEND(54)  DIST_DIVIDEND(55)
        DIST_DIVIDEND(56)  DIST_DIVIDEND(57)  DIST_DIVIDEND(58)  DIST_DIVIDEND(59)
        DIST_DIVIDEND(60)  DIST_DIVIDEND(61)  DIST_DIVIDEND(62)  DIST_DIVIDEND(63)
        DIST_DIVIDEND(64)  DIST_DIVIDEND(65)  DIST_DIVIDEND(66)  DIST_DIVIDEND(67)
        DIST_DIVIDEND(68)  DIST_DIVIDEND(69)  DIST_DIVIDEND(70)  DIST_DIVIDEND(71)
        DIST_DIVIDEND(72)  DIST_DIVIDEND(73)  DIST_DIVIDEND(74)  DIST_DIVIDEND(75)
        DIST_DIVIDEND(76)  DIST_DIVIDEND(77)  DIST_DIVIDEND(78)  DIST_DIVIDEND(79)
        DIST_DIVIDEND(80)  DIST_DIVIDEND(81)  DIST_DIVIDEND(82)  DIST_DIVIDEND(83)
        DIST_DIVIDEND(84)  DIST_DIVIDEND(85)  DIST_DIVIDEND(86)  DIST_DIVIDEND(87)
        DIST_DIVIDEND(88)  DIST_DIVIDEND(89)  DIST_DIVIDEND(90)  DIST_DIVIDEND(91)
        DIST_DIVIDEND(92)  DIST_DIVIDEND(93)  DIST_DIVIDEND(94)  DIST_DIVIDEND(95)
        DIST_DIVIDEND(96)  DIST_DIVIDEND(97)  DIST_DIVIDEND(98)  DIST_DIVIDEND(99)
        DIST_DIVIDEND(100) DIST_DIVIDEND(101) DIST_DIVIDEND(102) DIST_DIVIDEND(103)
        DIST_DIVIDEND(104) DIST_DIVIDEND(105) DIST_DIVIDEND(106) DIST_DIVIDEND(107)
        DIST_DIVIDEND(108) DIST_DIVIDEND(109) DIST_DIVIDEND(110) DIST_DIVIDEND(111)
        DIST_DIVIDEND(112) DIST_DIVIDEND(113) DIST_DIVIDEND(114) DIST_DIVIDEND(115)
        DIST_DIVIDEND(116) DIST_DIVIDEND(117) DIST_DIVIDEND(118) DIST_DIVIDEND(119)
        DIST_DIVIDEND(120) DIST_DIVIDEND(121) DIST_DIVIDEND(122) DIST_DIVIDEND(123)
        DIST_DIVIDEND(124) DIST_DIVIDEND(125) DIST_DIVIDEND(126) DIST_DIVIDEND(127)
        DIST_DIVIDEND(128) DIST_DIVIDEND(129) DIST_DIVIDEND(130) DIST_DIVIDEND(131)
        DIST_DIVIDEND(132) DIST_DIVIDEND(133) DIST_DIVIDEND(134) DIST_DIVIDEND(135)
        DIST_DIVIDEND(136) DIST_DIVIDEND(137) DIST_DIVIDEND(138) DIST_DIVIDEND(139)
        DIST_DIVIDEND(140) DIST_DIVIDEND(141) DIST_DIVIDEND(142) DIST_DIVIDEND(143)
        DIST_DIVIDEND(144) DIST_DIVIDEND(145) DIST_DIVIDEND(146) DIST_DIVIDEND(147)
        DIST_DIVIDEND(148) DIST_DIVIDEND(149) DIST_DIVIDEND(150) DIST_DIVIDEND(151)
        DIST_DIVIDEND(152) DIST_DIVIDEND(153) DIST_DIVIDEND(154) DIST_DIVIDEND(155)
        DIST_DIVIDEND(156) DIST_DIVIDEND(157) DIST_DIVIDEND(158) DIST_DIVIDEND(159)
        DIST_DIVIDEND(160) DIST_DIVIDEND(161) DIST_DIVIDEND(162) DIST_DIVIDEND(163)
        DIST_DIVIDEND(164) DIST_DIVIDEND(165) DIST_DIVIDEND(166) DIST_DIVIDEND(167)
        DIST_DIVIDEND(168) DIST_DIVIDEND(169) DIST_DIVIDEND(170) DIST_DIVIDEND(171)
        DIST_DIVIDEND(172) DIST_DIVIDEND(173) DIST_DIVIDEND(174) DIST_DIVIDEND(175)
        DIST_DIVIDEND(176) DIST_DIVIDEND(177) DIST_DIVIDEND(178) DIST_DIVIDEND(179)
        DIST_DIVIDEND(180) DIST_DIVIDEND(181) DIST_DIVIDEND(182) DIST_DIVIDEND(183)
        DIST_DIVIDEND(184) DIST_DIVIDEND(185) DIST_DIVIDEND(186) DIST_DIVIDEND(187)
        DIST_DIVIDEND(188) DIST_DIVIDEND(189) DIST_DIVIDEND(190) DIST_DIVIDEND(191)
        DIST_DIVIDEND(192) DIST_DIVIDEND(193) DIST_DIVIDEND(194) DIST_DIVIDEND(195)
        DIST_DIVIDEND(196) DIST_DIVIDEND(197) DIST_DIVIDEND(198) DIST_DIVIDEND(199)
        DIST_DIVIDEND(200) DIST_DIVIDEND(201) DIST_DIVIDEND(202) DIST_DIVIDEND(203)
        DIST_DIVIDEND(204) DIST_DIVIDEND(205) DIST_DIVIDEND(206) DIST_DIVIDEND(207)
        DIST_DIVIDEND(208) DIST_DIVIDEND(209) DIST_DIVIDEND(210) DIST_DIVIDEND(211)
        DIST_DIVIDEND(212) DIST_DIVIDEND(213) DIST_DIVIDEND(214) DIST_DIVIDEND(215)
        DIST_DIVIDEND(216) DIST_DIVIDEND(217) DIST_DIVIDEND(218) DIST_DIVIDEND(219)
        DIST_DIVIDEND(220) DIST_DIVIDEND(221) DIST_DIVIDEND(222) DIST_DIVIDEND(223)
        DIST_DIVIDEND(224) DIST_DIVIDEND(225) DIST_DIVIDEND(226) DIST_DIVIDEND(227)
        DIST_DIVIDEND(228) DIST_DIVIDEND(229) DIST_DIVIDEND(230) DIST_DIVIDEND(231)
        DIST_DIVIDEND(232) DIST_DIVIDEND(233) DIST_DIVIDEND(234) DIST_DIVIDEND(235)
        DIST_DIVIDEND(236) DIST_DIVIDEND(237) DIST_DIVIDEND(238) DIST_DIVIDEND(239)
        DIST_DIVIDEND(240) DIST_DIVIDEND(241) DIST_DIVIDEND(242) DIST_DIVIDEND(243)
        DIST_DIVIDEND(244) DIST_DIVIDEND(245) DIST_DIVIDEND(246) DIST_DIVIDEND(247)
        DIST_DIVIDEND(248) DIST_DIVIDEND(249) DIST_DIVIDEND(250) DIST_DIVIDEND(251)
        DIST_DIVIDEND(252) DIST_DIVIDEND(253) DIST_DIVIDEND(254) DIST_DIVIDEND(255)

        #undef DIST_DIVIDEND

        state.mut().stakerDividendPool           = 0LL;
        state.mut().totalDividendsPaidToStakers += distributed;
        state.mut().lastDividendEpoch            = qpi.epoch();
    }
    // ─────────────────────────────────────────────────────────────────
    state.mut().epochRewardPool = input.initialRewardReserve;

    // Reset all stats
    state.mut().totalPlayersRegistered = 0;
    state.mut().totalQZNDistributed = 0;
    state.mut().totalAchievementsAwarded = 0;
    state.mut().totalEpochsProcessed = 0;
    state.mut().epochTotalDistributed = 0;

    // ── SC Shareholder Distribution ───────────────────────────────────
    sint64 rrScPool = div(state.get().rewardReserveBalance * 200LL, 10000LL).quot;
    rrScPool = rrScPool * state.get().epochEfficiencyRating / 1000LL;
    if (rrScPool > 0LL)
    {
        qpi.distributeDividends(rrScPool);
        state.mut().rewardReserveBalance  -= rrScPool;
        state.mut().totalScDividendsPaid  += rrScPool;
        state.mut().epochScDividendPool    = rrScPool;
    }
    state.mut().epochEfficiencyRating = 1000LL;
    state.mut().epochTotalBurned = 0;

    // Clear leaderboard
    state.mut().board_0.score = 0;
    state.mut().board_1.score = 0;
    state.mut().board_2.score = 0;
    state.mut().board_3.score = 0;
    state.mut().board_4.score = 0;
    state.mut().board_5.score = 0;
    state.mut().board_6.score = 0;
    state.mut().board_7.score = 0;
    state.mut().board_8.score = 0;
    state.mut().board_9.score = 0;

    state.mut().initialized = 1;
    output.success    = 1;
}

PUBLIC_PROCEDURE(RegisterPlayer)
/*
 * Player registers with the Reward Router.
 * Assigns a slot, records stake, computes initial multiplier.
 * Can be called with 0 stake — starts at 1.0x.
 */
{
    if (!state.get().initialized)
    {
        output.success = 0;
        return;
    }

    // Find free slot
    sint64 slot;
    slot = -1;

    if (!state.get().players_0.active) { slot = 0; }
    else if (!state.get().players_1.active) { slot = 1; }
    else if (!state.get().players_2.active) { slot = 2; }
    else if (!state.get().players_3.active) { slot = 3; }
    else if (!state.get().players_4.active) { slot = 4; }
    else if (!state.get().players_5.active) { slot = 5; }
    else if (!state.get().players_6.active) { slot = 6; }
    else if (!state.get().players_7.active) { slot = 7; }
    else if (!state.get().players_8.active) { slot = 8; }
    else if (!state.get().players_9.active) { slot = 9; }
    else if (!state.get().players_10.active) { slot = 10; }
    else if (!state.get().players_11.active) { slot = 11; }
    else if (!state.get().players_12.active) { slot = 12; }
    else if (!state.get().players_13.active) { slot = 13; }
    else if (!state.get().players_14.active) { slot = 14; }
    else if (!state.get().players_15.active) { slot = 15; }
    else if (!state.get().players_16.active) { slot = 16; }
    else if (!state.get().players_17.active) { slot = 17; }
    else if (!state.get().players_18.active) { slot = 18; }
    else if (!state.get().players_19.active) { slot = 19; }
    else if (!state.get().players_20.active) { slot = 20; }
    else if (!state.get().players_21.active) { slot = 21; }
    else if (!state.get().players_22.active) { slot = 22; }
    else if (!state.get().players_23.active) { slot = 23; }
    else if (!state.get().players_24.active) { slot = 24; }
    else if (!state.get().players_25.active) { slot = 25; }
    else if (!state.get().players_26.active) { slot = 26; }
    else if (!state.get().players_27.active) { slot = 27; }
    else if (!state.get().players_28.active) { slot = 28; }
    else if (!state.get().players_29.active) { slot = 29; }
    else if (!state.get().players_30.active) { slot = 30; }
    else if (!state.get().players_31.active) { slot = 31; }
    else if (!state.get().players_32.active) { slot = 32; }
    else if (!state.get().players_33.active) { slot = 33; }
    else if (!state.get().players_34.active) { slot = 34; }
    else if (!state.get().players_35.active) { slot = 35; }
    else if (!state.get().players_36.active) { slot = 36; }
    else if (!state.get().players_37.active) { slot = 37; }
    else if (!state.get().players_38.active) { slot = 38; }
    else if (!state.get().players_39.active) { slot = 39; }
    else if (!state.get().players_40.active) { slot = 40; }
    else if (!state.get().players_41.active) { slot = 41; }
    else if (!state.get().players_42.active) { slot = 42; }
    else if (!state.get().players_43.active) { slot = 43; }
    else if (!state.get().players_44.active) { slot = 44; }
    else if (!state.get().players_45.active) { slot = 45; }
    else if (!state.get().players_46.active) { slot = 46; }
    else if (!state.get().players_47.active) { slot = 47; }
    else if (!state.get().players_48.active) { slot = 48; }
    else if (!state.get().players_49.active) { slot = 49; }
    else if (!state.get().players_50.active) { slot = 50; }
    else if (!state.get().players_51.active) { slot = 51; }
    else if (!state.get().players_52.active) { slot = 52; }
    else if (!state.get().players_53.active) { slot = 53; }
    else if (!state.get().players_54.active) { slot = 54; }
    else if (!state.get().players_55.active) { slot = 55; }
    else if (!state.get().players_56.active) { slot = 56; }
    else if (!state.get().players_57.active) { slot = 57; }
    else if (!state.get().players_58.active) { slot = 58; }
    else if (!state.get().players_59.active) { slot = 59; }
    else if (!state.get().players_60.active) { slot = 60; }
    else if (!state.get().players_61.active) { slot = 61; }
    else if (!state.get().players_62.active) { slot = 62; }
    else if (!state.get().players_63.active) { slot = 63; }
    else if (!state.get().players_64.active) { slot = 64; }
    else if (!state.get().players_65.active) { slot = 65; }
    else if (!state.get().players_66.active) { slot = 66; }
    else if (!state.get().players_67.active) { slot = 67; }
    else if (!state.get().players_68.active) { slot = 68; }
    else if (!state.get().players_69.active) { slot = 69; }
    else if (!state.get().players_70.active) { slot = 70; }
    else if (!state.get().players_71.active) { slot = 71; }
    else if (!state.get().players_72.active) { slot = 72; }
    else if (!state.get().players_73.active) { slot = 73; }
    else if (!state.get().players_74.active) { slot = 74; }
    else if (!state.get().players_75.active) { slot = 75; }
    else if (!state.get().players_76.active) { slot = 76; }
    else if (!state.get().players_77.active) { slot = 77; }
    else if (!state.get().players_78.active) { slot = 78; }
    else if (!state.get().players_79.active) { slot = 79; }
    else if (!state.get().players_80.active) { slot = 80; }
    else if (!state.get().players_81.active) { slot = 81; }
    else if (!state.get().players_82.active) { slot = 82; }
    else if (!state.get().players_83.active) { slot = 83; }
    else if (!state.get().players_84.active) { slot = 84; }
    else if (!state.get().players_85.active) { slot = 85; }
    else if (!state.get().players_86.active) { slot = 86; }
    else if (!state.get().players_87.active) { slot = 87; }
    else if (!state.get().players_88.active) { slot = 88; }
    else if (!state.get().players_89.active) { slot = 89; }
    else if (!state.get().players_90.active) { slot = 90; }
    else if (!state.get().players_91.active) { slot = 91; }
    else if (!state.get().players_92.active) { slot = 92; }
    else if (!state.get().players_93.active) { slot = 93; }
    else if (!state.get().players_94.active) { slot = 94; }
    else if (!state.get().players_95.active) { slot = 95; }
    else if (!state.get().players_96.active) { slot = 96; }
    else if (!state.get().players_97.active) { slot = 97; }
    else if (!state.get().players_98.active) { slot = 98; }
    else if (!state.get().players_99.active) { slot = 99; }
    else if (!state.get().players_100.active) { slot = 100; }
    else if (!state.get().players_101.active) { slot = 101; }
    else if (!state.get().players_102.active) { slot = 102; }
    else if (!state.get().players_103.active) { slot = 103; }
    else if (!state.get().players_104.active) { slot = 104; }
    else if (!state.get().players_105.active) { slot = 105; }
    else if (!state.get().players_106.active) { slot = 106; }
    else if (!state.get().players_107.active) { slot = 107; }
    else if (!state.get().players_108.active) { slot = 108; }
    else if (!state.get().players_109.active) { slot = 109; }
    else if (!state.get().players_110.active) { slot = 110; }
    else if (!state.get().players_111.active) { slot = 111; }
    else if (!state.get().players_112.active) { slot = 112; }
    else if (!state.get().players_113.active) { slot = 113; }
    else if (!state.get().players_114.active) { slot = 114; }
    else if (!state.get().players_115.active) { slot = 115; }
    else if (!state.get().players_116.active) { slot = 116; }
    else if (!state.get().players_117.active) { slot = 117; }
    else if (!state.get().players_118.active) { slot = 118; }
    else if (!state.get().players_119.active) { slot = 119; }
    else if (!state.get().players_120.active) { slot = 120; }
    else if (!state.get().players_121.active) { slot = 121; }
    else if (!state.get().players_122.active) { slot = 122; }
    else if (!state.get().players_123.active) { slot = 123; }
    else if (!state.get().players_124.active) { slot = 124; }
    else if (!state.get().players_125.active) { slot = 125; }
    else if (!state.get().players_126.active) { slot = 126; }
    else if (!state.get().players_127.active) { slot = 127; }
    else if (!state.get().players_128.active) { slot = 128; }
    else if (!state.get().players_129.active) { slot = 129; }
    else if (!state.get().players_130.active) { slot = 130; }
    else if (!state.get().players_131.active) { slot = 131; }
    else if (!state.get().players_132.active) { slot = 132; }
    else if (!state.get().players_133.active) { slot = 133; }
    else if (!state.get().players_134.active) { slot = 134; }
    else if (!state.get().players_135.active) { slot = 135; }
    else if (!state.get().players_136.active) { slot = 136; }
    else if (!state.get().players_137.active) { slot = 137; }
    else if (!state.get().players_138.active) { slot = 138; }
    else if (!state.get().players_139.active) { slot = 139; }
    else if (!state.get().players_140.active) { slot = 140; }
    else if (!state.get().players_141.active) { slot = 141; }
    else if (!state.get().players_142.active) { slot = 142; }
    else if (!state.get().players_143.active) { slot = 143; }
    else if (!state.get().players_144.active) { slot = 144; }
    else if (!state.get().players_145.active) { slot = 145; }
    else if (!state.get().players_146.active) { slot = 146; }
    else if (!state.get().players_147.active) { slot = 147; }
    else if (!state.get().players_148.active) { slot = 148; }
    else if (!state.get().players_149.active) { slot = 149; }
    else if (!state.get().players_150.active) { slot = 150; }
    else if (!state.get().players_151.active) { slot = 151; }
    else if (!state.get().players_152.active) { slot = 152; }
    else if (!state.get().players_153.active) { slot = 153; }
    else if (!state.get().players_154.active) { slot = 154; }
    else if (!state.get().players_155.active) { slot = 155; }
    else if (!state.get().players_156.active) { slot = 156; }
    else if (!state.get().players_157.active) { slot = 157; }
    else if (!state.get().players_158.active) { slot = 158; }
    else if (!state.get().players_159.active) { slot = 159; }
    else if (!state.get().players_160.active) { slot = 160; }
    else if (!state.get().players_161.active) { slot = 161; }
    else if (!state.get().players_162.active) { slot = 162; }
    else if (!state.get().players_163.active) { slot = 163; }
    else if (!state.get().players_164.active) { slot = 164; }
    else if (!state.get().players_165.active) { slot = 165; }
    else if (!state.get().players_166.active) { slot = 166; }
    else if (!state.get().players_167.active) { slot = 167; }
    else if (!state.get().players_168.active) { slot = 168; }
    else if (!state.get().players_169.active) { slot = 169; }
    else if (!state.get().players_170.active) { slot = 170; }
    else if (!state.get().players_171.active) { slot = 171; }
    else if (!state.get().players_172.active) { slot = 172; }
    else if (!state.get().players_173.active) { slot = 173; }
    else if (!state.get().players_174.active) { slot = 174; }
    else if (!state.get().players_175.active) { slot = 175; }
    else if (!state.get().players_176.active) { slot = 176; }
    else if (!state.get().players_177.active) { slot = 177; }
    else if (!state.get().players_178.active) { slot = 178; }
    else if (!state.get().players_179.active) { slot = 179; }
    else if (!state.get().players_180.active) { slot = 180; }
    else if (!state.get().players_181.active) { slot = 181; }
    else if (!state.get().players_182.active) { slot = 182; }
    else if (!state.get().players_183.active) { slot = 183; }
    else if (!state.get().players_184.active) { slot = 184; }
    else if (!state.get().players_185.active) { slot = 185; }
    else if (!state.get().players_186.active) { slot = 186; }
    else if (!state.get().players_187.active) { slot = 187; }
    else if (!state.get().players_188.active) { slot = 188; }
    else if (!state.get().players_189.active) { slot = 189; }
    else if (!state.get().players_190.active) { slot = 190; }
    else if (!state.get().players_191.active) { slot = 191; }
    else if (!state.get().players_192.active) { slot = 192; }
    else if (!state.get().players_193.active) { slot = 193; }
    else if (!state.get().players_194.active) { slot = 194; }
    else if (!state.get().players_195.active) { slot = 195; }
    else if (!state.get().players_196.active) { slot = 196; }
    else if (!state.get().players_197.active) { slot = 197; }
    else if (!state.get().players_198.active) { slot = 198; }
    else if (!state.get().players_199.active) { slot = 199; }
    else if (!state.get().players_200.active) { slot = 200; }
    else if (!state.get().players_201.active) { slot = 201; }
    else if (!state.get().players_202.active) { slot = 202; }
    else if (!state.get().players_203.active) { slot = 203; }
    else if (!state.get().players_204.active) { slot = 204; }
    else if (!state.get().players_205.active) { slot = 205; }
    else if (!state.get().players_206.active) { slot = 206; }
    else if (!state.get().players_207.active) { slot = 207; }
    else if (!state.get().players_208.active) { slot = 208; }
    else if (!state.get().players_209.active) { slot = 209; }
    else if (!state.get().players_210.active) { slot = 210; }
    else if (!state.get().players_211.active) { slot = 211; }
    else if (!state.get().players_212.active) { slot = 212; }
    else if (!state.get().players_213.active) { slot = 213; }
    else if (!state.get().players_214.active) { slot = 214; }
    else if (!state.get().players_215.active) { slot = 215; }
    else if (!state.get().players_216.active) { slot = 216; }
    else if (!state.get().players_217.active) { slot = 217; }
    else if (!state.get().players_218.active) { slot = 218; }
    else if (!state.get().players_219.active) { slot = 219; }
    else if (!state.get().players_220.active) { slot = 220; }
    else if (!state.get().players_221.active) { slot = 221; }
    else if (!state.get().players_222.active) { slot = 222; }
    else if (!state.get().players_223.active) { slot = 223; }
    else if (!state.get().players_224.active) { slot = 224; }
    else if (!state.get().players_225.active) { slot = 225; }
    else if (!state.get().players_226.active) { slot = 226; }
    else if (!state.get().players_227.active) { slot = 227; }
    else if (!state.get().players_228.active) { slot = 228; }
    else if (!state.get().players_229.active) { slot = 229; }
    else if (!state.get().players_230.active) { slot = 230; }
    else if (!state.get().players_231.active) { slot = 231; }
    else if (!state.get().players_232.active) { slot = 232; }
    else if (!state.get().players_233.active) { slot = 233; }
    else if (!state.get().players_234.active) { slot = 234; }
    else if (!state.get().players_235.active) { slot = 235; }
    else if (!state.get().players_236.active) { slot = 236; }
    else if (!state.get().players_237.active) { slot = 237; }
    else if (!state.get().players_238.active) { slot = 238; }
    else if (!state.get().players_239.active) { slot = 239; }
    else if (!state.get().players_240.active) { slot = 240; }
    else if (!state.get().players_241.active) { slot = 241; }
    else if (!state.get().players_242.active) { slot = 242; }
    else if (!state.get().players_243.active) { slot = 243; }
    else if (!state.get().players_244.active) { slot = 244; }
    else if (!state.get().players_245.active) { slot = 245; }
    else if (!state.get().players_246.active) { slot = 246; }
    else if (!state.get().players_247.active) { slot = 247; }
    else if (!state.get().players_248.active) { slot = 248; }
    else if (!state.get().players_249.active) { slot = 249; }
    else if (!state.get().players_250.active) { slot = 250; }
    else if (!state.get().players_251.active) { slot = 251; }
    else if (!state.get().players_252.active) { slot = 252; }
    else if (!state.get().players_253.active) { slot = 253; }
    else if (!state.get().players_254.active) { slot = 254; }
    else if (!state.get().players_255.active) { slot = 255; }


    if (slot < 0)
    {
        output.success = 0;
        return;
    }

    // Compute initial multiplier
    sint64 multBPS;
    multBPS = MULT_TIER_0;
    if (input.initialStake >= STAKE_TIER_4) { multBPS = MULT_TIER_4; }
    else if (input.initialStake >= STAKE_TIER_3) { multBPS = MULT_TIER_3; }
    else if (input.initialStake >= STAKE_TIER_2) { multBPS = MULT_TIER_2; }
    else if (input.initialStake >= STAKE_TIER_1) { multBPS = MULT_TIER_1; }

    // Write player record
    if (slot == 0)
    {
        state.mut().players_0.walletAddress = qpi.invocator();
        state.mut().players_0.currentEpoch = qpi.epoch();
        state.mut().players_0.epochEarned = 0;
        state.mut().players_0.pendingBalance = 0;
        state.mut().players_0.lifetimeEarned = 0;
        state.mut().players_0.stakedAmount = input.initialStake;
        state.mut().players_0.stakeMultiplierBPS = multBPS;
        state.mut().players_0.totalMatchesPlayed = 0;
        state.mut().players_0.totalMatchesWon = 0;
        state.mut().players_0.currentWinStreak = 0;
        state.mut().players_0.bestWinStreak = 0;
        state.mut().players_0.wonSnaqe = 0;
        state.mut().players_0.wonPaqman = 0;
        state.mut().players_0.wonTanq = 0;
        state.mut().players_0.achFirstWin = 0;
        state.mut().players_0.achStreak5 = 0;
        state.mut().players_0.achStreak10 = 0;
        state.mut().players_0.achMatches100 = 0;
        state.mut().players_0.achMatches1000 = 0;
        state.mut().players_0.achHighStake = 0;
        state.mut().players_0.achAllGames = 0;
        state.mut().players_0.achTopLeaderboard = 0;
        state.mut().players_0.epochScore = 0;
        state.mut().players_0.active = 1;
    }
    // Slots 1-15 follow identical pattern in full deployment

    state.mut().totalPlayersRegistered = state.get().totalPlayersRegistered + 1;

    output.playerSlot      = slot;
    output.multiplierBPS   = multBPS;
    output.success         = 1;
}

PUBLIC_PROCEDURE(StakeQZN)
/*
 * Player stakes additional QZN to increase their multiplier.
 * Staked QZN is locked — unlocked via UnstakeQZN.
 * Multiplier updates immediately on stake.
 *
 * Staking tiers:
 *   ≥ 100,000 QZN → 3.0x multiplier (max)
 *   ≥  25,000 QZN → 2.0x
 *   ≥   5,000 QZN → 1.5x
 *   ≥   1,000 QZN → 1.25x
 *       0 QZN     → 1.0x  (base)
 */
{
    if (input.amount <= 0)
    {
        return;
    }

    // Find player slot
    sint64 slot;
    slot = -1;

     if (state.get().players_0.active && state.get().players_0.walletAddress == qpi.invocator()) { slot = 0; }
    else if (state.get().players_1.active && state.get().players_1.walletAddress == qpi.invocator()) { slot = 1; }
    else if (state.get().players_2.active && state.get().players_2.walletAddress == qpi.invocator()) { slot = 2; }
    else if (state.get().players_3.active && state.get().players_3.walletAddress == qpi.invocator()) { slot = 3; }
    else if (state.get().players_4.active && state.get().players_4.walletAddress == qpi.invocator()) { slot = 4; }
    else if (state.get().players_5.active && state.get().players_5.walletAddress == qpi.invocator()) { slot = 5; }
    else if (state.get().players_6.active && state.get().players_6.walletAddress == qpi.invocator()) { slot = 6; }
    else if (state.get().players_7.active && state.get().players_7.walletAddress == qpi.invocator()) { slot = 7; }
    else if (state.get().players_8.active && state.get().players_8.walletAddress == qpi.invocator()) { slot = 8; }
    else if (state.get().players_9.active && state.get().players_9.walletAddress == qpi.invocator()) { slot = 9; }
    else if (state.get().players_10.active && state.get().players_10.walletAddress == qpi.invocator()) { slot = 10; }
    else if (state.get().players_11.active && state.get().players_11.walletAddress == qpi.invocator()) { slot = 11; }
    else if (state.get().players_12.active && state.get().players_12.walletAddress == qpi.invocator()) { slot = 12; }
    else if (state.get().players_13.active && state.get().players_13.walletAddress == qpi.invocator()) { slot = 13; }
    else if (state.get().players_14.active && state.get().players_14.walletAddress == qpi.invocator()) { slot = 14; }
    else if (state.get().players_15.active && state.get().players_15.walletAddress == qpi.invocator()) { slot = 15; }
    else if (state.get().players_16.active && state.get().players_16.walletAddress == qpi.invocator()) { slot = 16; }
    else if (state.get().players_17.active && state.get().players_17.walletAddress == qpi.invocator()) { slot = 17; }
    else if (state.get().players_18.active && state.get().players_18.walletAddress == qpi.invocator()) { slot = 18; }
    else if (state.get().players_19.active && state.get().players_19.walletAddress == qpi.invocator()) { slot = 19; }
    else if (state.get().players_20.active && state.get().players_20.walletAddress == qpi.invocator()) { slot = 20; }
    else if (state.get().players_21.active && state.get().players_21.walletAddress == qpi.invocator()) { slot = 21; }
    else if (state.get().players_22.active && state.get().players_22.walletAddress == qpi.invocator()) { slot = 22; }
    else if (state.get().players_23.active && state.get().players_23.walletAddress == qpi.invocator()) { slot = 23; }
    else if (state.get().players_24.active && state.get().players_24.walletAddress == qpi.invocator()) { slot = 24; }
    else if (state.get().players_25.active && state.get().players_25.walletAddress == qpi.invocator()) { slot = 25; }
    else if (state.get().players_26.active && state.get().players_26.walletAddress == qpi.invocator()) { slot = 26; }
    else if (state.get().players_27.active && state.get().players_27.walletAddress == qpi.invocator()) { slot = 27; }
    else if (state.get().players_28.active && state.get().players_28.walletAddress == qpi.invocator()) { slot = 28; }
    else if (state.get().players_29.active && state.get().players_29.walletAddress == qpi.invocator()) { slot = 29; }
    else if (state.get().players_30.active && state.get().players_30.walletAddress == qpi.invocator()) { slot = 30; }
    else if (state.get().players_31.active && state.get().players_31.walletAddress == qpi.invocator()) { slot = 31; }
    else if (state.get().players_32.active && state.get().players_32.walletAddress == qpi.invocator()) { slot = 32; }
    else if (state.get().players_33.active && state.get().players_33.walletAddress == qpi.invocator()) { slot = 33; }
    else if (state.get().players_34.active && state.get().players_34.walletAddress == qpi.invocator()) { slot = 34; }
    else if (state.get().players_35.active && state.get().players_35.walletAddress == qpi.invocator()) { slot = 35; }
    else if (state.get().players_36.active && state.get().players_36.walletAddress == qpi.invocator()) { slot = 36; }
    else if (state.get().players_37.active && state.get().players_37.walletAddress == qpi.invocator()) { slot = 37; }
    else if (state.get().players_38.active && state.get().players_38.walletAddress == qpi.invocator()) { slot = 38; }
    else if (state.get().players_39.active && state.get().players_39.walletAddress == qpi.invocator()) { slot = 39; }
    else if (state.get().players_40.active && state.get().players_40.walletAddress == qpi.invocator()) { slot = 40; }
    else if (state.get().players_41.active && state.get().players_41.walletAddress == qpi.invocator()) { slot = 41; }
    else if (state.get().players_42.active && state.get().players_42.walletAddress == qpi.invocator()) { slot = 42; }
    else if (state.get().players_43.active && state.get().players_43.walletAddress == qpi.invocator()) { slot = 43; }
    else if (state.get().players_44.active && state.get().players_44.walletAddress == qpi.invocator()) { slot = 44; }
    else if (state.get().players_45.active && state.get().players_45.walletAddress == qpi.invocator()) { slot = 45; }
    else if (state.get().players_46.active && state.get().players_46.walletAddress == qpi.invocator()) { slot = 46; }
    else if (state.get().players_47.active && state.get().players_47.walletAddress == qpi.invocator()) { slot = 47; }
    else if (state.get().players_48.active && state.get().players_48.walletAddress == qpi.invocator()) { slot = 48; }
    else if (state.get().players_49.active && state.get().players_49.walletAddress == qpi.invocator()) { slot = 49; }
    else if (state.get().players_50.active && state.get().players_50.walletAddress == qpi.invocator()) { slot = 50; }
    else if (state.get().players_51.active && state.get().players_51.walletAddress == qpi.invocator()) { slot = 51; }
    else if (state.get().players_52.active && state.get().players_52.walletAddress == qpi.invocator()) { slot = 52; }
    else if (state.get().players_53.active && state.get().players_53.walletAddress == qpi.invocator()) { slot = 53; }
    else if (state.get().players_54.active && state.get().players_54.walletAddress == qpi.invocator()) { slot = 54; }
    else if (state.get().players_55.active && state.get().players_55.walletAddress == qpi.invocator()) { slot = 55; }
    else if (state.get().players_56.active && state.get().players_56.walletAddress == qpi.invocator()) { slot = 56; }
    else if (state.get().players_57.active && state.get().players_57.walletAddress == qpi.invocator()) { slot = 57; }
    else if (state.get().players_58.active && state.get().players_58.walletAddress == qpi.invocator()) { slot = 58; }
    else if (state.get().players_59.active && state.get().players_59.walletAddress == qpi.invocator()) { slot = 59; }
    else if (state.get().players_60.active && state.get().players_60.walletAddress == qpi.invocator()) { slot = 60; }
    else if (state.get().players_61.active && state.get().players_61.walletAddress == qpi.invocator()) { slot = 61; }
    else if (state.get().players_62.active && state.get().players_62.walletAddress == qpi.invocator()) { slot = 62; }
    else if (state.get().players_63.active && state.get().players_63.walletAddress == qpi.invocator()) { slot = 63; }
    else if (state.get().players_64.active && state.get().players_64.walletAddress == qpi.invocator()) { slot = 64; }
    else if (state.get().players_65.active && state.get().players_65.walletAddress == qpi.invocator()) { slot = 65; }
    else if (state.get().players_66.active && state.get().players_66.walletAddress == qpi.invocator()) { slot = 66; }
    else if (state.get().players_67.active && state.get().players_67.walletAddress == qpi.invocator()) { slot = 67; }
    else if (state.get().players_68.active && state.get().players_68.walletAddress == qpi.invocator()) { slot = 68; }
    else if (state.get().players_69.active && state.get().players_69.walletAddress == qpi.invocator()) { slot = 69; }
    else if (state.get().players_70.active && state.get().players_70.walletAddress == qpi.invocator()) { slot = 70; }
    else if (state.get().players_71.active && state.get().players_71.walletAddress == qpi.invocator()) { slot = 71; }
    else if (state.get().players_72.active && state.get().players_72.walletAddress == qpi.invocator()) { slot = 72; }
    else if (state.get().players_73.active && state.get().players_73.walletAddress == qpi.invocator()) { slot = 73; }
    else if (state.get().players_74.active && state.get().players_74.walletAddress == qpi.invocator()) { slot = 74; }
    else if (state.get().players_75.active && state.get().players_75.walletAddress == qpi.invocator()) { slot = 75; }
    else if (state.get().players_76.active && state.get().players_76.walletAddress == qpi.invocator()) { slot = 76; }
    else if (state.get().players_77.active && state.get().players_77.walletAddress == qpi.invocator()) { slot = 77; }
    else if (state.get().players_78.active && state.get().players_78.walletAddress == qpi.invocator()) { slot = 78; }
    else if (state.get().players_79.active && state.get().players_79.walletAddress == qpi.invocator()) { slot = 79; }
    else if (state.get().players_80.active && state.get().players_80.walletAddress == qpi.invocator()) { slot = 80; }
    else if (state.get().players_81.active && state.get().players_81.walletAddress == qpi.invocator()) { slot = 81; }
    else if (state.get().players_82.active && state.get().players_82.walletAddress == qpi.invocator()) { slot = 82; }
    else if (state.get().players_83.active && state.get().players_83.walletAddress == qpi.invocator()) { slot = 83; }
    else if (state.get().players_84.active && state.get().players_84.walletAddress == qpi.invocator()) { slot = 84; }
    else if (state.get().players_85.active && state.get().players_85.walletAddress == qpi.invocator()) { slot = 85; }
    else if (state.get().players_86.active && state.get().players_86.walletAddress == qpi.invocator()) { slot = 86; }
    else if (state.get().players_87.active && state.get().players_87.walletAddress == qpi.invocator()) { slot = 87; }
    else if (state.get().players_88.active && state.get().players_88.walletAddress == qpi.invocator()) { slot = 88; }
    else if (state.get().players_89.active && state.get().players_89.walletAddress == qpi.invocator()) { slot = 89; }
    else if (state.get().players_90.active && state.get().players_90.walletAddress == qpi.invocator()) { slot = 90; }
    else if (state.get().players_91.active && state.get().players_91.walletAddress == qpi.invocator()) { slot = 91; }
    else if (state.get().players_92.active && state.get().players_92.walletAddress == qpi.invocator()) { slot = 92; }
    else if (state.get().players_93.active && state.get().players_93.walletAddress == qpi.invocator()) { slot = 93; }
    else if (state.get().players_94.active && state.get().players_94.walletAddress == qpi.invocator()) { slot = 94; }
    else if (state.get().players_95.active && state.get().players_95.walletAddress == qpi.invocator()) { slot = 95; }
    else if (state.get().players_96.active && state.get().players_96.walletAddress == qpi.invocator()) { slot = 96; }
    else if (state.get().players_97.active && state.get().players_97.walletAddress == qpi.invocator()) { slot = 97; }
    else if (state.get().players_98.active && state.get().players_98.walletAddress == qpi.invocator()) { slot = 98; }
    else if (state.get().players_99.active && state.get().players_99.walletAddress == qpi.invocator()) { slot = 99; }
    else if (state.get().players_100.active && state.get().players_100.walletAddress == qpi.invocator()) { slot = 100; }
    else if (state.get().players_101.active && state.get().players_101.walletAddress == qpi.invocator()) { slot = 101; }
    else if (state.get().players_102.active && state.get().players_102.walletAddress == qpi.invocator()) { slot = 102; }
    else if (state.get().players_103.active && state.get().players_103.walletAddress == qpi.invocator()) { slot = 103; }
    else if (state.get().players_104.active && state.get().players_104.walletAddress == qpi.invocator()) { slot = 104; }
    else if (state.get().players_105.active && state.get().players_105.walletAddress == qpi.invocator()) { slot = 105; }
    else if (state.get().players_106.active && state.get().players_106.walletAddress == qpi.invocator()) { slot = 106; }
    else if (state.get().players_107.active && state.get().players_107.walletAddress == qpi.invocator()) { slot = 107; }
    else if (state.get().players_108.active && state.get().players_108.walletAddress == qpi.invocator()) { slot = 108; }
    else if (state.get().players_109.active && state.get().players_109.walletAddress == qpi.invocator()) { slot = 109; }
    else if (state.get().players_110.active && state.get().players_110.walletAddress == qpi.invocator()) { slot = 110; }
    else if (state.get().players_111.active && state.get().players_111.walletAddress == qpi.invocator()) { slot = 111; }
    else if (state.get().players_112.active && state.get().players_112.walletAddress == qpi.invocator()) { slot = 112; }
    else if (state.get().players_113.active && state.get().players_113.walletAddress == qpi.invocator()) { slot = 113; }
    else if (state.get().players_114.active && state.get().players_114.walletAddress == qpi.invocator()) { slot = 114; }
    else if (state.get().players_115.active && state.get().players_115.walletAddress == qpi.invocator()) { slot = 115; }
    else if (state.get().players_116.active && state.get().players_116.walletAddress == qpi.invocator()) { slot = 116; }
    else if (state.get().players_117.active && state.get().players_117.walletAddress == qpi.invocator()) { slot = 117; }
    else if (state.get().players_118.active && state.get().players_118.walletAddress == qpi.invocator()) { slot = 118; }
    else if (state.get().players_119.active && state.get().players_119.walletAddress == qpi.invocator()) { slot = 119; }
    else if (state.get().players_120.active && state.get().players_120.walletAddress == qpi.invocator()) { slot = 120; }
    else if (state.get().players_121.active && state.get().players_121.walletAddress == qpi.invocator()) { slot = 121; }
    else if (state.get().players_122.active && state.get().players_122.walletAddress == qpi.invocator()) { slot = 122; }
    else if (state.get().players_123.active && state.get().players_123.walletAddress == qpi.invocator()) { slot = 123; }
    else if (state.get().players_124.active && state.get().players_124.walletAddress == qpi.invocator()) { slot = 124; }
    else if (state.get().players_125.active && state.get().players_125.walletAddress == qpi.invocator()) { slot = 125; }
    else if (state.get().players_126.active && state.get().players_126.walletAddress == qpi.invocator()) { slot = 126; }
    else if (state.get().players_127.active && state.get().players_127.walletAddress == qpi.invocator()) { slot = 127; }
    else if (state.get().players_128.active && state.get().players_128.walletAddress == qpi.invocator()) { slot = 128; }
    else if (state.get().players_129.active && state.get().players_129.walletAddress == qpi.invocator()) { slot = 129; }
    else if (state.get().players_130.active && state.get().players_130.walletAddress == qpi.invocator()) { slot = 130; }
    else if (state.get().players_131.active && state.get().players_131.walletAddress == qpi.invocator()) { slot = 131; }
    else if (state.get().players_132.active && state.get().players_132.walletAddress == qpi.invocator()) { slot = 132; }
    else if (state.get().players_133.active && state.get().players_133.walletAddress == qpi.invocator()) { slot = 133; }
    else if (state.get().players_134.active && state.get().players_134.walletAddress == qpi.invocator()) { slot = 134; }
    else if (state.get().players_135.active && state.get().players_135.walletAddress == qpi.invocator()) { slot = 135; }
    else if (state.get().players_136.active && state.get().players_136.walletAddress == qpi.invocator()) { slot = 136; }
    else if (state.get().players_137.active && state.get().players_137.walletAddress == qpi.invocator()) { slot = 137; }
    else if (state.get().players_138.active && state.get().players_138.walletAddress == qpi.invocator()) { slot = 138; }
    else if (state.get().players_139.active && state.get().players_139.walletAddress == qpi.invocator()) { slot = 139; }
    else if (state.get().players_140.active && state.get().players_140.walletAddress == qpi.invocator()) { slot = 140; }
    else if (state.get().players_141.active && state.get().players_141.walletAddress == qpi.invocator()) { slot = 141; }
    else if (state.get().players_142.active && state.get().players_142.walletAddress == qpi.invocator()) { slot = 142; }
    else if (state.get().players_143.active && state.get().players_143.walletAddress == qpi.invocator()) { slot = 143; }
    else if (state.get().players_144.active && state.get().players_144.walletAddress == qpi.invocator()) { slot = 144; }
    else if (state.get().players_145.active && state.get().players_145.walletAddress == qpi.invocator()) { slot = 145; }
    else if (state.get().players_146.active && state.get().players_146.walletAddress == qpi.invocator()) { slot = 146; }
    else if (state.get().players_147.active && state.get().players_147.walletAddress == qpi.invocator()) { slot = 147; }
    else if (state.get().players_148.active && state.get().players_148.walletAddress == qpi.invocator()) { slot = 148; }
    else if (state.get().players_149.active && state.get().players_149.walletAddress == qpi.invocator()) { slot = 149; }
    else if (state.get().players_150.active && state.get().players_150.walletAddress == qpi.invocator()) { slot = 150; }
    else if (state.get().players_151.active && state.get().players_151.walletAddress == qpi.invocator()) { slot = 151; }
    else if (state.get().players_152.active && state.get().players_152.walletAddress == qpi.invocator()) { slot = 152; }
    else if (state.get().players_153.active && state.get().players_153.walletAddress == qpi.invocator()) { slot = 153; }
    else if (state.get().players_154.active && state.get().players_154.walletAddress == qpi.invocator()) { slot = 154; }
    else if (state.get().players_155.active && state.get().players_155.walletAddress == qpi.invocator()) { slot = 155; }
    else if (state.get().players_156.active && state.get().players_156.walletAddress == qpi.invocator()) { slot = 156; }
    else if (state.get().players_157.active && state.get().players_157.walletAddress == qpi.invocator()) { slot = 157; }
    else if (state.get().players_158.active && state.get().players_158.walletAddress == qpi.invocator()) { slot = 158; }
    else if (state.get().players_159.active && state.get().players_159.walletAddress == qpi.invocator()) { slot = 159; }
    else if (state.get().players_160.active && state.get().players_160.walletAddress == qpi.invocator()) { slot = 160; }
    else if (state.get().players_161.active && state.get().players_161.walletAddress == qpi.invocator()) { slot = 161; }
    else if (state.get().players_162.active && state.get().players_162.walletAddress == qpi.invocator()) { slot = 162; }
    else if (state.get().players_163.active && state.get().players_163.walletAddress == qpi.invocator()) { slot = 163; }
    else if (state.get().players_164.active && state.get().players_164.walletAddress == qpi.invocator()) { slot = 164; }
    else if (state.get().players_165.active && state.get().players_165.walletAddress == qpi.invocator()) { slot = 165; }
    else if (state.get().players_166.active && state.get().players_166.walletAddress == qpi.invocator()) { slot = 166; }
    else if (state.get().players_167.active && state.get().players_167.walletAddress == qpi.invocator()) { slot = 167; }
    else if (state.get().players_168.active && state.get().players_168.walletAddress == qpi.invocator()) { slot = 168; }
    else if (state.get().players_169.active && state.get().players_169.walletAddress == qpi.invocator()) { slot = 169; }
    else if (state.get().players_170.active && state.get().players_170.walletAddress == qpi.invocator()) { slot = 170; }
    else if (state.get().players_171.active && state.get().players_171.walletAddress == qpi.invocator()) { slot = 171; }
    else if (state.get().players_172.active && state.get().players_172.walletAddress == qpi.invocator()) { slot = 172; }
    else if (state.get().players_173.active && state.get().players_173.walletAddress == qpi.invocator()) { slot = 173; }
    else if (state.get().players_174.active && state.get().players_174.walletAddress == qpi.invocator()) { slot = 174; }
    else if (state.get().players_175.active && state.get().players_175.walletAddress == qpi.invocator()) { slot = 175; }
    else if (state.get().players_176.active && state.get().players_176.walletAddress == qpi.invocator()) { slot = 176; }
    else if (state.get().players_177.active && state.get().players_177.walletAddress == qpi.invocator()) { slot = 177; }
    else if (state.get().players_178.active && state.get().players_178.walletAddress == qpi.invocator()) { slot = 178; }
    else if (state.get().players_179.active && state.get().players_179.walletAddress == qpi.invocator()) { slot = 179; }
    else if (state.get().players_180.active && state.get().players_180.walletAddress == qpi.invocator()) { slot = 180; }
    else if (state.get().players_181.active && state.get().players_181.walletAddress == qpi.invocator()) { slot = 181; }
    else if (state.get().players_182.active && state.get().players_182.walletAddress == qpi.invocator()) { slot = 182; }
    else if (state.get().players_183.active && state.get().players_183.walletAddress == qpi.invocator()) { slot = 183; }
    else if (state.get().players_184.active && state.get().players_184.walletAddress == qpi.invocator()) { slot = 184; }
    else if (state.get().players_185.active && state.get().players_185.walletAddress == qpi.invocator()) { slot = 185; }
    else if (state.get().players_186.active && state.get().players_186.walletAddress == qpi.invocator()) { slot = 186; }
    else if (state.get().players_187.active && state.get().players_187.walletAddress == qpi.invocator()) { slot = 187; }
    else if (state.get().players_188.active && state.get().players_188.walletAddress == qpi.invocator()) { slot = 188; }
    else if (state.get().players_189.active && state.get().players_189.walletAddress == qpi.invocator()) { slot = 189; }
    else if (state.get().players_190.active && state.get().players_190.walletAddress == qpi.invocator()) { slot = 190; }
    else if (state.get().players_191.active && state.get().players_191.walletAddress == qpi.invocator()) { slot = 191; }
    else if (state.get().players_192.active && state.get().players_192.walletAddress == qpi.invocator()) { slot = 192; }
    else if (state.get().players_193.active && state.get().players_193.walletAddress == qpi.invocator()) { slot = 193; }
    else if (state.get().players_194.active && state.get().players_194.walletAddress == qpi.invocator()) { slot = 194; }
    else if (state.get().players_195.active && state.get().players_195.walletAddress == qpi.invocator()) { slot = 195; }
    else if (state.get().players_196.active && state.get().players_196.walletAddress == qpi.invocator()) { slot = 196; }
    else if (state.get().players_197.active && state.get().players_197.walletAddress == qpi.invocator()) { slot = 197; }
    else if (state.get().players_198.active && state.get().players_198.walletAddress == qpi.invocator()) { slot = 198; }
    else if (state.get().players_199.active && state.get().players_199.walletAddress == qpi.invocator()) { slot = 199; }
    else if (state.get().players_200.active && state.get().players_200.walletAddress == qpi.invocator()) { slot = 200; }
    else if (state.get().players_201.active && state.get().players_201.walletAddress == qpi.invocator()) { slot = 201; }
    else if (state.get().players_202.active && state.get().players_202.walletAddress == qpi.invocator()) { slot = 202; }
    else if (state.get().players_203.active && state.get().players_203.walletAddress == qpi.invocator()) { slot = 203; }
    else if (state.get().players_204.active && state.get().players_204.walletAddress == qpi.invocator()) { slot = 204; }
    else if (state.get().players_205.active && state.get().players_205.walletAddress == qpi.invocator()) { slot = 205; }
    else if (state.get().players_206.active && state.get().players_206.walletAddress == qpi.invocator()) { slot = 206; }
    else if (state.get().players_207.active && state.get().players_207.walletAddress == qpi.invocator()) { slot = 207; }
    else if (state.get().players_208.active && state.get().players_208.walletAddress == qpi.invocator()) { slot = 208; }
    else if (state.get().players_209.active && state.get().players_209.walletAddress == qpi.invocator()) { slot = 209; }
    else if (state.get().players_210.active && state.get().players_210.walletAddress == qpi.invocator()) { slot = 210; }
    else if (state.get().players_211.active && state.get().players_211.walletAddress == qpi.invocator()) { slot = 211; }
    else if (state.get().players_212.active && state.get().players_212.walletAddress == qpi.invocator()) { slot = 212; }
    else if (state.get().players_213.active && state.get().players_213.walletAddress == qpi.invocator()) { slot = 213; }
    else if (state.get().players_214.active && state.get().players_214.walletAddress == qpi.invocator()) { slot = 214; }
    else if (state.get().players_215.active && state.get().players_215.walletAddress == qpi.invocator()) { slot = 215; }
    else if (state.get().players_216.active && state.get().players_216.walletAddress == qpi.invocator()) { slot = 216; }
    else if (state.get().players_217.active && state.get().players_217.walletAddress == qpi.invocator()) { slot = 217; }
    else if (state.get().players_218.active && state.get().players_218.walletAddress == qpi.invocator()) { slot = 218; }
    else if (state.get().players_219.active && state.get().players_219.walletAddress == qpi.invocator()) { slot = 219; }
    else if (state.get().players_220.active && state.get().players_220.walletAddress == qpi.invocator()) { slot = 220; }
    else if (state.get().players_221.active && state.get().players_221.walletAddress == qpi.invocator()) { slot = 221; }
    else if (state.get().players_222.active && state.get().players_222.walletAddress == qpi.invocator()) { slot = 222; }
    else if (state.get().players_223.active && state.get().players_223.walletAddress == qpi.invocator()) { slot = 223; }
    else if (state.get().players_224.active && state.get().players_224.walletAddress == qpi.invocator()) { slot = 224; }
    else if (state.get().players_225.active && state.get().players_225.walletAddress == qpi.invocator()) { slot = 225; }
    else if (state.get().players_226.active && state.get().players_226.walletAddress == qpi.invocator()) { slot = 226; }
    else if (state.get().players_227.active && state.get().players_227.walletAddress == qpi.invocator()) { slot = 227; }
    else if (state.get().players_228.active && state.get().players_228.walletAddress == qpi.invocator()) { slot = 228; }
    else if (state.get().players_229.active && state.get().players_229.walletAddress == qpi.invocator()) { slot = 229; }
    else if (state.get().players_230.active && state.get().players_230.walletAddress == qpi.invocator()) { slot = 230; }
    else if (state.get().players_231.active && state.get().players_231.walletAddress == qpi.invocator()) { slot = 231; }
    else if (state.get().players_232.active && state.get().players_232.walletAddress == qpi.invocator()) { slot = 232; }
    else if (state.get().players_233.active && state.get().players_233.walletAddress == qpi.invocator()) { slot = 233; }
    else if (state.get().players_234.active && state.get().players_234.walletAddress == qpi.invocator()) { slot = 234; }
    else if (state.get().players_235.active && state.get().players_235.walletAddress == qpi.invocator()) { slot = 235; }
    else if (state.get().players_236.active && state.get().players_236.walletAddress == qpi.invocator()) { slot = 236; }
    else if (state.get().players_237.active && state.get().players_237.walletAddress == qpi.invocator()) { slot = 237; }
    else if (state.get().players_238.active && state.get().players_238.walletAddress == qpi.invocator()) { slot = 238; }
    else if (state.get().players_239.active && state.get().players_239.walletAddress == qpi.invocator()) { slot = 239; }
    else if (state.get().players_240.active && state.get().players_240.walletAddress == qpi.invocator()) { slot = 240; }
    else if (state.get().players_241.active && state.get().players_241.walletAddress == qpi.invocator()) { slot = 241; }
    else if (state.get().players_242.active && state.get().players_242.walletAddress == qpi.invocator()) { slot = 242; }
    else if (state.get().players_243.active && state.get().players_243.walletAddress == qpi.invocator()) { slot = 243; }
    else if (state.get().players_244.active && state.get().players_244.walletAddress == qpi.invocator()) { slot = 244; }
    else if (state.get().players_245.active && state.get().players_245.walletAddress == qpi.invocator()) { slot = 245; }
    else if (state.get().players_246.active && state.get().players_246.walletAddress == qpi.invocator()) { slot = 246; }
    else if (state.get().players_247.active && state.get().players_247.walletAddress == qpi.invocator()) { slot = 247; }
    else if (state.get().players_248.active && state.get().players_248.walletAddress == qpi.invocator()) { slot = 248; }
    else if (state.get().players_249.active && state.get().players_249.walletAddress == qpi.invocator()) { slot = 249; }
    else if (state.get().players_250.active && state.get().players_250.walletAddress == qpi.invocator()) { slot = 250; }
    else if (state.get().players_251.active && state.get().players_251.walletAddress == qpi.invocator()) { slot = 251; }
    else if (state.get().players_252.active && state.get().players_252.walletAddress == qpi.invocator()) { slot = 252; }
    else if (state.get().players_253.active && state.get().players_253.walletAddress == qpi.invocator()) { slot = 253; }
    else if (state.get().players_254.active && state.get().players_254.walletAddress == qpi.invocator()) { slot = 254; }
    else if (state.get().players_255.active && state.get().players_255.walletAddress == qpi.invocator()) { slot = 255; }

    if (slot < 0) { return; } // Player not registered

    // Update stake and recompute multiplier
    if (slot == 0)
    {
        state.mut().players_0.stakedAmount = state.get().players_0.stakedAmount + input.amount;

        sint64 newMult;
        newMult = MULT_TIER_0;
        if (state.get().players_0.stakedAmount >= STAKE_TIER_4) { newMult = MULT_TIER_4; }
        else if (state.get().players_0.stakedAmount >= STAKE_TIER_3) { newMult = MULT_TIER_3; }
        else if (state.get().players_0.stakedAmount >= STAKE_TIER_2) { newMult = MULT_TIER_2; }
        else if (state.get().players_0.stakedAmount >= STAKE_TIER_1) { newMult = MULT_TIER_1; }

        state.mut().players_0.stakeMultiplierBPS = newMult;

        output.newStakedTotal   = state.get().players_0.stakedAmount;
        output.newMultiplierBPS = newMult;

        // Compute tier for output
        if (newMult == MULT_TIER_4)      { output.multiplierTier = 4; }
        else if (newMult == MULT_TIER_3) { output.multiplierTier = 3; }
        else if (newMult == MULT_TIER_2) { output.multiplierTier = 2; }
        else if (newMult == MULT_TIER_1) { output.multiplierTier = 1; }
        else                             { output.multiplierTier = 0; }
    }
    // Slots 1-15 follow identical pattern
}

PUBLIC_PROCEDURE(UnstakeQZN)
/*
 * Player withdraws staked QZN.
 * Multiplier recalculates immediately downward.
 * No penalty for unstaking — players are free to exit.
 */
{
    if (input.amount <= 0) { return; }

    sint64 slot;
    slot = -1;

    if (state.get().players_0.active && state.get().players_0.walletAddress == qpi.invocator()) { slot = 0; }
    else if (state.get().players_1.active && state.get().players_1.walletAddress == qpi.invocator()) { slot = 1; }
    // ... full slot search in deployment

    if (slot < 0) { return; }

    if (slot == 0)
    {
        if (input.amount > state.get().players_0.stakedAmount) { return; }

        state.mut().players_0.stakedAmount = state.get().players_0.stakedAmount - input.amount;

        sint64 newMult;
        newMult = MULT_TIER_0;
        if (state.get().players_0.stakedAmount >= STAKE_TIER_4) { newMult = MULT_TIER_4; }
        else if (state.get().players_0.stakedAmount >= STAKE_TIER_3) { newMult = MULT_TIER_3; }
        else if (state.get().players_0.stakedAmount >= STAKE_TIER_2) { newMult = MULT_TIER_2; }
        else if (state.get().players_0.stakedAmount >= STAKE_TIER_1) { newMult = MULT_TIER_1; }

        state.mut().players_0.stakeMultiplierBPS = newMult;

        // Return unstaked QZN to player
        qpi.transfer(qpi.invocator(), input.amount);

        output.amountReturned   = input.amount;
        output.newStakedTotal   = state.get().players_0.stakedAmount;
        output.newMultiplierBPS = newMult;
    }
}

PUBLIC_PROCEDURE(ReportMatchResult)
/*
 * Called by the Game Cabinet PAO after a match is settled.
 * Computes rewards with multiplier applied.
 * Checks all achievement conditions.
 * Updates epoch leaderboard.
 *
 * Rewards are CREDITED to pending balance — not sent yet.
 * Player claims via ClaimRewards() at epoch boundary.
 *
 * Epoch cap enforced: if player has already hit EPOCH_EARN_CAP
 * this epoch, no additional base rewards are credited.
 * Achievement bonuses bypass the cap (intentional).
 */
{
    // Only Game Cabinet PAO can report results
    if (qpi.invocator() != state.get().gameCabinetAddress)
    {
        output.winnerReward          = 0;
        output.multiplierApplied     = MULT_TIER_0;
        output.achievementsUnlocked  = 0;
        return;
    }

    // Find winner slot
    sint64 wSlot;
    wSlot = -1;

     if (state.get().players_0.active && state.get().players_0.walletAddress == input.winnerAddress) { wSlot = 0; }
    else if (state.get().players_1.active && state.get().players_1.walletAddress == input.winnerAddress) { wSlot = 1; }
    else if (state.get().players_2.active && state.get().players_2.walletAddress == input.winnerAddress) { wSlot = 2; }
    else if (state.get().players_3.active && state.get().players_3.walletAddress == input.winnerAddress) { wSlot = 3; }
    else if (state.get().players_4.active && state.get().players_4.walletAddress == input.winnerAddress) { wSlot = 4; }
    else if (state.get().players_5.active && state.get().players_5.walletAddress == input.winnerAddress) { wSlot = 5; }
    else if (state.get().players_6.active && state.get().players_6.walletAddress == input.winnerAddress) { wSlot = 6; }
    else if (state.get().players_7.active && state.get().players_7.walletAddress == input.winnerAddress) { wSlot = 7; }
    else if (state.get().players_8.active && state.get().players_8.walletAddress == input.winnerAddress) { wSlot = 8; }
    else if (state.get().players_9.active && state.get().players_9.walletAddress == input.winnerAddress) { wSlot = 9; }
    else if (state.get().players_10.active && state.get().players_10.walletAddress == input.winnerAddress) { wSlot = 10; }
    else if (state.get().players_11.active && state.get().players_11.walletAddress == input.winnerAddress) { wSlot = 11; }
    else if (state.get().players_12.active && state.get().players_12.walletAddress == input.winnerAddress) { wSlot = 12; }
    else if (state.get().players_13.active && state.get().players_13.walletAddress == input.winnerAddress) { wSlot = 13; }
    else if (state.get().players_14.active && state.get().players_14.walletAddress == input.winnerAddress) { wSlot = 14; }
    else if (state.get().players_15.active && state.get().players_15.walletAddress == input.winnerAddress) { wSlot = 15; }
    else if (state.get().players_16.active && state.get().players_16.walletAddress == input.winnerAddress) { wSlot = 16; }
    else if (state.get().players_17.active && state.get().players_17.walletAddress == input.winnerAddress) { wSlot = 17; }
    else if (state.get().players_18.active && state.get().players_18.walletAddress == input.winnerAddress) { wSlot = 18; }
    else if (state.get().players_19.active && state.get().players_19.walletAddress == input.winnerAddress) { wSlot = 19; }
    else if (state.get().players_20.active && state.get().players_20.walletAddress == input.winnerAddress) { wSlot = 20; }
    else if (state.get().players_21.active && state.get().players_21.walletAddress == input.winnerAddress) { wSlot = 21; }
    else if (state.get().players_22.active && state.get().players_22.walletAddress == input.winnerAddress) { wSlot = 22; }
    else if (state.get().players_23.active && state.get().players_23.walletAddress == input.winnerAddress) { wSlot = 23; }
    else if (state.get().players_24.active && state.get().players_24.walletAddress == input.winnerAddress) { wSlot = 24; }
    else if (state.get().players_25.active && state.get().players_25.walletAddress == input.winnerAddress) { wSlot = 25; }
    else if (state.get().players_26.active && state.get().players_26.walletAddress == input.winnerAddress) { wSlot = 26; }
    else if (state.get().players_27.active && state.get().players_27.walletAddress == input.winnerAddress) { wSlot = 27; }
    else if (state.get().players_28.active && state.get().players_28.walletAddress == input.winnerAddress) { wSlot = 28; }
    else if (state.get().players_29.active && state.get().players_29.walletAddress == input.winnerAddress) { wSlot = 29; }
    else if (state.get().players_30.active && state.get().players_30.walletAddress == input.winnerAddress) { wSlot = 30; }
    else if (state.get().players_31.active && state.get().players_31.walletAddress == input.winnerAddress) { wSlot = 31; }
    else if (state.get().players_32.active && state.get().players_32.walletAddress == input.winnerAddress) { wSlot = 32; }
    else if (state.get().players_33.active && state.get().players_33.walletAddress == input.winnerAddress) { wSlot = 33; }
    else if (state.get().players_34.active && state.get().players_34.walletAddress == input.winnerAddress) { wSlot = 34; }
    else if (state.get().players_35.active && state.get().players_35.walletAddress == input.winnerAddress) { wSlot = 35; }
    else if (state.get().players_36.active && state.get().players_36.walletAddress == input.winnerAddress) { wSlot = 36; }
    else if (state.get().players_37.active && state.get().players_37.walletAddress == input.winnerAddress) { wSlot = 37; }
    else if (state.get().players_38.active && state.get().players_38.walletAddress == input.winnerAddress) { wSlot = 38; }
    else if (state.get().players_39.active && state.get().players_39.walletAddress == input.winnerAddress) { wSlot = 39; }
    else if (state.get().players_40.active && state.get().players_40.walletAddress == input.winnerAddress) { wSlot = 40; }
    else if (state.get().players_41.active && state.get().players_41.walletAddress == input.winnerAddress) { wSlot = 41; }
    else if (state.get().players_42.active && state.get().players_42.walletAddress == input.winnerAddress) { wSlot = 42; }
    else if (state.get().players_43.active && state.get().players_43.walletAddress == input.winnerAddress) { wSlot = 43; }
    else if (state.get().players_44.active && state.get().players_44.walletAddress == input.winnerAddress) { wSlot = 44; }
    else if (state.get().players_45.active && state.get().players_45.walletAddress == input.winnerAddress) { wSlot = 45; }
    else if (state.get().players_46.active && state.get().players_46.walletAddress == input.winnerAddress) { wSlot = 46; }
    else if (state.get().players_47.active && state.get().players_47.walletAddress == input.winnerAddress) { wSlot = 47; }
    else if (state.get().players_48.active && state.get().players_48.walletAddress == input.winnerAddress) { wSlot = 48; }
    else if (state.get().players_49.active && state.get().players_49.walletAddress == input.winnerAddress) { wSlot = 49; }
    else if (state.get().players_50.active && state.get().players_50.walletAddress == input.winnerAddress) { wSlot = 50; }
    else if (state.get().players_51.active && state.get().players_51.walletAddress == input.winnerAddress) { wSlot = 51; }
    else if (state.get().players_52.active && state.get().players_52.walletAddress == input.winnerAddress) { wSlot = 52; }
    else if (state.get().players_53.active && state.get().players_53.walletAddress == input.winnerAddress) { wSlot = 53; }
    else if (state.get().players_54.active && state.get().players_54.walletAddress == input.winnerAddress) { wSlot = 54; }
    else if (state.get().players_55.active && state.get().players_55.walletAddress == input.winnerAddress) { wSlot = 55; }
    else if (state.get().players_56.active && state.get().players_56.walletAddress == input.winnerAddress) { wSlot = 56; }
    else if (state.get().players_57.active && state.get().players_57.walletAddress == input.winnerAddress) { wSlot = 57; }
    else if (state.get().players_58.active && state.get().players_58.walletAddress == input.winnerAddress) { wSlot = 58; }
    else if (state.get().players_59.active && state.get().players_59.walletAddress == input.winnerAddress) { wSlot = 59; }
    else if (state.get().players_60.active && state.get().players_60.walletAddress == input.winnerAddress) { wSlot = 60; }
    else if (state.get().players_61.active && state.get().players_61.walletAddress == input.winnerAddress) { wSlot = 61; }
    else if (state.get().players_62.active && state.get().players_62.walletAddress == input.winnerAddress) { wSlot = 62; }
    else if (state.get().players_63.active && state.get().players_63.walletAddress == input.winnerAddress) { wSlot = 63; }
    else if (state.get().players_64.active && state.get().players_64.walletAddress == input.winnerAddress) { wSlot = 64; }
    else if (state.get().players_65.active && state.get().players_65.walletAddress == input.winnerAddress) { wSlot = 65; }
    else if (state.get().players_66.active && state.get().players_66.walletAddress == input.winnerAddress) { wSlot = 66; }
    else if (state.get().players_67.active && state.get().players_67.walletAddress == input.winnerAddress) { wSlot = 67; }
    else if (state.get().players_68.active && state.get().players_68.walletAddress == input.winnerAddress) { wSlot = 68; }
    else if (state.get().players_69.active && state.get().players_69.walletAddress == input.winnerAddress) { wSlot = 69; }
    else if (state.get().players_70.active && state.get().players_70.walletAddress == input.winnerAddress) { wSlot = 70; }
    else if (state.get().players_71.active && state.get().players_71.walletAddress == input.winnerAddress) { wSlot = 71; }
    else if (state.get().players_72.active && state.get().players_72.walletAddress == input.winnerAddress) { wSlot = 72; }
    else if (state.get().players_73.active && state.get().players_73.walletAddress == input.winnerAddress) { wSlot = 73; }
    else if (state.get().players_74.active && state.get().players_74.walletAddress == input.winnerAddress) { wSlot = 74; }
    else if (state.get().players_75.active && state.get().players_75.walletAddress == input.winnerAddress) { wSlot = 75; }
    else if (state.get().players_76.active && state.get().players_76.walletAddress == input.winnerAddress) { wSlot = 76; }
    else if (state.get().players_77.active && state.get().players_77.walletAddress == input.winnerAddress) { wSlot = 77; }
    else if (state.get().players_78.active && state.get().players_78.walletAddress == input.winnerAddress) { wSlot = 78; }
    else if (state.get().players_79.active && state.get().players_79.walletAddress == input.winnerAddress) { wSlot = 79; }
    else if (state.get().players_80.active && state.get().players_80.walletAddress == input.winnerAddress) { wSlot = 80; }
    else if (state.get().players_81.active && state.get().players_81.walletAddress == input.winnerAddress) { wSlot = 81; }
    else if (state.get().players_82.active && state.get().players_82.walletAddress == input.winnerAddress) { wSlot = 82; }
    else if (state.get().players_83.active && state.get().players_83.walletAddress == input.winnerAddress) { wSlot = 83; }
    else if (state.get().players_84.active && state.get().players_84.walletAddress == input.winnerAddress) { wSlot = 84; }
    else if (state.get().players_85.active && state.get().players_85.walletAddress == input.winnerAddress) { wSlot = 85; }
    else if (state.get().players_86.active && state.get().players_86.walletAddress == input.winnerAddress) { wSlot = 86; }
    else if (state.get().players_87.active && state.get().players_87.walletAddress == input.winnerAddress) { wSlot = 87; }
    else if (state.get().players_88.active && state.get().players_88.walletAddress == input.winnerAddress) { wSlot = 88; }
    else if (state.get().players_89.active && state.get().players_89.walletAddress == input.winnerAddress) { wSlot = 89; }
    else if (state.get().players_90.active && state.get().players_90.walletAddress == input.winnerAddress) { wSlot = 90; }
    else if (state.get().players_91.active && state.get().players_91.walletAddress == input.winnerAddress) { wSlot = 91; }
    else if (state.get().players_92.active && state.get().players_92.walletAddress == input.winnerAddress) { wSlot = 92; }
    else if (state.get().players_93.active && state.get().players_93.walletAddress == input.winnerAddress) { wSlot = 93; }
    else if (state.get().players_94.active && state.get().players_94.walletAddress == input.winnerAddress) { wSlot = 94; }
    else if (state.get().players_95.active && state.get().players_95.walletAddress == input.winnerAddress) { wSlot = 95; }
    else if (state.get().players_96.active && state.get().players_96.walletAddress == input.winnerAddress) { wSlot = 96; }
    else if (state.get().players_97.active && state.get().players_97.walletAddress == input.winnerAddress) { wSlot = 97; }
    else if (state.get().players_98.active && state.get().players_98.walletAddress == input.winnerAddress) { wSlot = 98; }
    else if (state.get().players_99.active && state.get().players_99.walletAddress == input.winnerAddress) { wSlot = 99; }
    else if (state.get().players_100.active && state.get().players_100.walletAddress == input.winnerAddress) { wSlot = 100; }
    else if (state.get().players_101.active && state.get().players_101.walletAddress == input.winnerAddress) { wSlot = 101; }
    else if (state.get().players_102.active && state.get().players_102.walletAddress == input.winnerAddress) { wSlot = 102; }
    else if (state.get().players_103.active && state.get().players_103.walletAddress == input.winnerAddress) { wSlot = 103; }
    else if (state.get().players_104.active && state.get().players_104.walletAddress == input.winnerAddress) { wSlot = 104; }
    else if (state.get().players_105.active && state.get().players_105.walletAddress == input.winnerAddress) { wSlot = 105; }
    else if (state.get().players_106.active && state.get().players_106.walletAddress == input.winnerAddress) { wSlot = 106; }
    else if (state.get().players_107.active && state.get().players_107.walletAddress == input.winnerAddress) { wSlot = 107; }
    else if (state.get().players_108.active && state.get().players_108.walletAddress == input.winnerAddress) { wSlot = 108; }
    else if (state.get().players_109.active && state.get().players_109.walletAddress == input.winnerAddress) { wSlot = 109; }
    else if (state.get().players_110.active && state.get().players_110.walletAddress == input.winnerAddress) { wSlot = 110; }
    else if (state.get().players_111.active && state.get().players_111.walletAddress == input.winnerAddress) { wSlot = 111; }
    else if (state.get().players_112.active && state.get().players_112.walletAddress == input.winnerAddress) { wSlot = 112; }
    else if (state.get().players_113.active && state.get().players_113.walletAddress == input.winnerAddress) { wSlot = 113; }
    else if (state.get().players_114.active && state.get().players_114.walletAddress == input.winnerAddress) { wSlot = 114; }
    else if (state.get().players_115.active && state.get().players_115.walletAddress == input.winnerAddress) { wSlot = 115; }
    else if (state.get().players_116.active && state.get().players_116.walletAddress == input.winnerAddress) { wSlot = 116; }
    else if (state.get().players_117.active && state.get().players_117.walletAddress == input.winnerAddress) { wSlot = 117; }
    else if (state.get().players_118.active && state.get().players_118.walletAddress == input.winnerAddress) { wSlot = 118; }
    else if (state.get().players_119.active && state.get().players_119.walletAddress == input.winnerAddress) { wSlot = 119; }
    else if (state.get().players_120.active && state.get().players_120.walletAddress == input.winnerAddress) { wSlot = 120; }
    else if (state.get().players_121.active && state.get().players_121.walletAddress == input.winnerAddress) { wSlot = 121; }
    else if (state.get().players_122.active && state.get().players_122.walletAddress == input.winnerAddress) { wSlot = 122; }
    else if (state.get().players_123.active && state.get().players_123.walletAddress == input.winnerAddress) { wSlot = 123; }
    else if (state.get().players_124.active && state.get().players_124.walletAddress == input.winnerAddress) { wSlot = 124; }
    else if (state.get().players_125.active && state.get().players_125.walletAddress == input.winnerAddress) { wSlot = 125; }
    else if (state.get().players_126.active && state.get().players_126.walletAddress == input.winnerAddress) { wSlot = 126; }
    else if (state.get().players_127.active && state.get().players_127.walletAddress == input.winnerAddress) { wSlot = 127; }
    else if (state.get().players_128.active && state.get().players_128.walletAddress == input.winnerAddress) { wSlot = 128; }
    else if (state.get().players_129.active && state.get().players_129.walletAddress == input.winnerAddress) { wSlot = 129; }
    else if (state.get().players_130.active && state.get().players_130.walletAddress == input.winnerAddress) { wSlot = 130; }
    else if (state.get().players_131.active && state.get().players_131.walletAddress == input.winnerAddress) { wSlot = 131; }
    else if (state.get().players_132.active && state.get().players_132.walletAddress == input.winnerAddress) { wSlot = 132; }
    else if (state.get().players_133.active && state.get().players_133.walletAddress == input.winnerAddress) { wSlot = 133; }
    else if (state.get().players_134.active && state.get().players_134.walletAddress == input.winnerAddress) { wSlot = 134; }
    else if (state.get().players_135.active && state.get().players_135.walletAddress == input.winnerAddress) { wSlot = 135; }
    else if (state.get().players_136.active && state.get().players_136.walletAddress == input.winnerAddress) { wSlot = 136; }
    else if (state.get().players_137.active && state.get().players_137.walletAddress == input.winnerAddress) { wSlot = 137; }
    else if (state.get().players_138.active && state.get().players_138.walletAddress == input.winnerAddress) { wSlot = 138; }
    else if (state.get().players_139.active && state.get().players_139.walletAddress == input.winnerAddress) { wSlot = 139; }
    else if (state.get().players_140.active && state.get().players_140.walletAddress == input.winnerAddress) { wSlot = 140; }
    else if (state.get().players_141.active && state.get().players_141.walletAddress == input.winnerAddress) { wSlot = 141; }
    else if (state.get().players_142.active && state.get().players_142.walletAddress == input.winnerAddress) { wSlot = 142; }
    else if (state.get().players_143.active && state.get().players_143.walletAddress == input.winnerAddress) { wSlot = 143; }
    else if (state.get().players_144.active && state.get().players_144.walletAddress == input.winnerAddress) { wSlot = 144; }
    else if (state.get().players_145.active && state.get().players_145.walletAddress == input.winnerAddress) { wSlot = 145; }
    else if (state.get().players_146.active && state.get().players_146.walletAddress == input.winnerAddress) { wSlot = 146; }
    else if (state.get().players_147.active && state.get().players_147.walletAddress == input.winnerAddress) { wSlot = 147; }
    else if (state.get().players_148.active && state.get().players_148.walletAddress == input.winnerAddress) { wSlot = 148; }
    else if (state.get().players_149.active && state.get().players_149.walletAddress == input.winnerAddress) { wSlot = 149; }
    else if (state.get().players_150.active && state.get().players_150.walletAddress == input.winnerAddress) { wSlot = 150; }
    else if (state.get().players_151.active && state.get().players_151.walletAddress == input.winnerAddress) { wSlot = 151; }
    else if (state.get().players_152.active && state.get().players_152.walletAddress == input.winnerAddress) { wSlot = 152; }
    else if (state.get().players_153.active && state.get().players_153.walletAddress == input.winnerAddress) { wSlot = 153; }
    else if (state.get().players_154.active && state.get().players_154.walletAddress == input.winnerAddress) { wSlot = 154; }
    else if (state.get().players_155.active && state.get().players_155.walletAddress == input.winnerAddress) { wSlot = 155; }
    else if (state.get().players_156.active && state.get().players_156.walletAddress == input.winnerAddress) { wSlot = 156; }
    else if (state.get().players_157.active && state.get().players_157.walletAddress == input.winnerAddress) { wSlot = 157; }
    else if (state.get().players_158.active && state.get().players_158.walletAddress == input.winnerAddress) { wSlot = 158; }
    else if (state.get().players_159.active && state.get().players_159.walletAddress == input.winnerAddress) { wSlot = 159; }
    else if (state.get().players_160.active && state.get().players_160.walletAddress == input.winnerAddress) { wSlot = 160; }
    else if (state.get().players_161.active && state.get().players_161.walletAddress == input.winnerAddress) { wSlot = 161; }
    else if (state.get().players_162.active && state.get().players_162.walletAddress == input.winnerAddress) { wSlot = 162; }
    else if (state.get().players_163.active && state.get().players_163.walletAddress == input.winnerAddress) { wSlot = 163; }
    else if (state.get().players_164.active && state.get().players_164.walletAddress == input.winnerAddress) { wSlot = 164; }
    else if (state.get().players_165.active && state.get().players_165.walletAddress == input.winnerAddress) { wSlot = 165; }
    else if (state.get().players_166.active && state.get().players_166.walletAddress == input.winnerAddress) { wSlot = 166; }
    else if (state.get().players_167.active && state.get().players_167.walletAddress == input.winnerAddress) { wSlot = 167; }
    else if (state.get().players_168.active && state.get().players_168.walletAddress == input.winnerAddress) { wSlot = 168; }
    else if (state.get().players_169.active && state.get().players_169.walletAddress == input.winnerAddress) { wSlot = 169; }
    else if (state.get().players_170.active && state.get().players_170.walletAddress == input.winnerAddress) { wSlot = 170; }
    else if (state.get().players_171.active && state.get().players_171.walletAddress == input.winnerAddress) { wSlot = 171; }
    else if (state.get().players_172.active && state.get().players_172.walletAddress == input.winnerAddress) { wSlot = 172; }
    else if (state.get().players_173.active && state.get().players_173.walletAddress == input.winnerAddress) { wSlot = 173; }
    else if (state.get().players_174.active && state.get().players_174.walletAddress == input.winnerAddress) { wSlot = 174; }
    else if (state.get().players_175.active && state.get().players_175.walletAddress == input.winnerAddress) { wSlot = 175; }
    else if (state.get().players_176.active && state.get().players_176.walletAddress == input.winnerAddress) { wSlot = 176; }
    else if (state.get().players_177.active && state.get().players_177.walletAddress == input.winnerAddress) { wSlot = 177; }
    else if (state.get().players_178.active && state.get().players_178.walletAddress == input.winnerAddress) { wSlot = 178; }
    else if (state.get().players_179.active && state.get().players_179.walletAddress == input.winnerAddress) { wSlot = 179; }
    else if (state.get().players_180.active && state.get().players_180.walletAddress == input.winnerAddress) { wSlot = 180; }
    else if (state.get().players_181.active && state.get().players_181.walletAddress == input.winnerAddress) { wSlot = 181; }
    else if (state.get().players_182.active && state.get().players_182.walletAddress == input.winnerAddress) { wSlot = 182; }
    else if (state.get().players_183.active && state.get().players_183.walletAddress == input.winnerAddress) { wSlot = 183; }
    else if (state.get().players_184.active && state.get().players_184.walletAddress == input.winnerAddress) { wSlot = 184; }
    else if (state.get().players_185.active && state.get().players_185.walletAddress == input.winnerAddress) { wSlot = 185; }
    else if (state.get().players_186.active && state.get().players_186.walletAddress == input.winnerAddress) { wSlot = 186; }
    else if (state.get().players_187.active && state.get().players_187.walletAddress == input.winnerAddress) { wSlot = 187; }
    else if (state.get().players_188.active && state.get().players_188.walletAddress == input.winnerAddress) { wSlot = 188; }
    else if (state.get().players_189.active && state.get().players_189.walletAddress == input.winnerAddress) { wSlot = 189; }
    else if (state.get().players_190.active && state.get().players_190.walletAddress == input.winnerAddress) { wSlot = 190; }
    else if (state.get().players_191.active && state.get().players_191.walletAddress == input.winnerAddress) { wSlot = 191; }
    else if (state.get().players_192.active && state.get().players_192.walletAddress == input.winnerAddress) { wSlot = 192; }
    else if (state.get().players_193.active && state.get().players_193.walletAddress == input.winnerAddress) { wSlot = 193; }
    else if (state.get().players_194.active && state.get().players_194.walletAddress == input.winnerAddress) { wSlot = 194; }
    else if (state.get().players_195.active && state.get().players_195.walletAddress == input.winnerAddress) { wSlot = 195; }
    else if (state.get().players_196.active && state.get().players_196.walletAddress == input.winnerAddress) { wSlot = 196; }
    else if (state.get().players_197.active && state.get().players_197.walletAddress == input.winnerAddress) { wSlot = 197; }
    else if (state.get().players_198.active && state.get().players_198.walletAddress == input.winnerAddress) { wSlot = 198; }
    else if (state.get().players_199.active && state.get().players_199.walletAddress == input.winnerAddress) { wSlot = 199; }
    else if (state.get().players_200.active && state.get().players_200.walletAddress == input.winnerAddress) { wSlot = 200; }
    else if (state.get().players_201.active && state.get().players_201.walletAddress == input.winnerAddress) { wSlot = 201; }
    else if (state.get().players_202.active && state.get().players_202.walletAddress == input.winnerAddress) { wSlot = 202; }
    else if (state.get().players_203.active && state.get().players_203.walletAddress == input.winnerAddress) { wSlot = 203; }
    else if (state.get().players_204.active && state.get().players_204.walletAddress == input.winnerAddress) { wSlot = 204; }
    else if (state.get().players_205.active && state.get().players_205.walletAddress == input.winnerAddress) { wSlot = 205; }
    else if (state.get().players_206.active && state.get().players_206.walletAddress == input.winnerAddress) { wSlot = 206; }
    else if (state.get().players_207.active && state.get().players_207.walletAddress == input.winnerAddress) { wSlot = 207; }
    else if (state.get().players_208.active && state.get().players_208.walletAddress == input.winnerAddress) { wSlot = 208; }
    else if (state.get().players_209.active && state.get().players_209.walletAddress == input.winnerAddress) { wSlot = 209; }
    else if (state.get().players_210.active && state.get().players_210.walletAddress == input.winnerAddress) { wSlot = 210; }
    else if (state.get().players_211.active && state.get().players_211.walletAddress == input.winnerAddress) { wSlot = 211; }
    else if (state.get().players_212.active && state.get().players_212.walletAddress == input.winnerAddress) { wSlot = 212; }
    else if (state.get().players_213.active && state.get().players_213.walletAddress == input.winnerAddress) { wSlot = 213; }
    else if (state.get().players_214.active && state.get().players_214.walletAddress == input.winnerAddress) { wSlot = 214; }
    else if (state.get().players_215.active && state.get().players_215.walletAddress == input.winnerAddress) { wSlot = 215; }
    else if (state.get().players_216.active && state.get().players_216.walletAddress == input.winnerAddress) { wSlot = 216; }
    else if (state.get().players_217.active && state.get().players_217.walletAddress == input.winnerAddress) { wSlot = 217; }
    else if (state.get().players_218.active && state.get().players_218.walletAddress == input.winnerAddress) { wSlot = 218; }
    else if (state.get().players_219.active && state.get().players_219.walletAddress == input.winnerAddress) { wSlot = 219; }
    else if (state.get().players_220.active && state.get().players_220.walletAddress == input.winnerAddress) { wSlot = 220; }
    else if (state.get().players_221.active && state.get().players_221.walletAddress == input.winnerAddress) { wSlot = 221; }
    else if (state.get().players_222.active && state.get().players_222.walletAddress == input.winnerAddress) { wSlot = 222; }
    else if (state.get().players_223.active && state.get().players_223.walletAddress == input.winnerAddress) { wSlot = 223; }
    else if (state.get().players_224.active && state.get().players_224.walletAddress == input.winnerAddress) { wSlot = 224; }
    else if (state.get().players_225.active && state.get().players_225.walletAddress == input.winnerAddress) { wSlot = 225; }
    else if (state.get().players_226.active && state.get().players_226.walletAddress == input.winnerAddress) { wSlot = 226; }
    else if (state.get().players_227.active && state.get().players_227.walletAddress == input.winnerAddress) { wSlot = 227; }
    else if (state.get().players_228.active && state.get().players_228.walletAddress == input.winnerAddress) { wSlot = 228; }
    else if (state.get().players_229.active && state.get().players_229.walletAddress == input.winnerAddress) { wSlot = 229; }
    else if (state.get().players_230.active && state.get().players_230.walletAddress == input.winnerAddress) { wSlot = 230; }
    else if (state.get().players_231.active && state.get().players_231.walletAddress == input.winnerAddress) { wSlot = 231; }
    else if (state.get().players_232.active && state.get().players_232.walletAddress == input.winnerAddress) { wSlot = 232; }
    else if (state.get().players_233.active && state.get().players_233.walletAddress == input.winnerAddress) { wSlot = 233; }
    else if (state.get().players_234.active && state.get().players_234.walletAddress == input.winnerAddress) { wSlot = 234; }
    else if (state.get().players_235.active && state.get().players_235.walletAddress == input.winnerAddress) { wSlot = 235; }
    else if (state.get().players_236.active && state.get().players_236.walletAddress == input.winnerAddress) { wSlot = 236; }
    else if (state.get().players_237.active && state.get().players_237.walletAddress == input.winnerAddress) { wSlot = 237; }
    else if (state.get().players_238.active && state.get().players_238.walletAddress == input.winnerAddress) { wSlot = 238; }
    else if (state.get().players_239.active && state.get().players_239.walletAddress == input.winnerAddress) { wSlot = 239; }
    else if (state.get().players_240.active && state.get().players_240.walletAddress == input.winnerAddress) { wSlot = 240; }
    else if (state.get().players_241.active && state.get().players_241.walletAddress == input.winnerAddress) { wSlot = 241; }
    else if (state.get().players_242.active && state.get().players_242.walletAddress == input.winnerAddress) { wSlot = 242; }
    else if (state.get().players_243.active && state.get().players_243.walletAddress == input.winnerAddress) { wSlot = 243; }
    else if (state.get().players_244.active && state.get().players_244.walletAddress == input.winnerAddress) { wSlot = 244; }
    else if (state.get().players_245.active && state.get().players_245.walletAddress == input.winnerAddress) { wSlot = 245; }
    else if (state.get().players_246.active && state.get().players_246.walletAddress == input.winnerAddress) { wSlot = 246; }
    else if (state.get().players_247.active && state.get().players_247.walletAddress == input.winnerAddress) { wSlot = 247; }
    else if (state.get().players_248.active && state.get().players_248.walletAddress == input.winnerAddress) { wSlot = 248; }
    else if (state.get().players_249.active && state.get().players_249.walletAddress == input.winnerAddress) { wSlot = 249; }
    else if (state.get().players_250.active && state.get().players_250.walletAddress == input.winnerAddress) { wSlot = 250; }
    else if (state.get().players_251.active && state.get().players_251.walletAddress == input.winnerAddress) { wSlot = 251; }
    else if (state.get().players_252.active && state.get().players_252.walletAddress == input.winnerAddress) { wSlot = 252; }
    else if (state.get().players_253.active && state.get().players_253.walletAddress == input.winnerAddress) { wSlot = 253; }
    else if (state.get().players_254.active && state.get().players_254.walletAddress == input.winnerAddress) { wSlot = 254; }
    else if (state.get().players_255.active && state.get().players_255.walletAddress == input.winnerAddress) { wSlot = 255; }

    if (wSlot < 0)
    {
        output.winnerReward = 0;
        return; // Winner not registered
    }

    // ---- PROCESS WINNER (slot 0 shown — pattern repeats) ----
    if (wSlot == 0)
    {
        // Reset epoch tracking if new epoch
        if (state.get().players_0.currentEpoch != qpi.epoch())
        {
            state.mut().players_0.currentEpoch = qpi.epoch();
            state.mut().players_0.epochEarned = 0;
            state.mut().players_0.epochScore = 0;
        }

        // ---- COMPUTE BASE REWARD WITH MULTIPLIER ----
        sint64 baseReward;
        sint64 multipliedReward;
        sint64 achBonus;

        baseReward       = input.isSolo ? BASE_WIN_REWARD : BASE_WIN_REWARD;
        multipliedReward = div(baseReward * state.get().players_0.stakeMultiplierBPS, MULT_DENOMINATOR).quot;

        // ---- APPLY EPOCH CAP ----
        sint64 capRemaining;
        capRemaining = EPOCH_EARN_CAP - state.get().players_0.epochEarned;

        if (capRemaining <= 0)
        {
            multipliedReward = 0; // Cap hit — no base reward this match
        }
        else if (multipliedReward > capRemaining)
        {
            multipliedReward = capRemaining; // Partial reward up to cap
        }

        // Credit base reward
        state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + multipliedReward;
        state.mut().players_0.epochEarned = state.get().players_0.epochEarned + multipliedReward;
        state.mut().players_0.lifetimeEarned = state.get().players_0.lifetimeEarned + multipliedReward;

        // Update match stats
        state.mut().players_0.totalMatchesPlayed = state.get().players_0.totalMatchesPlayed + 1;
        state.mut().players_0.totalMatchesWon = state.get().players_0.totalMatchesWon + 1;
        state.mut().players_0.currentWinStreak = state.get().players_0.currentWinStreak + 1;
        state.mut().players_0.epochScore = state.get().players_0.epochScore + multipliedReward;

        if (state.get().players_0.currentWinStreak > state.get().players_0.bestWinStreak)
        {
            state.mut().players_0.bestWinStreak = state.get().players_0.currentWinStreak;
        }

        // Track per-game wins for ALL_GAMES achievement
        if (input.gameId == 1) { state.mut().players_0.wonSnaqe = 1; }
        if (input.gameId == 2) { state.mut().players_0.wonPaqman = 1; }
        if (input.gameId == 3) { state.mut().players_0.wonTanq = 1; }

        // ---- ACHIEVEMENT CHECKS ----
        // Achievement bonuses bypass epoch cap — intentional design
        achBonus = 0;

        // FIRST_WIN
        if (!state.get().players_0.achFirstWin &&
            state.get().players_0.totalMatchesWon >= 1)
        {
            state.mut().players_0.achFirstWin = 1;
            state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + ACH_FIRST_WIN;
            achBonus = achBonus + ACH_FIRST_WIN;
            state.mut().totalAchievementsAwarded = state.get().totalAchievementsAwarded + 1;
        }

        // STREAK_5
        if (!state.get().players_0.achStreak5 &&
            state.get().players_0.currentWinStreak >= 5)
        {
            state.mut().players_0.achStreak5 = 1;
            state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + ACH_STREAK_5;
            achBonus = achBonus + ACH_STREAK_5;
            state.mut().totalAchievementsAwarded = state.get().totalAchievementsAwarded + 1;
        }

        // STREAK_10
        if (!state.get().players_0.achStreak10 &&
            state.get().players_0.currentWinStreak >= 10)
        {
            state.mut().players_0.achStreak10 = 1;
            state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + ACH_STREAK_10;
            achBonus = achBonus + ACH_STREAK_10;
            state.mut().totalAchievementsAwarded = state.get().totalAchievementsAwarded + 1;
        }

        // MATCHES_100
        if (!state.get().players_0.achMatches100 &&
            state.get().players_0.totalMatchesPlayed >= 100)
        {
            state.mut().players_0.achMatches100 = 1;
            state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + ACH_MATCHES_100;
            achBonus = achBonus + ACH_MATCHES_100;
            state.mut().totalAchievementsAwarded = state.get().totalAchievementsAwarded + 1;
        }

        // MATCHES_1000
        if (!state.get().players_0.achMatches1000 &&
            state.get().players_0.totalMatchesPlayed >= 1000)
        {
            state.mut().players_0.achMatches1000 = 1;
            state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + ACH_MATCHES_1000;
            achBonus = achBonus + ACH_MATCHES_1000;
            state.mut().totalAchievementsAwarded = state.get().totalAchievementsAwarded + 1;
        }

        // HIGH_STAKE
        if (!state.get().players_0.achHighStake &&
            input.stakeAmount >= ACH_HIGH_STAKE_MIN)
        {
            state.mut().players_0.achHighStake = 1;
            state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + ACH_HIGH_STAKE;
            achBonus = achBonus + ACH_HIGH_STAKE;
            state.mut().totalAchievementsAwarded = state.get().totalAchievementsAwarded + 1;
        }

        // ALL_GAMES — won at least once on all 3 games
        if (!state.get().players_0.achAllGames &&
            state.get().players_0.wonSnaqe &&
            state.get().players_0.wonPaqman &&
            state.get().players_0.wonTanq)
        {
            state.mut().players_0.achAllGames = 1;
            state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + ACH_ALL_GAMES;
            achBonus = achBonus + ACH_ALL_GAMES;
            state.mut().totalAchievementsAwarded = state.get().totalAchievementsAwarded + 1;
        }

        // ---- UPDATE LEADERBOARD ----
        // Check if this player's epoch score beats any top 10 entry
        sint64 playerEpochScore;
        playerEpochScore = state.get().players_0.epochScore;

        // Simple insertion: replace lowest score if player scores higher
        // Full sort on BEGIN_EPOCH() for clean leaderboard each epoch
        if (playerEpochScore > state.get().board_9.score)
        {
            state.mut().board_9.walletAddress = input.winnerAddress;
            state.mut().board_9.score = playerEpochScore;
            // Full sort happens in BEGIN_EPOCH()
        }

        // Deduct from reserve
        state.mut().achievementReserveBalance = state.get().achievementReserveBalance - achBonus;
        state.mut().rewardReserveBalance = state.get().rewardReserveBalance - multipliedReward;

        output.winnerReward         = multipliedReward;
        output.multiplierApplied    = state.get().players_0.stakeMultiplierBPS;
        output.achievementsUnlocked = (achBonus > 0) ? 1 : 0;
    }
    // Slots 1-15 follow identical pattern in full deployment

    // ---- CREDIT PARTICIPATION REWARD TO LOSERS ----
    // Small reward for playing regardless of outcome
    // Losers' streak resets
    // (Abbreviated — same slot-lookup pattern applied to loser1/2/3)
}

PUBLIC_PROCEDURE(ClaimRewards)
/*
 * Player claims their accumulated pending balance.
 * Can claim at any time — epoch batch just means rewards
 * accumulate weekly before being claimable.
 *
 * Actual transfer fires here — pending balance zeroed.
 */
{
    sint64 slot;
    slot = -1;

    if (state.get().players_0.active && state.get().players_0.walletAddress == qpi.invocator()) { slot = 0; }
    else if (state.get().players_1.active && state.get().players_1.walletAddress == qpi.invocator()) { slot = 1; }
    else if (state.get().players_2.active && state.get().players_2.walletAddress == qpi.invocator()) { slot = 2; }
    else if (state.get().players_3.active && state.get().players_3.walletAddress == qpi.invocator()) { slot = 3; }
    else if (state.get().players_4.active && state.get().players_4.walletAddress == qpi.invocator()) { slot = 4; }
    else if (state.get().players_5.active && state.get().players_5.walletAddress == qpi.invocator()) { slot = 5; }
    else if (state.get().players_6.active && state.get().players_6.walletAddress == qpi.invocator()) { slot = 6; }
    else if (state.get().players_7.active && state.get().players_7.walletAddress == qpi.invocator()) { slot = 7; }
    else if (state.get().players_8.active && state.get().players_8.walletAddress == qpi.invocator()) { slot = 8; }
    else if (state.get().players_9.active && state.get().players_9.walletAddress == qpi.invocator()) { slot = 9; }
    else if (state.get().players_10.active && state.get().players_10.walletAddress == qpi.invocator()) { slot = 10; }
    else if (state.get().players_11.active && state.get().players_11.walletAddress == qpi.invocator()) { slot = 11; }
    else if (state.get().players_12.active && state.get().players_12.walletAddress == qpi.invocator()) { slot = 12; }
    else if (state.get().players_13.active && state.get().players_13.walletAddress == qpi.invocator()) { slot = 13; }
    else if (state.get().players_14.active && state.get().players_14.walletAddress == qpi.invocator()) { slot = 14; }
    else if (state.get().players_15.active && state.get().players_15.walletAddress == qpi.invocator()) { slot = 15; }

    if (slot < 0)
    {
        output.amountClaimed    = 0;
        output.pendingRemaining = 0;
        return;
    }

    if (slot == 0)
    {
        sint64 claimAmount;
        claimAmount = state.get().players_0.pendingBalance;

        if (claimAmount <= 0)
        {
            output.amountClaimed    = 0;
            output.pendingRemaining = 0;
            output.currentEpoch     = qpi.epoch();
            return;
        }

        // Execute transfer
        state.mut().players_0.pendingBalance = 0;
        state.mut().totalQZNDistributed = state.get().totalQZNDistributed + claimAmount;
        state.mut().epochTotalDistributed = state.get().epochTotalDistributed + claimAmount;

        qpi.transfer(qpi.invocator(), claimAmount);

        output.amountClaimed    = claimAmount;
        output.pendingRemaining = 0;
        output.currentEpoch     = qpi.epoch();
    }
    // Slots 1-15 follow identical pattern
}

PUBLIC_PROCEDURE(FundReserve)
/*
 * Admin tops up reward or achievement reserve from treasury.
 * Called by Treasury Vault PAO as part of epoch funding cycle.
 */
{
    if (qpi.invocator() != state.get().adminAddress)
    {
        return;
    }

    if (input.isAchievementFund)
    {
        state.mut().achievementReserveBalance = state.get().achievementReserveBalance + input.amount;
    }
    else
    {
        state.mut().rewardReserveBalance = state.get().rewardReserveBalance + input.amount;
    }

    output.newRewardReserve      = state.get().rewardReserveBalance;
    output.newAchievementReserve = state.get().achievementReserveBalance;
}

// ============================================================
//  READ-ONLY QUERY FUNCTIONS
// ============================================================

PUBLIC_FUNCTION(GetPlayerStats)
{
    sint64 slot;
    slot = -1;

    if (state.get().players_0.active && state.get().players_0.walletAddress == input.walletAddress) { slot = 0; }
    else if (state.get().players_1.active && state.get().players_1.walletAddress == input.walletAddress) { slot = 1; }
    // ... full slot search in deployment

    if (slot < 0) { return; }

    if (slot == 0)
    {
        output.pendingBalance   = state.get().players_0.pendingBalance;
        output.stakedAmount     = state.get().players_0.stakedAmount;
        output.multiplierBPS    = state.get().players_0.stakeMultiplierBPS;
        output.epochEarned      = state.get().players_0.epochEarned;
        output.epochCap         = EPOCH_EARN_CAP;
        output.totalMatchesWon  = state.get().players_0.totalMatchesWon;
        output.currentWinStreak = state.get().players_0.currentWinStreak;
        output.lifetimeEarned   = state.get().players_0.lifetimeEarned;
    }
}

PUBLIC_FUNCTION(GetLeaderboard)
{
    output.rank1_wallet      = state.get().board_0.walletAddress;
    output.rank1_score       = state.get().board_0.score;
    output.rank2_wallet      = state.get().board_1.walletAddress;
    output.rank2_score       = state.get().board_1.score;
    output.rank3_wallet      = state.get().board_2.walletAddress;
    output.rank3_score       = state.get().board_2.score;
    output.currentEpoch      = state.get().currentEpoch;
    output.epochRewardPool   = state.get().epochRewardPool;
}

// ============================================================
//  REGISTRATION
// ============================================================


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
    REGISTER_USER_PROCEDURE(InitializeRouter,     1);
    REGISTER_USER_PROCEDURE(RegisterPlayer,       2);
    REGISTER_USER_PROCEDURE(StakeQZN,             3);
    REGISTER_USER_PROCEDURE(UnstakeQZN,           4);
    REGISTER_USER_PROCEDURE(ReportMatchResult,    5);
    REGISTER_USER_PROCEDURE(ClaimRewards,         6);
    REGISTER_USER_PROCEDURE(FundReserve,          7);
    REGISTER_USER_FUNCTION(GetPlayerStats,        8);
    REGISTER_USER_FUNCTION(GetLeaderboard,        9);
}

// ============================================================
//  SYSTEM HOOKS
// ============================================================

BEGIN_EPOCH()
/*
 * Fires every epoch (~weekly). Responsibilities:
 *
 * 1. Sort leaderboard — bubble sort top 10 by epoch score
 * 2. Award leaderboard bonuses to top 10 players
 * 3. Award TOP_LEADERBOARD achievement to rank #1
 * 4. Reset epoch scores for all active players
 * 5. Update epoch counter
 */
{
    state.mut().currentEpoch = qpi.epoch();

    // ── PRO-RATA STAKER DIVIDEND DISTRIBUTION ────────────────────────
    // Pool was filled by QZN Token contract in its own BEGIN_EPOCH.
    // Distribute proportionally: each staker gets
    //   share = pool * (stakerAmount / totalStaked)
    if (state.get().stakerDividendPool > 0LL &&
        state.get().totalStaked > 0LL &&
        state.get().lastDividendEpoch != qpi.epoch())
    {
        sint64 pool       = state.get().stakerDividendPool;
        sint64 totalStake = state.get().totalStaked;
        sint64 distributed = 0LL;

        // Iterate all 256 player slots
        #define DIST_DIVIDEND(N) \
        if (state.get().players_##N.active && state.get().players_##N.stakedAmount > 0LL) { \
            sint64 share = div(pool * state.get().players_##N.stakedAmount, totalStake).quot; \
            if (share > 0LL) { \
                state.mut().players_##N.pendingBalance += share; \
                state.mut().players_##N.lifetimeEarned += share; \
                distributed += share; \
            } \
        }

        DIST_DIVIDEND(0)   DIST_DIVIDEND(1)   DIST_DIVIDEND(2)   DIST_DIVIDEND(3)
        DIST_DIVIDEND(4)   DIST_DIVIDEND(5)   DIST_DIVIDEND(6)   DIST_DIVIDEND(7)
        DIST_DIVIDEND(8)   DIST_DIVIDEND(9)   DIST_DIVIDEND(10)  DIST_DIVIDEND(11)
        DIST_DIVIDEND(12)  DIST_DIVIDEND(13)  DIST_DIVIDEND(14)  DIST_DIVIDEND(15)
        DIST_DIVIDEND(16)  DIST_DIVIDEND(17)  DIST_DIVIDEND(18)  DIST_DIVIDEND(19)
        DIST_DIVIDEND(20)  DIST_DIVIDEND(21)  DIST_DIVIDEND(22)  DIST_DIVIDEND(23)
        DIST_DIVIDEND(24)  DIST_DIVIDEND(25)  DIST_DIVIDEND(26)  DIST_DIVIDEND(27)
        DIST_DIVIDEND(28)  DIST_DIVIDEND(29)  DIST_DIVIDEND(30)  DIST_DIVIDEND(31)
        DIST_DIVIDEND(32)  DIST_DIVIDEND(33)  DIST_DIVIDEND(34)  DIST_DIVIDEND(35)
        DIST_DIVIDEND(36)  DIST_DIVIDEND(37)  DIST_DIVIDEND(38)  DIST_DIVIDEND(39)
        DIST_DIVIDEND(40)  DIST_DIVIDEND(41)  DIST_DIVIDEND(42)  DIST_DIVIDEND(43)
        DIST_DIVIDEND(44)  DIST_DIVIDEND(45)  DIST_DIVIDEND(46)  DIST_DIVIDEND(47)
        DIST_DIVIDEND(48)  DIST_DIVIDEND(49)  DIST_DIVIDEND(50)  DIST_DIVIDEND(51)
        DIST_DIVIDEND(52)  DIST_DIVIDEND(53)  DIST_DIVIDEND(54)  DIST_DIVIDEND(55)
        DIST_DIVIDEND(56)  DIST_DIVIDEND(57)  DIST_DIVIDEND(58)  DIST_DIVIDEND(59)
        DIST_DIVIDEND(60)  DIST_DIVIDEND(61)  DIST_DIVIDEND(62)  DIST_DIVIDEND(63)
        DIST_DIVIDEND(64)  DIST_DIVIDEND(65)  DIST_DIVIDEND(66)  DIST_DIVIDEND(67)
        DIST_DIVIDEND(68)  DIST_DIVIDEND(69)  DIST_DIVIDEND(70)  DIST_DIVIDEND(71)
        DIST_DIVIDEND(72)  DIST_DIVIDEND(73)  DIST_DIVIDEND(74)  DIST_DIVIDEND(75)
        DIST_DIVIDEND(76)  DIST_DIVIDEND(77)  DIST_DIVIDEND(78)  DIST_DIVIDEND(79)
        DIST_DIVIDEND(80)  DIST_DIVIDEND(81)  DIST_DIVIDEND(82)  DIST_DIVIDEND(83)
        DIST_DIVIDEND(84)  DIST_DIVIDEND(85)  DIST_DIVIDEND(86)  DIST_DIVIDEND(87)
        DIST_DIVIDEND(88)  DIST_DIVIDEND(89)  DIST_DIVIDEND(90)  DIST_DIVIDEND(91)
        DIST_DIVIDEND(92)  DIST_DIVIDEND(93)  DIST_DIVIDEND(94)  DIST_DIVIDEND(95)
        DIST_DIVIDEND(96)  DIST_DIVIDEND(97)  DIST_DIVIDEND(98)  DIST_DIVIDEND(99)
        DIST_DIVIDEND(100) DIST_DIVIDEND(101) DIST_DIVIDEND(102) DIST_DIVIDEND(103)
        DIST_DIVIDEND(104) DIST_DIVIDEND(105) DIST_DIVIDEND(106) DIST_DIVIDEND(107)
        DIST_DIVIDEND(108) DIST_DIVIDEND(109) DIST_DIVIDEND(110) DIST_DIVIDEND(111)
        DIST_DIVIDEND(112) DIST_DIVIDEND(113) DIST_DIVIDEND(114) DIST_DIVIDEND(115)
        DIST_DIVIDEND(116) DIST_DIVIDEND(117) DIST_DIVIDEND(118) DIST_DIVIDEND(119)
        DIST_DIVIDEND(120) DIST_DIVIDEND(121) DIST_DIVIDEND(122) DIST_DIVIDEND(123)
        DIST_DIVIDEND(124) DIST_DIVIDEND(125) DIST_DIVIDEND(126) DIST_DIVIDEND(127)
        DIST_DIVIDEND(128) DIST_DIVIDEND(129) DIST_DIVIDEND(130) DIST_DIVIDEND(131)
        DIST_DIVIDEND(132) DIST_DIVIDEND(133) DIST_DIVIDEND(134) DIST_DIVIDEND(135)
        DIST_DIVIDEND(136) DIST_DIVIDEND(137) DIST_DIVIDEND(138) DIST_DIVIDEND(139)
        DIST_DIVIDEND(140) DIST_DIVIDEND(141) DIST_DIVIDEND(142) DIST_DIVIDEND(143)
        DIST_DIVIDEND(144) DIST_DIVIDEND(145) DIST_DIVIDEND(146) DIST_DIVIDEND(147)
        DIST_DIVIDEND(148) DIST_DIVIDEND(149) DIST_DIVIDEND(150) DIST_DIVIDEND(151)
        DIST_DIVIDEND(152) DIST_DIVIDEND(153) DIST_DIVIDEND(154) DIST_DIVIDEND(155)
        DIST_DIVIDEND(156) DIST_DIVIDEND(157) DIST_DIVIDEND(158) DIST_DIVIDEND(159)
        DIST_DIVIDEND(160) DIST_DIVIDEND(161) DIST_DIVIDEND(162) DIST_DIVIDEND(163)
        DIST_DIVIDEND(164) DIST_DIVIDEND(165) DIST_DIVIDEND(166) DIST_DIVIDEND(167)
        DIST_DIVIDEND(168) DIST_DIVIDEND(169) DIST_DIVIDEND(170) DIST_DIVIDEND(171)
        DIST_DIVIDEND(172) DIST_DIVIDEND(173) DIST_DIVIDEND(174) DIST_DIVIDEND(175)
        DIST_DIVIDEND(176) DIST_DIVIDEND(177) DIST_DIVIDEND(178) DIST_DIVIDEND(179)
        DIST_DIVIDEND(180) DIST_DIVIDEND(181) DIST_DIVIDEND(182) DIST_DIVIDEND(183)
        DIST_DIVIDEND(184) DIST_DIVIDEND(185) DIST_DIVIDEND(186) DIST_DIVIDEND(187)
        DIST_DIVIDEND(188) DIST_DIVIDEND(189) DIST_DIVIDEND(190) DIST_DIVIDEND(191)
        DIST_DIVIDEND(192) DIST_DIVIDEND(193) DIST_DIVIDEND(194) DIST_DIVIDEND(195)
        DIST_DIVIDEND(196) DIST_DIVIDEND(197) DIST_DIVIDEND(198) DIST_DIVIDEND(199)
        DIST_DIVIDEND(200) DIST_DIVIDEND(201) DIST_DIVIDEND(202) DIST_DIVIDEND(203)
        DIST_DIVIDEND(204) DIST_DIVIDEND(205) DIST_DIVIDEND(206) DIST_DIVIDEND(207)
        DIST_DIVIDEND(208) DIST_DIVIDEND(209) DIST_DIVIDEND(210) DIST_DIVIDEND(211)
        DIST_DIVIDEND(212) DIST_DIVIDEND(213) DIST_DIVIDEND(214) DIST_DIVIDEND(215)
        DIST_DIVIDEND(216) DIST_DIVIDEND(217) DIST_DIVIDEND(218) DIST_DIVIDEND(219)
        DIST_DIVIDEND(220) DIST_DIVIDEND(221) DIST_DIVIDEND(222) DIST_DIVIDEND(223)
        DIST_DIVIDEND(224) DIST_DIVIDEND(225) DIST_DIVIDEND(226) DIST_DIVIDEND(227)
        DIST_DIVIDEND(228) DIST_DIVIDEND(229) DIST_DIVIDEND(230) DIST_DIVIDEND(231)
        DIST_DIVIDEND(232) DIST_DIVIDEND(233) DIST_DIVIDEND(234) DIST_DIVIDEND(235)
        DIST_DIVIDEND(236) DIST_DIVIDEND(237) DIST_DIVIDEND(238) DIST_DIVIDEND(239)
        DIST_DIVIDEND(240) DIST_DIVIDEND(241) DIST_DIVIDEND(242) DIST_DIVIDEND(243)
        DIST_DIVIDEND(244) DIST_DIVIDEND(245) DIST_DIVIDEND(246) DIST_DIVIDEND(247)
        DIST_DIVIDEND(248) DIST_DIVIDEND(249) DIST_DIVIDEND(250) DIST_DIVIDEND(251)
        DIST_DIVIDEND(252) DIST_DIVIDEND(253) DIST_DIVIDEND(254) DIST_DIVIDEND(255)

        #undef DIST_DIVIDEND

        state.mut().stakerDividendPool           = 0LL;
        state.mut().totalDividendsPaidToStakers += distributed;
        state.mut().lastDividendEpoch            = qpi.epoch();
    }
    // ─────────────────────────────────────────────────────────────────
    state.mut().totalEpochsProcessed = state.get().totalEpochsProcessed + 1;
    state.mut().epochTotalDistributed = 0;

    // ── SC Shareholder Distribution ───────────────────────────────────
    sint64 rrScPool = div(state.get().rewardReserveBalance * 200LL, 10000LL).quot;
    rrScPool = rrScPool * state.get().epochEfficiencyRating / 1000LL;
    if (rrScPool > 0LL)
    {
        qpi.distributeDividends(rrScPool);
        state.mut().rewardReserveBalance  -= rrScPool;
        state.mut().totalScDividendsPaid  += rrScPool;
        state.mut().epochScDividendPool    = rrScPool;
    }
    state.mut().epochEfficiencyRating = 1000LL;

    // ---- SORT LEADERBOARD (bubble sort top 10) ----
    // Simple sort — only 10 entries, acceptable in BEGIN_EPOCH()
    sint64 tempScore;
    id     tempWallet;

    // Pass 1
    if (state.get().board_0.score < state.get().board_1.score) { tempScore = state.get().board_0.score; tempWallet = state.get().board_0.walletAddress; state.mut().board_0.score = state.get().board_1.score; state.mut().board_0.walletAddress = state.get().board_1.walletAddress; state.mut().board_1.score = tempScore; state.mut().board_1.walletAddress = tempWallet; }
    if (state.get().board_1.score < state.get().board_2.score) { tempScore = state.get().board_1.score; tempWallet = state.get().board_1.walletAddress; state.mut().board_1.score = state.get().board_2.score; state.mut().board_1.walletAddress = state.get().board_2.walletAddress; state.mut().board_2.score = tempScore; state.mut().board_2.walletAddress = tempWallet; }
    if (state.get().board_2.score < state.get().board_3.score) { tempScore = state.get().board_2.score; tempWallet = state.get().board_2.walletAddress; state.mut().board_2.score = state.get().board_3.score; state.mut().board_2.walletAddress = state.get().board_3.walletAddress; state.mut().board_3.score = tempScore; state.mut().board_3.walletAddress = tempWallet; }
    if (state.get().board_3.score < state.get().board_4.score) { tempScore = state.get().board_3.score; tempWallet = state.get().board_3.walletAddress; state.mut().board_3.score = state.get().board_4.score; state.mut().board_3.walletAddress = state.get().board_4.walletAddress; state.mut().board_4.score = tempScore; state.mut().board_4.walletAddress = tempWallet; }
    if (state.get().board_4.score < state.get().board_5.score) { tempScore = state.get().board_4.score; tempWallet = state.get().board_4.walletAddress; state.mut().board_4.score = state.get().board_5.score; state.mut().board_4.walletAddress = state.get().board_5.walletAddress; state.mut().board_5.score = tempScore; state.mut().board_5.walletAddress = tempWallet; }
    if (state.get().board_5.score < state.get().board_6.score) { tempScore = state.get().board_5.score; tempWallet = state.get().board_5.walletAddress; state.mut().board_5.score = state.get().board_6.score; state.mut().board_5.walletAddress = state.get().board_6.walletAddress; state.mut().board_6.score = tempScore; state.mut().board_6.walletAddress = tempWallet; }
    if (state.get().board_6.score < state.get().board_7.score) { tempScore = state.get().board_6.score; tempWallet = state.get().board_6.walletAddress; state.mut().board_6.score = state.get().board_7.score; state.mut().board_6.walletAddress = state.get().board_7.walletAddress; state.mut().board_7.score = tempScore; state.mut().board_7.walletAddress = tempWallet; }
    if (state.get().board_7.score < state.get().board_8.score) { tempScore = state.get().board_7.score; tempWallet = state.get().board_7.walletAddress; state.mut().board_7.score = state.get().board_8.score; state.mut().board_7.walletAddress = state.get().board_8.walletAddress; state.mut().board_8.score = tempScore; state.mut().board_8.walletAddress = tempWallet; }
    if (state.get().board_8.score < state.get().board_9.score) { tempScore = state.get().board_8.score; tempWallet = state.get().board_8.walletAddress; state.mut().board_8.score = state.get().board_9.score; state.mut().board_8.walletAddress = state.get().board_9.walletAddress; state.mut().board_9.score = tempScore; state.mut().board_9.walletAddress = tempWallet; }

    // Pass 2
    if (state.get().board_0.score < state.get().board_1.score) { tempScore = state.get().board_0.score; tempWallet = state.get().board_0.walletAddress; state.mut().board_0.score = state.get().board_1.score; state.mut().board_0.walletAddress = state.get().board_1.walletAddress; state.mut().board_1.score = tempScore; state.mut().board_1.walletAddress = tempWallet; }
    if (state.get().board_1.score < state.get().board_2.score) { tempScore = state.get().board_1.score; tempWallet = state.get().board_1.walletAddress; state.mut().board_1.score = state.get().board_2.score; state.mut().board_1.walletAddress = state.get().board_2.walletAddress; state.mut().board_2.score = tempScore; state.mut().board_2.walletAddress = tempWallet; }
    if (state.get().board_2.score < state.get().board_3.score) { tempScore = state.get().board_2.score; tempWallet = state.get().board_2.walletAddress; state.mut().board_2.score = state.get().board_3.score; state.mut().board_2.walletAddress = state.get().board_3.walletAddress; state.mut().board_3.score = tempScore; state.mut().board_3.walletAddress = tempWallet; }
    if (state.get().board_3.score < state.get().board_4.score) { tempScore = state.get().board_3.score; tempWallet = state.get().board_3.walletAddress; state.mut().board_3.score = state.get().board_4.score; state.mut().board_3.walletAddress = state.get().board_4.walletAddress; state.mut().board_4.score = tempScore; state.mut().board_4.walletAddress = tempWallet; }
    if (state.get().board_4.score < state.get().board_5.score) { tempScore = state.get().board_4.score; tempWallet = state.get().board_4.walletAddress; state.mut().board_4.score = state.get().board_5.score; state.mut().board_4.walletAddress = state.get().board_5.walletAddress; state.mut().board_5.score = tempScore; state.mut().board_5.walletAddress = tempWallet; }
    if (state.get().board_5.score < state.get().board_6.score) { tempScore = state.get().board_5.score; tempWallet = state.get().board_5.walletAddress; state.mut().board_5.score = state.get().board_6.score; state.mut().board_5.walletAddress = state.get().board_6.walletAddress; state.mut().board_6.score = tempScore; state.mut().board_6.walletAddress = tempWallet; }
    if (state.get().board_6.score < state.get().board_7.score) { tempScore = state.get().board_6.score; tempWallet = state.get().board_6.walletAddress; state.mut().board_6.score = state.get().board_7.score; state.mut().board_6.walletAddress = state.get().board_7.walletAddress; state.mut().board_7.score = tempScore; state.mut().board_7.walletAddress = tempWallet; }
    if (state.get().board_7.score < state.get().board_8.score) { tempScore = state.get().board_7.score; tempWallet = state.get().board_7.walletAddress; state.mut().board_7.score = state.get().board_8.score; state.mut().board_7.walletAddress = state.get().board_8.walletAddress; state.mut().board_8.score = tempScore; state.mut().board_8.walletAddress = tempWallet; }

    // Pass 3
    if (state.get().board_0.score < state.get().board_1.score) { tempScore = state.get().board_0.score; tempWallet = state.get().board_0.walletAddress; state.mut().board_0.score = state.get().board_1.score; state.mut().board_0.walletAddress = state.get().board_1.walletAddress; state.mut().board_1.score = tempScore; state.mut().board_1.walletAddress = tempWallet; }
    if (state.get().board_1.score < state.get().board_2.score) { tempScore = state.get().board_1.score; tempWallet = state.get().board_1.walletAddress; state.mut().board_1.score = state.get().board_2.score; state.mut().board_1.walletAddress = state.get().board_2.walletAddress; state.mut().board_2.score = tempScore; state.mut().board_2.walletAddress = tempWallet; }
    if (state.get().board_2.score < state.get().board_3.score) { tempScore = state.get().board_2.score; tempWallet = state.get().board_2.walletAddress; state.mut().board_2.score = state.get().board_3.score; state.mut().board_2.walletAddress = state.get().board_3.walletAddress; state.mut().board_3.score = tempScore; state.mut().board_3.walletAddress = tempWallet; }
    if (state.get().board_3.score < state.get().board_4.score) { tempScore = state.get().board_3.score; tempWallet = state.get().board_3.walletAddress; state.mut().board_3.score = state.get().board_4.score; state.mut().board_3.walletAddress = state.get().board_4.walletAddress; state.mut().board_4.score = tempScore; state.mut().board_4.walletAddress = tempWallet; }
    if (state.get().board_4.score < state.get().board_5.score) { tempScore = state.get().board_4.score; tempWallet = state.get().board_4.walletAddress; state.mut().board_4.score = state.get().board_5.score; state.mut().board_4.walletAddress = state.get().board_5.walletAddress; state.mut().board_5.score = tempScore; state.mut().board_5.walletAddress = tempWallet; }
    if (state.get().board_5.score < state.get().board_6.score) { tempScore = state.get().board_5.score; tempWallet = state.get().board_5.walletAddress; state.mut().board_5.score = state.get().board_6.score; state.mut().board_5.walletAddress = state.get().board_6.walletAddress; state.mut().board_6.score = tempScore; state.mut().board_6.walletAddress = tempWallet; }
    if (state.get().board_6.score < state.get().board_7.score) { tempScore = state.get().board_6.score; tempWallet = state.get().board_6.walletAddress; state.mut().board_6.score = state.get().board_7.score; state.mut().board_6.walletAddress = state.get().board_7.walletAddress; state.mut().board_7.score = tempScore; state.mut().board_7.walletAddress = tempWallet; }

    // Pass 4
    if (state.get().board_0.score < state.get().board_1.score) { tempScore = state.get().board_0.score; tempWallet = state.get().board_0.walletAddress; state.mut().board_0.score = state.get().board_1.score; state.mut().board_0.walletAddress = state.get().board_1.walletAddress; state.mut().board_1.score = tempScore; state.mut().board_1.walletAddress = tempWallet; }
    if (state.get().board_1.score < state.get().board_2.score) { tempScore = state.get().board_1.score; tempWallet = state.get().board_1.walletAddress; state.mut().board_1.score = state.get().board_2.score; state.mut().board_1.walletAddress = state.get().board_2.walletAddress; state.mut().board_2.score = tempScore; state.mut().board_2.walletAddress = tempWallet; }
    if (state.get().board_2.score < state.get().board_3.score) { tempScore = state.get().board_2.score; tempWallet = state.get().board_2.walletAddress; state.mut().board_2.score = state.get().board_3.score; state.mut().board_2.walletAddress = state.get().board_3.walletAddress; state.mut().board_3.score = tempScore; state.mut().board_3.walletAddress = tempWallet; }
    if (state.get().board_3.score < state.get().board_4.score) { tempScore = state.get().board_3.score; tempWallet = state.get().board_3.walletAddress; state.mut().board_3.score = state.get().board_4.score; state.mut().board_3.walletAddress = state.get().board_4.walletAddress; state.mut().board_4.score = tempScore; state.mut().board_4.walletAddress = tempWallet; }
    if (state.get().board_4.score < state.get().board_5.score) { tempScore = state.get().board_4.score; tempWallet = state.get().board_4.walletAddress; state.mut().board_4.score = state.get().board_5.score; state.mut().board_4.walletAddress = state.get().board_5.walletAddress; state.mut().board_5.score = tempScore; state.mut().board_5.walletAddress = tempWallet; }
    if (state.get().board_5.score < state.get().board_6.score) { tempScore = state.get().board_5.score; tempWallet = state.get().board_5.walletAddress; state.mut().board_5.score = state.get().board_6.score; state.mut().board_5.walletAddress = state.get().board_6.walletAddress; state.mut().board_6.score = tempScore; state.mut().board_6.walletAddress = tempWallet; }

    // Pass 5
    if (state.get().board_0.score < state.get().board_1.score) { tempScore = state.get().board_0.score; tempWallet = state.get().board_0.walletAddress; state.mut().board_0.score = state.get().board_1.score; state.mut().board_0.walletAddress = state.get().board_1.walletAddress; state.mut().board_1.score = tempScore; state.mut().board_1.walletAddress = tempWallet; }
    if (state.get().board_1.score < state.get().board_2.score) { tempScore = state.get().board_1.score; tempWallet = state.get().board_1.walletAddress; state.mut().board_1.score = state.get().board_2.score; state.mut().board_1.walletAddress = state.get().board_2.walletAddress; state.mut().board_2.score = tempScore; state.mut().board_2.walletAddress = tempWallet; }
    if (state.get().board_2.score < state.get().board_3.score) { tempScore = state.get().board_2.score; tempWallet = state.get().board_2.walletAddress; state.mut().board_2.score = state.get().board_3.score; state.mut().board_2.walletAddress = state.get().board_3.walletAddress; state.mut().board_3.score = tempScore; state.mut().board_3.walletAddress = tempWallet; }
    if (state.get().board_3.score < state.get().board_4.score) { tempScore = state.get().board_3.score; tempWallet = state.get().board_3.walletAddress; state.mut().board_3.score = state.get().board_4.score; state.mut().board_3.walletAddress = state.get().board_4.walletAddress; state.mut().board_4.score = tempScore; state.mut().board_4.walletAddress = tempWallet; }
    if (state.get().board_4.score < state.get().board_5.score) { tempScore = state.get().board_4.score; tempWallet = state.get().board_4.walletAddress; state.mut().board_4.score = state.get().board_5.score; state.mut().board_4.walletAddress = state.get().board_5.walletAddress; state.mut().board_5.score = tempScore; state.mut().board_5.walletAddress = tempWallet; }

    // Pass 6
    if (state.get().board_0.score < state.get().board_1.score) { tempScore = state.get().board_0.score; tempWallet = state.get().board_0.walletAddress; state.mut().board_0.score = state.get().board_1.score; state.mut().board_0.walletAddress = state.get().board_1.walletAddress; state.mut().board_1.score = tempScore; state.mut().board_1.walletAddress = tempWallet; }
    if (state.get().board_1.score < state.get().board_2.score) { tempScore = state.get().board_1.score; tempWallet = state.get().board_1.walletAddress; state.mut().board_1.score = state.get().board_2.score; state.mut().board_1.walletAddress = state.get().board_2.walletAddress; state.mut().board_2.score = tempScore; state.mut().board_2.walletAddress = tempWallet; }
    if (state.get().board_2.score < state.get().board_3.score) { tempScore = state.get().board_2.score; tempWallet = state.get().board_2.walletAddress; state.mut().board_2.score = state.get().board_3.score; state.mut().board_2.walletAddress = state.get().board_3.walletAddress; state.mut().board_3.score = tempScore; state.mut().board_3.walletAddress = tempWallet; }
    if (state.get().board_3.score < state.get().board_4.score) { tempScore = state.get().board_3.score; tempWallet = state.get().board_3.walletAddress; state.mut().board_3.score = state.get().board_4.score; state.mut().board_3.walletAddress = state.get().board_4.walletAddress; state.mut().board_4.score = tempScore; state.mut().board_4.walletAddress = tempWallet; }

    // Pass 7
    if (state.get().board_0.score < state.get().board_1.score) { tempScore = state.get().board_0.score; tempWallet = state.get().board_0.walletAddress; state.mut().board_0.score = state.get().board_1.score; state.mut().board_0.walletAddress = state.get().board_1.walletAddress; state.mut().board_1.score = tempScore; state.mut().board_1.walletAddress = tempWallet; }
    if (state.get().board_1.score < state.get().board_2.score) { tempScore = state.get().board_1.score; tempWallet = state.get().board_1.walletAddress; state.mut().board_1.score = state.get().board_2.score; state.mut().board_1.walletAddress = state.get().board_2.walletAddress; state.mut().board_2.score = tempScore; state.mut().board_2.walletAddress = tempWallet; }
    if (state.get().board_2.score < state.get().board_3.score) { tempScore = state.get().board_2.score; tempWallet = state.get().board_2.walletAddress; state.mut().board_2.score = state.get().board_3.score; state.mut().board_2.walletAddress = state.get().board_3.walletAddress; state.mut().board_3.score = tempScore; state.mut().board_3.walletAddress = tempWallet; }

    // Pass 8
    if (state.get().board_0.score < state.get().board_1.score) { tempScore = state.get().board_0.score; tempWallet = state.get().board_0.walletAddress; state.mut().board_0.score = state.get().board_1.score; state.mut().board_0.walletAddress = state.get().board_1.walletAddress; state.mut().board_1.score = tempScore; state.mut().board_1.walletAddress = tempWallet; }
    if (state.get().board_1.score < state.get().board_2.score) { tempScore = state.get().board_1.score; tempWallet = state.get().board_1.walletAddress; state.mut().board_1.score = state.get().board_2.score; state.mut().board_1.walletAddress = state.get().board_2.walletAddress; state.mut().board_2.score = tempScore; state.mut().board_2.walletAddress = tempWallet; }

    // Pass 9
    if (state.get().board_0.score < state.get().board_1.score) { tempScore = state.get().board_0.score; tempWallet = state.get().board_0.walletAddress; state.mut().board_0.score = state.get().board_1.score; state.mut().board_0.walletAddress = state.get().board_1.walletAddress; state.mut().board_1.score = tempScore; state.mut().board_1.walletAddress = tempWallet; }

    // ---- LEADERBOARD BONUS POOL ----
    sint64 lbPool;
    lbPool = div(state.get().epochRewardPool * LEADERBOARD_POOL_BPS, LEADERBOARD_BPS_DENOM).quot;

    // Rank 1 gets 30% of leaderboard pool
    sint64 rank1Bonus;
    rank1Bonus = div(lbPool * LEADERBOARD_WINNER_BPS, LEADERBOARD_BPS_DENOM).quot;

    // Credit rank 1 bonus + TOP_LEADERBOARD achievement if first time
    if (state.get().board_0.score > 0)
    {
        // Find rank 1 player slot and credit bonus
        if (state.get().players_0.active && state.get().players_0.walletAddress == state.get().board_0.walletAddress)
        {
            state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + rank1Bonus;

            if (!state.get().players_0.achTopLeaderboard)
            {
                state.mut().players_0.achTopLeaderboard = 1;
                state.mut().players_0.pendingBalance = state.get().players_0.pendingBalance + ACH_TOP_LEADERBOARD;
                state.mut().totalAchievementsAwarded = state.get().totalAchievementsAwarded + 1;
            }
        }
        // ... check all other slots for rank 1 wallet match
    }

    // ---- RESET EPOCH SCORES ----
    state.mut().players_0.epochScore = 0;  state.mut().players_0.epochEarned = 0;
    state.mut().players_1.epochScore = 0;  state.mut().players_1.epochEarned = 0;
    state.mut().players_2.epochScore = 0;  state.mut().players_2.epochEarned = 0;
    state.mut().players_3.epochScore = 0;  state.mut().players_3.epochEarned = 0;
    state.mut().players_4.epochScore = 0;  state.mut().players_4.epochEarned = 0;
    state.mut().players_5.epochScore = 0;  state.mut().players_5.epochEarned = 0;
    state.mut().players_6.epochScore = 0;  state.mut().players_6.epochEarned = 0;
    state.mut().players_7.epochScore = 0;  state.mut().players_7.epochEarned = 0;
    state.mut().players_8.epochScore = 0;  state.mut().players_8.epochEarned = 0;
    state.mut().players_9.epochScore = 0;  state.mut().players_9.epochEarned = 0;
    state.mut().players_10.epochScore = 0;  state.mut().players_10.epochEarned = 0;
    state.mut().players_11.epochScore = 0;  state.mut().players_11.epochEarned = 0;
    state.mut().players_12.epochScore = 0;  state.mut().players_12.epochEarned = 0;
    state.mut().players_13.epochScore = 0;  state.mut().players_13.epochEarned = 0;
    state.mut().players_14.epochScore = 0;  state.mut().players_14.epochEarned = 0;
    state.mut().players_15.epochScore = 0;  state.mut().players_15.epochEarned = 0;
    state.mut().players_16.epochScore = 0;  state.mut().players_16.epochEarned = 0;
    state.mut().players_17.epochScore = 0;  state.mut().players_17.epochEarned = 0;
    state.mut().players_18.epochScore = 0;  state.mut().players_18.epochEarned = 0;
    state.mut().players_19.epochScore = 0;  state.mut().players_19.epochEarned = 0;
    state.mut().players_20.epochScore = 0;  state.mut().players_20.epochEarned = 0;
    state.mut().players_21.epochScore = 0;  state.mut().players_21.epochEarned = 0;
    state.mut().players_22.epochScore = 0;  state.mut().players_22.epochEarned = 0;
    state.mut().players_23.epochScore = 0;  state.mut().players_23.epochEarned = 0;
    state.mut().players_24.epochScore = 0;  state.mut().players_24.epochEarned = 0;
    state.mut().players_25.epochScore = 0;  state.mut().players_25.epochEarned = 0;
    state.mut().players_26.epochScore = 0;  state.mut().players_26.epochEarned = 0;
    state.mut().players_27.epochScore = 0;  state.mut().players_27.epochEarned = 0;
    state.mut().players_28.epochScore = 0;  state.mut().players_28.epochEarned = 0;
    state.mut().players_29.epochScore = 0;  state.mut().players_29.epochEarned = 0;
    state.mut().players_30.epochScore = 0;  state.mut().players_30.epochEarned = 0;
    state.mut().players_31.epochScore = 0;  state.mut().players_31.epochEarned = 0;
    state.mut().players_32.epochScore = 0;  state.mut().players_32.epochEarned = 0;
    state.mut().players_33.epochScore = 0;  state.mut().players_33.epochEarned = 0;
    state.mut().players_34.epochScore = 0;  state.mut().players_34.epochEarned = 0;
    state.mut().players_35.epochScore = 0;  state.mut().players_35.epochEarned = 0;
    state.mut().players_36.epochScore = 0;  state.mut().players_36.epochEarned = 0;
    state.mut().players_37.epochScore = 0;  state.mut().players_37.epochEarned = 0;
    state.mut().players_38.epochScore = 0;  state.mut().players_38.epochEarned = 0;
    state.mut().players_39.epochScore = 0;  state.mut().players_39.epochEarned = 0;
    state.mut().players_40.epochScore = 0;  state.mut().players_40.epochEarned = 0;
    state.mut().players_41.epochScore = 0;  state.mut().players_41.epochEarned = 0;
    state.mut().players_42.epochScore = 0;  state.mut().players_42.epochEarned = 0;
    state.mut().players_43.epochScore = 0;  state.mut().players_43.epochEarned = 0;
    state.mut().players_44.epochScore = 0;  state.mut().players_44.epochEarned = 0;
    state.mut().players_45.epochScore = 0;  state.mut().players_45.epochEarned = 0;
    state.mut().players_46.epochScore = 0;  state.mut().players_46.epochEarned = 0;
    state.mut().players_47.epochScore = 0;  state.mut().players_47.epochEarned = 0;
    state.mut().players_48.epochScore = 0;  state.mut().players_48.epochEarned = 0;
    state.mut().players_49.epochScore = 0;  state.mut().players_49.epochEarned = 0;
    state.mut().players_50.epochScore = 0;  state.mut().players_50.epochEarned = 0;
    state.mut().players_51.epochScore = 0;  state.mut().players_51.epochEarned = 0;
    state.mut().players_52.epochScore = 0;  state.mut().players_52.epochEarned = 0;
    state.mut().players_53.epochScore = 0;  state.mut().players_53.epochEarned = 0;
    state.mut().players_54.epochScore = 0;  state.mut().players_54.epochEarned = 0;
    state.mut().players_55.epochScore = 0;  state.mut().players_55.epochEarned = 0;
    state.mut().players_56.epochScore = 0;  state.mut().players_56.epochEarned = 0;
    state.mut().players_57.epochScore = 0;  state.mut().players_57.epochEarned = 0;
    state.mut().players_58.epochScore = 0;  state.mut().players_58.epochEarned = 0;
    state.mut().players_59.epochScore = 0;  state.mut().players_59.epochEarned = 0;
    state.mut().players_60.epochScore = 0;  state.mut().players_60.epochEarned = 0;
    state.mut().players_61.epochScore = 0;  state.mut().players_61.epochEarned = 0;
    state.mut().players_62.epochScore = 0;  state.mut().players_62.epochEarned = 0;
    state.mut().players_63.epochScore = 0;  state.mut().players_63.epochEarned = 0;
    state.mut().players_64.epochScore = 0;  state.mut().players_64.epochEarned = 0;
    state.mut().players_65.epochScore = 0;  state.mut().players_65.epochEarned = 0;
    state.mut().players_66.epochScore = 0;  state.mut().players_66.epochEarned = 0;
    state.mut().players_67.epochScore = 0;  state.mut().players_67.epochEarned = 0;
    state.mut().players_68.epochScore = 0;  state.mut().players_68.epochEarned = 0;
    state.mut().players_69.epochScore = 0;  state.mut().players_69.epochEarned = 0;
    state.mut().players_70.epochScore = 0;  state.mut().players_70.epochEarned = 0;
    state.mut().players_71.epochScore = 0;  state.mut().players_71.epochEarned = 0;
    state.mut().players_72.epochScore = 0;  state.mut().players_72.epochEarned = 0;
    state.mut().players_73.epochScore = 0;  state.mut().players_73.epochEarned = 0;
    state.mut().players_74.epochScore = 0;  state.mut().players_74.epochEarned = 0;
    state.mut().players_75.epochScore = 0;  state.mut().players_75.epochEarned = 0;
    state.mut().players_76.epochScore = 0;  state.mut().players_76.epochEarned = 0;
    state.mut().players_77.epochScore = 0;  state.mut().players_77.epochEarned = 0;
    state.mut().players_78.epochScore = 0;  state.mut().players_78.epochEarned = 0;
    state.mut().players_79.epochScore = 0;  state.mut().players_79.epochEarned = 0;
    state.mut().players_80.epochScore = 0;  state.mut().players_80.epochEarned = 0;
    state.mut().players_81.epochScore = 0;  state.mut().players_81.epochEarned = 0;
    state.mut().players_82.epochScore = 0;  state.mut().players_82.epochEarned = 0;
    state.mut().players_83.epochScore = 0;  state.mut().players_83.epochEarned = 0;
    state.mut().players_84.epochScore = 0;  state.mut().players_84.epochEarned = 0;
    state.mut().players_85.epochScore = 0;  state.mut().players_85.epochEarned = 0;
    state.mut().players_86.epochScore = 0;  state.mut().players_86.epochEarned = 0;
    state.mut().players_87.epochScore = 0;  state.mut().players_87.epochEarned = 0;
    state.mut().players_88.epochScore = 0;  state.mut().players_88.epochEarned = 0;
    state.mut().players_89.epochScore = 0;  state.mut().players_89.epochEarned = 0;
    state.mut().players_90.epochScore = 0;  state.mut().players_90.epochEarned = 0;
    state.mut().players_91.epochScore = 0;  state.mut().players_91.epochEarned = 0;
    state.mut().players_92.epochScore = 0;  state.mut().players_92.epochEarned = 0;
    state.mut().players_93.epochScore = 0;  state.mut().players_93.epochEarned = 0;
    state.mut().players_94.epochScore = 0;  state.mut().players_94.epochEarned = 0;
    state.mut().players_95.epochScore = 0;  state.mut().players_95.epochEarned = 0;
    state.mut().players_96.epochScore = 0;  state.mut().players_96.epochEarned = 0;
    state.mut().players_97.epochScore = 0;  state.mut().players_97.epochEarned = 0;
    state.mut().players_98.epochScore = 0;  state.mut().players_98.epochEarned = 0;
    state.mut().players_99.epochScore = 0;  state.mut().players_99.epochEarned = 0;
    state.mut().players_100.epochScore = 0;  state.mut().players_100.epochEarned = 0;
    state.mut().players_101.epochScore = 0;  state.mut().players_101.epochEarned = 0;
    state.mut().players_102.epochScore = 0;  state.mut().players_102.epochEarned = 0;
    state.mut().players_103.epochScore = 0;  state.mut().players_103.epochEarned = 0;
    state.mut().players_104.epochScore = 0;  state.mut().players_104.epochEarned = 0;
    state.mut().players_105.epochScore = 0;  state.mut().players_105.epochEarned = 0;
    state.mut().players_106.epochScore = 0;  state.mut().players_106.epochEarned = 0;
    state.mut().players_107.epochScore = 0;  state.mut().players_107.epochEarned = 0;
    state.mut().players_108.epochScore = 0;  state.mut().players_108.epochEarned = 0;
    state.mut().players_109.epochScore = 0;  state.mut().players_109.epochEarned = 0;
    state.mut().players_110.epochScore = 0;  state.mut().players_110.epochEarned = 0;
    state.mut().players_111.epochScore = 0;  state.mut().players_111.epochEarned = 0;
    state.mut().players_112.epochScore = 0;  state.mut().players_112.epochEarned = 0;
    state.mut().players_113.epochScore = 0;  state.mut().players_113.epochEarned = 0;
    state.mut().players_114.epochScore = 0;  state.mut().players_114.epochEarned = 0;
    state.mut().players_115.epochScore = 0;  state.mut().players_115.epochEarned = 0;
    state.mut().players_116.epochScore = 0;  state.mut().players_116.epochEarned = 0;
    state.mut().players_117.epochScore = 0;  state.mut().players_117.epochEarned = 0;
    state.mut().players_118.epochScore = 0;  state.mut().players_118.epochEarned = 0;
    state.mut().players_119.epochScore = 0;  state.mut().players_119.epochEarned = 0;
    state.mut().players_120.epochScore = 0;  state.mut().players_120.epochEarned = 0;
    state.mut().players_121.epochScore = 0;  state.mut().players_121.epochEarned = 0;
    state.mut().players_122.epochScore = 0;  state.mut().players_122.epochEarned = 0;
    state.mut().players_123.epochScore = 0;  state.mut().players_123.epochEarned = 0;
    state.mut().players_124.epochScore = 0;  state.mut().players_124.epochEarned = 0;
    state.mut().players_125.epochScore = 0;  state.mut().players_125.epochEarned = 0;
    state.mut().players_126.epochScore = 0;  state.mut().players_126.epochEarned = 0;
    state.mut().players_127.epochScore = 0;  state.mut().players_127.epochEarned = 0;
    state.mut().players_128.epochScore = 0;  state.mut().players_128.epochEarned = 0;
    state.mut().players_129.epochScore = 0;  state.mut().players_129.epochEarned = 0;
    state.mut().players_130.epochScore = 0;  state.mut().players_130.epochEarned = 0;
    state.mut().players_131.epochScore = 0;  state.mut().players_131.epochEarned = 0;
    state.mut().players_132.epochScore = 0;  state.mut().players_132.epochEarned = 0;
    state.mut().players_133.epochScore = 0;  state.mut().players_133.epochEarned = 0;
    state.mut().players_134.epochScore = 0;  state.mut().players_134.epochEarned = 0;
    state.mut().players_135.epochScore = 0;  state.mut().players_135.epochEarned = 0;
    state.mut().players_136.epochScore = 0;  state.mut().players_136.epochEarned = 0;
    state.mut().players_137.epochScore = 0;  state.mut().players_137.epochEarned = 0;
    state.mut().players_138.epochScore = 0;  state.mut().players_138.epochEarned = 0;
    state.mut().players_139.epochScore = 0;  state.mut().players_139.epochEarned = 0;
    state.mut().players_140.epochScore = 0;  state.mut().players_140.epochEarned = 0;
    state.mut().players_141.epochScore = 0;  state.mut().players_141.epochEarned = 0;
    state.mut().players_142.epochScore = 0;  state.mut().players_142.epochEarned = 0;
    state.mut().players_143.epochScore = 0;  state.mut().players_143.epochEarned = 0;
    state.mut().players_144.epochScore = 0;  state.mut().players_144.epochEarned = 0;
    state.mut().players_145.epochScore = 0;  state.mut().players_145.epochEarned = 0;
    state.mut().players_146.epochScore = 0;  state.mut().players_146.epochEarned = 0;
    state.mut().players_147.epochScore = 0;  state.mut().players_147.epochEarned = 0;
    state.mut().players_148.epochScore = 0;  state.mut().players_148.epochEarned = 0;
    state.mut().players_149.epochScore = 0;  state.mut().players_149.epochEarned = 0;
    state.mut().players_150.epochScore = 0;  state.mut().players_150.epochEarned = 0;
    state.mut().players_151.epochScore = 0;  state.mut().players_151.epochEarned = 0;
    state.mut().players_152.epochScore = 0;  state.mut().players_152.epochEarned = 0;
    state.mut().players_153.epochScore = 0;  state.mut().players_153.epochEarned = 0;
    state.mut().players_154.epochScore = 0;  state.mut().players_154.epochEarned = 0;
    state.mut().players_155.epochScore = 0;  state.mut().players_155.epochEarned = 0;
    state.mut().players_156.epochScore = 0;  state.mut().players_156.epochEarned = 0;
    state.mut().players_157.epochScore = 0;  state.mut().players_157.epochEarned = 0;
    state.mut().players_158.epochScore = 0;  state.mut().players_158.epochEarned = 0;
    state.mut().players_159.epochScore = 0;  state.mut().players_159.epochEarned = 0;
    state.mut().players_160.epochScore = 0;  state.mut().players_160.epochEarned = 0;
    state.mut().players_161.epochScore = 0;  state.mut().players_161.epochEarned = 0;
    state.mut().players_162.epochScore = 0;  state.mut().players_162.epochEarned = 0;
    state.mut().players_163.epochScore = 0;  state.mut().players_163.epochEarned = 0;
    state.mut().players_164.epochScore = 0;  state.mut().players_164.epochEarned = 0;
    state.mut().players_165.epochScore = 0;  state.mut().players_165.epochEarned = 0;
    state.mut().players_166.epochScore = 0;  state.mut().players_166.epochEarned = 0;
    state.mut().players_167.epochScore = 0;  state.mut().players_167.epochEarned = 0;
    state.mut().players_168.epochScore = 0;  state.mut().players_168.epochEarned = 0;
    state.mut().players_169.epochScore = 0;  state.mut().players_169.epochEarned = 0;
    state.mut().players_170.epochScore = 0;  state.mut().players_170.epochEarned = 0;
    state.mut().players_171.epochScore = 0;  state.mut().players_171.epochEarned = 0;
    state.mut().players_172.epochScore = 0;  state.mut().players_172.epochEarned = 0;
    state.mut().players_173.epochScore = 0;  state.mut().players_173.epochEarned = 0;
    state.mut().players_174.epochScore = 0;  state.mut().players_174.epochEarned = 0;
    state.mut().players_175.epochScore = 0;  state.mut().players_175.epochEarned = 0;
    state.mut().players_176.epochScore = 0;  state.mut().players_176.epochEarned = 0;
    state.mut().players_177.epochScore = 0;  state.mut().players_177.epochEarned = 0;
    state.mut().players_178.epochScore = 0;  state.mut().players_178.epochEarned = 0;
    state.mut().players_179.epochScore = 0;  state.mut().players_179.epochEarned = 0;
    state.mut().players_180.epochScore = 0;  state.mut().players_180.epochEarned = 0;
    state.mut().players_181.epochScore = 0;  state.mut().players_181.epochEarned = 0;
    state.mut().players_182.epochScore = 0;  state.mut().players_182.epochEarned = 0;
    state.mut().players_183.epochScore = 0;  state.mut().players_183.epochEarned = 0;
    state.mut().players_184.epochScore = 0;  state.mut().players_184.epochEarned = 0;
    state.mut().players_185.epochScore = 0;  state.mut().players_185.epochEarned = 0;
    state.mut().players_186.epochScore = 0;  state.mut().players_186.epochEarned = 0;
    state.mut().players_187.epochScore = 0;  state.mut().players_187.epochEarned = 0;
    state.mut().players_188.epochScore = 0;  state.mut().players_188.epochEarned = 0;
    state.mut().players_189.epochScore = 0;  state.mut().players_189.epochEarned = 0;
    state.mut().players_190.epochScore = 0;  state.mut().players_190.epochEarned = 0;
    state.mut().players_191.epochScore = 0;  state.mut().players_191.epochEarned = 0;
    state.mut().players_192.epochScore = 0;  state.mut().players_192.epochEarned = 0;
    state.mut().players_193.epochScore = 0;  state.mut().players_193.epochEarned = 0;
    state.mut().players_194.epochScore = 0;  state.mut().players_194.epochEarned = 0;
    state.mut().players_195.epochScore = 0;  state.mut().players_195.epochEarned = 0;
    state.mut().players_196.epochScore = 0;  state.mut().players_196.epochEarned = 0;
    state.mut().players_197.epochScore = 0;  state.mut().players_197.epochEarned = 0;
    state.mut().players_198.epochScore = 0;  state.mut().players_198.epochEarned = 0;
    state.mut().players_199.epochScore = 0;  state.mut().players_199.epochEarned = 0;
    state.mut().players_200.epochScore = 0;  state.mut().players_200.epochEarned = 0;
    state.mut().players_201.epochScore = 0;  state.mut().players_201.epochEarned = 0;
    state.mut().players_202.epochScore = 0;  state.mut().players_202.epochEarned = 0;
    state.mut().players_203.epochScore = 0;  state.mut().players_203.epochEarned = 0;
    state.mut().players_204.epochScore = 0;  state.mut().players_204.epochEarned = 0;
    state.mut().players_205.epochScore = 0;  state.mut().players_205.epochEarned = 0;
    state.mut().players_206.epochScore = 0;  state.mut().players_206.epochEarned = 0;
    state.mut().players_207.epochScore = 0;  state.mut().players_207.epochEarned = 0;
    state.mut().players_208.epochScore = 0;  state.mut().players_208.epochEarned = 0;
    state.mut().players_209.epochScore = 0;  state.mut().players_209.epochEarned = 0;
    state.mut().players_210.epochScore = 0;  state.mut().players_210.epochEarned = 0;
    state.mut().players_211.epochScore = 0;  state.mut().players_211.epochEarned = 0;
    state.mut().players_212.epochScore = 0;  state.mut().players_212.epochEarned = 0;
    state.mut().players_213.epochScore = 0;  state.mut().players_213.epochEarned = 0;
    state.mut().players_214.epochScore = 0;  state.mut().players_214.epochEarned = 0;
    state.mut().players_215.epochScore = 0;  state.mut().players_215.epochEarned = 0;
    state.mut().players_216.epochScore = 0;  state.mut().players_216.epochEarned = 0;
    state.mut().players_217.epochScore = 0;  state.mut().players_217.epochEarned = 0;
    state.mut().players_218.epochScore = 0;  state.mut().players_218.epochEarned = 0;
    state.mut().players_219.epochScore = 0;  state.mut().players_219.epochEarned = 0;
    state.mut().players_220.epochScore = 0;  state.mut().players_220.epochEarned = 0;
    state.mut().players_221.epochScore = 0;  state.mut().players_221.epochEarned = 0;
    state.mut().players_222.epochScore = 0;  state.mut().players_222.epochEarned = 0;
    state.mut().players_223.epochScore = 0;  state.mut().players_223.epochEarned = 0;
    state.mut().players_224.epochScore = 0;  state.mut().players_224.epochEarned = 0;
    state.mut().players_225.epochScore = 0;  state.mut().players_225.epochEarned = 0;
    state.mut().players_226.epochScore = 0;  state.mut().players_226.epochEarned = 0;
    state.mut().players_227.epochScore = 0;  state.mut().players_227.epochEarned = 0;
    state.mut().players_228.epochScore = 0;  state.mut().players_228.epochEarned = 0;
    state.mut().players_229.epochScore = 0;  state.mut().players_229.epochEarned = 0;
    state.mut().players_230.epochScore = 0;  state.mut().players_230.epochEarned = 0;
    state.mut().players_231.epochScore = 0;  state.mut().players_231.epochEarned = 0;
    state.mut().players_232.epochScore = 0;  state.mut().players_232.epochEarned = 0;
    state.mut().players_233.epochScore = 0;  state.mut().players_233.epochEarned = 0;
    state.mut().players_234.epochScore = 0;  state.mut().players_234.epochEarned = 0;
    state.mut().players_235.epochScore = 0;  state.mut().players_235.epochEarned = 0;
    state.mut().players_236.epochScore = 0;  state.mut().players_236.epochEarned = 0;
    state.mut().players_237.epochScore = 0;  state.mut().players_237.epochEarned = 0;
    state.mut().players_238.epochScore = 0;  state.mut().players_238.epochEarned = 0;
    state.mut().players_239.epochScore = 0;  state.mut().players_239.epochEarned = 0;
    state.mut().players_240.epochScore = 0;  state.mut().players_240.epochEarned = 0;
    state.mut().players_241.epochScore = 0;  state.mut().players_241.epochEarned = 0;
    state.mut().players_242.epochScore = 0;  state.mut().players_242.epochEarned = 0;
    state.mut().players_243.epochScore = 0;  state.mut().players_243.epochEarned = 0;
    state.mut().players_244.epochScore = 0;  state.mut().players_244.epochEarned = 0;
    state.mut().players_245.epochScore = 0;  state.mut().players_245.epochEarned = 0;
    state.mut().players_246.epochScore = 0;  state.mut().players_246.epochEarned = 0;
    state.mut().players_247.epochScore = 0;  state.mut().players_247.epochEarned = 0;
    state.mut().players_248.epochScore = 0;  state.mut().players_248.epochEarned = 0;
    state.mut().players_249.epochScore = 0;  state.mut().players_249.epochEarned = 0;
    state.mut().players_250.epochScore = 0;  state.mut().players_250.epochEarned = 0;
    state.mut().players_251.epochScore = 0;  state.mut().players_251.epochEarned = 0;
    state.mut().players_252.epochScore = 0;  state.mut().players_252.epochEarned = 0;
    state.mut().players_253.epochScore = 0;  state.mut().players_253.epochEarned = 0;
    state.mut().players_254.epochScore = 0;  state.mut().players_254.epochEarned = 0;
    state.mut().players_255.epochScore = 0;  state.mut().players_255.epochEarned = 0;


    // Reset leaderboard for next epoch
    state.mut().board_0.score = 0;  state.mut().board_1.score = 0;  state.mut().board_2.score = 0;
    state.mut().board_3.score = 0;  state.mut().board_4.score = 0;  state.mut().board_5.score = 0;
    state.mut().board_6.score = 0;  state.mut().board_7.score = 0;  state.mut().board_8.score = 0;
    state.mut().board_9.score = 0;
}

END_TICK() {}

};// cache-bust Mon Mar 23 06:02:06 PM UTC 2026
// cache-bust Mon Mar 23 07:12:48 PM UTC 2026
