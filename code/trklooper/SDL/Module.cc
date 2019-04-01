#include "Module.h"

SDL::Module::Module()
{
}

SDL::Module::~Module()
{
}

const unsigned short& SDL::Module::order()
{
    return order_;
}

const unsigned short& SDL::Module::rod()
{
    return rod_;
}

const unsigned short& SDL::Module::ring()
{
    return ring_;
}

const unsigned short& SDL::Module::subdet()
{
    return subdet_;
}

const unsigned short& SDL::Module::module()
{
    return module_;
}

const unsigned short& SDL::Module::isLower()
{
    return isLower_;
}

const unsigned int& SDL::Module::detId()
{
    return detId_;
}

const bool& SDL::Module::isInverted()
{
    return isInverted_;
}

const std::vector<SDL::Hit*>& SDL::Module::hits()
{
    return hits_;
}

const std::vector<SDL::MiniDoublet*>& SDL::Module::miniDoublets()
{
    return miniDoublets_;
}

void SDL::Module::addHit(SDL::Hit* hit)
{
    hits_.push_back(hit);
}

void SDL::Module::makeMiniDoublets()
{

    // Loop over lower hits
    for (auto& lowerHit : lowerHits_)
    {

        // Loop over upper hits
        for (auto& upperHit : upperHits_)
        {


        }

    }

}
