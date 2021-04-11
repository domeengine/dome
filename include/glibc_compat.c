/* This file is only used on GNU/Linux (ie glibc) when compiling with portable=1,
   in order to produce binaries that don't depend on functions only present
   in newer glibc versions.
   (The .symver lines below could instead be placed in a header included everywhere
   if we weren't linking to any libraries compiled without the header, eg libfb)

   See https://rpg.hamsterrepublic.com/ohrrpgce/Portable_GNU-Linux_binaries
   for more info and instructions for updating.

   Placed in the public domain.
*/

#ifdef __linux__
#include <stdarg.h>
#include <math.h>

// x86_64 glibc -
#define asm __asm__
asm (".symver pow, pow@GLIBC_2.2.5");
asm (".symver exp, exp@GLIBC_2.2.5");
asm (".symver log, log@GLIBC_2.2.5");
asm (".symver log2, log2@GLIBC_2.2.5");
asm (".symver logf, logf@GLIBC_2.2.5");
asm (".symver powf, powf@GLIBC_2.2.5");
asm (".symver expf, expf@GLIBC_2.2.5");
#undef asm

// I couldn't figure out what has changed in pow, exp, log in glibc 2.29.
// Interestingly despite compiling with -fno-omit-frame-pointer, GCC
// optimises the following to a jmp anyway.

double __wrap_pow(double x, double y)
{
    return pow(x, y);
}

double __wrap_exp(double x)
{
    return exp(x);
}

double __wrap_log(double x)
{
    return log(x);
}

double __wrap_log2(double x)
{
    return log2(x);
}

double __wrap_logf(double x)
{
    return logf(x);
}

double __wrap_powf(double x, double y)
{
    return powf(x, y);
}

double __wrap_expf(double x)
{
    return expf(x);
}
#endif
