#include "Hit.h"

SDL::Hit::Hit()
{
}

SDL::Hit::Hit(float x, float y, float z): x_(x), y_(y), z_(z)
{
}

SDL::Hit::Hit(const Hit& hit): x_(hit.x()), y_(hit.y()), z_(hit.z())
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

const float& SDL::Hit::x() const
{
    return x_;
}

const float& SDL::Hit::y() const
{
    return y_;
}

const float& SDL::Hit::z() const
{
    return z_;
}

const float& SDL::Hit::X() const
{
    return x_;
}

const float& SDL::Hit::Y() const
{
    return y_;
}

const float& SDL::Hit::Z() const
{
    return z_;
}

float SDL::Hit::deltaPhi(SDL::Hit* hit)
{
}

float SDL::Hit::deltaPhiChange(SDL::Hit* hit)
{
}
// operators
bool SDL::Hit::operator !=(const Hit& hit) const
{
    return hit.x() != x_ or hit.y() != y_ or hit.z() != z_ ? true : false;
}

bool SDL::Hit::operator ==(const Hit& hit) const
{
    return hit.x() == x_ and hit.y() == y_ and hit.z() == z_ ? true : false;
}

SDL::Hit& SDL::Hit::operator = (const Hit& hit)
{
    x_ = hit.x();
    y_ = hit.y();
    z_ = hit.z();
    return *this;
}

SDL::Hit& SDL::Hit::operator -=(const Hit& hit)
{
    x_ -= hit.x();
    y_ -= hit.y();
    z_ -= hit.z();
    return *this;
}

SDL::Hit& SDL::Hit::operator +=(const Hit& hit)
{
    x_ += hit.x();
    y_ += hit.y();
    z_ += hit.z();
    return *this;
}

