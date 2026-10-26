#ifndef DISPLAY_SPU_VIEW_H
#define DISPLAY_SPU_VIEW_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
	class C_View;
}



namespace Display
{
namespace Spu
{



class ALIGN16 C_View
{
public:

	C_View() {}
#ifndef PLATFORM_SPU
	C_View(const C_View &view);
	C_View(const Display::C_View * const pView);
#endif

public:

	U16							m_ScissorRect[4];
	F32							m_MinZ;
	F32							m_MaxZ;
	MathsStorage::MatrixCol44	m_ViewProjMatrix;
	F32							m_ViewportScales[4];
	F32							m_ViewportOffsets[4];
	U8							m_MultisampleType;
	U8							m_Padding[3];

};



}; // namespace Spu
}; // namespace Display



#endif
