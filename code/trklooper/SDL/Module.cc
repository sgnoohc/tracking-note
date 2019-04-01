#include "Module.h"

SDL::Module::Module()
{
}

SDL::Module::~Module()
{
}

unsigned short SDL::Module::order()
{
    return order_;
}

unsigned short SDL::Module::rod()
{
    return rod_;
}

unsigned short SDL::Module::ring()
{
    return ring_;
}

unsigned short SDL::Module::subdet()
{
    return subdet_;
}

unsigned short SDL::Module::module()
{
    return module_;
}

unsigned short SDL::Module::isLower()
{
    return isLower_;
}

unsigned int SDL::Module::detId()
{
    return detId_;
}

bool SDL::Module::isInverted()
{
    return isInverted_;
}

std::vector<Hit> SDL::Module::hits()
{
    return hits_;
}

std::vector<MiniDoublet> SDL::Module::miniDoublets()
{
    return miniDoublets_;
}

