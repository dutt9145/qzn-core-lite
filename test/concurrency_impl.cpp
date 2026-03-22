// Stub implementations of BusyWaitingTracker for test builds
// Avoids pulling in concurrency_impl.h which requires full Qubic system context

#include "platform/concurrency.h"

BusyWaitingTracker::BusyWaitingTracker(const char*, const char*, unsigned int) {}
BusyWaitingTracker::~BusyWaitingTracker() {}
void BusyWaitingTracker::pause() {}
