#ifndef PREFETCH_H
#define PREFETCH_H

#include <cstdint>
#include <vector>
#include <map>

class System;

class Prefetch {
public:
    virtual int prefetchMiss(uint64_t address, unsigned int tid,
                             System* sys)=0;
    virtual int prefetchHit(uint64_t address, unsigned int tid,
                            System* sys)=0;
};

//"Prefetcher" that does nothing
class NullPrefetch : public Prefetch {
public:
    int prefetchMiss(uint64_t address, unsigned int tid,
                     System* sys);
    int prefetchHit(uint64_t address, unsigned int tid,
                    System* sys);
};

// Prefetches adjacent cache line
class AdjPrefetch : public Prefetch {
public:
    int prefetchMiss(uint64_t address, unsigned int tid,
                     System* sys);
    int prefetchHit(uint64_t address, unsigned int tid,
                    System* sys);
};

// Prefetches next n cache lines
class SeqPrefetch : public Prefetch {
    unsigned int n;
public:
    SeqPrefetch(unsigned int num);
    int prefetchMiss(uint64_t address, unsigned int tid,
                     System* sys);
    int prefetchHit(uint64_t address, unsigned int tid,
                    System* sys);
};

// Implements BestEffortPrefetch
class BestEffortPrefetch : public Prefetch {
    // Previous address
    uint64_t prev;
    // Sequential prefetch
    SeqPrefetch *seqPrefetch;
    // Map of relations
    std::map<uint64_t, std::map<uint64_t, unsigned int>> allRelations;
    // Method to find maxes in relations
    std::vector<uint64_t> findMaxes(uint64_t add);
    // Method for a new relation
    void newRelation(uint64_t relation1, uint64_t relation2);
public:
    BestEffortPrefetch();
    int prefetchMiss(uint64_t address, unsigned int tid,
                     System* sys);
    int prefetchHit(uint64_t address, unsigned int tid,
                    System* sys);
};

#endif

