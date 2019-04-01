#include "RecHit.h"

SDL::RecHit::RecHit()
{
}

SDL::RecHit::RecHit(float x, float y, float z): x_(x), y_(y), z_(z)
{
}

SDL::RecHit::~RecHit()
{
}

void SDL::RecHit::setX(float x)
{
    x_ = x;
}

void SDL::RecHit::setY(float y)
{
    y_ = y;
}

void SDL::RecHit::setZ(float z)
{
    z_ = z;
}

float SDL::RecHit::x()
{
    return x_;
}

float SDL::RecHit::y()
{
    return y_;
}

float SDL::RecHit::z()
{
    return z_;
}

float SDL::RecHit::X()
{
    return x_;
}

float SDL::RecHit::Y()
{
    return y_;
}

float SDL::RecHit::Z()
{
    return z_;
}
