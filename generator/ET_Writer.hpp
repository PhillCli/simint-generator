#ifndef ETWRITER_HPP
#define ETWRITER_HPP

#include <iostream>

#include "generator/Classes.hpp"


class ET_Algorithm_Base;

class ET_Writer
{   
    public:
        ET_Writer(const ET_Algorithm_Base & et_algo); 

        void WriteIncludes(std::ostream & os) const;
        void AddConstants(std::ostream & os) const;
        void DeclarePrimArrays(std::ostream & os) const;
        void DeclarePrimPointers(std::ostream & os) const;

        void WriteETInline(std::ostream & os) const;

    private:
        ETStepList etsl_;
        QAMSet etint_;

        static std::string ETStepString(const ETStep & et);
        static std::string ETStepVar(const Quartet & q);
};

#endif