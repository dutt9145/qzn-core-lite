// Platform stubs - opaque blobs matching GLOBAL_VAR_DECL symbols
#include <cstddef>
#include <cstdint>
#include <cstring>

// unsigned long long frequency (time_stamp_counter.h)
unsigned long long frequency = 1000000000ULL;

// ProfilingDataCollector gProfilingDataCollector (profiling.h)
static char _gPDC[4096] = {};
extern "C++" {
    namespace { struct ProfilingDataCollector { char _p[4096]; }; }
}
void* gProfilingDataCollector_ptr = _gPDC;
// Use weak symbol trick
__attribute__((weak)) extern char gProfilingDataCollector[4096];
char gProfilingDataCollector[4096] = {};

// qLogger logger (logging.h)
__attribute__((weak)) extern char logger[4096];
char logger[4096] = {};

// volatile char universeLock (assets.h)
volatile char universeLock = 0;

// EFI_TIME utcTime (time.h) - EFI_TIME is 16 bytes
__attribute__((weak)) extern char utcTime[16];
char utcTime[16] = {};
