#ifndef DISPLAY_SPU_UTIL_TRANSFORM_H
#define DISPLAY_SPU_UTIL_TRANSFORM_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



void TransformVertexTable(const Maths::HMat44 &matrix, C_VertexTable * pVertexTable);



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_UTIL_TRANSFORM_H
