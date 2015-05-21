#include <cstdio>
#include <cmath>

#include "eri/eri.h"
#include "boys/boys.h"
#include "common.hpp"

#define NCART(am) ((am>=0)?((((am)+2)*((am)+1))>>1):0)

#define MAXAM 2

typedef int (*erifunc)(struct multishell_pair const, struct multishell_pair const, double * const restrict);


int eri_notyetimplemented(struct multishell_pair const P,
                          struct multishell_pair const Q,
                          double * const restrict dummy)
{
    printf("****************************\n");
    printf("*** NOT YET IMPLEMENTED! ***\n");
    printf("***  ( %2d %2d | %2d %2d )   ***\n", P.am1, P.am2, Q.am1, Q.am2);
    printf("****************************\n");
    exit(1);
    return 0;
}


bool ValidQuartet(std::array<int, 4> am)
{
    if(am[0] < am[1])
        return false;
    if(am[2] < am[3])
        return false;
    if( (am[0] + am[1]) < (am[2] + am[3]) )
        return false;
    if(am[0] < am[2])
        return false;
    return true;
}


int main(int argc, char ** argv)
{
    
    erifunc funcs_FO[MAXAM+1][MAXAM+1][MAXAM+1][MAXAM+1];
    erifunc funcs_vref[MAXAM+1][MAXAM+1][MAXAM+1][MAXAM+1];
    for(int i = 0; i <= MAXAM; i++)
    for(int j = 0; j <= MAXAM; j++)
    for(int k = 0; k <= MAXAM; k++)
    for(int l = 0; l <= MAXAM; l++)
    {
        funcs_FO[i][j][k][l] = eri_notyetimplemented; 
        funcs_vref[i][j][k][l] = eri_notyetimplemented; 
    }


    funcs_FO[0][0][0][0] = eri_FO_s_s_s_s;
    funcs_FO[1][0][0][0] = eri_FO_p_s_s_s;
    funcs_FO[1][0][1][0] = eri_FO_p_s_p_s;
    funcs_FO[1][1][0][0] = eri_FO_p_p_s_s;
    funcs_FO[1][1][1][0] = eri_FO_p_p_p_s;
    funcs_FO[1][1][1][1] = eri_FO_p_p_p_p;
    funcs_FO[2][0][0][0] = eri_FO_d_s_s_s;
    funcs_FO[2][0][1][0] = eri_FO_d_s_p_s;
    funcs_FO[2][0][1][1] = eri_FO_d_s_p_p;
    funcs_FO[2][0][2][0] = eri_FO_d_s_d_s;
    funcs_FO[2][1][0][0] = eri_FO_d_p_s_s;
    funcs_FO[2][1][1][0] = eri_FO_d_p_p_s;
    funcs_FO[2][1][1][1] = eri_FO_d_p_p_p;
    funcs_FO[2][1][2][0] = eri_FO_d_p_d_s;
    funcs_FO[2][1][2][1] = eri_FO_d_p_d_p;
    funcs_FO[2][2][0][0] = eri_FO_d_d_s_s;
    funcs_FO[2][2][1][0] = eri_FO_d_d_p_s;
    funcs_FO[2][2][1][1] = eri_FO_d_d_p_p;
    funcs_FO[2][2][2][0] = eri_FO_d_d_d_s;
    funcs_FO[2][2][2][1] = eri_FO_d_d_d_p;
    funcs_FO[2][2][2][2] = eri_FO_d_d_d_d;

    funcs_vref[0][0][0][0] = eri_vref_s_s_s_s;
    funcs_vref[1][0][0][0] = eri_vref_p_s_s_s;
    funcs_vref[1][0][1][0] = eri_vref_p_s_p_s;
    funcs_vref[1][1][0][0] = eri_vref_p_p_s_s;
    funcs_vref[1][1][1][0] = eri_vref_p_p_p_s;
    funcs_vref[1][1][1][1] = eri_vref_p_p_p_p;
    funcs_vref[2][0][0][0] = eri_vref_d_s_s_s;
    funcs_vref[2][0][1][0] = eri_vref_d_s_p_s;
    funcs_vref[2][0][1][1] = eri_vref_d_s_p_p;
    funcs_vref[2][0][2][0] = eri_vref_d_s_d_s;
    funcs_vref[2][1][0][0] = eri_vref_d_p_s_s;
    funcs_vref[2][1][1][0] = eri_vref_d_p_p_s;
    funcs_vref[2][1][1][1] = eri_vref_d_p_p_p;
    funcs_vref[2][1][2][0] = eri_vref_d_p_d_s;
    funcs_vref[2][1][2][1] = eri_vref_d_p_d_p;
    funcs_vref[2][2][0][0] = eri_vref_d_d_s_s;
    funcs_vref[2][2][1][0] = eri_vref_d_d_p_s;
    funcs_vref[2][2][1][1] = eri_vref_d_d_p_p;
    funcs_vref[2][2][2][0] = eri_vref_d_d_d_s;
    funcs_vref[2][2][2][1] = eri_vref_d_d_d_p;
    funcs_vref[2][2][2][2] = eri_vref_d_d_d_d;



    if(argc != 9)
    {
        printf("Give me 8 arguments! I got %d\n", argc-1);
        return 1;
    }

    srand(time(NULL));

    std::array<int, 4> nshell = {atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])};
    std::array<int, 4> nprim = {atoi(argv[5]), atoi(argv[6]), atoi(argv[7]), atoi(argv[8])};

    const int maxncart = pow(NCART(MAXAM), 4);

    const int nshell1234 = nshell[0] * nshell[1] * nshell[2] * nshell[3];


    // doesn't do anything at the moment
    Boys_Init(0, 7); // need F0 + 7 for interpolation

    // initialize valeev stuff
    Valeev_Init();



    /* Storage of test results */
    double * res_FO              = (double *)ALLOC(maxncart * nshell1234 * sizeof(double));
    double * res_vref           = (double *)ALLOC(maxncart * nshell1234 * sizeof(double));
    //double * res_liberd          = (double *)ALLOC(maxncart * nshell1234 * sizeof(double));
    double * res_valeev          = (double *)ALLOC(maxncart * nshell1234 * sizeof(double));


    printf("\n");
    printf("%17s    %20s  %20s    %20s  %20s\n", "Quartet", "FO MaxErr", "vref MaxErr", "FO MaxRelErr", "vref MaxRelErr");

    // loop over all quartets, choosing only valid ones
    for(int i = 0; i <= MAXAM; i++)
    for(int j = 0; j <= MAXAM; j++)
    for(int k = 0; k <= MAXAM; k++)
    for(int l = 0; l <= MAXAM; l++)
    {
        std::array<int, 4> am{i, j, k, l};

        if(!ValidQuartet(am))
            continue;
 
        int ncart = NCART(i) * NCART(j) * NCART(k) * NCART(l);   


        // allocate gaussian shell memory
        VecQuartet gshells(  CreateRandomQuartets(nshell, nprim, am) );


        // Actually calculate
        struct multishell_pair P = create_multishell_pair(nshell[0], gshells[0].data(),
                                                          nshell[1], gshells[1].data());
        struct multishell_pair Q = create_multishell_pair(nshell[2], gshells[2].data(),
                                                          nshell[3], gshells[3].data());


        // calcualate with my code
        funcs_FO[am[0]][am[1]][am[2]][am[3]](P, Q, res_FO);
        funcs_vref[am[0]][am[1]][am[2]][am[3]](P, Q, res_vref);

        // test with valeev & liberd
        ValeevIntegrals(gshells, res_valeev);
        //ERDIntegrals(gshells, res_liberd);


        //printf("( %d %d | %d %d )\n", am[0], am[1], am[2], am[3]);
        //printf("%22s %22s %22s\n", "liberd", "FO", "valeev");

        double maxerr_FO = 0;
        double maxerr_vref = 0;

        double maxrelerr_FO = 0;
        double maxrelerr_vref = 0;

        int idx = 0;
        for(int i = 0; i < nshell1234; i++)
        {
            for(int j = 0; j < ncart; j++)
            {
                //printf("%22.4e  %22.4e  %22.4e\n", res_liberd[idx], res_FO[idx], res_valeev[idx]);

                const double v = res_valeev[idx];
                //double diff_liberd  = fabs(res_liberd[idx]         - v);
                double diff_FO      = fabs(res_FO[idx]     - v);
                double diff_vref   = fabs(res_vref[idx]     - v);
                double rel_FO       = fabs(diff_FO / v);
                double rel_vref    = fabs(diff_vref / v);
                //printf("%22.4e  %22.4e\n", diff_liberd, diff_FO);
                //printf("\n");

                if(diff_FO > maxerr_FO)
                    maxerr_FO = diff_FO;
                if(rel_FO > maxrelerr_FO)
                    maxrelerr_FO = rel_FO;

                if(diff_vref > maxerr_vref)
                    maxerr_vref = diff_vref;
                if(rel_vref > maxrelerr_vref)
                    maxrelerr_vref = rel_vref;

                idx++;
            }
            //printf("\n");
        }

        printf("( %2d %2d | %2d %2d )    %20.8e  %20.8e    %20.8e  %20.8e\n", am[0], am[1], am[2], am[3],
                                                      maxerr_FO, maxerr_vref,
                                                      maxrelerr_FO, maxrelerr_vref);

        free_multishell_pair(P);
        free_multishell_pair(Q);
        FreeRandomQuartets(gshells);
    }

    printf("\n");

    Valeev_Finalize();
    Boys_Finalize();

    FREE(res_valeev);
    //FREE(res_liberd);
    FREE(res_vref);
    FREE(res_FO);

    return 0;
}
