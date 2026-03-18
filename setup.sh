#!/bin/bash
set -e

CONTRACTS_SRC="/qzn/contracts"
TESTS_SRC="/qzn/test"
CORE="/app"

echo ">>> Copying QZN contract headers..."
cp $CONTRACTS_SRC/QZN_Token_v2.h             $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_GameCabinet_PAO.h      $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_RewardRouter_PAO.h     $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_TreasuryVault_PAO.h    $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_Portal_PAO.h           $CORE/src/contracts/
cp $CONTRACTS_SRC/QZN_TournamentEngine_PAO.h $CORE/src/contracts/

echo ">>> Copying QZN test files..."
cp $TESTS_SRC/contract_qzn_token.cpp            $CORE/test/
cp $TESTS_SRC/contract_qzn_gamecabinet.cpp      $CORE/test/
cp $TESTS_SRC/contract_qzn_rewardrouter.cpp     $CORE/test/
cp $TESTS_SRC/contract_qzn_treasuryvault.cpp    $CORE/test/
cp $TESTS_SRC/contract_qzn_portal.cpp           $CORE/test/
cp $TESTS_SRC/contract_qzn_tournamentengine.cpp $CORE/test/

echo ">>> Patching contract headers, contract_def.h, and CMakeLists.txt..."

python3 << 'PYEOF'
import re

# ── 1. STATE2 structs already present in pre-fixed contract headers ──────────
print("  STATE2 structs already present — skipping injection")

# ── 2. QZN_TOKEN_CONTRACT_INDEX already renamed in pre-fixed header ─────────
print("  QZN_TOKEN_CONTRACT_INDEX already handled — skipping")

# ── 3. Register QZN contracts in contract_def.h ───────────────────────────
contract_def = "/app/src/contract_core/contract_def.h"
with open(contract_def, "r") as f:
    content = f.read()

qzn_block = """
// QZN ARCADE PROTOCOL CONTRACTS
#define QZN_TOKEN_CONTRACT_INDEX 26
#define CONTRACT_INDEX QZN_TOKEN_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZN
#define CONTRACT_STATE2_TYPE QZN2
#include "contracts/QZN_Token_v2.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_GAMECABINET_CONTRACT_INDEX 27
#define CONTRACT_INDEX QZN_GAMECABINET_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNCABINET
#define CONTRACT_STATE2_TYPE QZNCABINET2
#include "contracts/QZN_GameCabinet_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_REWARDROUTER_CONTRACT_INDEX 28
#define CONTRACT_INDEX QZN_REWARDROUTER_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNREWARDROUTER
#define CONTRACT_STATE2_TYPE QZNREWARDROUTER2
#include "contracts/QZN_RewardRouter_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_TREASURYVAULT_CONTRACT_INDEX 29
#define CONTRACT_INDEX QZN_TREASURYVAULT_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNTREASVAULT
#define CONTRACT_STATE2_TYPE QZNTREASVAULT2
#include "contracts/QZN_TreasuryVault_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_PORTAL_CONTRACT_INDEX 30
#define CONTRACT_INDEX QZN_PORTAL_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNPORTAL
#define CONTRACT_STATE2_TYPE QZNPORTAL2
#include "contracts/QZN_Portal_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

#define QZN_TOURNAMENT_CONTRACT_INDEX 31
#define CONTRACT_INDEX QZN_TOURNAMENT_CONTRACT_INDEX
#define CONTRACT_STATE_TYPE QZNTOUR
#define CONTRACT_STATE2_TYPE QZNTOUR2
#include "contracts/QZN_TournamentEngine_PAO.h"

#undef CONTRACT_INDEX
#undef CONTRACT_STATE_TYPE
#undef CONTRACT_STATE2_TYPE

"""

marker = '#endif\n\n// new contracts should be added above this line'
if "QZN_TOKEN_CONTRACT_INDEX" not in content:
    content = content.replace(marker, qzn_block + marker)
    with open(contract_def, "w") as f:
        f.write(content)
    print("  Registered all 6 QZN contracts in contract_def.h")
else:
    print("  QZN contracts already registered — skipping")

# ── 4. Add QZN tests to test/CMakeLists.txt ───────────────────────────────
cmake = "/app/test/CMakeLists.txt"
with open(cmake, "r") as f:
    content = f.read()

qzn_tests = """  contract_qzn_token.cpp
  contract_qzn_gamecabinet.cpp
  contract_qzn_rewardrouter.cpp
  contract_qzn_treasuryvault.cpp
  contract_qzn_portal.cpp
  contract_qzn_tournamentengine.cpp
"""

if "contract_qzn_token.cpp" not in content:
    content = content.replace(
        "  contract_vottunbridge.cpp",
        "  contract_vottunbridge.cpp\n" + qzn_tests
    )
    with open(cmake, "w") as f:
        f.write(content)
    print("  Added QZN tests to CMakeLists.txt")
else:
    print("  QZN tests already in CMakeLists.txt — skipping")

print(">>> All patches applied successfully.")
PYEOF