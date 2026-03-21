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
echo ">>> Patching contract_def.h and CMakeLists.txt..."
python3 << 'PYEOF'
contract_def = "/app/src/contract_core/contract_def.h"
with open(contract_def, "r") as f:
    content = f.read()
qzn_block = "\n// QZN ARCADE PROTOCOL CONTRACTS\n#define QZN_TOKEN_CONTRACT_INDEX 26\n#define CONTRACT_INDEX QZN_TOKEN_CONTRACT_INDEX\n#define CONTRACT_STATE_TYPE QZN\n#define CONTRACT_STATE2_TYPE QZN2\n#include \"contracts/QZN_Token_v2.h\"\n#undef CONTRACT_INDEX\n#undef CONTRACT_STATE_TYPE\n#undef CONTRACT_STATE2_TYPE\n#define QZN_GAMECABINET_CONTRACT_INDEX 27\n#define CONTRACT_INDEX QZN_GAMECABINET_CONTRACT_INDEX\n#define CONTRACT_STATE_TYPE QZNCABINET\n#define CONTRACT_STATE2_TYPE QZNCABINET2\n#include \"contracts/QZN_GameCabinet_PAO.h\"\n#undef CONTRACT_INDEX\n#undef CONTRACT_STATE_TYPE\n#undef CONTRACT_STATE2_TYPE\n#define QZN_REWARDROUTER_CONTRACT_INDEX 28\n#define CONTRACT_INDEX QZN_REWARDROUTER_CONTRACT_INDEX\n#define CONTRACT_STATE_TYPE QZNREWARDROUTER\n#define CONTRACT_STATE2_TYPE QZNREWARDROUTER2\n#include \"contracts/QZN_RewardRouter_PAO.h\"\n#undef CONTRACT_INDEX\n#undef CONTRACT_STATE_TYPE\n#undef CONTRACT_STATE2_TYPE\n#define QZN_TREASURYVAULT_CONTRACT_INDEX 29\n#define CONTRACT_INDEX QZN_TREASURYVAULT_CONTRACT_INDEX\n#define CONTRACT_STATE_TYPE QZNTREASVAULT\n#define CONTRACT_STATE2_TYPE QZNTREASVAULT2\n#include \"contracts/QZN_TreasuryVault_PAO.h\"\n#undef CONTRACT_INDEX\n#undef CONTRACT_STATE_TYPE\n#undef CONTRACT_STATE2_TYPE\n#define QZN_PORTAL_CONTRACT_INDEX 30\n#define CONTRACT_INDEX QZN_PORTAL_CONTRACT_INDEX\n#define CONTRACT_STATE_TYPE QZNPORTAL\n#define CONTRACT_STATE2_TYPE QZNPORTAL2\n#include \"contracts/QZN_Portal_PAO.h\"\n#undef CONTRACT_INDEX\n#undef CONTRACT_STATE_TYPE\n#undef CONTRACT_STATE2_TYPE\n#define QZN_TOURNAMENT_CONTRACT_INDEX 31\n#define CONTRACT_INDEX QZN_TOURNAMENT_CONTRACT_INDEX\n#define CONTRACT_STATE_TYPE QZNTOUR\n#define CONTRACT_STATE2_TYPE QZNTOUR2\n#include \"contracts/QZN_TournamentEngine_PAO.h\"\n#undef CONTRACT_INDEX\n#undef CONTRACT_STATE_TYPE\n#undef CONTRACT_STATE2_TYPE\n"
marker = "// new contracts should be added above this line\n\n#ifdef INCLUDE_CONTRACT_TEST_EXAMPLES"
if "QZN_TOKEN_CONTRACT_INDEX" not in content:
    content = content.replace(marker, "// new contracts should be added above this line\n" + qzn_block + "\n#ifdef INCLUDE_CONTRACT_TEST_EXAMPLES")
    with open(contract_def, "w") as f:
        f.write(content)
    print("  Registered all 6 QZN contracts in contract_def.h")
else:
    print("  QZN contracts already registered — skipping")
cmake = "/app/test/CMakeLists.txt"
with open(cmake, "r") as f:
    content = f.read()
qzn_tests = "  contract_qzn_token.cpp\n  contract_qzn_gamecabinet.cpp\n  contract_qzn_rewardrouter.cpp\n  contract_qzn_treasuryvault.cpp\n  contract_qzn_portal.cpp\n  contract_qzn_tournamentengine.cpp\n"
if "contract_qzn_token.cpp" not in content:
    content = content.replace("  contract_vottunbridge.cpp", "  contract_vottunbridge.cpp\n" + qzn_tests)
    with open(cmake, "w") as f:
        f.write(content)
    print("  Added QZN tests to CMakeLists.txt")
else:
    print("  QZN tests already in CMakeLists.txt — skipping")
print(">>> All patches applied successfully.")
PYEOF
cp $TESTS_SRC/contract_tester.h $CORE/test/
