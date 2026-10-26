#include "stdafx.h"
#include "Display/Utils/GCM/GCMMatrix.h"

#include <vectormath/cpp/vectormath_aos.h>

using namespace Vectormath;
using namespace Aos;

namespace Display
{
namespace GCM
{

void MatrixOrtho(C_Matrix* pMatrix, const U32 left, const U32 right, const U32 bottom, const U32 top, const F32 minZ, const F32 maxZ)
{
	//Matrix4 pMatrix = Matrix4::orthographic(left, right, bottom, top, minZ, maxZ);

	//memcpy(pMatrix, &pMatrix[0], sizeof(C_Matrix));
}

void MatrixLookAt(C_Matrix* pMatrix, const MathsStorage::Float3& eye, const MathsStorage::Float3& at, const MathsStorage::Float3& up)
{
	Point3 gcmEye(eye.x, eye.y, eye.z);
	Point3 gcmAt(at.x, at.y, at.z);
	Vector3 gcmUp(up.x, up.y, up.z);

	Matrix4 gcmMatrix = Matrix4::lookAt(gcmEye, gcmAt, gcmUp);

	memcpy(pMatrix, &gcmMatrix[0], sizeof(C_Matrix));
}

void MatrixPerspective(C_Matrix* pMatrix, const F32 fieldOfView, const F32 aspectRatio, const F32 nearClip, const F32 farClip)
{
	Matrix4 gcmMatrix = Matrix4::perspective(Maths::DegToRad(fieldOfView), aspectRatio, nearClip, farClip);

	memcpy(pMatrix, &gcmMatrix[0], sizeof(C_Matrix));
}

}; // namespace GCM
}; // namespace Display
