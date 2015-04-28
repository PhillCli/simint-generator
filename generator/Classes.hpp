#ifndef GENERATOR_CLASSES_HPP
#define GENERATOR_CLASSES_HPP

#include <array>
#include <string>
#include <sstream>
#include <ostream>
#include <vector>
#include <set>
#include <utility>



// some flags
#define QUARTET_INITIAL     1  // what we are looking for at the end
#define QUARTET_HRRTOPLEVEL 2

#define DOUBLET_INITIAL     1  // what we are looking for at the end
#define DOUBLET_HRRTOPLEVEL 2


typedef std::array<int, 2> DAMList;
typedef std::array<int, 4> QAMList;
typedef std::array<int, 3> ExpList;

// In Helpers.cpp
int GaussianOrder(const ExpList & ijk);

enum class DoubletType
{
    BRA,
    KET
};

enum class XYZStep
{
    STEP_X,
    STEP_Y,
    STEP_Z
};


inline std::ostream & operator<<(std::ostream & os, const XYZStep xyz)
{
    if(xyz == XYZStep::STEP_X)
        os << "x";
    else if(xyz == XYZStep::STEP_Y)
        os << "y";
    else if(xyz == XYZStep::STEP_Z)
        os << "z";
    return os;
}

inline XYZStep IdxToXYZStep(int xyz)
{
    if(xyz == 0) 
        return XYZStep::STEP_X;
    else if(xyz == 1) 
        return XYZStep::STEP_Y;
    else
        return XYZStep::STEP_Z;
}



struct Gaussian
{
    ExpList ijk;

    int am(void) const { return ijk[0] + ijk[1] + ijk[2]; }
    int idx(void) const { return GaussianOrder(ijk); } 
    int ncart(void) const { return ((am()+1)*(am()+2))/2; }

    std::string str(void) const
    {
       const char * amchar = "SPDFGHIJKLMNOQRTUVWXYZABCE";
       std::stringstream ss;
       ss << amchar[am()] << "_" << ijk[0] << ijk[1] << ijk[2];
       return ss.str(); 
    }


    // notice the reversing of the exponents
    bool operator<(const Gaussian & rhs) const
    {
        if(am() < rhs.am())
            return true;
        else if(am() == rhs.am())
        {
            if(ijk > rhs.ijk)
                return true;
        }

        return false;
    }

    bool operator==(const Gaussian & rhs) const
    {
        return (ijk == rhs.ijk);
    }

    bool Iterate(void)
    {
        if(ijk[2] == am())  // at the end
            return false;

        if(ijk[2] < (am() - ijk[0]))
            ijk = {ijk[0],   ijk[1]-1,      ijk[2]+1 };
        else
            ijk = {ijk[0]-1, am()-ijk[0]+1, 0        };
        return true;
    }
};


inline std::ostream & operator<<(std::ostream & os, const Gaussian & g)
{
    os << g.str();
    return os;
}



// A single bra or ket, containing two gaussians
struct Doublet
{
    DoubletType type;
    Gaussian left;
    Gaussian right;
    int flags;

    int am(void) const { return left.am() + right.am(); }    
    int idx(void) const { return left.idx() * right.ncart() + right.idx(); }
    int ncart(void) const { return left.ncart() * right.ncart(); }

    std::string flagstr(void) const
    {
        if(flags == 0)
            return std::string();

        std::stringstream ss;
        ss << "_{";
        if(flags & DOUBLET_INITIAL)
            ss << "i";
        if(flags & DOUBLET_HRRTOPLEVEL)
            ss << "t";
        ss << "}";
        
        return ss.str();
    }

    std::string str(void) const
    {
        std::stringstream ss;
        if(type == DoubletType::BRA)
          ss << "(" << left << " " << right << "|";
        else
          ss << "|" << left << " " << right << ")";
        ss << flagstr();

        return ss.str(); 
    }

    bool operator<(const Doublet & rhs) const
    {
        if(am() < rhs.am())
            return true;
        else if(am() == rhs.am())
        {
            if(left < rhs.left)
                return true;
            else if(left == rhs.left)
            {
                if(right < rhs.right)
                    return true;
                else if(right == rhs.right)
                {
                    if(flags > rhs.flags)
                        return true;
                }
            }
        }
        return false;
    }

    bool operator==(const Doublet & rhs) const
    {
        return (left == rhs.left &&
                right == rhs.right &&
                type == rhs.type &&
                flags == rhs.flags);
    }
};

inline std::ostream & operator<<(std::ostream & os, const Doublet & d)
{
    os << d.str();
    return os;
}


struct Quartet
{
    Doublet bra;
    Doublet ket;
    int m;
    int flags;

    int am(void) const { return bra.am() + ket.am(); }
    int idx(void) const { return bra.idx() * ket.ncart() + ket.idx(); }
    int ncart(void) const { return bra.ncart() * ket.ncart(); }

    Doublet get(DoubletType type) const
    {
        return (type == DoubletType::BRA ? bra : ket);
    }

    std::string flagstr(void) const
    {
        if(flags == 0)
            return std::string();

        std::stringstream ss;
        ss << "_{";
        if(flags & QUARTET_INITIAL)
            ss << "i";
        if(flags & QUARTET_HRRTOPLEVEL)
            ss << "t";
        ss << "}";
        
        return ss.str();
    }

    std::string str(void) const
    {
        std::stringstream ss;
        ss << "( " << bra.left << " " << bra.right << " | "
                   << ket.left << " " << ket.right << " )"
                   << m << flagstr();
        return ss.str();
    }


