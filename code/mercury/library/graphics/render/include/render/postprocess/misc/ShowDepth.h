#ifndef RENDER_POSTPROCESS_SHOWDEPTH_H
#define RENDER_POSTPROCESS_SHOWDEPTH_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

class C_ShowDepth : public I_PostProcess
{
public:

	C_ShowDepth(C_Manager& manager) : I_PostProcess( manager ) {}
	~C_ShowDepth() {}

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}
	void Update() {}

	void Display() const;
	
private:
	Display::C_PixelShader		m_Shader;
};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_SHOWDEPTH_H
