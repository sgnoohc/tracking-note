#include "Math.h"

float SDL::Math::Phi_mpi_pi(float x)
{
    if (isnan(x))
    {
        std::cout << "SDL::Math::Phi_0_2pi() function called with NaN" << std::endl;
        return x;
    }

    while (x >= M_PI)
        x -= 2. * M_PI;

    while (x < -M_PI)
        x += 2. * M_PI;

    return x;

}
