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

# ── 1. Add STATE2 structs to each contract header ──────────────────────────
headers = {
    "/app/src/contracts/QZN_Token_v2.h":             "QZN",
    "/app/src/contracts/QZN_GameCabinet_PAO.h":      "QZNCABINET",
    "/app/src/contracts/QZN_RewardRouter_PAO.h":     "QZNREWARDROUTER",
    "/app/src/contracts/QZN_TreasuryVault_PAO.h":    "QZNTREASVAULT",
    "/app/src/contracts/QZN_Portal_PAO.h":           "QZNPORTAL",
    "/app/src/contracts/QZN_TournamentEngine_PAO.h": "QZNTOUR",
}

for path, name in headers.items():
    with open(path, "r") as f:
        content = f.read()
    state2 = f"struct {name}2\n{{\n}};\n\n"
    if f"struct {name}2" not in content:
        content = content.replace("using namespace QPI;", "using namespace QPI;\n\n" + state2, 1)
        with open(path, "w") as f:
            f.write(content)
        print(f"  Added {name}2 to {path}")

# ── 2. Fix QZN_TOKEN_CONTRACT_INDEX in GameCabinet ────────────────────────
path = "/app/src/contracts/QZN_GameCabinet_PAO.h"
with open(path, "r") as f:
    content = f.read()
content = content.replace(
    "constexpr uint32 QZN_TOKEN_CONTRACT_INDEX  = 0;",
    "constexpr uint32 QZN_TOKEN_CONTRACT_INDEX  = 26;"
)
with open(path, "w") as f:
    f.write(content)
print("  Fixed QZN_TOKEN_CONTRACT_INDEX = 26")

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

marker = "// new contracts should be added above this line"
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