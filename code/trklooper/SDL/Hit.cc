#include "Hit.h"

SDL::Hit::Hit()
{
}

SDL::Hit::Hit(float x, float y, float z): x_(x), y_(y), z_(z)
{
}

SDL::Hit::~Hit()
{
}

void SDL::Hit::setX(float x)
{
    x_ = x;
}

void SDL::Hit::setY(float y)
{
    y_ = y;
}

void SDL::Hit::setZ(float z)
{
    z_ = z;
}

float SDL::Hit::x()
{
    return x_;
}

float SDL::Hit::y()
{
    return y_;
}

float SDL::Hit::z()
{
    return z_;
}

float SDL::Hit::X()
{
    return x_;
}

float SDL::Hit::Y()
{
    return y_;
}

float SDL::Hit::Z()
{
    return z_;
}
