f = '/app/src/contract_core/contract_exec.h'
content = open(f).read()
content = content.replace('TESTEXA_CONTRACT_INDEX', '32')
open(f, 'w').write(content)
print("Fixed contract_exec.h")