    bool operator<(const Quartet & rhs) const
    {
        if(am() < rhs.am())
            return true;
        else if(am() == rhs.am())
        {
            if(bra < rhs.bra)
                return true;
            else if(bra == rhs.bra)
            {
                if(ket < rhs.ket)
                    return true;
                else if(ket == rhs.ket)
                {
                    if(m < rhs.m)
                        return true;
                    else if(m == rhs.m)
                    {
                        if(flags > rhs.flags)
                            return true;
                    }
                }
            }
        }

        return false;
    }

    bool operator==(const Quartet & rhs) const
    {
        return (bra == rhs.bra && ket == rhs.ket && m == rhs.m && flags == rhs.flags);
    }

};


inline std::ostream & operator<<(std::ostream & os, const Quartet & q)
{
    os << q.str();
    return os;
}



struct ShellQuartet
{
    QAMList amlist;
    int m;
    int flags; // is an HRR top-level quartet, etc

    int am(void) const { return amlist[0] + amlist[1] + amlist[2] + amlist[3]; }


    std::string flagstr(void) const
    {
        if(flags == 0)
            return std::string();

        std::stringstream ss;
        ss << "_{";
        if(flags & QUARTET_INITIAL)
            ss << "i";
        if(flags & QUARTET_HRRTOPLEVEL)
            ss << "t";
        ss << "}";
        
        return ss.str();
    }

    std::string str(void) const
    {
        std::stringstream ss;
        ss << "< " << amlist[0] << " " << amlist[1] << " | "
                   << amlist[2] << " " << amlist[3] << " >"
                   << "^" << m << flagstr();
        return ss.str();
    }


    bool operator<(const ShellQuartet & rhs) const
    {
        if(am() < rhs.am())
            return true;
        else if(am() == rhs.am())
        {
            
            if(amlist < rhs.amlist)
                return true;
            else if(amlist == rhs.amlist)
            {
                if(m < rhs.m)
                    return true;
                else if(m == rhs.m)
                {
                    if(flags > rhs.flags)
                        return true;
                }
            }
        }

        return false;
    }

    bool operator==(const ShellQuartet & rhs) const
    {
        return (amlist == rhs.amlist && m == rhs.m);
    }

    ShellQuartet(const Quartet & q)
    {
        amlist = { q.bra.left.am(), q.bra.right.am(), q.ket.left.am(), q.ket.right.am() };
        m = q.m;
    }
    
    ShellQuartet(const Doublet & bra, const Doublet & ket, int m = 0)
              : amlist{bra.left.am(), bra.right.am(), ket.left.am(), ket.right.am()}, m(m)
    {
    }

    ShellQuartet(const QAMList & amlist, int m, int flags)
              : amlist(amlist), m(m), flags(flags) { }

    ShellQuartet(const ShellQuartet & q) = default;

};


inline std::ostream & operator<<(std::ostream & os, const ShellQuartet & q)
{
    os << q.str();
    return os;
}


struct HRRDoubletStep
{
    Doublet target;
    Doublet src1;
    Doublet src2;
    XYZStep xyz;    

    std::string str(void) const
    {
        const char * xyztype = (target.type == DoubletType::BRA ? "ab" : "cd");
        std::stringstream ss;
        ss << target << " = " << src1 << " + " << xyz << "_" << xyztype << " * " << src2;
        return ss.str();
    }

    bool operator==(const HRRDoubletStep & rhs) const
    {
        return (target == rhs.target &&
                src1 == rhs.src1 &&
                src2 == rhs.src2 &&
                xyz == rhs.xyz);
    }
};



inline std::ostream & operator<<(std::ostream & os, const HRRDoubletStep & hrr)
{
    os << hrr.str();
    return os;
}


struct HRRQuartetStep
{
    Quartet target;
    Quartet src1;
    Quartet src2;
    DoubletType steptype;  // bra or ket being stepped
    XYZStep xyz;    


    std::string str(void) const
    {
        const char * xyztype = (steptype == DoubletType::BRA ? "ab" : "cd");
        std::stringstream ss;
        ss << target << " = " << src1 << " + " << xyz << "_" << xyztype << " * " << src2;
        return ss.str();
    }

    bool operator==(const HRRQuartetStep & rhs) const
    {
        return (target == rhs.target &&
                src1 == rhs.src1 &&
                src2 == rhs.src2 &&
                steptype == rhs.steptype &&
                xyz == rhs.xyz);
    }
};

inline std::ostream & operator<<(std::ostream & os, const HRRQuartetStep & hrr)
{
    os << hrr.str();
    return os;
}




struct ETStep
{
    std::string str(void) const
    {
        return "TOTO";
    } 
};

inline std::ostream & operator<<(std::ostream & os, const ETStep & et)
{
    os << et.str();
    return os;
}



struct VRRStep
{
    std::string str(void) const
    {
        return "TOTO";
    } 
};

inline std::ostream & operator<<(std::ostream & os, const VRRStep & is)
{
    os << is.str();
    return os;
}



typedef std::vector<HRRDoubletStep> HRRDoubletStepList;
typedef std::pair<HRRDoubletStepList, HRRDoubletStepList> HRRBraKetStepList;

typedef std::vector<HRRQuartetStep> HRRQuartetStepList;
typedef std::vector<ETStep> ETStepList;
typedef std::vector<VRRStep> VRRStepList;

typedef std::set<ShellQuartet> ShellQuartetSet;
typedef std::set<Quartet> QuartetSet;
typedef std::set<Doublet> DoubletSet;


#endif