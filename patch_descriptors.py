f = open('/app/src/contract_core/contract_def.h').read()
old = '    // new contracts should be added above this line'
new = '    {"QZN", 205, 10000, sizeof(QZN::StateData)},\n    {"QZNCAB", 205, 10000, sizeof(QZNCABINET::StateData)},\n    {"QZNRR", 205, 10000, sizeof(QZNREWARDROUTER::StateData)},\n    {"QZNTV", 205, 10000, sizeof(QZNTREASVAULT::StateData)},\n    {"QZNP", 205, 10000, sizeof(QZNPORTAL::StateData)},\n    {"QZNTOUR", 205, 10000, sizeof(QZNTOUR::StateData)},\n    // new contracts should be added above this line'
open('/app/src/contract_core/contract_def.h', 'w').write(f.replace(old, new, 1))
print("descriptor patch OK")
