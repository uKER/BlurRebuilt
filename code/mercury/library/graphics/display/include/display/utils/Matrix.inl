#ifndef DISPLAY_UTILS_MATRIX_INL
#define DISPLAY_UTILS_MATRIX_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE void MatrixSet(C_Matrix* pOut)
{
	using namespace Maths;
	assert(pOut);
	#if 0 // JASOND: Optimised this to use HMat44 constant and the HMat44 Store() function.
	pOut->v[0] = pOut->v[1] = pOut->v[2] = pOut->v[3] = pOut->v[4] = pOut->v[5] = pOut->v[6] = pOut->v[7] = pOut->v[8] = pOut->v[9] = pOut->v[10] = pOut->v[11] = pOut->v[12] = pOut->v[13] = pOut->v[14] = pOut->v[15] = 0.f;
	#else
	Store( Constants<HMat44>::Zero(), pOut );
	#endif
}

INLINE void MatrixSetIdentity(C_Matrix* pOut)
{
	using namespace Maths;
	assert(pOut);
	#if 0 // JASOND: Optimised this to use HMat44 constant and the HMat44 Store() function.
	pOut->v[1] = pOut->v[2] = pOut->v[3] = pOut->v[4] = pOut->v[6] = pOut->v[7] = pOut->v[8] = pOut->v[9] = pOut->v[11] = pOut->v[12] = pOut->v[13] = pOut->v[14] = 0.f;
	pOut->v[0] = pOut->v[5] = pOut->v[10] = pOut->v[15] = 1.f;
	#else
	Store( Constants<HMat44>::Identity(), pOut );
	#endif
}

INLINE void MatrixSetScale(C_Matrix* pOut, const F32 scale)
{
	assert(pOut);
	pOut->_11 = scale;
	pOut->_22 = scale;
	pOut->_33 = scale;
}

INLINE void MatrixSetScale(C_Matrix* pOut, const MathsStorage::Float3& scale)
{
	assert(pOut);
	pOut->_11 = scale.x;
	pOut->_22 = scale.y;
	pOut->_33 = scale.z;
}

INLINE void MatrixSetTranslation(C_Matrix* pOut, const F32 x, const F32 y, const F32 z)
{
	assert(pOut);
	pOut->_41 = x;
	pOut->_42 = y;
	pOut->_43 = z;
}

INLINE void MatrixSetTranslation(C_Matrix* pOut, const MathsStorage::Float3& translation)
{
	assert(pOut);
	pOut->_41 = translation.x;
	pOut->_42 = translation.y;
	pOut->_43 = translation.z;
}

INLINE void MatrixStore(C_Matrix* pOut, const Maths::HMat43& matrix)
{
	assert(pOut);
	Maths::Store(((Maths::HMat44)matrix), pOut);

/*	pOut->_11 = matrix.AX().X();
	pOut->_12 = matrix.AX().Y();
	pOut->_13 = matrix.AX().Z();
	pOut->_14 = 0.f;
	pOut->_21 = matrix.AY().X();
	pOut->_22 = matrix.AY().Y();
	pOut->_23 = matrix.AY().Z();
	pOut->_24 = 0.f;
	pOut->_31 = matrix.AZ().X();
	pOut->_32 = matrix.AZ().Y();
	pOut->_33 = matrix.AZ().Z();
	pOut->_34 = 0.f;
	pOut->_41 = matrix.AW().X();
	pOut->_42 = matrix.AW().Y();
	pOut->_43 = matrix.AW().Z();
	pOut->_44 = 1.f;
*/
}

INLINE void MatrixLoad(Maths::HMat43* pOut, const C_Matrix& matrix)
{
	using namespace Maths;
	assert(pOut);
	#if 0 // JASOND: optimised this to use the HMat44 Load() function
	pOut->AX(HVec3(matrix._11, matrix._12, matrix._13));
	pOut->AY(HVec3(matrix._21, matrix._22, matrix._23));
	pOut->AZ(HVec3(matrix._31, matrix._32, matrix._33));
	pOut->AW(HVec3(matrix._41, matrix._42, matrix._43));
	#else
	*pOut = HMat43( Load<HMat44>( &matrix ) );
	#endif
}

INLINE void MatrixLookAt(Maths::HMat43& cameraToWorld, Maths::HVec3 cameraPos, Maths::HVec3 cameraAt, Maths::HVec3 cameraUp)
{
	cameraUp.Normalise();

	Maths::HVec3 cameraDir = cameraAt - cameraPos;
	
	cameraDir.Normalise();

	Maths::HVec3 rotZ = cameraDir;
	rotZ.Normalise();

	Maths::HVec3 rotX = cameraUp ^ rotZ;
	rotX.Normalise();

	Maths::HVec3 rotY = rotZ ^ rotX;
	rotY.Normalise();

	Maths::HMat33 rot;
	rot.AX(rotX);
	rot.AY(rotY);
	rot.AZ(rotZ);

	cameraToWorld = Maths::HMat43(rot);
	cameraToWorld.AW(cameraPos);
}

}; // namespace Display

#endif // DISPLAY_UTILS_MATRIX_INL
