#ifndef DISPLAY_UTILS_MATRIX_H
#define DISPLAY_UTILS_MATRIX_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

typedef MathsStorage::MatrixRow44A16 C_Matrix;

void MatrixSet(C_Matrix* pOut);
void MatrixSetIdentity(C_Matrix* pOut);
void MatrixSetScale(C_Matrix* pOut, const F32 scale);
void MatrixSetScale(C_Matrix* pOut, const MathsStorage::Float3& scale);
void MatrixSetTranslation(C_Matrix* pOut, const F32 x, const F32 y, const F32 z);
void MatrixSetTranslation(C_Matrix* pOut, const MathsStorage::Float3& translation);
void MatrixStore(C_Matrix* pOut, const Maths::HMat43& matrix);
void MatrixLoad(Maths::HMat43* pOut, const C_Matrix& matrix);
void MatrixLookAt(Maths::HMat43& cameraToWorld, Maths::HVec3 cameraPos, Maths::HVec3 cameraAt, Maths::HVec3 cameraUp);

}; // namespace Display

#include "Display/Utils/Matrix.inl"

#endif // DISPLAY_UTILS_MATRIX_H
