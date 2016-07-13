#include <math.h>
#include <string.h>

#include "simint/vectorization/vectorization.h"
#include "simint/constants.h"
#include "simint/shell/shell.h"
#include "simint/shell/shell_constants.h"


#if defined(__ICC) || defined(__INTEL_COMPILER)
    #pragma warning(disable:1338)                  // Pointer arithmetic on void
#elif defined(__GNUC__) || defined(__GNUG__)
    #pragma GCC diagnostic ignored -Wpointer-arith // Pointer arithmetic on void
#endif


#define NCART(am) ((am>=0)?((((am)+2)*((am)+1))>>1):0)

extern double const norm_fac[SHELL_PRIM_NORMFAC_MAXL+1];

// Allocate a gaussian shell with correct alignment
void simint_allocate_shell(int nprim, struct simint_shell * const restrict G)
{
    int prim_size = SIMINT_SIMD_ROUND(nprim);
    size_t size = 2 * prim_size * sizeof(double);   

    double * mem = ALLOC(size);
    G->alpha = mem;
    G->coef = mem + prim_size;

    G->ptr = mem;
    G->memsize = size;
}


void simint_free_shell(struct simint_shell G)
{
    FREE(G.ptr);
    G.ptr = NULL;
    G.memsize = 0;
}

struct simint_shell simint_copy_shell(const struct simint_shell G)
{
    struct simint_shell G_copy;
    simint_allocate_shell(G.nprim, &G_copy);

    G_copy.nprim = G.nprim;
    G_copy.am = G.am;
    G_copy.x = G.x;
    G_copy.y = G.y;
    G_copy.z = G.z;

    memcpy(G_copy.ptr, G.ptr, G.memsize);
    return G_copy;
}


void simint_normalize_shells(int n, struct simint_shell * const restrict G)
{
    for(int i = 0; i < n; ++i)
    {
        const int iam = G[i].am;
        const double am = (double)iam;
        const double m = am + 1.5;
        const double m2 = 0.5 * m;

        double sum = 0.0;

        for(int j = 0; j < G[i].nprim; j++)
        {
            const double a1 = G[i].alpha[j];
            const double c1 = G[i].coef[j];

            for(int k = 0; k < G[i].nprim; k++)
            {
                const double a2 = G[i].alpha[k];
                const double c2 = G[i].coef[k];
                sum += ( c1 * c2 *  pow(a1*a2, m2) ) / ( pow(a1+a2, m) );
            }
        }

        const double norm = 1.0 / sqrt(sum * norm_fac[iam]);

        // apply the rest of the normalization
        for (int j = 0; j < G[i].nprim; ++j)
            G[i].coef[j] *= norm * pow(G[i].alpha[j], m2);
    }
}




void simint_allocate_multi_shellpair(int na, struct simint_shell const * const restrict A,
                                     int nb, struct simint_shell const * const restrict B,
                                     struct simint_multi_shellpair * const restrict P)
{
    int nprim = 0;

    int nanb = 0;
    int batchprim = 0;
    for(int i = 0; i < na; ++i)
    for(int j = 0; j < nb; ++j)
    {
        batchprim += A[i].nprim * B[j].nprim;
        nanb++;

        if((nanb % SIMINT_NSHELL_SIMD) == 0 || nanb >= na*nb)
        {        
            nprim += SIMINT_SIMD_ROUND(batchprim);
            batchprim = 0;
        }
    }


    int nshell12 = na*nb;
    int nbatch = nshell12 / SIMINT_NSHELL_SIMD;
    if(nbatch % SIMINT_NSHELL_SIMD)
        nbatch++;


    const size_t dprim_size = nprim * sizeof(double);
    const size_t ishell12_size = nshell12 * sizeof(int);
    const size_t dshell12_size = nshell12 * sizeof(double);
    const size_t ibatch_size = nbatch * sizeof(int);

    int ndprim_size = 11;

    #ifdef SIMINT_ERI_USE_ET
    ndprim_size += 3;
    #endif

    const size_t memsize = dprim_size*ndprim_size + dshell12_size*3 + ishell12_size + ibatch_size;
    P->memsize = memsize;

    int dcount = 0;
    // allocate one large space
    P->ptr = ALLOC(memsize); 
    P->x          = P->ptr + dprim_size*(dcount++);
    P->y          = P->ptr + dprim_size*(dcount++);
    P->z          = P->ptr + dprim_size*(dcount++);
    P->PA_x       = P->ptr + dprim_size*(dcount++);
    P->PA_y       = P->ptr + dprim_size*(dcount++);
    P->PA_z       = P->ptr + dprim_size*(dcount++);
    P->PB_x       = P->ptr + dprim_size*(dcount++);
    P->PB_y       = P->ptr + dprim_size*(dcount++);
    P->PB_z       = P->ptr + dprim_size*(dcount++);
    P->alpha      = P->ptr + dprim_size*(dcount++);
    P->prefac     = P->ptr + dprim_size*(dcount++);

    #ifdef SIMINT_ERI_USE_ET
    P->bAB_x      = P->ptr + dprim_size*(dcount++);
    P->bAB_y      = P->ptr + dprim_size*(dcount++);
    P->bAB_z      = P->ptr + dprim_size*(dcount++);
    #endif
    

