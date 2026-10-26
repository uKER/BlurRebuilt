#include "MiscSharedPch.h"

#include <misc_shared/NewCurve/Definitions.h>


namespace Curve
{

const F32 kReparamResLow = 1;
const F32 kReparamResMedium = 4;
const F32 kReparamResHigh = 8;
const F32 kReparamResDefault = kReparamResMedium;

//Note that this matrix is transposed as compared to the matrix given on p460 of
//"Mathematics for 3D Game Programming and Computer Graphics"
//Note that it's been necessary to flip matrix horizontally so that the we traverse
//the curve in the correct direction.
extern const Maths::HMat44 kBezierBasis = Maths::HMat44(
	Maths::HVec4(-1.f,  3.f, -3.f, 1.f),
	Maths::HVec4( 3.f, -6.f,  3.f, 0),
	Maths::HVec4(-3.f,	3.f,  0.f, 0),
	Maths::HVec4( 1.f,	0.f,  0.f, 0));

extern const Maths::HMat44 kHermiteBasis = Maths::HMat44(
	Maths::HVec4( 2.f, -2.f,   1.f,  1.f),
	Maths::HVec4(-3.f,  3.f,  -2.f, -1.f),
	Maths::HVec4( 0.f,	0.f,   1.f,  0),
	Maths::HVec4( 1.f,	0.f,   0.f,  0));

//Taken from Mathematics for Game Programming and Computer Graphics p475.
extern const Maths::HMat44 kBSplineBasis = Maths::HMat44(
	Maths::HVec4(-1.f, 3.f,   -3.f,  1.f)/6.f,
	Maths::HVec4( 3.f,  -6.f,  3.f, 0.f)/6.f,
	Maths::HVec4(-3.f,	0.f,   3.f,  0)/6.f,
	Maths::HVec4(1.f,	4.f,   1.f,  0)/6.f);
 
const char* g_pEReparamStringTable[] =
{
#define DEF(e, str) str,
	#include <Misc_Shared/NewCurve/EReparamRes.h>
#undef DEF
};

const char* GetReparamString(U32 i)
{
	return g_pEReparamStringTable[i];
}

}

