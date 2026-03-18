#!/bin/bash
set -e

# ============================================================
# QZN Core-Lite Setup Script
# Patches cloned core-lite repo with QZN contracts
# Run AFTER cloning core-lite but BEFORE cmake build
# ============================================================

CONTRACTS_SRC="/qzn/contracts"
TESTS_SRC="/qzn/test"
CORE="/app"

echo ">>> Copying QZN contract headers..."
cp $CONTRACTS_SRC/QZN_Token_v2.h            $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_GameCabinet_PAO.h     $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_RewardRouter_PAO.h    $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_TreasuryVault_PAO.h   $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_Portal_PAO.h          $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_TournamentEngine_PAO.h $CORE/src/contracts/

echo ">>> Copying QZN test files..."
cp $TESTS_SRC/contract_qzn_token.cpp           $CORE/test/
cp $TESTS_SRC/contract_qzn_gamecabinet.cpp     $CORE/test/
cp $TESTS_SRC/contract_qzn_rewardrouter.cpp    $CORE/test/
cp $TESTS_SRC/contract_qzn_treasuryvault.cpp   $CORE/test/
cp $TESTS_SRC/contract_qzn_portal.cpp          $CORE/test/
cp $TESTS_SRC/contract_qzn_tournamentengine.cpp $CORE/test/

# ============================================================
# Prepend STATE2 structs to each QZN contract header
# (Required by Qubic's contract_def.h registration pattern)
# ============================================================

echo ">>> Adding STATE2 structs to contract headers..."

prepend_state2() {
  local file=$1
  local struct_name=$2
  # Only prepend if not already there
  if ! grep -q "struct ${struct_name}2" "$file"; then
    sed -i "s/^using namespace QPI;/using namespace QPI;\n\nstruct ${struct_name}2\n{\n};/" "$file"
  fi
}

prepend_state2 $CORE/src/contracts/QZN_Token_v2.h            QZN
prepend_state2 $CORE/src/contracts/QZN_GameCabinet_PAO.h     QZNCABINET
prepend_state2 $CORE/src/contracts/QZN_RewardRouter_PAO.h    QZNREWARDROUTER
prepend_state2 $CORE/src/contracts/QZN_TreasuryVault_PAO.h   QZNTREASVAULT
prepend_state2 $CORE/src/contracts/QZN_Portal_PAO.h          QZNPORTAL
prepend_state2 $CORE/src/contracts/QZN_TournamentEngine_PAO.h QZNTOUR

# ============================================================
# Fix QZN_TOKEN_CONTRACT_INDEX in GameCabinet
# (Was 0/placeholder — set to actual index 26 for local testnet)
# ============================================================

echo ">>> Setting QZN_TOKEN_CONTRACT_INDEX to 26 in GameCabinet..."
sed -i 's/constexpr uint32 QZN_TOKEN_CONTRACT_INDEX  = 0;/constexpr uint32 QZN_TOKEN_CONTRACT_INDEX  = 26;/' \
    $CORE/src/contracts/QZN_GameCabinet_PAO.h

# ============================================================
# Register QZN contracts in contract_def.h
# Insert before the "new contracts should be added above" comment
# Indices: Token=26, Cabinet=27, Router=28, Vault=29, Portal=30, Tour=31
# CRITICAL: Token MUST have lowest index (cross-contract calls go to lower indices only)
# ============================================================

echo ">>> Registering QZN contracts in contract_def.h..."

QZN_BLOCK='
\/\/ QZN ARCADE PROTOCOL CONTRACTS
#define QZN_TOKEN_CONTRACT_INDEX 26
#define CONTRACT_INDEX QZN_TOKEN_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZN
#define CONTRACT_STATE2_TYPE QZN2
#include "contracts\/QZN_Token_v2.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_GAMECABINET_CONTRACT_INDEX 27
#define CONTRACT_INDEX QZN_GAMECABINET_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNCABINET
#define CONTRACT_STATE2_TYPE QZNCABINET2
#include "contracts\/QZN_GameCabinet_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_REWARDROUTER_CONTRACT_INDEX 28
#define CONTRACT_INDEX QZN_REWARDROUTER_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNREWARDROUTER
#define CONTRACT_STATE2_TYPE QZNREWARDROUTER2
#include "contracts\/QZN_RewardRouter_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_TREASURYVAULT_CONTRACT_INDEX 29
#define CONTRACT_INDEX QZN_TREASURYVAULT_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNTREASVAULT
#define CONTRACT_STATE2_TYPE QZNTREASVAULT2
#include "contracts\/QZN_TreasuryVault_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_PORTAL_CONTRACT_INDEX 30
#define CONTRACT_INDEX QZN_PORTAL_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNPORTAL
#define CONTRACT_STATE2_TYPE QZNPORTAL2
#include "contracts\/QZN_Portal_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_TOURNAMENT_CONTRACT_INDEX 31
#define CONTRACT_INDEX QZN_TOURNAMENT_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNTOUR
#define CONTRACT_STATE2_TYPE QZNTOUR2
#include "contracts\/QZN_TournamentEngine_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE
'

# Insert before the "new contracts should be added above this line" comment
sed -i "s/\/\/ new contracts should be added above this line/${QZN_BLOCK}\n\/\/ new contracts should be added above this line/" \
    $CORE/src/contract_core/contract_def.h

# ============================================================
# Add QZN test files to test/CMakeLists.txt
# ============================================================

echo ">>> Adding QZN tests to CMakeLists.txt..."

sed -i 's/contract_vottunbridge.cpp/contract_vottunbridge.cpp\n  contract_qzn_token.cpp\n  contract_qzn_gamecabinet.cpp\n  contract_qzn_rewardrouter.cpp\n  contract_qzn_treasuryvault.cpp\n  contract_qzn_portal.cpp\n  contract_qzn_tournamentengine.cpp/' \
    $CORE/test/CMakeLists.txt

echo ">>> QZN setup complete."
