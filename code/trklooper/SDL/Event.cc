#include "Event.h"

SDL::Event::Event()
{
}

SDL::Event::~Event()
{
}

bool SDL::Event::hasModule(unsigned int detId)
{
    if (modulesMapByDetId_.find(detId) == modulesMapByDetId_.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

SDL::Module& SDL::Event::getModule(unsigned int detId)
{
    // If not exists, create and add and return
    // If exists, just return;
    auto& inserted_or_existing = (*(modulesMapByDetId_.emplace(detId, detId).first)).second;
    return inserted_or_existing;
}

void SDL::Event::addHitToModule(SDL::Hit hit, unsigned int detId)
{
    // Add to global list of hits, where it will hold the object's instance
    hits_.push_back(hit);

    // And get the module (if not exists, then create), and add the address to Module.hits_
    getModule(detId).addHit(&(hits_.back()));
}
