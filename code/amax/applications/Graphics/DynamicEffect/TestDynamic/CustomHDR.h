#ifndef CUSTOMHDR_H
#define CUSTOMHDR_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/HDR/HDR.h>
typedef PostProcess::C_HighDynamicRange		C_HighDynamicRangeBase;

class C_CustomHDR : public C_HighDynamicRangeBase
{
public:

	C_CustomHDR();
	~C_CustomHDR();

	bool Create();
	void Destroy();

	void Display() const;


protected:
	Display::C_PixelShader		m_ToneMapCustom;
};

#endif // CUSTOMHDR_H
