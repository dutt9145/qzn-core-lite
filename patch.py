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

marker = "// new contracts should be added above this line\n\n#ifdef INCLUDE_CONTRACT_TEST_EXAMPLES"
replacement = "// new contracts should be added above this line\n" + qzn_block + "\n#ifdef INCLUDE_CONTRACT_TEST_EXAMPLES"

if "QZN_TOKEN_CONTRACT_INDEX" not in content:
    new_content = content.replace(marker, replacement)
    if new_content == content:
        print("ERROR: marker not found in contract_def.h — patch failed")
        exit(1)
    with open(contract_def, "w") as f:
        f.write(new_content)
    print("Registered all 6 QZN contracts in contract_def.h")
else:
    print("QZN contracts already registered — skipping")

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
    print("Added QZN tests to CMakeLists.txt")
else:
    print("QZN tests already in CMakeLists.txt — skipping")

print("All patches applied successfully.")
