#pragma once

#include "simint/simint_config.h" // for USE_ET define

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


/*! \brief Information about a gaussian shell */
struct gaussian_shell
{
    int am;          //!< Angular momentum (0 = s, etc)
    int nprim;       //!< Number of primitives in this shell

    double x;        //!< X
    double y;
    double z;

    double * alpha;
    double * coef;
};

struct multishell_pair
{
    int am1, am2;          // angular momentum.
    int nprim;             // Total number of primitives
    int nprim_length;      // Acutal length of alpha, etc, arrays, including padding (for alignment)

    size_t memsize;          // Total memory for doubles (in bytes)

    int nshell1, nshell2;  // number of shells
    int nshell12;          // nshell1 * nshell2
    int * nprim12;    // length nshell12;

    int nbatch;
    int * nbatchprim;   // length nbatch. number of primitives in a batch, including padding

    // length nshell12
    double * AB_x;
    double * AB_y;
    double * AB_z;

    // these are all of length nprim
    double * x;
    double * y;
    double * z;
    double * PA_x;
    double * PA_y;
    double * PA_z;
    double * PB_x;
    double * PB_y;
    double * PB_z;

    #ifdef SIMINT_ERI_USE_ET
    double * bAB_x;
    double * bAB_y;
    double * bAB_z;
    #endif

    double * alpha;
    double * prefac;
};


void allocate_gaussian_shell(int nprim, struct gaussian_shell * const restrict G);
void free_gaussian_shell(struct gaussian_shell G);
struct gaussian_shell copy_gaussian_shell(const struct gaussian_shell G);
void normalize_gaussian_shells(int n, struct gaussian_shell * const restrict G);


void allocate_multishell_pair(int na, struct gaussian_shell const * const restrict A,
                              int nb, struct gaussian_shell const * const restrict B,
                              struct multishell_pair * const restrict P);

void free_multishell_pair(struct multishell_pair P);

void fill_multishell_pair(int na, struct gaussian_shell const * const restrict A,
                          int nb, struct gaussian_shell const * const restrict B,
                          struct multishell_pair * const restrict P);

struct multishell_pair
create_multishell_pair(int na, struct gaussian_shell const * const restrict A,
                       int nb, struct gaussian_shell const * const restrict B);


#ifdef __cplusplus
}
#endif
