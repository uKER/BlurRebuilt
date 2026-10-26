#ifndef MATHSDEBUG_H
#define MATHSDEBUG_H

#include <core/Maths.h>

namespace Maths {
//--------------------------------


// These functions convert maths objects to strings - useful for debug output.
const Util::C_String MakeString(const HF32 & f);
const Util::C_String MakeString(const HVec3 & vec);
const Util::C_String MakeString(const HVec4 & vec);
const Util::C_String MakeString(const HMat33 & mat);
const Util::C_String MakeString(const HMat43 & mat);
const Util::C_String MakeString(const HMat44 & mat);
const Util::C_String MakeString(const C_Quaternion & q);
const Util::C_String MakeString(const C_QuatTrans & qt);


//--------------------------------
};

#endif
