#ifndef DISPLAY_SPU_UTIL_CULL_H
#define DISPLAY_SPU_UTIL_CULL_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



class C_View;
class C_PositionTable;
class C_IndexTable;
enum E_CullMode;



void Cull(Display::Spu::C_IndexTable * pIndexTable, Display::Spu::C_PositionTable * pPositionTable, Display::Spu::E_CullMode cullMode, Display::Spu::C_View * pView, const MathsStorage::MatrixCol44 &matWorld);



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_UTIL_CULL_H
