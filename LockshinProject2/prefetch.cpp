#include "prefetch.h"
#include "system.h"
#include <stdio.h>

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

// Prefetch adjacent cache line: miss (performs same as hit)
int AdjPrefetch::prefetchMiss(uint64_t address, unsigned int tid,
                              System* sys)
{
    sys->memAccess(address + (1 << sys->SET_SHIFT), 'R', tid, true);
    return 1;
}

// Prefetch adjacent cache line: hit
int AdjPrefetch::prefetchHit(uint64_t address, unsigned int tid,
                             System* sys)
{
    sys->memAccess(address + (1 << sys->SET_SHIFT), 'R', tid, true);
    return 1;
}

// Prefetch next n cache lines
SeqPrefetch :: SeqPrefetch(unsigned int num)
{
    n = num;
}

// Prefetch next n cache lines: miss (performs same as hit)
int SeqPrefetch::prefetchMiss(uint64_t address, unsigned int tid,
                              System* sys)
{
    for (unsigned int i = 1; i <= n; i++)
    {
        sys->memAccess(address +  i * (sys ->LINE_MASK +1) , 'R' , tid, true);
    }
    return n;
}

// Prefetch next n cache lines: hit
int SeqPrefetch::prefetchHit(uint64_t address, unsigned int tid,
                             System* sys)
{
    for (unsigned int i = 1; i <= n; i++)
    {
        sys->memAccess(address +  i * (sys ->LINE_MASK +1) , 'R' , tid, true);
    }
    return n;
}

// Implements Best Effort Prefetch
BestEffortPrefetch :: BestEffortPrefetch()
{
    seqPrefetch = new SeqPrefetch(9);
}

// Best Effort Prefetch: prefetch miss (performs same as hit)
int BestEffortPrefetch::prefetchMiss(uint64_t address, unsigned int tid,
                                     System* sys)
{
    // Adds a new relation (previous address, current address)
    newRelation(prev, address);
    // Sets the current address to the previous address
    prev = address;
    // Finds the maxes
    std::vector<uint64_t> allMaxes = findMaxes(address);
    // Prefetches the maxes
    int n = 7;
    for (unsigned int i = 0; i < allMaxes.size(); i++ )
    {
        sys->memAccess(allMaxes[i], 'R', tid, true);
        n--;
    }
    seqPrefetch -> prefetchHit(address, tid, sys);
    return n;
}

// Best Effort Prefetch: prefetch hit
int BestEffortPrefetch::prefetchHit(uint64_t address, unsigned int tid,
                                    System* sys)
{
    // Adds a new relation (previous address, current address)
    newRelation(prev, address);
    // Sets the current address to the previous address
    prev = address;
    // Finds the maxes
    std::vector<uint64_t> allMaxes = findMaxes(address);
    // Prefetches the maxes
    int n = 7;
    for (unsigned int i = 0; i < allMaxes.size(); i++ )
    {
        sys->memAccess(allMaxes[i], 'R', tid, true);
        n--;
    }
    seqPrefetch -> prefetchHit(address, tid, sys);
    return n;
}

// Method that adds a new relation to the relations map
void BestEffortPrefetch::newRelation(uint64_t relation1, uint64_t relation2)
{
    allRelations[relation1][relation2]++;
}

// Method that finds/returns the maximum four relations
std::vector<uint64_t> BestEffortPrefetch::findMaxes(uint64_t address)
{
    // Sets the current map
    std::map<uint64_t, unsigned int> map = allRelations[address];
    std::vector<uint64_t> maxes;
    std::vector<std::pair<uint64_t, unsigned int>> max_four(4);
    
    // Finds/sorts maximum four
    std::partial_sort_copy(map.begin(), map.end(), max_four.begin(), max_four.end(),
                           [] (std::pair<const uint64_t, unsigned int> const& l,
                               std::pair<const uint64_t, unsigned int> const& r)
                           {
                               return l.second > r.second;
                           });
    
    // Adds top four maxes to the back of the allMaxes vector
    for (int i = 0; i < 4; i++)
    {
        maxes.push_back(max_four[i].first);
    }
    
    return maxes;
}
