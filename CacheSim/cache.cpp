#include <cassert>
#include <cmath>
#include <iostream>
#include <utility>
#include "misc.h"
#include "cache.h"

SetCache::SetCache(unsigned int num_lines, unsigned int assoc)
{
   assert(num_lines % assoc == 0);
   // The set bits of the address will be used as an index
   // into sets. Each set is a set containing "assoc" items
   sets.resize(num_lines / assoc);
   lruLists.resize(num_lines / assoc);
   lruMaps.resize(num_lines / assoc);
   for(unsigned int i=0; i < sets.size(); i++) {
      for(unsigned int j=0; j < assoc; ++j) {
         cacheLine temp;
         temp.tag = j;
         temp.state = INV;
         sets[i].insert(temp);
         lruLists[i].push_front(j);
         lruMaps[i].insert(make_pair(j, lruLists[i].begin()));
      }
   }
}

/* FIXME invalid vs not found */
// Given the set and tag, return the cache lines state
cacheState SetCache::findTag(uint64_t set,
                              uint64_t tag) const
{
    cacheLine temp;
    cacheState state = INV;
    temp.tag = tag;
    std::set<cacheLine>::const_iterator it = sets[set].find(temp); // finds the tag.
    
    // Sets the state to the current cache state
    // If it is not found, returns invalid
    if(it != sets[set].end()) {
        state = it->state;
    }
    return state;
}

/* FIXME invalid vs not found */
// Changes the cache line specificed by "set" and "tag" to "state"
void SetCache::changeState(uint64_t set, uint64_t tag,
                              cacheState state)
{
   cacheLine temp;
   temp.tag = tag;
   std::set<cacheLine>::const_iterator it = sets[set].find(temp);

   if(it != sets[set].end()) {
      cacheLine *target;
      target = (cacheLine*)&*it;
      target->state = state;
   }
}

// A complete LRU is mantained for each set, using a separate
// list and map. The front of the list is considered most recently used.
void SetCache::updateLRU(uint64_t set, uint64_t tag)
{
    std::unordered_map<uint64_t, std::list<uint64_t>::iterator>::iterator maps;
    std::list<uint64_t>::iterator it;
    
    // Find the tag in the set
    maps = lruMaps[set].find(tag);

    // Find the tag in the LRU
    it = maps->second;
    
    // Erase it from the list
    lruLists[set].erase(it);
    // Put it in the front of the list
    lruLists[set].push_front(*it);
    // Map maps a tag to the address in memory to the list
    lruMaps[set].erase(maps);
    // Now our new address is at the front
    lruMaps[set].insert(make_pair(tag, lruLists[set].begin()));
}

// Called if a new cache line is to be inserted. Checks if
// the least recently used line needs to be written back to
// main memory.
bool SetCache::checkWriteback(uint64_t set,
                                 uint64_t& tag) const
{
   cacheLine evict, temp;
   tag = lruLists[set].back();
   temp.tag = tag;
   evict = *sets[set].find(temp);

   return (evict.state == MOD || evict.state == OWN);
}

// FIXME: invalid vs not found
// Insert a new cache line by popping the least recently used line
// and pushing the new line to the back (most recently used)
void SetCache::insertLine(uint64_t set, uint64_t tag,
                           cacheState state)
{
    uint64_t eviction = lruLists[set].back();
    cacheLine evict;
    cacheLine newCacheLine;
    
    // Set the tag of the new cache line
    newCacheLine.tag = tag;
    
    // Set the state of the new cache line
    newCacheLine.state = state;
    
    // Set temp cache line to evict
    evict.tag = eviction;
    
    // Erase temp cache line, insert the new line
    sets[set].erase(evict);
    sets[set].insert(newCacheLine);
    
    // Pop the least recently used line
    lruMaps[set].erase(eviction);
    lruLists[set].pop_back();
    
    // Insert the new cache line
    lruLists[set].push_front(tag);
    lruMaps[set].insert(make_pair(tag, lruLists[set].begin()));
}

