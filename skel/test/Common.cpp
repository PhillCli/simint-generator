#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <fstream>
#include <cmath>
#include <array>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "test/Common.hpp"


bool ValidQuartet(std::array<int, 4> am)
{
    if(am[0] < am[1])
        return false;
    if(am[2] < am[3])
        return false;
    if( (am[0] + am[1]) < (am[2] + am[3]) )
        return false;
    if( (am[0] + am[1]) == (am[2] + am[3]) && (am[0] < am[2]) ) 
        return false;
    return true;
}



bool ValidQuartet(int i, int j, int k, int l)
{
    return ValidQuartet({i, j, k, l});
}



bool IterateGaussian(std::array<int, 3> & g)
{
    int am = g[0] + g[1] + g[2];

    if(g[2] == am)  // at the end
        return false;

    if(g[2] < (am - g[0]))
        g = {g[0],   g[1]-1,      g[2]+1 };
    else
        g = {g[0]-1, am-g[0]+1, 0        };

    return (g[0] >= 0 && g[1] >= 0 && g[2] >= 0); 
}




GaussianVec CopyGaussianVec(const GaussianVec & v)
{
    GaussianVec copy;
    copy.reserve(v.size());
    for(const auto & it : v)
        copy.push_back(copy_gaussian_shell(it));
    return copy;
}



void FreeGaussianVec(GaussianVec & agv)
{
    for(auto & it : agv)
        free_gaussian_shell(it);
    agv.clear();
}



ShellMap CopyShellMap(const ShellMap & m)
{
    ShellMap copy;
    for(auto & it : m)
        copy.insert({it.first, CopyGaussianVec(it.second)});
    return copy;
}



void FreeShellMap(ShellMap & m)
{
    for(auto & it : m)
        FreeGaussianVec(it.second);
    m.clear();
}



ShellMap ReadBasis(const std::string & file)
{
    std::map<char, int> ammap = {
                                         { 'S',     0 },
                                         { 'P',     1 },
                                         { 'D',     2 },
                                         { 'F',     3 },
                                         { 'G',     4 },
                                         { 'H',     5 },
                                         { 'I',     6 },
                                         { 'J',     7 },
                                         { 'K',     8 },
                                         { 'L',     9 },
                                         { 'M',    10 },
                                         { 'N',    11 },
                                         { 'O',    12 },
                                         { 'Q',    13 },
                                         { 'R',    14 },
                                         { 'T',    15 },
                                         { 'U',    16 },
                                         { 'V',    17 },
                                         { 'W',    18 },
                                         { 'X',    19 },
                                         { 'Y',    20 },
                                         { 'Z',    21 },
                                         { 'A',    22 },
                                         { 'B',    23 },
                                         { 'C',    24 },
                                         { 'E',    25 }
                                 };

    std::ifstream f(file.c_str());
    if(!f.is_open())
        throw std::runtime_error(std::string("Error opening file ") + file);

    f.exceptions(std::ifstream::badbit | std::ifstream::failbit | std::ifstream::eofbit);

    ShellMap shellmap;

    int natom;
    f >> natom;


    for(int i = 0; i < natom; i++)
    {
        std::string sym; // not really used
        int nshell, nallprim, nallprimg;  // nprim, nprimg not really used
        double x, y, z;

        f >> sym >> nshell >> nallprim >> nallprimg >> x >> y >> z;

        for(int j = 0; j < nshell; j++)
        {
            std::string type;
            int nprim, ngen;
            f >> type >> nprim >> ngen;


            // allocate the shell
            // set the angular momentum
            // (circularly loop throught the type to handle sp, spd, etc)
            auto itg = type.begin();
            GaussianVec gvec(ngen);
            for(auto & it : gvec)
            {
                allocate_gaussian_shell(nprim, &it);
                it.am = ammap[*itg];
                it.nprim = nprim;
                it.x = x;
                it.y = y;
                it.z = z;

                itg++;
                if(itg == type.end())
                    itg = type.begin();
            }

            // loop through general contractions
            for(int k = 0; k < nprim; k++)
            {
                double alpha;
                f >> alpha;

                for(int l = 0; l < ngen; l++)
                {
                    gvec[l].alpha[k] = alpha;
                    f >> gvec[l].coef[k];
                }

            }

            // add shell to the vector for its am
            for(auto & it : gvec)
                shellmap[it.am].push_back(it);
        }
    }

    return shellmap;
}



std::array<int, 3> FindMapMaxParams(const ShellMap & m)
{
    int maxnprim = 0;
    int maxam = 0;
    int maxel = 0;
    for(auto & it : m)
    {
        const int nca = NCART(it.first);
        const int nsh = it.second.size();
        const int n = nca * nsh;
        if(n > maxel)
            maxel = n;

        if(it.first > maxam)
            maxam = it.first;

        for(auto & it2 : it.second)
        {
            if(it2.nprim > maxnprim)
                maxnprim = it2.nprim;
        }
    }

    return {maxam, maxnprim, maxel*maxel*NCART(maxam)*NCART(maxam)};
}







void Chop(double * const restrict calc, int ncalc)
{
    for(int i = 0; i < ncalc; i++)
    {
        if(fabs(calc[i]) < RND_ZERO)
            calc[i] = 0.0;
    }
}



std::pair<double, double> CalcError(double const * const restrict calc, double const * const restrict ref, int ncalc)
{
    double maxerr = 0;
    double maxrelerr = 0;

    for(int i = 0; i < ncalc; i++)
    {
        const double r = ref[i];
        const double diff = fabs(calc[i] - r);
        const double rel = (fabs(ref[i]) > RND_ZERO ? fabs(diff / r) : 0.0);
        if(diff > maxerr)
            maxerr = diff;
        if(rel > maxrelerr)
            maxrelerr = rel;
    }

    return std::pair<double, double>(maxerr, maxrelerr);
}

    


void PrintTimingHeader(void)
{
    // Timing header
    printf("%13s %12s  %12s  %16s  %16s  %16s  %16s  %16s  %16s  %16s  %12s\n",
                           "Quartet", "NCont", "NPrim", "Ticks(Fill)", "Ticks(Copy)", "Ticks(Pre)",
                                                        "Ticks(Boys)", "Ticks(Ints)", "Ticks(Perm)",
                                                        "Ticks(Total)", "Ticks/Prim");
}


void PrintAMTimingInfo(int i, int j, int k, int l, size_t nshell1234, size_t nprim1234, const TimeContrib & info)
{
        printf("( %d %d | %d %d ) %12lu  %12lu  %16llu  %16llu  %16llu  %16llu  %16llu  %16llu  %16llu  %12.3f\n",
                                                                      i, j, k, l,
                                                                      nshell1234, nprim1234,
                                                                      info.fill_shell_pair, info.copy_data, info.calc_pre,
                                                                      info.boys, info.integrals, info.permute,
                                                                      info.TotalTime(),
                                                                      (double)(info.TotalTime())/(double)(nprim1234));
}

