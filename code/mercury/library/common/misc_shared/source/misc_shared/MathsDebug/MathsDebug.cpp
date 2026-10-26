#include "MiscSharedPch.h"
#include <misc_shared/MathsDebug/MathsDebug.h>

namespace Maths {
//-------------------------------------------


const Util::C_String MakeString(const HF32 & f)
{
	Util::C_String ret;
	ret.Printf( "%f", F32(f) );
	return ret;
}


const Util::C_String MakeString(const HVec3 & vec)
{
	Util::C_String ret;
	ret.Printf( "(%f, %f, %f)", F32(vec.X()), F32(vec.Y()), F32(vec.Z()) );
	return ret;
}


const Util::C_String MakeString(const HVec4 & vec)
{
	Util::C_String ret;
	ret.Printf( "(%f, %f, %f, %f)", F32(vec.X()), F32(vec.Y()), F32(vec.Z()), F32(vec.W()) );
	return ret;
}


const Util::C_String MakeString(const HMat33 & mat)
{
	Util::C_String ret;
	ret.Printf( "%s\n%s\n%s",
		MakeString(mat.AX()).Chars(),
		MakeString(mat.AY()).Chars(),
		MakeString(mat.AZ()).Chars() );
	return ret;
}


const Util::C_String MakeString(const HMat43 & mat)
{
	Util::C_String ret;
	ret.Printf( "%s\n%s\n%s\n%s",
		MakeString(mat.AX()).Chars(),
		MakeString(mat.AY()).Chars(),
		MakeString(mat.AZ()).Chars(), 
		MakeString(mat.AW()).Chars() );
	return ret;
}


const Util::C_String MakeString(const HMat44 & mat)
{
	Util::C_String ret;
	ret.Printf( "%s\n%s\n%s\n%s",
		MakeString(mat.AX()).Chars(),
		MakeString(mat.AY()).Chars(),
		MakeString(mat.AZ()).Chars(), 
		MakeString(mat.AW()).Chars() );
	return ret;
}


const Util::C_String MakeString(const C_Quaternion & q)
{
	return MakeString(HVec4(q));
}


const Util::C_String MakeString(const C_QuatTrans & qt)
{
	Util::C_String ret;
	ret.Printf( "[ %s , %s ]",
		MakeString(qt.Quaternion()).Chars(),
		MakeString(qt.Translation()).Chars() );
	return ret;
}


//-------------------------------------------
};
