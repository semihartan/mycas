#include <mcmath.h>

MC_INLINE double mc_fmod(double x, double y)
{
    return x - y * floor(x / y);
}