    // below are unaligned
    P->AB_x       = P->ptr + 11*dprim_size;
    P->AB_y       = P->ptr + 11*dprim_size +   dshell12_size;
    P->AB_z       = P->ptr + 11*dprim_size + 2*dshell12_size;
    P->nprim12    = P->ptr + 11*dprim_size + 3*dshell12_size;
    P->nbatchprim = P->ptr + 11*dprim_size + 3*dshell12_size + ishell12_size;
}


void simint_free_multi_shellpair(struct simint_multi_shellpair P)
{
   // Only need to free P.x since that points to the beginning of mem
   FREE(P.ptr);
   P.ptr = NULL;
   P.memsize = 0;
}


void simint_fill_multi_shellpair(int na, struct simint_shell const * const restrict A,
                                 int nb, struct simint_shell const * const restrict B,
                                 struct simint_multi_shellpair * const restrict P)
{
    int i, j, sa, sb, sasb, idx, ibatch, batchprim;

    P->nshell1 = na;
    P->nshell2 = nb;
    P->nshell12 = na * nb;
    P->nprim = 0;

    P->nbatch = P->nshell12 / SIMINT_NSHELL_SIMD;
    if(P->nshell12 % SIMINT_NSHELL_SIMD)
        P->nbatch++;

    // zero out
    memset(P->x, 0, P->memsize);

    sasb = 0;
    idx = 0;
    ibatch = 0;
    batchprim = 0;
    for(sa = 0; sa < na; ++sa)
    {
        P->am1 = A[sa].am;

        for(sb = 0; sb < nb; ++sb)
        {
            P->am2 = B[sb].am;

            // do Xab = (Xab_x **2 + Xab_y ** 2 + Xab_z **2)
            const double Xab_x = A[sa].x - B[sb].x;
            const double Xab_y = A[sa].y - B[sb].y;
            const double Xab_z = A[sa].z - B[sb].z;
            const double Xab = Xab_x*Xab_x + Xab_y*Xab_y + Xab_z*Xab_z;

            for(i = 0; i < A[sa].nprim; ++i)
            {
                const double AxAa = A[sa].x * A[sa].alpha[i];
                const double AyAa = A[sa].y * A[sa].alpha[i];
                const double AzAa = A[sa].z * A[sa].alpha[i];

                for(j = 0; j < B[sb].nprim; ++j)
                {
                    const double p_ab = A[sa].alpha[i] + B[sb].alpha[j];
                    const double ABalpha = A[sa].alpha[i] * B[sb].alpha[j];

                    P->prefac[idx] = A[sa].coef[i] * B[sb].coef[j]
                                     * exp(-Xab * ABalpha / p_ab)
                                     * SQRT_TWO_PI_52 / p_ab;

                    P->x[idx] = (AxAa + B[sb].alpha[j]*B[sb].x)/p_ab;
                    P->y[idx] = (AyAa + B[sb].alpha[j]*B[sb].y)/p_ab;
                    P->z[idx] = (AzAa + B[sb].alpha[j]*B[sb].z)/p_ab;
                    P->PA_x[idx] = P->x[idx] - A[sa].x;
                    P->PA_y[idx] = P->y[idx] - A[sa].y;
                    P->PA_z[idx] = P->z[idx] - A[sa].z;
                    P->PB_x[idx] = P->x[idx] - B[sb].x;
                    P->PB_y[idx] = P->y[idx] - B[sb].y;
                    P->PB_z[idx] = P->z[idx] - B[sb].z;

                    #ifdef SIMINT_ERI_USE_ET
                    P->bAB_x[idx] = B[sb].alpha[j]*Xab_x;
                    P->bAB_y[idx] = B[sb].alpha[j]*Xab_y;
                    P->bAB_z[idx] = B[sb].alpha[j]*Xab_z;
                    #endif

                    P->alpha[idx] = p_ab;
                    ++idx;
                }

            }

            P->nprim12[sasb] = A[sa].nprim*B[sb].nprim;
            P->nprim += A[sa].nprim*B[sb].nprim;

            P->AB_x[sasb] = Xab_x;
            P->AB_y[sasb] = Xab_y;
            P->AB_z[sasb] = Xab_z;

            batchprim += P->nprim12[sasb]; 

            sasb++;

            if((sasb % SIMINT_NSHELL_SIMD) == 0 || sasb >= na*nb)
            {
                // fill in alpha = 1 until next boundary
                while(idx < SIMINT_SIMD_ROUND(idx))
                    P->alpha[idx++] = 1.0;

                // increment the batch and store the number of primitives
                // in this batch
                P->nbatchprim[ibatch] = SIMINT_SIMD_ROUND(batchprim);
                batchprim = 0;
                ibatch++;
            }
        }
    }
}


struct simint_multi_shellpair
simint_create_multi_shellpair(int na, struct simint_shell const * const restrict A,
                              int nb, struct simint_shell const * const restrict B)
{
    struct simint_multi_shellpair P;
    simint_allocate_multi_shellpair(na, A, nb, B, &P);
    simint_fill_multi_shellpair(na, A, nb, B, &P);
    return P; 
}
