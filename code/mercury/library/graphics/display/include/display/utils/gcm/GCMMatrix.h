#ifndef DISPLAY_UTILS_GCM_GCM_MATRIX_H
#define DISPLAY_UTILS_GCM_GCM_MATRIX_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Utils/Matrix.h"

namespace Display
{
namespace GCM
{

void MatrixOrtho(C_Matrix* pMatrix, const U32 left, const U32 right, const U32 bottom, const U32 top, const F32 minZ, const F32 maxZ);
void MatrixLookAt(C_Matrix* pMatrix, const MathsStorage::Float3& eye, const MathsStorage::Float3& at, const MathsStorage::Float3& up);
void MatrixPerspective(C_Matrix* pMatrix, const F32 fieldOfView, const F32 aspectRatio, const F32 nearClip, const F32 farClip);

}; // namespace GCM
}; // namespace Display

#endif // DISPLAY_UTILS_GCM_GCM_MATRIX_H
