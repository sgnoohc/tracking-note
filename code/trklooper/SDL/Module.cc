#include "Module.h"

SDL::Module::Module()
{
}

SDL::Module::~Module()
{
}

const unsigned short& SDL::Module::order() const
{
    return order_;
}

const unsigned short& SDL::Module::rod() const
{
    return rod_;
}

const unsigned short& SDL::Module::ring() const
{
    return ring_;
}

const unsigned short& SDL::Module::subdet() const
{
    return subdet_;
}

const unsigned short& SDL::Module::module() const
{
    return module_;
}

const unsigned short& SDL::Module::isLower() const
{
    return isLower_;
}

const unsigned int& SDL::Module::detId() const
{
    return detId_;
}

const bool& SDL::Module::isInverted() const
{
    return isInverted_;
}

const std::vector<SDL::Hit*>& SDL::Module::hits() const
{
    return hits_;
}

void SDL::Module::addHit(SDL::Hit* hit)
{
    hits_.push_back(hit);
}

