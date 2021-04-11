/* This file is only used on GNU/Linux (ie glibc) when compiling with portable=1,
   in order to produce binaries that don't depend on functions only present
   in newer glibc versions.
   Functions like fcntl are redirected by ld to __wrap_fcntl etc (see
   SConscript), defined in this file, which are wrappers around a shadowed but
   still present symbol in libc.so or libm.so.
   (The .symver lines below could instead be placed in a header included everywhere
   if we weren't linking to any libraries compiled without the header, eg libfb)

   See https://rpg.hamsterrepublic.com/ohrrpgce/Portable_GNU-Linux_binaries
   for more info and instructions for updating.

   Placed in the public domain.
*/

#ifdef __linux__
#include <stdarg.h>
#include <math.h>

#ifdef HOST_64BIT
// x86_64 glibc... I'm guessing ARM64, etc, has the same versioned symbols
asm (".symver pow, pow@GLIBC_2.2.5");
asm (".symver exp, exp@GLIBC_2.2.5");
asm (".symver log, log@GLIBC_2.2.5");
asm (".symver log2, log@GLIBC_2.2.5");
#else
// x86 glibc... I'm guessing ARM32, etc, has the same versioned symbols
asm (".symver pow, pow@GLIBC_2.0");
asm (".symver exp, exp@GLIBC_2.0");
asm (".symver log, log@GLIBC_2.0");
asm (".symver log2, log@GLIBC_2.0");
#endif

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
#endif
