#pragma once

#include <stdint.h>


// Store recurrence information
//
// This is a little complicated. The vector element recur_map[am] stores a vector
// of info for cartesian gaussians for that am. For each cartesian gaussian:
//
// ijk: The exponents on x, y, and z (ie, 0,0,0 = s, 1,1,0 = dxy, etc)
// dir: The x, y, z direction that should be used for the vertical recurrence.
//      Many cartesians have several different possible ways they can be formed.
//      For example:
//         dxx - only one way - increment i (the exponent on x)
//         dxy - Two ways: increment i OR j
//         fxyz - increment i, j, or k
//      The dir member represents what is the "best" way (or at least a valid way).
//      0 = x, 1 = y, 2 = z
//
// idx: The index of the cartesian if ijk has been decremented once or twice.
//      For example:
//         fxxy, with i decremented once, results in dxy, whose index is 1 in our ordering.
//               if i is decremented twice, the result is py, whose index is also 1
//         dyz,  decrement j once, results in pz (index 2). decremented twice, the
//               result is -1 (does not exist).
//
//  Having these in a lookup table reduces the need to determine this on the fly, at the
//  expense of a bigger binary (and a compile-time fixed ordering).
//  
//  This information is generated via the helper script generate-potential-recur.py 


struct RecurInfo
{
    int8_t ijk[3];
    int8_t dir;
    int8_t idx[3][2];
};


extern struct RecurInfo const recurinfo_array[455];
extern int const am_recur_map[13];

