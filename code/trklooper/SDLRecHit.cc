#include "SDLRecHit.h"

SDLRecHit::SDLRecHit()
{
}

SDLRecHit::SDLRecHit(float x_, float y_, float z_): x(x_), y(y_), z(z_)
{
}

~SDLRecHit::SDLRecHit()
{
}

void SDLRecHit::setX(float x_)
{
    x = x_;
}

void SDLRecHit::setY(float y_)
{
    y = y_;
}

void SDLRecHit::setZ(float z_)
{
    z = z_;
}

float SDLRecHit::x()
{
    return x;
}

float SDLRecHit::y()
{
    return y;
}

float SDLRecHit::z()
{
    return z;
}

float SDLRecHit::X()
{
    return x;
}

float SDLRecHit::Y()
{
    return y;
}

float SDLRecHit::Z()
{
    return z;
}
