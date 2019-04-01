#include "RecHit.h"

SDL::RecHit::RecHit()
{
}

SDL::RecHit::RecHit(float x_, float y_, float z_): x(x_), y(y_), z(z_)
{
}

~SDL::RecHit::RecHit()
{
}

void SDL::RecHit::setX(float x_)
{
    x = x_;
}

void SDL::RecHit::setY(float y_)
{
    y = y_;
}

void SDL::RecHit::setZ(float z_)
{
    z = z_;
}

float SDL::RecHit::x()
{
    return x;
}

float SDL::RecHit::y()
{
    return y;
}

float SDL::RecHit::z()
{
    return z;
}

float SDL::RecHit::X()
{
    return x;
}

float SDL::RecHit::Y()
{
    return y;
}

float SDL::RecHit::Z()
{
    return z;
}
