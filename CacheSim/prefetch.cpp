#include "prefetch.h"
#include "system.h"

int NullPrefetch::prefetchMiss(uint64_t address __attribute__((unused)),
                              unsigned int tid __attribute__((unused)),
                              System* sys __attribute__((unused)))
{
   return 0;
}
int NullPrefetch::prefetchHit(uint64_t address __attribute__((unused)),
                           unsigned int tid __attribute__((unused)),
                           System* sys __attribute__((unused)))
{
   return 0;
}

