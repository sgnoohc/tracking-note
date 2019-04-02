#include "Event.h"

SDL::Event::Event()
{
}

SDL::Event::~Event()
{
}

void SDL::Event::addModule(SDL::Module* module)
{
    if (modulesMapByDetId_.find(module->detId()) == modulesMapByDetId_.end())
    {
        modules_.push_back(module);
    }
    else
    {
    }
}
