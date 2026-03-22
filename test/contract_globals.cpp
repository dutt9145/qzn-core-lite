#include <stdlib.h>
#include <string.h>
#include <stddef.h>

// Memory functions
void setMem(void* buffer, unsigned long long size, unsigned char value) { memset(buffer, value, (size_t)size); }
void copyMem(void* dest, void const* src, unsigned long long size) { memcpy(dest, src, (size_t)size); }

// Define all GLOBAL_VAR_DECL symbols as opaque blobs
// contractCount=1024, contractSystemProcedureCount=256, NUMBER_OF_CONTRACT_EXECUTION_BUFFERS=64

static unsigned char* _contractStates[1024] = {};
unsigned char** contractStates = _contractStates;

static char _contractSystemProcedures[1024][256][64] = {};
void* (*contractSystemProcedures)[256] = (void*(*)[256])_contractSystemProcedures;

static unsigned short _contractSystemProcedureLocalsSizes[1024][256] = {};
unsigned short (*contractSystemProcedureLocalsSizes)[256] = _contractSystemProcedureLocalsSizes;

static char _contractUserFunctions[1024][65536][64] = {};
void* (*contractUserFunctions)[65536] = (void*(*)[65536])_contractUserFunctions;

static unsigned short _contractUserFunctionInputSizes[1024][65536] = {};
unsigned short (*contractUserFunctionInputSizes)[65536] = _contractUserFunctionInputSizes;
static unsigned short _contractUserFunctionOutputSizes[1024][65536] = {};
unsigned short (*contractUserFunctionOutputSizes)[65536] = _contractUserFunctionOutputSizes;
static unsigned short _contractUserFunctionLocalsSizes[1024][65536] = {};
unsigned short (*contractUserFunctionLocalsSizes)[65536] = _contractUserFunctionLocalsSizes;

static char _contractUserProcedures[1024][65536][64] = {};
void* (*contractUserProcedures)[65536] = (void*(*)[65536])_contractUserProcedures;
static unsigned short _contractUserProcedureInputSizes[1024][65536] = {};
unsigned short (*contractUserProcedureInputSizes)[65536] = _contractUserProcedureInputSizes;
static unsigned short _contractUserProcedureOutputSizes[1024][65536] = {};
unsigned short (*contractUserProcedureOutputSizes)[65536] = _contractUserProcedureOutputSizes;
static unsigned short _contractUserProcedureLocalsSizes[1024][65536] = {};
unsigned short (*contractUserProcedureLocalsSizes)[65536] = _contractUserProcedureLocalsSizes;

// exec globals
static char _contractLocalsStack[64][65536] = {};
void* contractLocalsStack = _contractLocalsStack;
static volatile char _contractLocalsStackLock[64] = {};
volatile char* contractLocalsStackLock = _contractLocalsStackLock;
volatile long contractLocalsStackLockWaitingCount = 0;
long contractLocalsStackLockWaitingCountMax = 0;
static char _contractExecutionErrorData[1024][256] = {};
void* contractExecutionErrorData = _contractExecutionErrorData;
static char _contractStateLock[1024][64] = {};
void* contractStateLock = _contractStateLock;
volatile long long contractTotalExecutionTime[1024] = {};
static char _executionTimeAccumulator[4096] = {};
void* executionTimeAccumulator = _executionTimeAccumulator;
unsigned int contractError[1024] = {};
unsigned long long* contractStateChangeFlags = nullptr;
volatile unsigned int contractCallbacksRunning = 0;
static char _contractActionTracker[65536] = {};
void* contractActionTracker = _contractActionTracker;
void* userProcedureRegistry = nullptr;

// system
static char _system[65536] = {};
void* qubicSystemStruct = _system;

// other common globals
static char _operatorPublicKey[32] = {};
void* operatorPublicKey = _operatorPublicKey;
static char _computorSubseeds[676][32] = {};
void* computorSubseeds = _computorSubseeds;

// Node globals stubs
static char _computorPrivateKeys[676][32] = {};
void* computorPrivateKeys = _computorPrivateKeys;
static char _computorPublicKeys[676][32] = {};
void* computorPublicKeys = _computorPublicKeys;
static char _arbitratorPublicKey[32] = {};
void* arbitratorPublicKey = _arbitratorPublicKey;
static char _dispatcherPublicKey[32] = {};
void* dispatcherPublicKey = _dispatcherPublicKey;
static char _broadcastedComputors[65536] = {};
void* broadcastedComputors = _broadcastedComputors;

// Spectrum
static char _spectrum[65536*64] = {};
void* spectrum = _spectrum;
static char _spectrumDigests[65536*32] = {};
void* spectrumDigests = _spectrumDigests;
static volatile char _spectrumLock = 0;
volatile char* spectrumLock = &_spectrumLock;
static char _spectrumInfo[4096] = {};
void* spectrumInfo = _spectrumInfo;
static long long _entityCategoryPopulations[256] = {};
void* entityCategoryPopulations = _entityCategoryPopulations;
static long long _dustThresholdBurnAll = 0;
void* dustThresholdBurnAll = &_dustThresholdBurnAll;
static long long _dustThresholdBurnHalf = 0;
void* dustThresholdBurnHalf = &_dustThresholdBurnHalf;
static long long _spectrumReorgTotalExecutionTicks = 0;
void* spectrumReorgTotalExecutionTicks = &_spectrumReorgTotalExecutionTicks;

// Assets
static char _assets[65536*128] = {};
void* assets = _assets;
static char _assetDigests[65536*32] = {};
void* assetDigests = _assetDigests;
static unsigned long long _assetChangeFlags = 0;
void* assetChangeFlags = &_assetChangeFlags;

// Common
static char _commonBuffers[65536*4] = {};
void* commonBuffers = _commonBuffers;

void freePool(void* p) { free(p); }
