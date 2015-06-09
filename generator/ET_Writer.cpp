#include "generator/ET_Writer.hpp"
#include "generator/WriterBase.hpp"
#include "generator/ET_Algorithm_Base.hpp"


ET_Writer::ET_Writer(const ET_Algorithm_Base & et_algo) 
{ 
    etsl_ = et_algo.ETSteps();

    // see what we need for arrays
    for(const auto & it : etsl_)
    {
        if(it.src1)
            etint_.insert(it.src1.amlist());
        if(it.src2)
            etint_.insert(it.src2.amlist());
        if(it.src3)
            etint_.insert(it.src3.amlist());
        if(it.src3)
            etint_.insert(it.src4.amlist());
        if(it.target)
            etint_.insert(it.target.amlist());
    }
}



void ET_Writer::WriteIncludes(std::ostream & os, const WriterBase & base) const
{
}



void ET_Writer::DeclarePrimArrays(std::ostream & os, const WriterBase & base) const
{
    if(etint_.size())
    {
        os << "                    // Holds temporary integrals for electron transfer\n";

        for(const auto & it : etint_)
        {
            // only if these aren't from vrr
            if(it[1] > 0 || it[2] > 0 || it[3] > 0)
                os << "                    double " << base.PrimVarName(it) << "[" << NCART(it[0]) * NCART(it[2]) << "];\n";
        } 

        os << "\n\n";

    }
}


void ET_Writer::DeclarePrimPointers(std::ostream & os, const WriterBase & base) const
{
    if(etint_.size())
    {
        for(const auto & it : etint_)
        {
            if(base.IsContArray(it))
                os << "                    double * const restrict " << base.PrimPtrName(it)
                   << " = " << base.ArrVarName(it) << " + abcd * " << NCART(it[0]) * NCART(it[2]) << ";\n";
        }

        os << "\n\n";

    }
}


void ET_Writer::WriteETInline(std::ostream & os, const WriterBase & base) const
{
    os << "\n";
    os << "                    //////////////////////////////////////////////\n";
    os << "                    // Primitive integrals: Electron transfer\n";
    os << "                    //////////////////////////////////////////////\n";
    os << "\n";

    std::string indent1(20, ' ');
    std::string indent2(24, ' ');

    if(etsl_.size() == 0)
        os << indent1 << "//...nothing to do...\n";
    else
    {
        for(const auto & it : etsl_)
        {
            os << indent1 << "// " << it << "\n";
            os << ETStepString(it, base);
            os << "\n";
        }
    }

    // add to needed contracted integrals
    for(const auto & it : etint_)
    {
        if(base.IsContArray(it))
        {
            int ncart = NCART(it[0])*NCART(it[2]);

            os << "\n";
            os << indent1 << "// Accumulating in contracted workspace\n";
            os << indent1 << "for(n = 0; n < " << ncart << "; n++)\n";
            os << indent2 << base.PrimPtrName(it) << "[n] += " << base.PrimVarName(it) << "[n];\n";
        }
    }
}



std::string ET_Writer::ETStepVar(const Quartet & q, const WriterBase & base)
{
    std::stringstream ss; 
    ss << base.PrimVarName(q.amlist()) << "[" << q.idx() << "]";
    return ss.str();
}



std::string ET_Writer::ETStepString(const ETStep & et, const WriterBase & base)
{
    int stepidx = XYZStepToIdx(et.xyz);
    int ival = et.target.bra.left.ijk[stepidx];
    int kval = et.target.ket.left.ijk[stepidx]-1;

    std::stringstream ss;
    ss << std::string(20, ' ');
    ss << ETStepVar(et.target, base);

    ss << " = ";

    ss << "etfac[" << stepidx << "] * " << ETStepVar(et.src1, base);

    if(et.src2.bra.left && et.src2.ket.left)
        ss << " + " << ival << " * one_over_2q * " << ETStepVar(et.src2, base);
    if(et.src3.bra.left && et.src3.ket.left)
        ss << " + " << kval << " * one_over_2q * " << ETStepVar(et.src3, base);
    if(et.src4.bra.left && et.src4.ket.left)
        ss << " - p_over_q * " << ETStepVar(et.src4, base);
    ss << ";\n";

    return ss.str();
}

