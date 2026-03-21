// Minimal stubs for Qubic contract runtime globals
#include <string.h>

void setMem(void* buffer, unsigned long long size, unsigned char value) {
    memset(buffer, value, (size_t)size);
}

// contractCount = 1024, max procedures = 256, max functions = 65536
static void* _sysproc[1024][256] = {};
static unsigned short _sysproc_locals[1024][256] = {};
static void* _userfunc[1024][65536] = {};
static unsigned short _userfunc_in[1024][65536] = {};
static unsigned short _userfunc_out[1024][65536] = {};
static unsigned short _userfunc_locals[1024][65536] = {};
static void* _userproc[1024][65536] = {};
static unsigned short _userproc_in[1024][65536] = {};
static unsigned short _userproc_out[1024][65536] = {};
static unsigned short _userproc_locals[1024][65536] = {};
static unsigned char* _states[1024] = {};

void* (*contractSystemProcedures)[256] = _sysproc;
unsigned short (*contractSystemProcedureLocalsSizes)[256] = _sysproc_locals;
void* (*contractUserFunctions)[65536] = _userfunc;
unsigned short (*contractUserFunctionInputSizes)[65536] = _userfunc_in;
unsigned short (*contractUserFunctionOutputSizes)[65536] = _userfunc_out;
unsigned short (*contractUserFunctionLocalsSizes)[65536] = _userfunc_locals;
void* (*contractUserProcedures)[65536] = _userproc;
unsigned short (*contractUserProcedureInputSizes)[65536] = _userproc_in;
unsigned short (*contractUserProcedureOutputSizes)[65536] = _userproc_out;
unsigned short (*contractUserProcedureLocalsSizes)[65536] = _userproc_locals;
unsigned char** contractStates = _states;
